#include <Wire.h>
#include <Zumo32U4.h>

Zumo32U4OLED    display;
Zumo32U4ButtonA buttonA;
Zumo32U4Buzzer  buzzer;
Zumo32U4Motors  motors;
Zumo32U4LineSensors      lineSensors;
Zumo32U4ProximitySensors proxSensors;
Zumo32U4IMU     imu;

int lineValues[3];
const int reverseSpeed   = -300;   // faster reverse to get away from edge
const int turnSpeed      = 60;     // slower search spin for better sensor reads
const int fastTurnSpeed  = 350;
const int forwardSpeed   = 300;
const int rammingSpeed   = 400;    // full power ram

const int waitTime       = 5200;

const int reverseTime    = 600;    // longer reverse to really reach center
const int escapeTurnTime = 900;    // wide sweep spin

const int rammingTimeout   = 1500;
const int openingTurnTime  = 260;
const int openingLungeTime = 600;
const int repositionTime   = 400;  // drive inward after spin if no target found

int whiteBaseline[3]       = {0, 0, 0};
int lineSensorThreshold[3] = {0, 0, 0};
int32_t gyroOffsetZ = 0;
const int32_t PUSH_RATE_THRESHOLD = 300;

enum State
{

  Pausing,
  Waiting,
  OpeningTurn,
  OpeningLunge,
  Scanning,
  Driving,
  Ramming,
  Backing,
  EscapeTurn,
  Reposition,
};

State currentState = Pausing;

int scanDir      = 1;
int opponentSide = 1;

uint32_t stateStartTime;
uint32_t roundStartTime;
uint32_t lostTargetTime = 0;   // tracks when Driving last lost sight of opponent
bool justChangedState;

void calibrateLineSensors();
void calibrateGyro();
bool borderDetection();
void gyroResistPush(int baseSpeed);
void changeState(uint8_t newState);
uint32_t timeInThisState();

void setup()
{
  lineSensors.initThreeSensors();
  proxSensors.initThreeSensors();
  Wire.begin();
  imu.init();
  imu.enableDefault();

  calibrateLineSensors();
  calibrateGyro();
  changeState(Pausing);
}

void calibrateLineSensors()
{
  // --- White surface sample ---
  display.clear();
  display.print(F("On WHITE"));
  display.gotoXY(0, 1);
  display.print(F("press A"));
  while (!buttonA.getSingleDebouncedPress()) {}

  int32_t whiteTotals[3] = {0, 0, 0};
  for (int i = 0; i < 20; i++)
  {
    lineSensors.read(lineValues);
    whiteTotals[0] += lineValues[0];
    whiteTotals[1] += lineValues[1];
    whiteTotals[2] += lineValues[2];
    delay(20);
  }
  int whiteSamples[3];
  for (int s = 0; s < 3; s++)
    whiteSamples[s] = whiteTotals[s] / 20;

  display.clear();
  display.print(F("W:"));
  display.print(whiteSamples[0]);
  display.print(F(" "));
  display.print(whiteSamples[1]);
  display.gotoXY(0, 1);
  display.print(F("  "));
  display.print(whiteSamples[2]);
  delay(800);

  // --- Dark surface sample ---
  display.clear();
  display.print(F("On DARK"));
  display.gotoXY(0, 1);
  display.print(F("press A"));
  while (!buttonA.getSingleDebouncedPress()) {}

  int32_t darkTotals[3] = {0, 0, 0};
  for (int i = 0; i < 20; i++)
  {
    lineSensors.read(lineValues);
    darkTotals[0] += lineValues[0];
    darkTotals[1] += lineValues[1];
    darkTotals[2] += lineValues[2];
    delay(20);
  }
  int darkSamples[3];
  for (int s = 0; s < 3; s++)
    darkSamples[s] = darkTotals[s] / 20;

  display.clear();
  display.print(F("D:"));
  display.print(darkSamples[0]);
  display.print(F(" "));
  display.print(darkSamples[1]);
  display.gotoXY(0, 1);
  display.print(F("  "));
  display.print(darkSamples[2]);
  delay(800);

  for (int s = 0; s < 3; s++)
  {
    whiteBaseline[s]       = whiteSamples[s];
    lineSensorThreshold[s] = (whiteSamples[s] + darkSamples[s]) / 2;
  }

  display.clear();
  display.print(F("Thr:"));
  display.print(lineSensorThreshold[0]);
  display.print(F(" "));
  display.print(lineSensorThreshold[1]);
  display.gotoXY(0, 1);
  display.print(F("    "));
  display.print(lineSensorThreshold[2]);
  display.print(F(" OK"));
  delay(1200);
}

void calibrateGyro()
{
  display.clear();
  display.print(F("GyroCal"));
  int32_t total = 0;
  for (int i = 0; i < 100; i++)
  {
    imu.read();
    total += imu.g.z;
    delay(10);
  }
  gyroOffsetZ = total / 100;
  display.clear();
  display.print(F("Ready"));
  delay(400);
}

