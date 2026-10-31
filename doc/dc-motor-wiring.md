# DC motor test with Grove TB6612FNG

This example assumes the Grove I2C Motor Driver TB6612FNG already supported by
the bundled driver, at address `0x14`. It is not for a bare TB6612FNG breakout or
an L298N module. The active sketch is `src/main.cpp`; a reference copy is in
`examples/dc-motor/dc-motor.cpp`. Hardware testing is still pending.

With power disconnected, connect:

| Connection | Destination |
| --- | --- |
| ESP32 GPIO21 | Driver SDA |
| ESP32 GPIO22 | Driver SCL |
| ESP32 GND | Driver GND and motor supply negative |
| Driver logic supply | Supply specified for the Grove board |
| Driver motor power input | External supply suitable for both board and motor |
| Motor's two leads | Channel A output pair |

Check the board's logic interface and pull-ups are compatible with the ESP32's
3.3 V GPIO. Record the motor rating and supply voltage before powering the motor;
do not power it from an ESP32 GPIO. Disconnect the previous servo for this test.
Use a free-running DC motor, not a turnout solenoid: this sketch runs for one
second, unlike the earlier short turnout pulses.

The firmware requests stop on both channels and standby at startup. It sends no
movement command until requested over serial at **115200 baud**:

| Command | Action |
| --- | --- |
| `1` | Run channel A forward for 10 seconds |
| `2` | Run channel A in reverse for 10 seconds |
| `s` | Stop both channels and enter standby |

The activity LED is configured as an active-high user LED on GPIO2 (D2), common
on ESP32 DevKit boards. It lights just before the run commands are transmitted,
stays on during the 10-second commanded run, and turns off after stop commands
or on a communication fault. This indicates firmware activity, not measured
motor movement or confirmed output voltage. The power LED is not controllable.
If your board's user LED uses another pin or polarity, change
`ACTIVITY_LED_PIN` / `ACTIVITY_LED_ON` in the sketch.

Wait one second after stopping before another run. Directions depend on the
motor lead orientation. PWM magnitude is `TEST_SPEED = 255` out of 255; it does
not limit supply voltage or guarantee that a loaded motor will start. Adjust
`TEST_SPEED` and `RUN_MS` in the sketch as needed for the actual motor.

I2C acknowledgements confirm communication, not motor movement. Communication
errors latch a fault and trigger repeated best-effort stop attempts. The timeout
runs on the ESP32; it cannot guarantee shutdown if the ESP32 or I2C bus fails.
Switch motor power off if a stop fails, then fix wiring and reset.

Build with `pio run`, upload with `pio run --target upload`, and open the monitor
with `pio device monitor --baud 115200`. The saved servo example remains in
`examples/servo/servo.cpp`.
