#include <Arduino.h>
#include <esp_arduino_version.h>

// Standard positional hobby servo: 50 Hz, 1000–2000 us pulse range.
constexpr uint8_t SERVO_PIN = 18;
constexpr uint32_t SERVO_HZ = 50;
constexpr uint8_t PWM_BITS = 16;
constexpr uint8_t PWM_CHANNEL = 0;
constexpr uint16_t POSITION_A = 60;
constexpr uint16_t POSITION_B = 120;
bool ready = false;

void moveServo(uint16_t angle) {
  const uint32_t pulseUs = 1000UL + (1000UL * angle / 180UL);
  const uint32_t duty = pulseUs * ((1UL << PWM_BITS) - 1) / 20000UL;
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWrite(SERVO_PIN, duty);
#else
  ledcWrite(PWM_CHANNEL, duty);
#endif
  Serial.printf("Servo: %u degrees (%lu us).\n", angle, static_cast<unsigned long>(pulseUs));
}

void setup() {
  Serial.begin(115200);
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ready = ledcAttach(SERVO_PIN, SERVO_HZ, PWM_BITS);
#else
  ready = ledcSetup(PWM_CHANNEL, SERVO_HZ, PWM_BITS) > 0;
  if (ready) ledcAttachPin(SERVO_PIN, PWM_CHANNEL);
#endif
  if (!ready) {
    Serial.println("Servo PWM setup failed. Reset to retry.");
    return;
  }
  moveServo(90);
  Serial.println("Servo on GPIO18. 1 = 60 degrees, 2 = 120 degrees, c = centre, s = release signal.");
}

void loop() {
  if (!ready || !Serial.available()) return;
  switch (Serial.read()) {
    case '1': moveServo(POSITION_A); break;
    case '2': moveServo(POSITION_B); break;
    case 'c': moveServo(90); break;
    case 's':
      // Remove pulses; this does not disconnect servo power.
#if ESP_ARDUINO_VERSION_MAJOR >= 3
      ledcWrite(SERVO_PIN, 0);
#else
      ledcWrite(PWM_CHANNEL, 0);
#endif
      Serial.println("Servo signal released. 1, 2 or c resumes control.");
      break;
    default: break;
  }
}