void loop()
{
  bool buttonPress = buttonA.getSingleDebouncedPress();

  if (currentState == Pausing)
  {
    motors.setSpeeds(0, 0);

    if (justChangedState)
    {
      justChangedState = false;
      display.clear();
      display.print(F("Press A"));
    }

    if (millis() % 1000 < 10)
    {
      display.gotoXY(0, 1);
      display.print(readBatteryMillivolts());
      display.print(F("mV"));
    }

    if (buttonPress)
      changeState(Waiting);
  }
  else if (currentState == Waiting)
  {
    motors.setSpeeds(0, 0);

    if (justChangedState)
    {
      justChangedState = false;
      display.clear();
      display.print(F("WAIT"));
      buzzer.playNote(NOTE_A(4), 50, 15);
    }

    uint32_t elapsed  = timeInThisState();
    uint32_t timeLeft = waitTime - elapsed;
    display.gotoXY(0, 1);
    display.print(timeLeft / 1000);
    display.print(F("."));
    display.print((timeLeft % 1000) / 100);
    display.print(F("s "));

    proxSensors.read();
    int sideL = proxSensors.countsLeftWithLeftLeds();
    int sideR = proxSensors.countsRightWithRightLeds();
    if (sideL > 0 || sideR > 0)
      opponentSide = (sideL >= sideR) ? -1 : 1;
    scanDir = opponentSide;

    if (elapsed >= waitTime)
    {
      roundStartTime = millis();
      buzzer.playNote(NOTE_A(5), 100, 15);
      changeState(OpeningTurn);
    }
  }

  else if (currentState == OpeningTurn)
  {
    if (justChangedState)
    {
      justChangedState = false;
      display.clear();
      display.print(F("TURN"));
      ledGreen(1);
    }

    proxSensors.read();
    int frontL = proxSensors.countsFrontWithLeftLeds();
    int frontR = proxSensors.countsFrontWithRightLeds();

    if (frontL >= 2 || frontR >= 2)
      changeState(OpeningLunge);
    else if (timeInThisState() >= openingTurnTime)
      changeState(OpeningLunge);
    else
      motors.setSpeeds(opponentSide * fastTurnSpeed, -opponentSide * fastTurnSpeed);

    borderDetection();
  }

  else if (currentState == OpeningLunge)
  {
    if (justChangedState)
    {
      justChangedState = false;
      display.clear();
      display.print(F("LUNGE"));
      ledRed(1);
    }

    motors.setSpeeds(rammingSpeed, rammingSpeed);

    proxSensors.read();
    int frontL = proxSensors.countsFrontWithLeftLeds();
    int frontR = proxSensors.countsFrontWithRightLeds();

    if (frontL >= 4 && frontR >= 4)
      changeState(Ramming);
    else if (timeInThisState() >= openingLungeTime)
    {
      if (frontL >= 2 || frontR >= 2) changeState(Driving);
      else                            changeState(Scanning);
    }

    borderDetection();
  }

  else if (currentState == Scanning)
  {
    if (justChangedState)
    {
      justChangedState = false;
      display.clear();
      display.print(F("Scan"));
    }

    proxSensors.read();
    int frontL = proxSensors.countsFrontWithLeftLeds();
    int frontR = proxSensors.countsFrontWithRightLeds();
    int sideL  = proxSensors.countsLeftWithLeftLeds();
    int sideR  = proxSensors.countsRightWithRightLeds();

    if (frontL >= 2 || frontR >= 2)
      changeState(Driving);
    else if (sideL >= 2 || sideR >= 2)
    {
      if (sideL > sideR) { scanDir = -1; motors.setSpeeds(-fastTurnSpeed,  fastTurnSpeed); }
      else               { scanDir =  1; motors.setSpeeds( fastTurnSpeed, -fastTurnSpeed); }
    }
    else
      motors.setSpeeds(scanDir * turnSpeed, -scanDir * turnSpeed);

    borderDetection();
  }

  else if (currentState == Driving)
  {
    if (justChangedState)
    {
      justChangedState = false;
      display.clear();
      display.print(F("Drive"));
      lostTargetTime = 0;
    }

    proxSensors.read();
    int frontL = proxSensors.countsFrontWithLeftLeds();
    int frontR = proxSensors.countsFrontWithRightLeds();

    if (frontL >= 2 || frontR >= 2)
    {
      lostTargetTime = 0;  // reset — we still see them

      if (frontL >= 4 && frontR >= 4)
      {
        changeState(Ramming);
      }
      else
      {
        int signalDiff = frontL - frontR;
        int speedDiff  = signalDiff * 80;
        motors.setSpeeds(
          constrain(forwardSpeed - speedDiff, -400, 400),
          constrain(forwardSpeed + speedDiff, -400, 400)
        );
        gyroResistPush(forwardSpeed);
      }
    }
    else
    {
      // Lost sight — keep driving forward briefly in case it's a bad read
      if (lostTargetTime == 0)
        lostTargetTime = millis();

      if (millis() - lostTargetTime > 150)
        changeState(Scanning);  // truly gone, start searching
      else
        motors.setSpeeds(forwardSpeed, forwardSpeed);  // coast forward
    }

    borderDetection();
  }

  else if (currentState == Ramming)
  {
    if (justChangedState)
    {
      justChangedState = false;
      display.clear();
      display.print(F("RAM!"));
      ledRed(1);
    }

    motors.setSpeeds(rammingSpeed, rammingSpeed);
    gyroResistPush(rammingSpeed);

    proxSensors.read();
    int frontL = proxSensors.countsFrontWithLeftLeds();
    int frontR = proxSensors.countsFrontWithRightLeds();

    if (frontL == 0 && frontR == 0 && timeInThisState() > 300)
      changeState(Scanning);
    if (timeInThisState() > rammingTimeout)
      changeState(Scanning);

    borderDetection();
  }

  else if (currentState == Backing)
  {
    if (justChangedState)
    {
      justChangedState = false;
      display.clear();
      display.print(F("BACK!"));
      ledYellow(1);
    }

    motors.setSpeeds(reverseSpeed, reverseSpeed);

    if (timeInThisState() >= reverseTime)
      changeState(EscapeTurn);

    // *** CHANGED: do NOT call borderDetection() during Backing.
    // The old code would re-trigger Backing immediately if the sensor
    // was still over the border line, causing the stuck-at-edge loop.
  }

  else if (currentState == EscapeTurn)
  {
    if (justChangedState)
    {
      justChangedState = false;
      display.clear();
      display.print(F("EscTurn"));
    }

    motors.setSpeeds(scanDir * fastTurnSpeed, -scanDir * fastTurnSpeed);

    // *** CHANGED: check prox sensors during spin — lock on immediately
    proxSensors.read();
    int frontL = proxSensors.countsFrontWithLeftLeds();
    int frontR = proxSensors.countsFrontWithRightLeds();

    if ((frontL >= 2 || frontR >= 2) && timeInThisState() > 100)
      changeState(Driving);   // opponent found mid-spin, lock on!
    else if (timeInThisState() >= escapeTurnTime)
      changeState(Reposition); // no target found — drive inward first

    borderDetection();
  }

  else if (currentState == Reposition)
  {
    // Drive forward (away from edge) to get toward center before scanning
    if (justChangedState)
    {
      justChangedState = false;
      display.clear();
      display.print(F("REPOS"));
      ledGreen(1);
    }

    motors.setSpeeds(forwardSpeed, forwardSpeed);

    // If we spot the opponent while repositioning, lock on immediately
    proxSensors.read();
    int frontL = proxSensors.countsFrontWithLeftLeds();
    int frontR = proxSensors.countsFrontWithRightLeds();

    if (frontL >= 2 || frontR >= 2)
      changeState(Driving);
    else if (timeInThisState() >= repositionTime)
      changeState(Scanning);

    borderDetection();
  }

  else
    changeState(Pausing);
}

