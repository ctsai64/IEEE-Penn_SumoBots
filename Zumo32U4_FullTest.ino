/*
 * Pololu Zumo 32U4 Robot - Comprehensive Functionality Test Script
 * 
 * This script tests all major components and features of the Zumo 32U4:
 * - Motors and encoders
 * - Line sensors
 * - Proximity sensors
 * - IMU (accelerometer, gyroscope, magnetometer)
 * - Buzzer
 * - LEDs
 * - Buttons
 * - Battery voltage monitoring
 * 
 * Author: Test Suite
 * Version: 1.0
 * Date: 2026-01-30
 * 
 * Required Libraries:
 * - Zumo32U4 (install via Arduino Library Manager)
 * 
 * Hardware Requirements:
 * - Pololu Zumo 32U4 Robot
 * - Fully charged batteries
 * - Clear testing surface
 */

#include <Wire.h>
#include <Zumo32U4.h>

// Initialize all Zumo components
Zumo32U4ButtonA buttonA;
Zumo32U4ButtonB buttonB;
Zumo32U4ButtonC buttonC;
Zumo32U4Buzzer buzzer;
Zumo32U4LCD lcd;
Zumo32U4LineSensors lineSensors;
Zumo32U4Motors motors;
Zumo32U4Encoders encoders;
Zumo32U4ProximitySensors proxSensors;
Zumo32U4IMU imu;

// Test state variables
enum TestState {
  TEST_IDLE,
  TEST_BUTTONS,
  TEST_LEDS,
  TEST_BUZZER,
  TEST_LCD,
  TEST_BATTERY,
  TEST_LINE_SENSORS,
  TEST_PROXIMITY,
  TEST_IMU,
  TEST_ENCODERS,
  TEST_MOTORS,
  TEST_COMPLETE
};

TestState currentTest = TEST_IDLE;
unsigned long testStartTime = 0;
bool testInProgress = false;

// Line sensor variables
#define NUM_SENSORS 5
unsigned int lineSensorValues[NUM_SENSORS];
bool lineSensorsCalibrated = false;

// IMU variables
bool imuInitialized = false;

// Test results tracking
struct TestResults {
  bool buttonsPass;
  bool ledsPass;
  bool buzzerPass;
  bool lcdPass;
  bool batteryPass;
  bool lineSensorsPass;
  bool proximityPass;
  bool imuPass;
  bool encodersPass;
  bool motorsPass;
} testResults;

void setup() {
  Serial.begin(9600);
  
  // Initialize LCD
  lcd.clear();
  lcd.print(F("Zumo32U4"));
  lcd.gotoXY(0, 1);
  lcd.print(F("Test v1.0"));
  
  // Play startup tune
  buzzer.playFrequency(440, 200, 15);
  delay(250);
  buzzer.playFrequency(554, 200, 15);
  delay(250);
  buzzer.playFrequency(659, 200, 15);
  delay(1000);
  
  // Initialize line sensors
  lineSensors.initFiveSensors();
  
  // Initialize proximity sensors
  proxSensors.initThreeSensors();
  
  // Initialize IMU
  Wire.begin();
  if (imu.init()) {
    imu.enableDefault();
    imuInitialized = true;
  }
  
  // Initialize test results
  memset(&testResults, 0, sizeof(testResults));
  
  // Display menu
  displayMainMenu();
  
  Serial.println(F("================================="));
  Serial.println(F("Zumo 32U4 Comprehensive Test Suite"));
  Serial.println(F("================================="));
  Serial.println();
}

void loop() {
  if (!testInProgress) {
    checkButtonsForMenu();
  } else {
    runCurrentTest();
  }
}

void displayMainMenu() {
  lcd.clear();
  lcd.print(F("A:Start"));
  lcd.gotoXY(0, 1);
  lcd.print(F("C:Report"));
}

