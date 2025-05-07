int ENB = 10;
int motor2pin1 = 8;
int motor2pin2 = 9;

void setup() {
  pinMode(ENB, OUTPUT);
  pinMode(motor2pin1, OUTPUT);
  pinMode(motor2pin2, OUTPUT);
Serial.begin(9600);
  digitalWrite(motor2pin1, HIGH);  // direction
  digitalWrite(motor2pin2, LOW);

  // gradually ramp up speed
  for (int i = 45; i <= 255; i++) {
    analogWrite(ENB, i);
    Serial.println(i);
    delay(20);
  }
  delay(1000);

    for (int g = 255; g >28; g--) {
    analogWrite(ENB, g);
    Serial.println(g);
    delay(20);
  }
}
void loop() {};
