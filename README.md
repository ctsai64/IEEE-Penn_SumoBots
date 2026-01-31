# IEEE-Penn_SumoBots

# Pololu Zumo 32U4 Robot - Comprehensive Test Suite

## Overview

This test suite provides a complete functionality test for the Pololu Zumo 32U4 Robot. It systematically tests all major components and subsystems to verify proper operation and identify any hardware or software issues.

## Features Tested

The test suite validates the following components:

### 1. **Buttons (A, B, C)**
- Tests all three onboard pushbuttons
- Verifies debouncing functionality
- Confirms proper electrical connections

### 2. **LEDs**
- Red LED
- Yellow LED  
- Green LED
- Tests individual and simultaneous operation

### 3. **Buzzer**
- Tone generation
- Frequency accuracy
- Volume control (PWM)
- Musical note playback

### 4. **LCD Display**
- 8x2 character display
- Character rendering
- Cursor positioning
- Clear and update functions

### 5. **Battery Monitor**
- Voltage measurement
- Safe operating range verification (3.0V - 7.0V)
- Low battery detection capability

### 6. **Line Sensors (5 sensors)**
- Automatic calibration routine
- Individual sensor readings
- Reflectance detection
- Dark/light surface discrimination

### 7. **Proximity Sensors**
- Left front sensor
- Center front sensor
- Right sensor
- IR LED emitters and detectors
- Distance measurement

### 8. **IMU (Inertial Measurement Unit)**
- 3-axis accelerometer (LSM6DS33)
- 3-axis gyroscope (LSM6DS33)
- 3-axis magnetometer (LIS3MDL)
- I2C communication
- Sensor fusion capability

### 9. **Encoders**
- Left motor encoder
- Right motor encoder
- Quadrature decoding
- Position tracking
- Count accuracy

### 10. **Motors**
- Left motor drive
- Right motor drive
- Forward motion
- Reverse motion
- Rotation (left and right)
- Speed control

## Hardware Requirements

- **Pololu Zumo 32U4 Robot** (fully assembled)
- **4x AA batteries** (fully charged recommended)
- **USB cable** (Micro-B) for programming
- **Clear testing surface** (for motor and line sensor tests)
- **Computer** with Arduino IDE installed

## Software Requirements

### Arduino IDE
- Version 1.8.x or higher (or Arduino IDE 2.x)
- Download from: https://www.arduino.cc/en/software

### Required Libraries

Install the following library via Arduino Library Manager:

1. **Zumo32U4** library by Pololu
   - Open Arduino IDE
   - Go to Sketch → Include Library → Manage Libraries
   - Search for "Zumo32U4"
   - Click Install

### Board Support

Install the Pololu A-Star 32U4 board support:

1. Go to File → Preferences
2. Add this URL to "Additional Board Manager URLs":
   ```
   https://files.pololu.com/arduino/package_pololu_index.json
   ```
3. Go to Tools → Board → Boards Manager
4. Search for "Pololu A-Star"
5. Install "Pololu A-Star Boards by Pololu"

## Installation

1. **Download the test script:**
   - Save `Zumo32U4_FullTest.ino` to your computer

2. **Open in Arduino IDE:**
   - Double-click the `.ino` file
   - Or open Arduino IDE and use File → Open

3. **Select the board:**
   - Tools → Board → Pololu A-Star Boards → Pololu A-Star 32U4

4. **Select the port:**
   - Connect Zumo via USB
   - Tools → Port → Select the appropriate COM port

5. **Upload:**
   - Click the Upload button (→)
   - Wait for "Done uploading" message

## Usage Instructions

### Initial Setup

1. **Power on the robot:**
   - Ensure batteries are installed and fresh
   - Turn on the power switch

2. **Connect Serial Monitor (optional):**
   - Tools → Serial Monitor
   - Set baud rate to 9600
   - Provides detailed test output and results

### Running the Tests

#### Automatic Test Sequence

1. **Start tests:**
   - Press **Button A** on the Zumo
   - Tests will run automatically in sequence

2. **Follow on-screen prompts:**
   - LCD displays current test name
   - Some tests require user interaction

#### Manual Test Review

