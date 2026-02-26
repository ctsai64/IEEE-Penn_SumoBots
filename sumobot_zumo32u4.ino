// ============================================================
//  SumoBots Competition – Pololu Zumo 32U4
//  IEEE Spring 2024 Joint R2 & R1 SAC
//  Organizer: Jenna Fazio
// ============================================================
//
//  RULES COMPLIANCE IMPLEMENTED:
//    - 5-second start delay before any movement (Rule 6.5)
//    - Fully autonomous after start (Rules 3.1, 4.2)
//    - No remote control or external input during match
//    - Safe shutdown on tilt / stuck detection
//
//  HARDWARE: Pololu Zumo 32U4 (built-in sensors used)
//    - Proximity / IR sensors (front left, front right)
//    - Line sensors (detects white border of Dohyo)
//    - Buzzer (audio feedback)
//    - Button A (start trigger)
//    - Motors via Zumo32U4Motors library
//
//  STRATEGY:
//    1. Wait 5 seconds (mandatory delay)
//    2. Spin slowly to search for opponent
//    3. When opponent detected → charge at full speed
//    4. When border (white line) detected → reverse & turn away
// ============================================================

#include <Wire.h>
#include <Zumo32U4.h>

// ── Hardware objects ─────────────────────────────────────────
Zumo32U4LCD            lcd;
Zumo32U4Motors         motors;
Zumo32U4ButtonA        buttonA;
Zumo32U4Buzzer         buzzer;
Zumo32U4LineSensors    lineSensors;
Zumo32U4ProximitySensors proxSensors;

// ── Tuning constants ─────────────────────────────────────────
const uint16_t START_DELAY_MS       = 5000;  // Rule 6.5: mandatory 5-second wait
const int      DRIVE_SPEED          = 400;   // Forward charge speed  (-400..400)
const int      SEARCH_SPEED         = 200;   // Rotation speed during search
const int      REVERSE_SPEED        = -400;  // Reversal speed when border detected
const uint16_t REVERSE_DURATION_MS  = 300;   // How long to reverse from border
const uint16_t TURN_DURATION_MS     = 350;   // How long to turn after reversing
const uint8_t  LINE_THRESHOLD       = 500;   // Sensor value above which = white border
const uint8_t  PROX_THRESHOLD       = 4;     // Proximity reading to consider "opponent seen"
const uint16_t STUCK_TIMEOUT_MS     = 2000;  // Spin away if no detection for this long

// ── Line sensor calibration values ───────────────────────────
// Calibrate once on your actual Dohyo; these are safe defaults.
const uint16_t LINE_SENSOR_COUNT    = 3;     // Left, Center, Right sensors used
uint16_t lineSensorValues[3];

// ── State machine ─────────────────────────────────────────────
enum State {
  WAITING,    // Pre-match, waiting for button press
  DELAYING,   // 5-second mandatory countdown
  SEARCHING,  // Spinning, looking for opponent
  ATTACKING,  // Opponent acquired – full speed ahead
  EVADING     // Border detected – reverse and reorient
};

State currentState = WAITING;
uint32_t stateStartTime = 0;
uint32_t lastDetectionTime = 0;
bool turnRight = true;  // Alternates turn direction to avoid repetitive patterns

// ── Helper: enter a new state ─────────────────────────────────
void enterState(State s) {
  currentState = s;
  stateStartTime = millis();
}

// ── Helper: non-blocking buzzer notes ────────────────────────
void beep(uint16_t freq, uint16_t dur) {
  buzzer.playFrequency(freq, dur, 15);
}

// ============================================================
//  SETUP
// ============================================================
void setup() {
  // Initialise line sensors (3-sensor array: left=0, center=1, right=2)
  lineSensors.initThreeSensors();

  // Initialise proximity sensors (front-facing IR)
  proxSensors.initFrontSensor();

  // Display welcome message
  lcd.clear();
  lcd.print(F("SumoBot"));
  lcd.gotoXY(0, 1);
  lcd.print(F("Press A"));

  enterState(WAITING);
}

