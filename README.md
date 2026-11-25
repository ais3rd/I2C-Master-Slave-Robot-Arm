# I2C Master-Slave Robot Arm

A 4-DOF mimic robot arm split across two Arduinos connected over **I²C**. The master reads a potentiometer "input arm", and the slave drives a servo "output arm" that copies its movements.

```
[Potentiometer arm] → Arduino MASTER ──I²C (0x08)──► Arduino SLAVE → [Servo arm]
                       InMaster.ino                  OutSlave.ino
```

## Joints
| Joint | Master pot pin | Slave servo pin |
|---|---|---|
| Pinch (gripper) | A0 | D3 |
| Wrist | A2 | D5 |
| Elbow | A1 | D6 |
| Waist | A3 | D9 |

## Master: `InMaster/InMaster.ino`
- Reads the four potentiometers
- Filters noise with an exponential moving average (α = 0.12, and 0.05 for the waist)
- Applies a deadband on the waist to stop jitter
- Maps each reading to a calibrated servo pulse width in µs
- Sends an 8-byte packet (4 × `uint16_t`) to slave address `0x08` every 15 ms

## Slave: `OutSlave/OutSlave.ino`
- Joins the I²C bus at address `0x08` and receives targets in an interrupt (`onReceive`)
- Smooths motion toward each target (smoothing factor 0.15, and 0.08 for the waist)
- Drives the four servos with a 500–2500 µs range

## Wiring
| Master | Slave |
|---|---|
| A4 (SDA) | A4 (SDA) |
| A5 (SCL) | A5 (SCL) |
| GND | GND |

Power the servos from a separate 5 V supply that shares GND with the Arduinos.

## Calibration
Set the `IN_*_MIN/MAX` (potentiometer) and `OUT_*_MIN/MAX` (servo µs) constants in `InMaster.ino` to match your arm.

## Upload
Open each sketch in the Arduino IDE and upload it to its own board (Uno or Nano). It uses only the built-in `Wire` and `Servo` libraries.

> `InMaster.ino` has a typo, `#define POT_ELBOW A1a`. Change it to `A1` before compiling.

## License
Released under the [MIT License](LICENSE).
