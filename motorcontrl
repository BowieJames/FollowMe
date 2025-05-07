/* Encoder.txt by Nathan   
does one full rotation - chat code adjusted for our values 
*/

// Motor control pins
const int ENB = 10;
const int motor2pin1 = 8;
const int motor2pin2 = 9;

// Encoder pins
const byte encoderPinA = 2;
const byte encoderPinB = 4;

volatile long tickCount = 0;
const long TICKS_PER_ROTATION = 700;
const long BRAKE_EARLY_TICK = 587;  // Stop early to allow for braking
bool rotationDone = false;

int motorSpeed = 45;

void setup() {
  pinMode(ENB, OUTPUT);
  pinMode(motor2pin1, OUTPUT);
  pinMode(motor2pin2, OUTPUT);

  pinMode(encoderPinA, INPUT);
  pinMode(encoderPinB, INPUT);
  attachInterrupt(digitalPinToInterrupt(encoderPinA), handleEncoder, RISING);

  Serial.begin(9600);

  digitalWrite(motor2pin1, HIGH);
  digitalWrite(motor2pin2, LOW);
  analogWrite(ENB, motorSpeed);
}

void loop() {
  if (!rotationDone) {
    Serial.print("Ticks: ");
    Serial.println(tickCount);

    // Slow down between 600–642
    if (tickCount >= 500 && tickCount < BRAKE_EARLY_TICK) {
      int slowSpeed = map(tickCount, 500, BRAKE_EARLY_TICK, 45, 28);
      slowSpeed = constrain(slowSpeed, 28, 45);
      analogWrite(ENB, slowSpeed);
      Serial.print("Slowing to: ");
      Serial.println(slowSpeed);
    }

    delay(50);
  } else {
    stopMotorWithBraking();
    Serial.println("Stopped early to compensate for overshoot.");
    detachInterrupt(digitalPinToInterrupt(encoderPinA));
    while (1);
  }
}

void handleEncoder() {
  tickCount++;
  if (tickCount >= BRAKE_EARLY_TICK) {
    rotationDone = true;
  }
}

void stopMotorWithBraking() {
  digitalWrite(motor2pin1, LOW);
  digitalWrite(motor2pin2, LOW);
  analogWrite(ENB, 255);  // Active brake
  delay(300);             // Hold brake
  analogWrite(ENB, 0);    // Fully stop
}
