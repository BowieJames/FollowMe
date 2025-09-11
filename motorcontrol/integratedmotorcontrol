#include <Servo.h>
#include <Wire.h>

#define I2C_SLAVE_ADDR 0x08

Servo servo1;
Servo servo2;

int currentAngle = 90; // Middle position
int targetspeed;
float angle;     
float distance;  

// Motor driver pins
const int ENA = 9;
const int ENB = 10;
const int IN1 = 4;
const int IN2 = 5;
const int IN3 = 6;
const int IN4 = 7;
const float TOLERANCE = 2.0; // Deadband for servo angle (degrees)

void setup() {
  Wire.begin();
  Serial.begin(9600);   
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
  // Steering with deadband
  if (abs(currentAngle - direction) > TOLERANCE) {
    currentAngle = direction; // Update currentAngle
    servo1.write(direction);
    servo2.write(direction);
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

  // Set speed directly
  analogWrite(ENA, abs(targetspeed));
  analogWrite(ENB, abs(targetspeed));
}

int distcompute(float distance) {
  int speed;
  if (distance <= 1) {
    speed = 0;
  } else if (distance <= 5) {
    speed = 100;
  } else {
    speed = 255;
  }
  return speed;
}

int request(void) {
  Wire.requestFrom(I2C_SLAVE_ADDR, sizeof(float) * 2); // request 8 bytes

  if (Wire.available() >= 8) {
    Wire.readBytes((char*)&angle, sizeof(float));    
    Wire.readBytes((char*)&distance, sizeof(float)); 
    Serial.print("Received -> Angle: ");
    Serial.print(angle, 2);
    Serial.print("  Distance: ");
    Serial.println(distance, 2);
    return 1; // success return
  } else {
    return 0; // fail return
  }
}

void loop() {
  // Request new data
  if (request()) {
    targetspeed = distcompute(distance);
    driveline(targetspeed, (int)angle);  // Use received values
  }

  delay(100); // Run every 100ms
}
