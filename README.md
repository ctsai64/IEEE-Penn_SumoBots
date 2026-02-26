# SumoBot – Arduino Program README
### IEEE Spring 2024 Joint R2 & R1 SAC | Organizer: Jenna Fazio

---

## Overview

This Arduino sketch is written for the **Pololu Zumo 32U4** robot (the required Kit category platform) and implements a fully autonomous sumo-fighting strategy that complies with the official SumoBots Competition Rules.

The robot requires **no human input** after the start button is pressed. It uses onboard IR proximity sensors to hunt down opponents and reflectance line sensors to detect and avoid the Dohyo border.

---

## Required Libraries

Install these via the Arduino Library Manager before compiling:

| Library | Purpose |
|---|---|
| `Zumo32U4` (Pololu) | Motors, sensors, LCD, buzzer, button |
| `Wire` | I²C communication (included with Arduino IDE) |

---

## Key Features

### 1. Rule-Compliant 5-Second Start Delay *(Rule 6.5)*
The robot will **not move** for the first 5 000 ms after Button A is pressed. A live countdown is displayed on the LCD and a beep sounds each second. This prevents warnings and match restarts.

### 2. Button-Activated Start *(Rule 4.4)*
The robot is armed by pressing the onboard **Button A**. This satisfies the competition's starting mechanism requirement and gives the team full control over exactly when the 5-second clock begins.

### 3. Fully Autonomous State Machine *(Rules 3.1, 4.2)*
After the delay, the robot runs a 4-state loop with no external input:

| State | Behaviour |
|---|---|
| `WAITING` | Idle; displays prompt on LCD |
| `DELAYING` | Counts down 5 seconds (Rule 6.5) |
| `SEARCHING` | Slow spin to locate opponent via IR |
| `ATTACKING` | Full-speed charge; steers toward stronger IR signal |
| `EVADING` | Detects white border → reverse → turn back to centre |

### 4. Opponent Detection via Proximity Sensors
The Zumo 32U4's front-facing IR proximity sensors are read continuously during `SEARCHING` and `ATTACKING`. The robot steers toward whichever side (left or right IR emitter) receives the stronger reflection, allowing it to arc around and push the opponent off-centre.

### 5. Border Detection via Line Sensors *(Rule 6.4)*
Three reflectance line sensors (left, centre, right) are polled every loop. When any sensor reads above `LINE_THRESHOLD` (indicating the **white border** of the Dohyo), the robot immediately enters `EVADING` — reversing away from the edge before turning back into the arena. Border detection always takes priority over attacking.

### 6. Adaptive Search Pattern
If no opponent is detected for `STUCK_TIMEOUT_MS` (default 2 s), the spin direction alternates. This prevents the robot from circling indefinitely in one direction and makes its behaviour less predictable to the opponent.

### 7. Directional Steering During Attack
Rather than blindly charging straight ahead, the robot compares left vs. right IR readings to **arc toward the opponent's strongest signal**, improving the chance of a clean push.

### 8. LCD Status Display
The 8×2 LCD shows the current state at each transition:
- `SumoBot / Press A` → waiting
- `Ready!` → button pressed
- `Start: N` → countdown
- `FIGHT!` → match live
- `TARGET!` → opponent acquired
- `Search` → lost target, resuming scan

### 9. Audio Feedback (Buzzer)
Single beeps during countdown, a double-frequency beep at match start, giving the team audible confirmation without needing to watch the LCD.

---

## Configuration Constants

All tuning values are defined at the top of the sketch for easy adjustment:

| Constant | Default | Description |
|---|---|---|
| `START_DELAY_MS` | `5000` | Mandatory pre-match delay (do not reduce) |
| `DRIVE_SPEED` | `400` | Attack charge speed (max 400) |
| `SEARCH_SPEED` | `200` | Rotation speed during scan |
| `REVERSE_SPEED` | `-400` | Speed when backing off border |
| `REVERSE_DURATION_MS` | `300` | How long to reverse from border |
| `TURN_DURATION_MS` | `350` | How long to turn after reversing |
| `LINE_THRESHOLD` | `500` | White border sensitivity (0–1000) |
| `PROX_THRESHOLD` | `4` | IR proximity trigger level (0–6) |
| `STUCK_TIMEOUT_MS` | `2000` | Search direction flip interval |

---

## Calibration Tips

- **Line sensors**: Call `lineSensors.calibrate()` in `setup()` while manually sliding the robot across both the black surface and the white border. This improves `LINE_THRESHOLD` accuracy on your specific Dohyo lighting conditions.
- **Proximity**: Lower `PROX_THRESHOLD` to detect opponents earlier (more sensitive); raise it to reduce false positives from walls or spectators outside the Interference Zone.
- **Speed tuning**: If the robot spins off the edge during `EVADING`, increase `REVERSE_DURATION_MS` or reduce `DRIVE_SPEED`.

---

## Rules Compliance Checklist

| Rule | Requirement | Implementation |
|---|---|---|
| 3.1 / 4.2 | Fully autonomous, no outside interference | State machine, no Serial/wireless input |
| 3.2 | Kit: software modifications only | Pure firmware, no hardware additions required |
| 3.3 / 4.5 | Non-destructive, non-harmful | No weapons, projectiles, or combustion |
| 4.4 | Robot starts on "Ready, Set, Go" command | Button A triggers the 5-second delay |
| 4.5 | Powered by standard removable batteries | Zumo 32U4 uses 4× AA batteries |
| 6.4 | Leaves Dohyo = round loss | Border detection actively prevents this |
| 6.5 | 5-second delay after button press | `START_DELAY_MS = 5000`, enforced in `DELAYING` state |

> ⚠️ **Never reduce `START_DELAY_MS` below 5000.** Early movement triggers a warning; three warnings forfeit the match (Rule 6.5).

---

## File Structure

```
sumobot_zumo32u4.ino   ← Main Arduino sketch (this file)
README.md              ← This documentation
```

---

## Competition Quick-Start

1. Flash `sumobot_zumo32u4.ino` to the Zumo 32U4.
2. Pass safety inspection (Rule 6.1).
3. Place the robot parallel to the opponent in the centre of the Dohyo (Rule 6.4).
4. When the referee says **"Ready, Set, Go!"** — press **Button A**.
5. Step back out of the 55-inch Interference Zone within 5 seconds (Rule 6.5).
6. The robot fights autonomously. First to 2 round wins takes the match (Rule 6.3).

---

*Built for the IEEE Spring 2024 SumoBots Competition – Kit Category*