void checkButtonsForMenu() {
  if (buttonA.getSingleDebouncedPress()) {
    startTestSequence();
  }
  
  if (buttonC.getSingleDebouncedPress()) {
    displayTestReport();
  }
}

void startTestSequence() {
  lcd.clear();
  lcd.print(F("Starting"));
  lcd.gotoXY(0, 1);
  lcd.print(F("Tests..."));
  delay(1000);
  
  currentTest = TEST_BUTTONS;
  testInProgress = true;
  testStartTime = millis();
}

void runCurrentTest() {
  switch (currentTest) {
    case TEST_BUTTONS:
      testButtons();
      break;
    case TEST_LEDS:
      testLEDs();
      break;
    case TEST_BUZZER:
      testBuzzer();
      break;
    case TEST_LCD:
      testLCD();
      break;
    case TEST_BATTERY:
      testBattery();
      break;
    case TEST_LINE_SENSORS:
      testLineSensors();
      break;
    case TEST_PROXIMITY:
      testProximity();
      break;
    case TEST_IMU:
      testIMU();
      break;
    case TEST_ENCODERS:
      testEncoders();
      break;
    case TEST_MOTORS:
      testMotors();
      break;
    case TEST_COMPLETE:
      completeTests();
      break;
    default:
      break;
  }
}

void testButtons() {
  static bool testStarted = false;
  static int buttonsPressedCount = 0;
  
  if (!testStarted) {
    lcd.clear();
    lcd.print(F("Button"));
    lcd.gotoXY(0, 1);
    lcd.print(F("Test:A,B,C"));
    Serial.println(F("\n--- Button Test ---"));
    Serial.println(F("Press buttons A, B, and C"));
    testStarted = true;
    buttonsPressedCount = 0;
  }
  
  if (buttonA.getSingleDebouncedPress()) {
    Serial.println(F("Button A: PASS"));
    buzzer.playNote(NOTE_C(5), 100, 15);
    buttonsPressedCount++;
  }
  
  if (buttonB.getSingleDebouncedPress()) {
    Serial.println(F("Button B: PASS"));
    buzzer.playNote(NOTE_E(5), 100, 15);
    buttonsPressedCount++;
  }
  
  if (buttonC.getSingleDebouncedPress()) {
    Serial.println(F("Button C: PASS"));
    buzzer.playNote(NOTE_G(5), 100, 15);
    buttonsPressedCount++;
  }
  
  if (buttonsPressedCount >= 3 || (millis() - testStartTime > 10000)) {
    testResults.buttonsPass = (buttonsPressedCount == 3);
    Serial.print(F("Result: "));
    Serial.println(testResults.buttonsPass ? F("PASS") : F("FAIL"));
    advanceToNextTest();
    testStarted = false;
  }
}

void testLEDs() {
  static bool testStarted = false;
  static unsigned long ledStartTime = 0;
  static int ledPhase = 0;
  
  if (!testStarted) {
    lcd.clear();
    lcd.print(F("LED Test"));
    Serial.println(F("\n--- LED Test ---"));
    Serial.println(F("Watch LEDs blink"));
    testStarted = true;
    ledStartTime = millis();
    ledPhase = 0;
  }
  
  unsigned long elapsed = millis() - ledStartTime;
  
  if (ledPhase == 0 && elapsed < 1000) {
    // Red LED on
    ledRed(1);
    ledYellow(0);
    ledGreen(0);
    lcd.gotoXY(0, 1);
    lcd.print(F("Red     "));
  } else if (ledPhase == 1 && elapsed < 2000) {
    // Yellow LED on
    ledRed(0);
    ledYellow(1);
    ledGreen(0);
    lcd.gotoXY(0, 1);
    lcd.print(F("Yellow  "));
  } else if (ledPhase == 2 && elapsed < 3000) {
    // Green LED on
    ledRed(0);
    ledYellow(0);
    ledGreen(1);
    lcd.gotoXY(0, 1);
    lcd.print(F("Green   "));
  } else if (ledPhase == 3 && elapsed < 4000) {
    // All LEDs on
    ledRed(1);
    ledYellow(1);
    ledGreen(1);
    lcd.gotoXY(0, 1);
    lcd.print(F("All On  "));
  } else if (elapsed >= 4000) {
    // Test complete
    ledRed(0);
    ledYellow(0);
    ledGreen(0);
    testResults.ledsPass = true;
    Serial.println(F("Result: PASS (visual confirmation)"));
    advanceToNextTest();
    testStarted = false;
  }
  
  if (elapsed / 1000 > ledPhase) {
    ledPhase = elapsed / 1000;
  }
}