- Press **Button C** at any time to view test results

### Test Sequence Details

#### Test 1: Buttons
- **Duration:** 10 seconds maximum
- **Action required:** Press buttons A, B, and C when prompted
- **Success:** All three buttons registered
- **LCD shows:** "Button Test:A,B,C"

#### Test 2: LEDs
- **Duration:** 4 seconds
- **Action required:** Observe LED colors
- **Success:** Red → Yellow → Green → All On sequence visible
- **LCD shows:** Current LED state

#### Test 3: Buzzer
- **Duration:** 3 seconds
- **Action required:** Listen for ascending tones
- **Success:** Four distinct tones heard (C4, E4, G4, C5)
- **LCD shows:** "Buzzer Test"

#### Test 4: LCD
- **Duration:** 3 seconds
- **Action required:** Observe display changes
- **Success:** Numbers "12345678" then letters "ABCDEFGH" displayed
- **LCD shows:** Test patterns

#### Test 5: Battery
- **Duration:** 2 seconds
- **Action required:** None
- **Success:** Voltage reading between 3.0V and 7.0V
- **LCD shows:** Current voltage (e.g., "5.24V")
- **Warning:** Replace batteries if voltage < 4.0V

#### Test 6: Line Sensors
- **Duration:** 5 seconds (plus calibration)
- **Action required:** Robot will rotate briefly for calibration
- **Success:** Sensors display values, respond to surface changes
- **LCD shows:** Sensor visualization (dots and asterisks)
- **Tip:** Move robot over different surfaces during test

#### Test 7: Proximity Sensors
- **Duration:** 5 seconds
- **Action required:** Wave hand in front of robot
- **Success:** Sensor values change when object detected
- **LCD shows:** Three sensor readings (L, C, R)

#### Test 8: IMU
- **Duration:** 5 seconds
- **Action required:** Tilt and move the robot
- **Success:** Accelerometer, gyroscope, magnetometer values update
- **LCD shows:** X-axis acceleration value
- **Serial Monitor:** Displays all 9 axes of data

#### Test 9: Encoders
- **Duration:** 3 seconds
- **Action required:** Robot will drive forward briefly
- **Success:** Both encoder counts increase (> 50 counts)
- **LCD shows:** Left and right encoder counts

#### Test 10: Motors
- **Duration:** 4 seconds
- **Action required:** Ensure robot has clear space
- **Success:** Robot moves forward, backward, rotates left, rotates right
- **LCD shows:** Current motor action
- **WARNING:** Hold robot or ensure clear 12" radius

### After Testing

1. **Review results:**
   - Press Button C to see summary
   - Check Serial Monitor for detailed output

2. **Interpret results:**
   - LCD shows "X/10 Pass" (X = number of passed tests)
   - Individual test results in Serial Monitor

3. **Completion melody:**
   - Four-note ascending melody plays when all tests complete

## Interpreting Results

### All Tests Pass (10/10)
✅ Robot is fully functional
- All systems operating normally
- Ready for programming and use

### Partial Failures (7-9/10)
⚠️ Minor issues detected
- Review Serial Monitor for specific failures
- Common issues:
  - Weak batteries → Battery test fail
  - Needs calibration → Line sensor issues
  - Loose connections → Intermittent failures

### Multiple Failures (< 7/10)
❌ Significant issues detected
- Check all physical connections
- Verify battery voltage and contacts
- Review assembly instructions
- Contact Pololu support if needed

## Troubleshooting

### Upload Issues

**Problem:** "Port not found" or "Upload failed"
- **Solution 1:** Install USB drivers from Pololu website
- **Solution 2:** Try different USB cable
- **Solution 3:** Press reset button before upload

**Problem:** "Board not detected"
- **Solution:** Verify correct board selected (Pololu A-Star 32U4)

### Runtime Issues

**Problem:** LCD shows nothing
- **Solution 1:** Adjust contrast potentiometer on back of LCD
- **Solution 2:** Check ribbon cable connection

**Problem:** Robot doesn't move during motor test
- **Solution 1:** Check battery voltage (should be > 4.5V)
- **Solution 2:** Verify power switch is ON
- **Solution 3:** Check motor connections

