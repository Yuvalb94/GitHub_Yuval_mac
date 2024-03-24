
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
int setpoint = 0;
int sensitivity = 15;
long temp = 0;
int cnt = 0;
int setpointChangeCount = 100;
int newAvgValue = 0;
int posSensitivity = 5;

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);
  myservo.attach(9);  // attaches the servo on pin 9 to the servo object

  // setpoint setup
  Serial.println("Initial setpoint determined...");
  for (int i = 0; i < 100; ++i) {
    sensorValue = analogRead(analogInPin);
    temp = temp + sensorValue;
  }
  delay(500);
  setpoint = temp/100;
  temp = 0;
  Serial.print("Initial setpoint: ");
  Serial.println(setpoint);
  delay(1000);
  // end setpoint setup code

}
void loop() {
  unsigned long startTime = micros(); // Record start time
  
  // Perform sensor reading
  int sensorValue = analogRead(A0); // Replace A0 with your sensor pin
  // Additional processing if needed
  
  unsigned long endTime = micros(); // Record end time
  
  // Calculate elapsed time
  unsigned long elapsedTime = endTime - startTime;
  
  // Calculate sampling rate
  float samplingRate = 1000000.0 / elapsedTime; // Convert microseconds to seconds
  
  // Print sampling rate
  Serial.print("Sampling Rate (samples per second): ");
  Serial.println(samplingRate);
  
  delay(1000); // Delay for visualization, adjust as needed

  // sensorValue = analogRead(analogInPin);

  // Serial.print("sensor = " );
  // Serial.print(sensorValue);
  // Serial.print(" ,pos = " );
  // Serial.print(pos);

  // pos = pos + posSensitivity;
  // // myservo.write(pos);

  // if (pos > 360) {
  //   pos = 90;
  //   posSensitivity = -posSensitivity;
  //   delay(500);
  // }
  // if (pos < -270) {
  //   pos = 90;
  //   posSensitivity = -posSensitivity;
  //   delay(500);
  // }
//   if(sensorValue> setpoint+sensitivity){
//       pos = pos+posSensitivity;
//   myservo.write(pos);
//   delay(150);
// }

//   if(sensorValue < setpoint-sensitivity){
//     pos = pos-posSensitivity;
//     myservo.write(pos);
//     delay(150);
// }

// if(sensorValue > setpoint-sensitivity && sensorValue < setpoint+sensitivity){
// pos = 90;
//    myservo.write(pos);
// }

// if(sensorValue < setpoint-sensitivity || sensorValue > setpoint+sensitivity){
//     temp = temp + sensorValue;
//     cnt = cnt + 1;
//     if(cnt > setpointChangeCount){
//       newAvgValue = temp / cnt;
//       setpoint = newAvgValue;
//       temp = 0;
//       cnt = 0;
//     }
//  } else{
//     temp = 0;
//     cnt = 0;
//     //setpoint = initialSetpoint;
// }
  // Serial.print(", cnt = ");
  // Serial.print(cnt);

  // Serial.print(", setpoint = ");
  // Serial.println(setpoint);

// // if(pos > 135 || pos < 45){
// //   pos = 90;
// //   delay(50);
// //   myservo.write(pos);
// // }

  // delay(500);
}