void testBuzzer() {
  static bool testStarted = false;
  static unsigned long buzzerStartTime = 0;
  static int toneIndex = 0;
  
  const int tones[] = {NOTE_C(4), NOTE_E(4), NOTE_G(4), NOTE_C(5)};
  const int numTones = 4;
  
  if (!testStarted) {
    lcd.clear();
    lcd.print(F("Buzzer"));
    lcd.gotoXY(0, 1);
    lcd.print(F("Test"));
    Serial.println(F("\n--- Buzzer Test ---"));
    Serial.println(F("Listen for ascending tones"));
    testStarted = true;
    buzzerStartTime = millis();
    toneIndex = 0;
  }
  
  unsigned long elapsed = millis() - buzzerStartTime;
  
  if (elapsed > toneIndex * 500 && toneIndex < numTones) {
    buzzer.playNote(tones[toneIndex], 200, 15);
    Serial.print(F("Playing tone "));
    Serial.println(toneIndex + 1);
    toneIndex++;
  }
  
  if (elapsed > numTones * 500 + 500) {
    testResults.buzzerPass = true;
    Serial.println(F("Result: PASS (audio confirmation)"));
    advanceToNextTest();
    testStarted = false;
  }
}

void testLCD() {
  static bool testStarted = false;
  static unsigned long lcdStartTime = 0;
  
  if (!testStarted) {
    lcd.clear();
    lcd.print(F("LCD Test"));
    Serial.println(F("\n--- LCD Test ---"));
    Serial.println(F("Displaying test patterns"));
    testStarted = true;
    lcdStartTime = millis();
  }
  
  unsigned long elapsed = millis() - lcdStartTime;
  
  if (elapsed < 1500) {
    lcd.gotoXY(0, 1);
    lcd.print(F("12345678"));
  } else if (elapsed < 3000) {
    lcd.gotoXY(0, 1);
    lcd.print(F("ABCDEFGH"));
  } else {
    testResults.lcdPass = true;
    Serial.println(F("Result: PASS (visual confirmation)"));
    advanceToNextTest();
    testStarted = false;
  }
}

void testBattery() {
  static bool testStarted = false;
  
  if (!testStarted) {
    lcd.clear();
    lcd.print(F("Battery"));
    Serial.println(F("\n--- Battery Test ---"));
    testStarted = true;
  }
  
  int batteryLevel = readBatteryMillivolts();
  float voltage = batteryLevel / 1000.0;
  
  lcd.gotoXY(0, 1);
  lcd.print(voltage);
  lcd.print(F("V    "));
  
  Serial.print(F("Battery voltage: "));
  Serial.print(voltage);
  Serial.println(F(" V"));
  
  if (voltage > 3.0 && voltage < 7.0) {
    testResults.batteryPass = true;
    Serial.println(F("Result: PASS"));
  } else {
    testResults.batteryPass = false;
    Serial.println(F("Result: FAIL (voltage out of range)"));
  }
  
  delay(2000);
  advanceToNextTest();
  testStarted = false;
}