bool borderDetection()
{
  lineSensors.read(lineValues);

  bool bL = (lineValues[0] < lineSensorThreshold[0]);
  bool bC = (lineValues[1] < lineSensorThreshold[1]);
  bool bR = (lineValues[2] < lineSensorThreshold[2]);

  if (!bL && !bC && !bR)
    return false;

  if (currentState == Backing || currentState == EscapeTurn)
  {
    if (bL && bR)
    {
      scanDir = -scanDir;
      changeState(Backing);
      return true;
    }
    return false;
  }

  if (bL && !bR)
  {
    scanDir = 1;        // left sensor on dark -> turn right (scanDir = 1)
    changeState(Backing);
    return true;
  }

  if (bR && !bL)
  {
    scanDir = -1;       // right sensor on dark -> turn left (scanDir = -1)
    changeState(Backing);
    return true;
  }

  if (bL && bR)
  {
    scanDir = -scanDir;
    changeState(Backing);
    return true;
  }

  if (bC)
  {
    scanDir = -scanDir;
    changeState(Backing);
    return true;
  }

  return false;
}

void gyroResistPush(int baseSpeed)
{
  imu.read();
  int32_t gyroZ = imu.g.z - gyroOffsetZ;

  if (abs(gyroZ) > PUSH_RATE_THRESHOLD)
  {
    int correction = constrain((int)(gyroZ / 7), -150, 150);
    int leftSpeed  = constrain(baseSpeed - correction, -400, 400);
    int rightSpeed = constrain(baseSpeed + correction, -400, 400);
    motors.setSpeeds(leftSpeed, rightSpeed);
  }
}

uint32_t timeInThisState()
{
  return (uint32_t)(millis() - stateStartTime);
}

void changeState(uint8_t newState)
{
  currentState     = (State)newState;
  justChangedState = true;
  stateStartTime   = millis();
}
