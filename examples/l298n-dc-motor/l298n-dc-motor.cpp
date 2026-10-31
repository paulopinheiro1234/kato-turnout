#include <Arduino.h>

// L298N module, channel A. Remove the ENA jumper before connecting GPIO25.
constexpr uint8_t ENA_PIN = 25;
constexpr uint8_t IN1_PIN = 26;
constexpr uint8_t IN2_PIN = 27;
constexpr uint8_t ACTIVITY_LED_PIN = 2;
constexpr uint32_t RUN_MS = 10000;
constexpr uint32_t COOLDOWN_MS = 1000;
bool running = false;
uint32_t startedAt = 0;
uint32_t stoppedAt = 0;

void stopMotor() {
  digitalWrite(ENA_PIN, LOW); // Disable bridge: coast stop.
  digitalWrite(IN1_PIN, LOW);
  digitalWrite(IN2_PIN, LOW);
  digitalWrite(ACTIVITY_LED_PIN, LOW);
  running = false;
  stoppedAt = millis();
}

void setup() {
  digitalWrite(ENA_PIN, LOW);
  pinMode(ENA_PIN, OUTPUT);
  digitalWrite(IN1_PIN, LOW);
  digitalWrite(IN2_PIN, LOW);
  pinMode(IN1_PIN, OUTPUT);
  pinMode(IN2_PIN, OUTPUT);
  pinMode(ACTIVITY_LED_PIN, OUTPUT);
  stopMotor();
  Serial.begin(115200);
  Serial.println("L298N: ENA D25, IN1 D26, IN2 D27; motor OUT1/OUT2.");
  Serial.println("1 = forward, 2 = reverse (10 seconds, full power); s = coast stop.");
}

void loop() {
  if (running && millis() - startedAt >= RUN_MS) {
    stopMotor();
    Serial.println("Timed run complete; ENA disabled.");
  }
  if (!Serial.available()) return;
  const char input = Serial.read();
  if (input == 's') {
    stopMotor();
    Serial.println("ENA disabled; motor coasting.");
  } else if (input == '1' || input == '2') {
    if (running || millis() - stoppedAt < COOLDOWN_MS) {
      Serial.println("Stop and wait one second before another run.");
    } else {
      digitalWrite(ENA_PIN, LOW);
      digitalWrite(IN1_PIN, input == '1' ? HIGH : LOW);
      digitalWrite(IN2_PIN, input == '1' ? LOW : HIGH);
      digitalWrite(ENA_PIN, HIGH);
      digitalWrite(ACTIVITY_LED_PIN, HIGH);
      running = true;
      startedAt = millis();
      Serial.println(input == '1' ? "Forward GPIO levels set." : "Reverse GPIO levels set.");
    }
  }
  while (Serial.available()) Serial.read();
}