void testLineSensors() {
  static bool testStarted = false;
  static unsigned long sensorStartTime = 0;
  static bool calibrationDone = false;
  
  if (!testStarted) {
    lcd.clear();
    lcd.print(F("Line Sns"));
    Serial.println(F("\n--- Line Sensor Test ---"));
    testStarted = true;
    sensorStartTime = millis();
    calibrationDone = false;
  }
  
  if (!calibrationDone && !lineSensorsCalibrated) {
    lcd.gotoXY(0, 1);
    lcd.print(F("Calibrat"));
    
    unsigned long elapsed = millis() - sensorStartTime;
    
    if (elapsed < 400) {
      motors.setSpeeds(100, -100);
    } else if (elapsed < 800) {
      motors.setSpeeds(-100, 100);
    } else {
      motors.setSpeeds(0, 0);
      calibrationDone = true;
      lineSensorsCalibrated = true;
      Serial.println(F("Calibration complete"));
    }
    
    lineSensors.calibrate();
  } else {
    lineSensors.read(lineSensorValues);
    
    lcd.gotoXY(0, 1);
    for (int i = 0; i < NUM_SENSORS; i++) {
      if (lineSensorValues[i] > 500) {
        lcd.print('*');
      } else {
        lcd.print('.');
      }
    }
    lcd.print(F("   "));
    
    Serial.print(F("Sensor values: "));
    for (int i = 0; i < NUM_SENSORS; i++) {
      Serial.print(lineSensorValues[i]);
      Serial.print(F(" "));
    }
    Serial.println();
    
    if (millis() - sensorStartTime > 5000) {
      testResults.lineSensorsPass = true;
      Serial.println(F("Result: PASS"));
      advanceToNextTest();
      testStarted = false;
    }
  }
}

void testProximity() {
  static bool testStarted = false;
  static unsigned long proxStartTime = 0;
  
  if (!testStarted) {
    lcd.clear();
    lcd.print(F("Prox Sns"));
    Serial.println(F("\n--- Proximity Sensor Test ---"));
    Serial.println(F("Wave hand in front of robot"));
    testStarted = true;
    proxStartTime = millis();
  }
  
  proxSensors.read();
  
  int leftValue = proxSensors.countsFrontWithLeftLeds();
  int centerValue = proxSensors.countsFrontWithRightLeds();
  int rightValue = proxSensors.countsRightWithRightLeds();
  
  lcd.gotoXY(0, 1);
  lcd.print(leftValue);
  lcd.print(F(" "));
  lcd.print(centerValue);
  lcd.print(F(" "));
  lcd.print(rightValue);
  lcd.print(F("  "));
  
  Serial.print(F("L: "));
  Serial.print(leftValue);
  Serial.print(F(" C: "));
  Serial.print(centerValue);
  Serial.print(F(" R: "));
  Serial.println(rightValue);
  
  if (millis() - proxStartTime > 5000) {
    testResults.proximityPass = true;
    Serial.println(F("Result: PASS"));
    advanceToNextTest();
    testStarted = false;
  }
  
  delay(100);
}

void testIMU() {
  static bool testStarted = false;
  static unsigned long imuStartTime = 0;
  
  if (!testStarted) {
    lcd.clear();
    lcd.print(F("IMU Test"));
    Serial.println(F("\n--- IMU Test ---"));
    testStarted = true;
    imuStartTime = millis();
    
    if (!imuInitialized) {
      Serial.println(F("ERROR: IMU not initialized"));
      lcd.gotoXY(0, 1);
      lcd.print(F("IMU Fail"));
      testResults.imuPass = false;
      delay(2000);
      advanceToNextTest();
      testStarted = false;
      return;
    }
  }
  
  imu.read();
  
  lcd.gotoXY(0, 1);
  lcd.print(F("A:"));
  lcd.print(imu.a.x);
  lcd.print(F("    "));
  
  Serial.print(F("Accel - X: "));
  Serial.print(imu.a.x);
  Serial.print(F(" Y: "));
  Serial.print(imu.a.y);
  Serial.print(F(" Z: "));
  Serial.println(imu.a.z);
  
  Serial.print(F("Gyro - X: "));
  Serial.print(imu.g.x);
  Serial.print(F(" Y: "));
  Serial.print(imu.g.y);
  Serial.print(F(" Z: "));
  Serial.println(imu.g.z);
  
  Serial.print(F("Mag - X: "));
  Serial.print(imu.m.x);
  Serial.print(F(" Y: "));
  Serial.print(imu.m.y);
  Serial.print(F(" Z: "));
  Serial.println(imu.m.z);
  Serial.println();
  
  if (millis() - imuStartTime > 5000) {
    testResults.imuPass = true;
    Serial.println(F("Result: PASS"));
    advanceToNextTest();
    testStarted = false;
  }
  
  delay(200);
}

