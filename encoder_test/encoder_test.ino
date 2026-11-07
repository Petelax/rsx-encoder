#define ENCODER_PIN 8

void setup() {
  Serial.begin(9600);
  pinMode(ENCODER_PIN, INPUT);

}

void loop() {
  unsigned long pulse = pulseIn(ENCODER_PIN, HIGH, 10000);
  float degrees = (pulse - 4.0f) * (360.0f / 996.0f);

  //Serial.print("pulse: ");
  //Serial.print(pulse);
  //Serial.print(", angle: ");
  Serial.println(degrees);
  delay(50);
}
