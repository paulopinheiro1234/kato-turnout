#include <Arduino.h>
#include <Wire.h>
#include <Grove_Motor_Driver_TB6612FNG.h>

constexpr uint8_t DRIVER_ADDRESS = 0x14;
constexpr uint8_t SDA_PIN = 21; // White Grove wire -> D21 on this ESP32.
constexpr uint8_t SCL_PIN = 22; // Yellow Grove wire -> D22 on this ESP32.
constexpr uint8_t ACTIVITY_LED_PIN = 2; // Typical ESP32 DevKit user LED (D2).
constexpr uint8_t ACTIVITY_LED_ON = HIGH; // Change to LOW for an active-low LED.
constexpr uint8_t TEST_SPEED = 255; // Full PWM; motor sees the external supply voltage.
constexpr uint32_t RUN_MS = 10000;
constexpr uint32_t COOLDOWN_MS = 1000;
bool ready = false;
bool running = false;
bool fault = false;
bool cooling = false;
uint32_t startedAt = 0;
uint32_t stoppedAt = 0;

void setActivityLed(bool on) {
  digitalWrite(ACTIVITY_LED_PIN, on ? ACTIVITY_LED_ON
                                   : (ACTIVITY_LED_ON == HIGH ? LOW : HIGH));
}

// Use the bundled driver's protocol, checking each I2C acknowledgement.
bool command(uint8_t operation, uint8_t value, bool withSpeed = false) {
  Wire.beginTransmission(DRIVER_ADDRESS);
  Wire.write(operation);
  Wire.write(value);
  if (withSpeed) Wire.write(TEST_SPEED);
  const bool ok = Wire.endTransmission() == 0;
  delay(1);
  return ok;
}

bool stopOutputs() {
  const bool a = command(GROVE_MOTOR_DRIVER_I2C_CMD_STOP, MOTOR_CHA);
  const bool b = command(GROVE_MOTOR_DRIVER_I2C_CMD_STOP, MOTOR_CHB);
  const bool standby = command(GROVE_MOTOR_DRIVER_I2C_CMD_STANDBY, 0);
  setActivityLed(false);
  running = false;
  cooling = true;
  stoppedAt = millis();
  return a && b && standby;
}

void latchFault() {
  fault = true;
  ready = false;
  setActivityLed(false);
  Serial.println("I2C fault: switch motor power OFF. Check wiring and reset to retry.");
}

void scanBus() {
  Serial.println("I2C scan: SDA D21, SCL D22 (no movement commands).");
  unsigned found = 0;
  for (uint8_t address = 1; address < 127; ++address) {
    Wire.beginTransmission(address);
    const uint8_t result = Wire.endTransmission();
    if (result == 0) {
      Serial.printf("Found I2C device at 0x%02X\n", address);
      ++found;
    }
  }
  Serial.printf("Scan complete: %u device(s).\n", found);
}

void setup() {
  setActivityLed(false);
  pinMode(ACTIVITY_LED_PIN, OUTPUT);
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);
  Wire.setTimeOut(10);
  delay(200);
  scanBus();
  ready = stopOutputs();
  if (!ready) {
    latchFault();
    return;
  }
  Serial.println("DC motor on Grove TB6612FNG channel A, I2C 0x14.");
  Serial.println("Wiring: white SDA -> D21, yellow SCL -> D22; common GND.");
  Serial.println("1 = forward, 2 = reverse (10 seconds at PWM 255/255); s = stop.");
  Serial.println("User LED on D2 lights during a commanded run; it does not measure motor movement.");
}

void loop() {
  if (Serial.available() && Serial.peek() == 'i') {
    Serial.read();
    if (running && !stopOutputs()) latchFault();
    scanBus();
  }
  if (fault) {
    stopOutputs(); // Best effort only; a broken bus cannot guarantee a stop.
    delay(20);
    return;
  }
  if (running && millis() - startedAt >= RUN_MS) {
    if (!stopOutputs()) latchFault();
    else Serial.println("Timed run complete; outputs stopped and in standby.");
  }
  if (!ready || !Serial.available()) return;
  const char input = Serial.read();
  if (input == 's') {
    if (!stopOutputs()) latchFault();
    else Serial.println("Outputs stopped and in standby.");
  } else if (input == '1' || input == '2') {
    if (running || (cooling && millis() - stoppedAt < COOLDOWN_MS)) {
      Serial.println("Stop and wait one second before another run.");
    } else {
      setActivityLed(true);
      const bool awake = command(GROVE_MOTOR_DRIVER_I2C_CMD_NOT_STANDBY, 0);
      const uint8_t direction = input == '1' ? GROVE_MOTOR_DRIVER_I2C_CMD_CW
                                            : GROVE_MOTOR_DRIVER_I2C_CMD_CCW;
      if (awake && command(direction, MOTOR_CHA, true)) {
        running = true;
        startedAt = millis();
        Serial.println(input == '1' ? "Forward command sent." : "Reverse command sent.");
      } else {
        stopOutputs();
        latchFault();
      }
    }
  }
  // Prevent pasted/queued commands from causing repeated starts.
  while (Serial.available()) Serial.read();
}