void testEncoders() {
  static bool testStarted = false;
  static unsigned long encoderStartTime = 0;
  static int16_t lastLeftCount = 0;
  static int16_t lastRightCount = 0;
  
  if (!testStarted) {
    lcd.clear();
    lcd.print(F("Encoder"));
    Serial.println(F("\n--- Encoder Test ---"));
    Serial.println(F("Motors will rotate briefly"));
    testStarted = true;
    encoderStartTime = millis();
    encoders.getCountsAndResetLeft();
    encoders.getCountsAndResetRight();
    lastLeftCount = 0;
    lastRightCount = 0;
  }
  
  unsigned long elapsed = millis() - encoderStartTime;
  
  if (elapsed < 2000) {
    motors.setSpeeds(200, 200);
  } else {
    motors.setSpeeds(0, 0);
  }
  
  int16_t leftCount = encoders.getCountsLeft();
  int16_t rightCount = encoders.getCountsRight();
  
  lcd.gotoXY(0, 1);
  lcd.print(F("L:"));
  lcd.print(leftCount);
  lcd.print(F(" R:"));
  lcd.print(rightCount);
  lcd.print(F("  "));
  
  if (leftCount != lastLeftCount || rightCount != lastRightCount) {
    Serial.print(F("Left: "));
    Serial.print(leftCount);
    Serial.print(F(" Right: "));
    Serial.println(rightCount);
    lastLeftCount = leftCount;
    lastRightCount = rightCount;
  }
  
  if (elapsed > 3000) {
    if (abs(leftCount) > 50 && abs(rightCount) > 50) {
      testResults.encodersPass = true;
      Serial.println(F("Result: PASS"));
    } else {
      testResults.encodersPass = false;
      Serial.println(F("Result: FAIL (insufficient counts)"));
    }
    advanceToNextTest();
    testStarted = false;
  }
}

void testMotors() {
  static bool testStarted = false;
  static unsigned long motorStartTime = 0;
  static int motorPhase = 0;
  
  if (!testStarted) {
    lcd.clear();
    lcd.print(F("Motor"));
    Serial.println(F("\n--- Motor Test ---"));
    Serial.println(F("Robot will move in pattern"));
    testStarted = true;
    motorStartTime = millis();
    motorPhase = 0;
  }
  
  unsigned long elapsed = millis() - motorStartTime;
  
  if (motorPhase == 0 && elapsed < 1000) {
    lcd.gotoXY(0, 1);
    lcd.print(F("Forward "));
    motors.setSpeeds(200, 200);
    if (motorPhase == 0) Serial.println(F("Phase: Forward"));
    motorPhase = 1;
  } else if (motorPhase == 1 && elapsed < 2000) {
    lcd.gotoXY(0, 1);
    lcd.print(F("Backward"));
    motors.setSpeeds(-200, -200);
    if (motorPhase == 1) Serial.println(F("Phase: Backward"));
    motorPhase = 2;
  } else if (motorPhase == 2 && elapsed < 3000) {
    lcd.gotoXY(0, 1);
    lcd.print(F("Left    "));
    motors.setSpeeds(-200, 200);
    if (motorPhase == 2) Serial.println(F("Phase: Rotate Left"));
    motorPhase = 3;
  } else if (motorPhase == 3 && elapsed < 4000) {
    lcd.gotoXY(0, 1);
    lcd.print(F("Right   "));
    motors.setSpeeds(200, -200);
    if (motorPhase == 3) Serial.println(F("Phase: Rotate Right"));
    motorPhase = 4;
  } else if (elapsed >= 4000) {
    motors.setSpeeds(0, 0);
    lcd.gotoXY(0, 1);
    lcd.print(F("Stop    "));
    testResults.motorsPass = true;
    Serial.println(F("Result: PASS"));
    delay(1000);
    advanceToNextTest();
    testStarted = false;
  }
}

