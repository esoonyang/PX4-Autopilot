# HAKRC F405 V2 Flight Controller

:::warning
PX4 does not manufacture this (or any) autopilot.
Contact the [manufacturer](https://www.hakrc.com) for hardware support or compliance issues.
This board port is community-maintained.
:::

The HAKRC F405 V2 is a compact flight controller based on the STM32F405RGT6
microcontroller.
It uses the same core silicon as the Omnibus F4 SD and is well suited for
freestyle quadrotors and lightweight research platforms such as the
[HAKRC Hinged Quad](../frames_multicopter/hakrc_hingedquad.md).

## Specifications

- **MCU:** STM32F405RGT6, 168 MHz, 1 MB Flash, 192 KB RAM
- **IMU:** MPU-6000 (SPI1, CS PA4)
- **Barometer:** BMP280 (SPI3, CS PB3)
- **OSD:** MAX7456 (SPI3, CS PA15)
- **SD card:** SPI2 (CS PB12)
- **USB:** Micro-USB OTG FS (PA11/PA12)
- **Crystal:** 8 MHz HSE
- **Operating voltage:** 3.3 V logic; accepts 3S–6S via integrated BEC

## Pinout

```
              [ USB ]
+----+----+----+----+----+----+----+----+
| M1 | M2 | M3 | M4 | S1 | S2 | S3 | S4 |   PWM/DShot outputs
| PB0| PB1| PA3| PA2| PC8| PC9| PC6| PC7|
+----+----+----+----+----+----+----+----+

UART1 (GPS):      PA9 TX  / PA10 RX
UART3 (Telem):    PB10 TX / PB11 RX
UART6 (RC):       PC6 TX  / PC7 RX   (shared with S3/S4 — mutually exclusive)
I2C1  (Compass):  PB6 SCL / PB7 SDA
SPI1  (IMU):      PA5 CLK / PA6 MISO / PA7 MOSI  CS=PA4
SPI2  (SD):       PB13 CLK / PB14 MISO / PB15 MOSI  CS=PB12
SPI3  (Baro/OSD): PC10 CLK / PC11 MISO / PC12 MOSI  CS_Baro=PB3 CS_OSD=PA15
ADC1  (Battery):  PC3 Voltage / PC2 Current
LED   (Blue):     PB5
```

:::note
PC6 and PC7 are shared between UART6 (RC serial) and TIM8_CH1/CH2 (servo outputs
S3/S4).
Only one function can be active at a time; configure via `RC_PORT_CONFIG`.
When RC serial is used, only six servo/motor outputs (S1, S2, M1–M4) are available.
:::

## Where to Buy

- [HAKRC Official Store (TBD)](https://www.hakrc.com)

## PX4 Setup

### Firmware

Build firmware for this target:

```sh
make hakrc_f405v2_default
```

Flash with DFU (hold BOOT button while connecting USB):

```sh
make hakrc_f405v2_default upload
```

### Wiring

| Connector | Signal    | Notes                                  |
|-----------|-----------|----------------------------------------|
| UART1     | GPS       | Set `GPS_1_CONFIG = 101`               |
| UART3     | Telemetry | Set `TEL1_CONFIG = 103`                |
| UART6     | RC Input  | Set `RC_PORT_CONFIG = 102` for SBUS    |
| I2C1      | Compass   | HMC5883 / QMC5883 compatible           |
| M1–M4    | Motors    | DShot/PWM, plus-quad layout            |
| S1–S4    | Servos    | Standard 50 Hz PWM                     |

### Sensor Rotation

The MPU-6000 is mounted without rotation.
Leave `SENS_BOARD_ROT = 0` unless the board is physically rotated in the airframe.

### Battery Monitor

| Parameter      | Value | Notes                         |
|----------------|-------|-------------------------------|
| `BAT1_V_DIV`   | 11.12 | Default voltage divider ratio |
| `BAT1_A_PER_V` | 31    | Current sensor scale          |
| `BAT_N_CELLS`  | 4     | 4S LiPo default               |

### RC Input

Connect an SBUS or DSM receiver to UART6 (PC6/PC7) and set `RC_PORT_CONFIG`
accordingly.

:::note
When UART6 is used for RC serial, servo outputs S3 and S4 are unavailable.
:::

## Serial Port Mapping

| UART   | Device      | Default Use |
|--------|-------------|-------------|
| UART1  | /dev/ttyS0  | GPS         |
| UART3  | /dev/ttyS1  | Telemetry   |
| UART6  | /dev/ttyS2  | RC Input    |

## Debug Port

SWD pads (SWDIO/SWDCLK) are exposed on the board.
Connect a J-Link or ST-Link v2 for firmware debugging.

## Schematics

Schematics are available from the manufacturer (TBD).
