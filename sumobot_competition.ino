// ============================================================
//  SumoBot Competition Code — Pololu Zumo 32U4
//  IEEE Spring 2024 Joint R2 & R1 SAC — Kit Category
//
//  State machine: IDLE → OPENING → SEARCH → ATTACK → EVADE
//
//  Rules enforced:
//    • Button-press starts the 5-second delay before any motion
//    • Robot is fully autonomous after that
//    • Retreats and reorients when line sensors detect the border
// ============================================================

#include <Wire.h>
#include <Zumo32U4.h>

// ─── Hardware ───────────────────────────────────────────────
Zumo32U4OLED            display;
Zumo32U4ButtonA         buttonA;
Zumo32U4Buzzer          buzzer;
Zumo32U4Motors          motors;
Zumo32U4LineSensors     lineSensors;
Zumo32U4ProximitySensors proxSensors;

// ─── Tunable Constants ──────────────────────────────────────

// Start delay: rules require ≥5 s; use 5.25 s for safety margin
const uint32_t START_DELAY_MS   = 5250;

// Motor speeds  (range: -400 … 400)
const int RAMMING_SPEED         =  400;   // full attack
const int FORWARD_SPEED         =  300;   // cautious advance
const int SEARCH_SPEED          =  200;   // slow rotate while hunting
const int TURN_SPEED            =  350;   // fast in-place turn
const int REVERSE_SPEED         = -350;   // back away from border

// Timing for the opening move
const uint32_t OPENING_TURN_MS  =  260;   // spin before initial lunge

// Timing for border-evade sequence
const uint32_t EVADE_REVERSE_MS =  220;   // back up
const uint32_t EVADE_TURN_MS    =  300;   // spin toward center

// Sensor thresholds
// Line sensors: HIGH value = dark surface, LOW value = white border strip
// Calibrated empirically; lower this if the robot falls off before reacting.
const int LINE_THRESHOLD        =  400;

// Proximity: 0-6 scale; 4+ is a reliable detection
const int PROX_THRESHOLD        =    4;

// Wide search: flip direction every N milliseconds
const uint32_t SEARCH_FLIP_MS   =  700;

// ─── State Machine ──────────────────────────────────────────

enum State : uint8_t {
  STATE_IDLE,       // waiting for button
  STATE_COUNTDOWN,  // 5-second mandatory delay
  STATE_OPENING,    // quick opening turn to face opponent
  STATE_SEARCH,     // spin until opponent found
  STATE_ATTACK,     // charge at detected opponent
  STATE_EVADE       // border detected — retreat & reorient
};

State    currentState  = STATE_IDLE;
uint32_t stateStart    = 0;

// Remember which side hit the border so evade turns the right way
int      evadeTurnDir  = 1;   // +1 = turn right, -1 = turn left

// ─── Helpers ────────────────────────────────────────────────

void setState(State s)
{
  currentState = s;
  stateStart   = millis();
}

uint32_t elapsed()
{
  return millis() - stateStart;
}

// Read all three line sensors in one call.
// Returns true if any sensor is over a white border stripe.
// Also sets evadeTurnDir so the robot turns toward the center.
int lineValues[3];

bool checkBorder()
{
  lineSensors.read(lineValues);

  bool left   = (lineValues[0] < LINE_THRESHOLD);
  bool center = (lineValues[1] < LINE_THRESHOLD);
  bool right  = (lineValues[2] < LINE_THRESHOLD);

  if (!left && !center && !right) return false;

  // Turn AWAY from the side that hit the border
  if (left && !right)       evadeTurnDir =  1;   // hit left edge → turn right
  else if (right && !left)  evadeTurnDir = -1;   // hit right edge → turn left
  else                      evadeTurnDir =  1;   // center / both → default right

  return true;
}

// Read proximity sensors and return true when an opponent is in front.
// Fills leftCount / rightCount so the caller can steer.
bool checkOpponent(int &leftCount, int &rightCount)
{
  proxSensors.read();
  leftCount  = proxSensors.countsFrontWithLeftLeds();
  rightCount = proxSensors.countsFrontWithRightLeds();
  return (leftCount >= PROX_THRESHOLD || rightCount >= PROX_THRESHOLD);
}

void showState(const __FlashStringHelper *label)
{
  display.clear();
  display.print(label);
}

// ─── Setup ──────────────────────────────────────────────────