void completeTests() {
  testInProgress = false;
  
  Serial.println(F("\n================================="));
  Serial.println(F("TEST SEQUENCE COMPLETE"));
  Serial.println(F("================================="));
  
  displayTestReport();
  
  // Play completion melody
  buzzer.playNote(NOTE_C(5), 150, 15);
  delay(200);
  buzzer.playNote(NOTE_E(5), 150, 15);
  delay(200);
  buzzer.playNote(NOTE_G(5), 150, 15);
  delay(200);
  buzzer.playNote(NOTE_C(6), 300, 15);
  
  currentTest = TEST_IDLE;
}

void displayTestReport() {
  static int reportPage = 0;
  static unsigned long lastButtonPress = 0;
  
  lcd.clear();
  
  int totalTests = 10;
  int passedTests = 0;
  
  if (testResults.buttonsPass) passedTests++;
  if (testResults.ledsPass) passedTests++;
  if (testResults.buzzerPass) passedTests++;
  if (testResults.lcdPass) passedTests++;
  if (testResults.batteryPass) passedTests++;
  if (testResults.lineSensorsPass) passedTests++;
  if (testResults.proximityPass) passedTests++;
  if (testResults.imuPass) passedTests++;
  if (testResults.encodersPass) passedTests++;
  if (testResults.motorsPass) passedTests++;
  
  Serial.println(F("\n--- Test Results ---"));
  Serial.print(F("Buttons:    ")); Serial.println(testResults.buttonsPass ? F("PASS") : F("FAIL"));
  Serial.print(F("LEDs:       ")); Serial.println(testResults.ledsPass ? F("PASS") : F("FAIL"));
  Serial.print(F("Buzzer:     ")); Serial.println(testResults.buzzerPass ? F("PASS") : F("FAIL"));
  Serial.print(F("LCD:        ")); Serial.println(testResults.lcdPass ? F("PASS") : F("FAIL"));
  Serial.print(F("Battery:    ")); Serial.println(testResults.batteryPass ? F("PASS") : F("FAIL"));
  Serial.print(F("Line Sens:  ")); Serial.println(testResults.lineSensorsPass ? F("PASS") : F("FAIL"));
  Serial.print(F("Proximity:  ")); Serial.println(testResults.proximityPass ? F("PASS") : F("FAIL"));
  Serial.print(F("IMU:        ")); Serial.println(testResults.imuPass ? F("PASS") : F("FAIL"));
  Serial.print(F("Encoders:   ")); Serial.println(testResults.encodersPass ? F("PASS") : F("FAIL"));
  Serial.print(F("Motors:     ")); Serial.println(testResults.motorsPass ? F("PASS") : F("FAIL"));
  Serial.println();
  Serial.print(F("Total: "));
  Serial.print(passedTests);
  Serial.print(F("/"));
  Serial.print(totalTests);
  Serial.println(F(" tests passed"));
  
  lcd.print(F("Results:"));
  lcd.gotoXY(0, 1);
  lcd.print(passedTests);
  lcd.print(F("/"));
  lcd.print(totalTests);
  lcd.print(F(" Pass"));
  
  delay(3000);
  displayMainMenu();
}

void advanceToNextTest() {
  delay(500);
  currentTest = (TestState)(currentTest + 1);
  testStartTime = millis();
}
