//Payloadpal motor controller firmware
#include <stdlib.h>
#include <LiquidCrystal.h>
#include <Servo.h>


//Servo pins:
#define servo_L_pin 45
#define servo_R_pin 46
Servo servoLeft;
Servo servoRight;

//Motor controller pins:
#define IN1 29         // Motor A direction 1 (GPIO32)
#define IN2 28         // Motor A direction 2 (GPIO33)
#define IN3 27         // Motor B direction 1 (GPIO19)
#define IN4 26         // Motor B direction 2 (GPIO23)
#define ENA 2         // Motor A PWM speed (GPIO14)
#define ENB 3         // Motor B PWM speed (GPIO12)

//Debug LCD pins
#define RS  8
#define EN  9
#define d4  4
#define d5  5
#define d6  6
#define d7  7
#define pin_BL = 10
LiquidCrystal lcd( RS,  EN,  d4,  d5,  d6,  d7);

int angle = 90;
int baseSpeed = 180;

//input angle 0 to 360 degrees
void ackermanSteer(int angle){
    float steering = 0.0;
    if (angle <= 180)
    {
        steering = 1.0 - (angle/90.0);
    }else
    {
        steering = ((angle - 180.0)/90.0) - 1.0;
    }
    steering = constrain(steering, -1.0, 1.0);
    float innerOffset = 30.0 * steering;
    float outerOffset = 30* steering * 0.8;
    float leftServoAngle;
    float rightServoAngle;
    if (steering > 0)
    {
        leftServoAngle = 90.0 - outerOffset;
        rightServoAngle = 90 + innerOffset;
    }else
    {
        leftServoAngle = 90.0 - innerOffset;
        rightServoAngle = 90.0 + outerOffset;
    }
    
    servoLeft.write(leftServoAngle);
    servoRight.write(rightServoAngle);

    if (angle == 90)
    {
        moveForward(baseSpeed);
    }else if (angle == 269)
    {
        //stopMotors();
        turnAround_left();
        moveForward(baseSpeed);

    }else if (angle == 271)
    {
        //stopMotors();
        turnAround_right();
        moveForward(baseSpeed);
    }else if (angle == 180)
    {
        turnLeft();
    }else if (angle == 0 || angle == 360)
    {
        turnRight();
    }else
    {
        stopMotors();
    }
    lcd.print()    
}
void stopMotors(){
    analogWrite(ENA, 0);
    analogWrite(ENB, 0);
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
}
void moveForward(int speedVal){
    analogWrite(ENA, speedVal);
    analogWrite(ENB, speedVal);
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
}
void moveBackward(int speedVal){
    analogWrite(ENA, speedVal);
    analogWrite(ENB, speedVal);
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
}
void turnLeft(){
    analogWrite(ENA, (baseSpeed));
    analogWrite(ENB, baseSpeed);
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN3, LOW);
}
void turnRight(){
    analogWrite(ENA, baseSpeed);
    analogWrite(ENB, baseSpeed);
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN3, HIGH);
}
void turnAround_left(){
    turnLeft();
    delay(1200);
    stopMotors();
}
void turnAround_right(){
    turnRight();
    delay(1200);
    stopMotors();
}

void setup(){
    Serial.begin(115200); //debug port initialize
    servoLeft.attach(servo_L_pin);
    servoRight.attach(servo_R_pin);
    pinMode(ENA, OUTPUT);
    pinMode(ENB, OUTPUT);
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);
    lcd.begin(16, 2);
    lcd.print("INIT_COMPLETE");
    servoLeft.write(0);
    servoRight.write(0);

}
void loop(){
    if (Serial.available()){
        angle = Serial.parseInt();
        if (angle >= 0 && angle <=360)
        {
            ackermanSteer(angle);

        }
        
    }
        
}

