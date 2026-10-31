# L298N DC motor example

This version uses a typical L298N module with logic supply, flyback diodes and
channel A terminals exposed. It uses GPIO directly, with no I2C or Grove library.
The L298 accepts TTL control inputs; its enable input disables the bridge when low.
Reference: https://www.st.com/resource/en/datasheet/l298.pdf

Disconnect power before wiring:

| L298N module | Connection |
| --- | --- |
| ENA | ESP32 GPIO25 (D25); remove the ENA jumper first |
| IN1 | ESP32 GPIO26 (D26) |
| IN2 | ESP32 GPIO27 (D27) |
| GND | ESP32 GND and bench supply negative |
| Motor supply / VS (often labelled +12V) | Bench positive, at the motor's rated voltage |
| OUT1 / OUT2 | Motor leads |
| ENB | Remove ENB jumper and connect ENB to GND to disable unused channel B |
| IN3 / IN4 | GND for unused channel B |

The L298 logic supply requires 5 V, not ESP32 3V3. Module regulator arrangements
vary: follow your board's documentation for the separate 5V-EN regulator jumper.
If using external regulated 5 V logic power, disable the onboard regulator as
specified by the module manufacturer. Do not combine competing 5 V supplies or
connect a module's 5 V output to ESP32 3V3. The ENA jumper and regulator jumper
serve different purposes. An ENA jumper left installed can connect 5 V to GPIO25.

Add a 10 kΩ resistor from ENA to GND to keep the bridge disabled while the ESP32
is resetting or disconnected. This is a recommended startup precaution; software
cannot hold GPIO levels before it starts. The L298 has appreciable voltage drop,
so motor voltage will be below the supply voltage even at full enable.

At 115200 baud: `1` runs forward, `2` reverse, and `s` disables ENA for a coast
stop. Runs last 10 seconds at full enable. Stop and wait one second before
another run. GPIO2's active-high user LED lights during the run. GPIO output
messages and the LED do not confirm motor movement. There is no driver feedback.

Build and upload the separate environment:

```sh
pio run -e l298n
pio run -e l298n --target upload
pio device monitor --baud 115200
```

The default `esp32dev` environment still builds the Grove TB6612 sketch in
`src/main.cpp`. Its named reference is
`examples/tb6612-dc-motor/tb6612-dc-motor.cpp`; the earlier
`examples/dc-motor/dc-motor.cpp` is retained for compatibility. The L298N source
is `examples/l298n-dc-motor/l298n-dc-motor.cpp`. Neither example is a finished
motor-control library; they preserve the separate hardware interfaces for reuse.
