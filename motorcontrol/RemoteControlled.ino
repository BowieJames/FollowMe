#include <Servo.h>

Servo myServo;

// Joystick pins
const int joystickX = A0;
const int joystickY = A1;

// Servo
int currentAngle = 90;

// Motor driver pins
const int ENA = 9;
const int ENB = 10;
const int IN1 = 4;
const int IN2 = 5;
const int IN3 = 6;
const int IN4 = 7;

void setup() {
  myServo.attach(11);  
  myServo.write(currentAngle);

  // Motor pins
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
}

void loop() {
  // --- Steering ---
  int joystickXVal = analogRead(joystickX);
  int targetAngle = map(joystickXVal, 0, 1023, 0, 180);

  if (currentAngle < targetAngle) {
    currentAngle++;
    myServo.write(currentAngle);
    delay(10);
  } else if (currentAngle > targetAngle) {
    currentAngle--;
    myServo.write(currentAngle);
    delay(10);
  }

  // --- Driving ---
  int joystickYVal = analogRead(joystickY);
  int deadzone = 50;

  int speed = 0;
  if (abs(joystickYVal - 512) > deadzone) {
    if (joystickYVal > 512) {
      // Forward
      speed = map(joystickYVal, 513, 1023, 45, 255);
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, LOW);
    } else {
      // Reverse
speed = map(joystickYVal, 0, 511, 255, 0);
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, HIGH);
    }
  } else { 
    // Stop
    speed = 0;   
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
  }

  analogWrite(ENA, speed);
  analogWrite(ENB, speed);
}
