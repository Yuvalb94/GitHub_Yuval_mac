
//  Active Commutator
// 01.13.15
// WALIII

// Hall sensor controlled active commutator, driven by a servo motor
// Inspired by :
//
//
#include <Servo.h>

Servo myservo;

int pos = 90;
const int analogInPin = A0;
int sensorValue = 0;        // value read from the pot
//int initialSetpoint = 510;
int setpoint = 510;
int sensitivity = 12;
int temp = 0;
int cnt = 0;
int newAvgValue = 0;

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);
  myservo.attach(9);  // attaches the servo on pin 9 to the servo object
}
void loop() {


  sensorValue = analogRead(analogInPin);

  Serial.print("sensor = " );
  Serial.print(sensorValue);
  Serial.print(" ,pos = " );
  Serial.print(pos);

  if(sensorValue> setpoint+sensitivity){
      pos = pos+5;
  myservo.write(pos);
  delay(15);
}

  if(sensorValue < setpoint-sensitivity){
    pos = pos-5;
    myservo.write(pos);
    delay(15);
}

if(sensorValue > setpoint-sensitivity && sensorValue < setpoint+sensitivity){
pos = 90;
   myservo.write(pos);
}

if(sensorValue < setpoint-10 || sensorValue > setpoint+10){
    temp = temp + sensorValue;
    cnt = cnt + 1;
    if(cnt > 50){
      newAvgValue = temp / cnt;
      setpoint = newAvgValue;
    }
 } else{
    temp = 0;
    cnt = 0;
    //setpoint = initialSetpoint;
}
  Serial.print(", setpoint = ");
  Serial.println(setpoint);

if(pos > 180 || pos < 0){
  pos = 90;
  delay(50);
  myservo.write(pos);
}

  delay(50);
}
