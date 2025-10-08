#include <Servo.h>

// --- Pins ---
const uint8_t SERVO_PIN = 11;
const uint8_t TRIG_PIN  = 9;
const uint8_t ECHO_PIN  = 10;

// --- Sweep params ---
const int MIN_ANGLE = 5;
const int MAX_ANGLE = 175;
const int STEP_DEG  = 1;

// --- Timing (tune for smoothness) ---
const uint16_t MOVE_INTERVAL_MS = 10;   // servo speed
const uint16_t PING_INTERVAL_MS = 25;   // how often to take a reading

// --- Ultrasonic ---
const uint32_t ECHO_TIMEOUT_US = 6000UL;  // ~1 m max; raise if you need more

Servo myServo;
int    angle        = 90;
int    dir          = +1;                 // +1 forward, -1 back
uint32_t lastMoveMs = 0;
uint32_t lastPingMs = 0;

float readDistanceCM_fast() {
  // trigger 10 µs pulse
  digitalWrite(TRIG_PIN, LOW);  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  // bounded blocking
  unsigned long dur = pulseIn(ECHO_PIN, HIGH, ECHO_TIMEOUT_US);
  if (!dur) return NAN;
  return (dur * 0.0343f) / 2.0f;
}

void setup() {
  Serial.begin(115200);                    // faster serial to avoid print lag
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  myServo.attach(SERVO_PIN);
  myServo.write(angle);

  Serial.println("angle_deg,distance_cm");
}

void loop() {
  const uint32_t now = millis();

  // 1) Move servo at a steady cadence (non-blocking)
  if (now - lastMoveMs >= MOVE_INTERVAL_MS) {
    lastMoveMs = now;

    angle += dir * STEP_DEG;
    if (angle >= MAX_ANGLE) { angle = MAX_ANGLE; dir = -1; }
    if (angle <= MIN_ANGLE) { angle = MIN_ANGLE; dir = +1; }

    myServo.write(angle);
  }

  // 2) Take ultrasonic reading occasionally (non-blocking cadence)
  if (now - lastPingMs >= PING_INTERVAL_MS) {
    lastPingMs = now;

    float d = readDistanceCM_fast();
    if (!isnan(d) && (d < 2.0f || d > 400.0f)) d = NAN;   // sanity clamp

    // print once per ping (lightweight)
    Serial.print(angle);
    Serial.print(",");
    if (isnan(d)) Serial.println("");
    else          Serial.println(d, 2);
  }
}