void setup()
{
  lineSensors.initThreeSensors();
  proxSensors.initThreeSensors();
  Wire.begin();

  display.clear();
  display.print(F("SUMOBOT"));
  display.gotoXY(0, 1);
  display.print(F("Press A"));

  // Wait for the physical button press described in rule 6.4
  while (!buttonA.getSingleDebouncedPress()) {}

  setState(STATE_COUNTDOWN);
}

// ─── Main Loop ──────────────────────────────────────────────

void loop()
{
  int leftCount  = 0;
  int rightCount = 0;

  switch (currentState)
  {
    // ── COUNTDOWN ────────────────────────────────────────────
    // Rules §6.5: robot must not move for at least 5 seconds.
    case STATE_COUNTDOWN:
    {
      uint32_t remaining_ms = (elapsed() < START_DELAY_MS)
                              ? (START_DELAY_MS - elapsed()) : 0;
      int secs = (int)(remaining_ms / 1000) + 1;

      display.clear();
      display.print(F("WAIT "));
      display.print(secs);

      motors.setSpeeds(0, 0);

      if (elapsed() >= START_DELAY_MS)
      {
        buzzer.playNote(NOTE_C(6), 80, 15);
        setState(STATE_OPENING);
      }
      break;
    }

    // ── OPENING MOVE ─────────────────────────────────────────
    // Spin briefly so we're unlikely to be facing away at the
    // start, then immediately lunge into ATTACK.
    case STATE_OPENING:
      showState(F("OPENING"));
      motors.setSpeeds(TURN_SPEED, -TURN_SPEED);   // spin right

      if (elapsed() >= OPENING_TURN_MS)
      {
        motors.setSpeeds(0, 0);
        setState(STATE_ATTACK);   // lunge straight into attack
      }
      break;

    // ── SEARCH ───────────────────────────────────────────────
    // Rotate slowly, flipping direction periodically, until the
    // proximity sensor picks up the opponent.
    case STATE_SEARCH:
    {
      showState(F("SEARCH"));

      // Border takes priority over everything
      if (checkBorder())
      {
        motors.setSpeeds(0, 0);
        setState(STATE_EVADE);
        break;
      }

      // Alternate turn direction to cover the whole arena
      int dir = ((elapsed() / SEARCH_FLIP_MS) % 2 == 0) ? 1 : -1;
      motors.setSpeeds(dir * SEARCH_SPEED, -dir * SEARCH_SPEED);

      if (checkOpponent(leftCount, rightCount))
      {
        motors.setSpeeds(0, 0);
        setState(STATE_ATTACK);
      }
      break;
    }

    // ── ATTACK ───────────────────────────────────────────────
    // Drive hard at the opponent.  Steer left/right based on
    // which proximity LED reads higher.  If we lose sight of
    // the opponent, drop back to SEARCH.
    case STATE_ATTACK:
      showState(F("ATTACK"));

      // Border check — highest priority
      if (checkBorder())
      {
        motors.setSpeeds(0, 0);
        setState(STATE_EVADE);
        break;
      }

      if (checkOpponent(leftCount, rightCount))
      {
        if (leftCount > rightCount + 1)
        {
          // Opponent veering left — steer left
          motors.setSpeeds(FORWARD_SPEED, RAMMING_SPEED);
        }
        else if (rightCount > leftCount + 1)
        {
          // Opponent veering right — steer right
          motors.setSpeeds(RAMMING_SPEED, FORWARD_SPEED);
        }
        else
        {
          // Straight ahead — full ram
          motors.setSpeeds(RAMMING_SPEED, RAMMING_SPEED);
        }
      }
      else
      {
        // Lost the opponent
        motors.setSpeeds(0, 0);
        setState(STATE_SEARCH);
      }
      break;

    // ── EVADE ────────────────────────────────────────────────
    // Phase 1: back straight up (get away from edge).
    // Phase 2: spin in-place toward the center.
    // Phase 3: return to SEARCH.
    case STATE_EVADE:
    {
      showState(F("EVADE"));

      uint32_t t = elapsed();

      if (t < EVADE_REVERSE_MS)
      {
        // Reverse away from the border
        motors.setSpeeds(REVERSE_SPEED, REVERSE_SPEED);
      }
      else if (t < EVADE_REVERSE_MS + EVADE_TURN_MS)
      {
        // Spin toward arena center
        motors.setSpeeds( evadeTurnDir * TURN_SPEED,
                         -evadeTurnDir * TURN_SPEED);
      }
      else
      {
        motors.setSpeeds(0, 0);
        setState(STATE_SEARCH);
      }
      break;
    }

    // ── IDLE (should not reach here after setup) ──────────────
    default:
      motors.setSpeeds(0, 0);
      break;
  }
}
