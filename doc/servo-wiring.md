# ESP32 servo wiring and tested example

The current `src/main.cpp` was successfully tested with a positional servo on
ESP32 GPIO18. An unchanged copy is saved in `examples/servo/servo.cpp` for reuse.
The servo model and exact power supply used in that test have not been recorded.

## Wiring

Disconnect power before changing connections. Use the servo manufacturer's pinout;
wire colours below are common conventions, not a guarantee.

| Servo connection | Connect to | Common wire colours |
| --- | --- | --- |
| Signal | ESP32 GPIO18 (pin labelled 18) | Orange, yellow or white |
| Power / V+ | External supply matching the servo's rated voltage and current | Red |
| Ground / GND | External supply negative and ESP32 GND | Brown or black |

The servo supply and ESP32 must share ground. GPIO18 carries only the control
signal; it does not power the servo. Do not power the servo from the ESP32 3.3 V
pin. Many hobby servos use a 5 V supply, but check the actual servo rating and
allow enough current for its load. The servo signal connects directly to GPIO18;
the Grove motor driver is not used in this example.

## Behaviour and commands

The firmware uses ESP32 hardware PWM at 50 Hz with 16-bit resolution. It maps
nominal angles from 0–180 degrees to 1000–2000 microsecond pulses. Actual travel
depends on the servo. This example is for a positional servo; a continuous
rotation servo interprets these pulses as speed and direction.

At startup the firmware commands the centre position (90 degrees, 1500 us).
Open the serial monitor at **115200 baud** and send a single character:

| Command | Action | Pulse width |
| --- | --- | --- |
| `1` | Position A: nominal 60 degrees | 1333 us |
| `2` | Position B: nominal 120 degrees | 1666 us |
| `c` | Centre: nominal 90 degrees | 1500 us |
| `s` | Disable control pulses | Constant low signal |

The `s` command leaves servo power connected. Sending `1`, `2` or `c` resumes
pulses. Serial acknowledgements report the requested position, not measured
physical movement. Adjust `POSITION_A` and `POSITION_B` to suit the mechanism;
keep angles within 0–180 degrees and check mechanical limits before attaching
the linkage. The current example moves immediately to each requested position.

## Build, upload and reuse

From the project root, with PlatformIO installed:

```sh
pio run
pio run --target upload
pio device monitor --baud 115200
```

`src/main.cpp` remains the active firmware. To restore the tested servo example
after changing the project for another motor, copy it back and build:

```sh
cp examples/servo/servo.cpp src/main.cpp
pio run
```

This replaces the active `main.cpp`, so save any changes there first. Keep the
saved example outside `src/`: it includes `setup()` and `loop()` and would
conflict with the active firmware if both were compiled together. It supports
Arduino ESP32 core versions 2 and 3 using their respective LEDC APIs.

## Direction for a motor control library

The saved example is a tested reference sketch, not yet a reusable library API.
As motor support grows, place reusable controllers under `lib/`, keep hardware
demonstrations under `examples/`, and keep wiring notes under `doc/`.

A future servo controller should accept the GPIO, pulse limits and positions as
configuration, expose initialisation, positioning and signal release methods,
and leave serial commands and `setup()` / `loop()` in the application. Keep servo
PWM control separate from the Grove I2C motor driver because they use different
hardware interfaces and control behaviour.
