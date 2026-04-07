# HAKRC Hinged Quad

The HAKRC Hinged Quad is a plus-configuration quadrotor with four **hinged motor arms**
and independent **control surfaces** (flaps) mounted below each motor.
The hinged design allows arms to flex downward under gravity while remaining rigid
under thrust, reducing structural stress during hard landings and transportation.

A dedicated PX4 module ([`mc_hinged_quad`](../modules/modules_controller.md#mc_hinged_quad))
provides smooth transition between standard 4-motor operation and a reduced
*dual-axis* mode in which one diagonal motor pair is idled and the control
surfaces compensate for the loss of yaw authority.

## Frame Description

- **Configuration:** Plus (+), arm half-length 0.2 m (motor-to-centre)
- **AUW:** ~3 kg (with 4S LiPo)
- **Motors:** 4× brushless — CCW: M1 front, M3 rear; CW: M2 right, M4 left
- **Servos:** 4× standard digital servo, one below each motor arm
- **Control surfaces:** Hinged flap on each arm, driven by the servos
- **Battery:** 4S LiPo recommended

## Hardware Requirements

| Item              | Specification                               |
|-------------------|---------------------------------------------|
| Flight controller | HAKRC F405 V2 (or compatible STM32F4 board) |
| Motors            | 4× brushless, appropriate KV for 3 kg frame |
| ESCs              | 4× 30–45 A, DShot600 capable                |
| Servos            | 4× 9–20 g digital servo                     |
| RC transmitter    | 7+ channels (AUX1 used for mode switch)      |
| Battery           | 4S LiPo, 2200–3300 mAh                      |

## Flight Modes

### 4-Axis Mode (Standard)

All four motors are active.
Roll, pitch, yaw, and thrust are handled by the standard PX4 multicopter
control allocator.
The control surfaces follow roll/pitch/yaw setpoints as supplementary trim
surfaces, scaled by `HQUAD_CTRL_SURF`.

### Dual-Axis Mode

One diagonal motor pair (selected by `HQUAD_FAIL_DIAG`) is ramped to idle
over `HQUAD_TRANS_TIME` seconds.
The surviving pair provides thrust and differential roll or pitch authority.
Yaw is primarily handled by differential deflection of the control surfaces,
scaled by `HQUAD_DAXIS_YAW`.

| Parameter          | Default | Description                                           |
|--------------------|---------|-------------------------------------------------------|
| `HQUAD_MODE_CH`    | 1       | RC channel for mode switch (1=AUX1 … 6=AUX6)         |
| `HQUAD_TRANS_TIME` | 2.0 s   | Motor fade-out duration                               |
| `HQUAD_DAXIS_YAW`  | 1.0     | Yaw authority scalar in dual-axis mode                |
| `HQUAD_CTRL_SURF`  | 0.5     | Control surface authority scalar                      |
| `HQUAD_FAIL_DIAG`  | 0       | 0 = M1/M3 (front/rear) idle; 1 = M2/M4 (L/R) idle   |

### Mode Switch

Assign a 2- or 3-position switch on your RC transmitter to channel 1 (AUX1).

- **Low position (<50%):** 4-axis mode
- **High position (>50%):** Dual-axis mode (gradual transition)

## Setup and Configuration

### Airframe Selection

Select airframe **5002** (HAKRC Hinged Quad) in *QGroundControl* under
**Vehicle Setup → Airframe**.

### Output Mapping

| Output | Function          | Connector |
|--------|-------------------|-----------|
| 1      | Motor 1 (front)   | M1        |
| 2      | Motor 2 (right)   | M2        |
| 3      | Motor 3 (rear)    | M3        |
| 4      | Motor 4 (left)    | M4        |
| 5      | Servo 1 (front)   | S1        |
| 6      | Servo 2 (right)   | S2        |
| 7      | Servo 3 (rear)    | S3        |
| 8      | Servo 4 (left)    | S4        |

### RC Calibration

1. Calibrate the RC transmitter in *QGroundControl*.
2. Assign a 2- or 3-position switch to channel 1 (AUX1).
3. Verify in the *Actuator* panel that moving the switch changes the
   `HQUAD_MODE_CH` channel value above/below 0.5.

## PID Tuning Guidelines

The default PIDs are a starting point for a 3 kg, 40 cm diagonal airframe.
Tune rate PIDs in 4-axis mode first before testing dual-axis mode.

| Parameter        | Default | Notes                                         |
|------------------|---------|-----------------------------------------------|
| `MC_ROLLRATE_P`  | 0.12    | Increase if roll response is sluggish         |
| `MC_PITCHRATE_P` | 0.12    | Match roll P unless CG is offset fore/aft     |
| `MC_YAWRATE_P`   | 0.18    | Higher than typical; yaw inertia is large     |
| `MPC_THR_HOVER`  | 0.45    | Re-tune if AUW differs significantly          |

In dual-axis mode, yaw authority is reduced.
Increase `HQUAD_DAXIS_YAW` if yaw response is insufficient, or decrease
`HQUAD_CTRL_SURF` if control surfaces cause roll/pitch coupling.

## Safety Notes

:::warning
**Hinged arms:** Inspect hinge pins and fasteners before every flight.
A loose hinge can cause asymmetric thrust and an unrecoverable tumble.
:::

:::warning
**Dual-axis mode:** Only activate dual-axis mode in stable hover above a
safe altitude.
The first transition should be performed in a large, open area.
The vehicle may yaw unpredictably if `HQUAD_DAXIS_YAW` is not well-tuned.
:::

- Always arm in 4-axis mode.
- Keep `HQUAD_TRANS_TIME` ≥ 1.5 s to allow the attitude controller to
  adapt to the changing motor configuration.
- Never engage dual-axis mode during takeoff or landing.
- Ensure ESCs are configured to remain armed at minimum throttle; idle
  motors are commanded to `0.0` (minimum) — ESCs must not cut out at that
  level.

## Resources

- [HAKRC F405 V2 Flight Controller](../flight_controller/hakrc_f405v2.md)