**Problem:** Line sensors always show zero
- **Solution 1:** Remove protective film from sensor array
- **Solution 2:** Check sensor ribbon cable

**Problem:** IMU test fails
- **Solution:** I2C connection issue - check solder joints on IMU

**Problem:** Encoders show no counts
- **Solution 1:** Check encoder cable connections
- **Solution 2:** Verify motor wires not reversed

### Common Error Messages

```
"IMU Fail" - IMU initialization failed
```
- Check I2C connections
- Verify IMU is properly seated

```
"FAIL (voltage out of range)"
```
- Battery voltage too low or too high
- Replace batteries or check connections

```
"FAIL (insufficient counts)"
```
- Encoders not registering movement
- Check encoder cables and motor operation

## Serial Monitor Output

The Serial Monitor provides detailed information:

```
=================================
Zumo 32U4 Comprehensive Test Suite
=================================

--- Button Test ---
Press buttons A, B, and C
Button A: PASS
Button B: PASS
Button C: PASS
Result: PASS

[... additional test output ...]

--- Test Results ---
Buttons:    PASS
LEDs:       PASS
Buzzer:     PASS
LCD:        PASS
Battery:    PASS
Line Sens:  PASS
Proximity:  PASS
IMU:        PASS
Encoders:   PASS
Motors:     PASS

Total: 10/10 tests passed
```

## Safety Precautions

⚠️ **Important Safety Notes:**

1. **Motor Tests:** Ensure robot has clear space or hold it during motor tests
2. **Battery Safety:** Use only AA batteries, not lithium rechargeables (voltage too high)
3. **Surface Protection:** Test on scratch-resistant surface
4. **USB Connection:** Disconnect USB before extensive motor testing
5. **Ventilation:** Ensure good airflow during extended operation

## Customization

### Adjusting Test Duration

Modify timeout values in the code:
```cpp
if (millis() - testStartTime > 10000)  // Change 10000 to desired milliseconds
```

### Changing Motor Speeds

Modify speed values (range: -400 to 400):
```cpp
motors.setSpeeds(200, 200);  // Change 200 to desired speed
```

### Adjusting Sensor Sensitivity

Modify threshold values:
```cpp
if (lineSensorValues[i] > 500)  // Adjust 500 for sensitivity
```

## Technical Specifications

### Zumo 32U4 Robot Specifications

- **Microcontroller:** ATmega32U4 (Arduino Leonardo compatible)
- **Operating Voltage:** 5V (logic), 3-7V (battery input)
- **Motor Voltage:** Up to 6V
- **Current per Motor:** 2A continuous, 3A peak
- **Encoders:** 12 counts per revolution per motor
- **Line Sensors:** 5x QTR-1A reflectance sensors
- **Proximity Sensors:** 3x IR emitter/detector pairs
- **IMU:** LSM6DS33 + LIS3MDL (9-axis)
- **LCD:** 8x2 character display
- **Buzzer:** Magnetic buzzer with PWM control

## Additional Resources

### Documentation
- **Zumo 32U4 User's Guide:** https://www.pololu.com/docs/0J63
- **Zumo32U4 Library Reference:** https://pololu.github.io/zumo-32u4-arduino-library/

### Example Code
- **Pololu GitHub:** https://github.com/pololu/zumo-32u4-arduino-library

### Support
- **Forum:** https://forum.pololu.com/
- **Email:** support@pololu.com
- **Phone:** (702) 262-6648

## License

This test script is provided as-is for educational and diagnostic purposes. Feel free to modify and distribute.

## Changelog

### Version 1.0 (2026-01-30)
- Initial release
- Complete test coverage for all Zumo 32U4 components
- Serial Monitor integration
- Automated test sequence
- Visual and audible feedback

## Contributing

Suggestions for improvements are welcome! Consider adding:
- Advanced IMU calibration tests
- Extended battery runtime analysis  
- Precision motor speed calibration
- Advanced line-following algorithm tests
- PID controller verification

## Author

Test Suite Development Team

---

**Last Updated:** January 30, 2026
**Version:** 1.0
**Compatible with:** Pololu Zumo 32U4 Robot (all variants)
