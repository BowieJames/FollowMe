#include <Servo.h>

Servo servo1;
Servo servo2;

int currentAngle = 90; // middle position
int currentspeed = 0;

// Motor driver pins
const int ENA = 9;
const int ENB = 10;
const int IN1 = 4;
const int IN2 = 5;
const int IN3 = 6;
const int IN4 = 7;

void setup() {
  servo1.attach(11);
  servo2.attach(3);
  servo1.write(currentAngle);
  servo2.write(currentAngle);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
}

void driveline(int targetspeed, int direction) {
  // Steering
  if (currentAngle < direction) {
    currentAngle++;
    servo1.write(currentAngle);
    servo2.write(currentAngle);
    delay(5);
  } else if (currentAngle > direction) {
    currentAngle--;
    servo1.write(currentAngle);
    servo2.write(currentAngle);
    delay(5);
  }

  // Motor Direction
  if (targetspeed > 0) {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
  } else if (targetspeed < 0) {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
  } else {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
  }

  // Speed Ramp
  
  if (currentspeed < targetspeed) {
    currentspeed += 1;
    if (currentspeed > targetspeed) currentspeed = targetspeed;
  } else if (currentspeed > targetspeed) {
    currentspeed -= 1;
    if (currentspeed < targetspeed) currentspeed = targetspeed;
  }

  analogWrite(ENA, abs(currentspeed));
  analogWrite(ENB, abs(currentspeed));
}

void loop() {
  driveline(255, 60);
  delay(20); // Run every 20ms (adjust as needed)

}