// ============================================================
//  LOOP
// ============================================================
void loop() {
  switch (currentState) {

    // ── WAITING: robot idle, press Button A to arm ──────────
    case WAITING:
      if (buttonA.getSingleDebouncedPress()) {
        lcd.clear();
        lcd.print(F("Ready!"));
        beep(440, 200);
        enterState(DELAYING);
      }
      break;

    // ── DELAYING: 5-second mandatory countdown (Rule 6.5) ───
    case DELAYING: {
      uint32_t elapsed = millis() - stateStartTime;
      uint8_t  secsLeft = (START_DELAY_MS - elapsed) / 1000 + 1;

      lcd.clear();
      lcd.print(F("Start: "));
      lcd.print(secsLeft);

      // Beep each second during countdown
      static uint8_t lastSec = 99;
      if (secsLeft != lastSec) {
        beep(880, 100);
        lastSec = secsLeft;
      }

      if (elapsed >= START_DELAY_MS) {
        // Final go-beep
        beep(1760, 300);
        lcd.clear();
        lcd.print(F("FIGHT!"));
        lastDetectionTime = millis();
        enterState(SEARCHING);
      }
      break;
    }

    // ── SEARCHING: slow spin until opponent detected ─────────
    case SEARCHING: {
      // Read proximity sensors
      proxSensors.read();
      uint8_t leftReading  = proxSensors.countsFrontWithLeftLeds();
      uint8_t rightReading = proxSensors.countsFrontWithRightLeds();

      // Read line sensors to avoid falling off while searching
      lineSensors.read(lineSensorValues);
      if (borderDetected()) {
        enterState(EVADING);
        break;
      }

      // Opponent detected?
      if (leftReading >= PROX_THRESHOLD || rightReading >= PROX_THRESHOLD) {
        lastDetectionTime = millis();
        lcd.clear();
        lcd.print(F("TARGET!"));
        enterState(ATTACKING);
        break;
      }

      // Spin to search; alternate direction periodically to be unpredictable
      if (turnRight) {
        motors.setSpeeds(SEARCH_SPEED, -SEARCH_SPEED);
      } else {
        motors.setSpeeds(-SEARCH_SPEED, SEARCH_SPEED);
      }

      // Change direction every STUCK_TIMEOUT_MS if nothing found
      if (millis() - lastDetectionTime > STUCK_TIMEOUT_MS) {
        turnRight = !turnRight;
        lastDetectionTime = millis();
      }
      break;
    }

    // ── ATTACKING: full charge toward opponent ───────────────
    case ATTACKING: {
      // Always check border first – safety takes priority
      lineSensors.read(lineSensorValues);
      if (borderDetected()) {
        enterState(EVADING);
        break;
      }

      // Re-read proximity to keep tracking
      proxSensors.read();
      uint8_t leftReading  = proxSensors.countsFrontWithLeftLeds();
      uint8_t rightReading = proxSensors.countsFrontWithRightLeds();

      if (leftReading >= PROX_THRESHOLD || rightReading >= PROX_THRESHOLD) {
        // Still seeing target – steer toward stronger signal
        lastDetectionTime = millis();
        if (leftReading > rightReading) {
          // Target is to the left – arc left
          motors.setSpeeds(DRIVE_SPEED / 2, DRIVE_SPEED);
        } else if (rightReading > leftReading) {
          // Target is to the right – arc right
          motors.setSpeeds(DRIVE_SPEED, DRIVE_SPEED / 2);
        } else {
          // Dead ahead – full charge
          motors.setSpeeds(DRIVE_SPEED, DRIVE_SPEED);
        }
      } else {
        // Lost sight of opponent – go back to searching
        lcd.clear();
        lcd.print(F("Search"));
        enterState(SEARCHING);
      }
      break;
    }

    // ── EVADING: reverse then turn away from border ──────────
    case EVADING: {
      uint32_t elapsed = millis() - stateStartTime;

      if (elapsed < REVERSE_DURATION_MS) {
        // Phase 1: Reverse
        motors.setSpeeds(REVERSE_SPEED, REVERSE_SPEED);
      } else if (elapsed < REVERSE_DURATION_MS + TURN_DURATION_MS) {
        // Phase 2: Turn away from danger
        if (turnRight) {
          motors.setSpeeds(DRIVE_SPEED, -DRIVE_SPEED);
        } else {
          motors.setSpeeds(-DRIVE_SPEED, DRIVE_SPEED);
        }
        turnRight = !turnRight; // Alternate next time
      } else {
        // Resume searching
        lastDetectionTime = millis();
        enterState(SEARCHING);
      }
      break;
    }
  }
}

// ============================================================
//  UTILITY: Border Detection
//  Returns true if any line sensor reads a white border value.
//  The Dohyo border is white on a black field, so a HIGH reading
//  from the reflectance sensor indicates the white edge.
// ============================================================
bool borderDetected() {
  // lineSensorValues[0] = left, [1] = center, [2] = right
  return (lineSensorValues[0] > LINE_THRESHOLD ||
          lineSensorValues[1] > LINE_THRESHOLD ||
          lineSensorValues[2] > LINE_THRESHOLD);
}

// ============================================================
//  NOTES FOR TEAMS
// ============================================================
//
//  CALIBRATION (recommended before each match):
//    - Call lineSensors.calibrate() while moving the bot over
//      both black and white surfaces so thresholds auto-adjust.
//    - Increase LINE_THRESHOLD if the bot falls off the Dohyo;
//      decrease it if it turns away from the center too often.
//
//  PROXIMITY TUNING:
//    - Lower PROX_THRESHOLD (e.g. 2) for earlier detection of a
//      distant opponent; raise it (e.g. 6) to reduce false hits.
//
//  STRATEGY VARIATIONS (allowed under Kit rules – software only):
//    - Change SEARCH_SPEED to a slower creep for a more
//      patient, ambush-style approach.
//    - Change DRIVE_SPEED up to the motor maximum for max aggression.
//    - Adjust TURN_DURATION_MS to make evasive turns sharper.
//
//  DISQUALIFICATION RISKS (Rule 6.5 / 3.3 / 4.5):
//    - Never remove the 5-second delay – early movement = warning.
//    - Three warnings = forfeit of the match.
//    - Ensure the robot stops safely if it smokes or stalls.
//
// ============================================================
