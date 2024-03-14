/***************************************************************************
* This is the main code for controlling our Arduino.
* It has two functionalities -
*   1. Turn the lights on/off, based on an input from the serial port.
*       Data from the serial port arrives as an int, and so we set a
*       threshold to decide whether the light goes on or off.
*
*   2. Read data from temp. and humidity sensors, as well as light and weight
*     sensors, and print int to the serial port (via Serial.print()).
*     This data will later be read by our Raspberry Pi device, whenever
*     we're ready to consume the data.
***************************************************************************/
#include <dht.h>
#include <Wire.h>
#include <EEPROM.h> //Needed to record user settings
#include "SparkFun_Qwiic_Scale_NAU7802_Arduino_Library.h"

#define DHT22_PIN 8
NAU7802 myScale;
dht DHT;

//EEPROM locations to store 4-byte variables
#define LOCATION_CALIBRATION_FACTOR 0 //Float, requires 4 bytes of EEPROM
#define LOCATION_ZERO_OFFSET 10 //Must be more than 4 away from previous spot. Long, requires 4 bytes of EEPROM
bool settingsDetected = false; //Used to prompt user to calibrate their scale

//Create an array to take average of weights. This helps smooth out jitter.
#define AVG_SIZE 4
float avgWeights[AVG_SIZE];
byte avgWeightSpot = 0;

int LIGHT_SWITCH_PIN = 13;
int BYTES_THRESHOLD = 100;
int incomingByte = 0;

int ldrPin = A0;
unsigned long seconds = 1000L;
unsigned long minutes = seconds * 1;
unsigned long DelayRate = minutes;

void setup() {
  pinMode(LIGHT_SWITCH_PIN, OUTPUT);
  pinMode(ldrPin, INPUT);
  Wire.begin();
  myScale.begin();

  Serial.begin(9600);

  //myScale setup and parameters acquisition
  if (myScale.begin() == false)
  {
    Serial.println("Scale not detected. Please check wiring. Freezing...");
    delay(1000);
  }
  Serial.println("Scale detected!");

  myScale.setSampleRate(NAU7802_SPS_320); //Increase to max sample rate
  myScale.calibrateAFE(); //Re-cal analog front end when we change gain, sample rate, or channel 

  readSystemSettings(); //Load zeroOffset and calibrationFactor from EEPROM

  Serial.print("Zero offset: ");
  Serial.println(myScale.getZeroOffset());
  Serial.print("Calibration factor: ");
  Serial.println(myScale.getCalibrationFactor());
}

void loop() {

  if(settingsDetected == false)
    {
      Serial.print("\tScale not calibrated. Press 'c'.");
    }
    
  // This code chunk allows the user to TARE or CALIBRATE the scale while the script is running.
  // if (Serial.available())
  // {
  //   byte incoming = Serial.read();

  //   if (incoming == 't') //Tare the scale
  //     myScale.calculateZeroOffset();
  //   else if (incoming == 'c') //Calibrate
  //   {
  //     calibrateScale();
  //   }
  // }

  //This part is for DHT data acquisition and printing out of all the data
  float currentScaleReading = myScale.getWeight();
  int LDRinput = analogRead(ldrPin);
  int chk = DHT.read22(DHT22_PIN);

  switch (chk) {
    case DHTLIB_OK:
      break;
    case DHTLIB_ERROR_CHECKSUM:
      Serial.print(0);
      break;
    case DHTLIB_ERROR_TIMEOUT:
      Serial.print(0);
      break;
    default:
      Serial.print(0);
      break;
  }


  Serial.print(DHT.humidity);
  Serial.print(";");
  Serial.print(DHT.temperature);
  Serial.print(";");
  Serial.print(LDRinput);
  Serial.print(";");
  Serial.print(currentScaleReading, 2);
  Serial.println("");



  if (Serial.available() > 0) {
    incomingByte = Serial.read();
    if (incomingByte > BYTES_THRESHOLD) {
      digitalWrite(LIGHT_SWITCH_PIN, HIGH);
    } 
    if (incomingByte < BYTES_THRESHOLD) {
      digitalWrite(LIGHT_SWITCH_PIN, LOW); 
    }

    if (incomingByte == 't') //Tare the scale
      myScale.calculateZeroOffset();
    else if (incomingByte == 'c') //Calibrate
    {
      calibrateScale();
    }
  }
  
  delay(DelayRate);
}


//Weight sensor Calibration and EEPROM(non-vlatile memory of arduino) stuff

//Gives user the ability to set a known weight on the scale and calculate a calibration factor
void calibrateScale(void)
{
  Serial.println();
  Serial.println();
  Serial.println(F("Scale calibration"));

  Serial.println(F("Setup scale with no weight on it. Press a key when ready."));
  while (Serial.available()) Serial.read(); //Clear anything in RX buffer
  while (Serial.available() == 0) delay(10); //Wait for user to press key

  myScale.calculateZeroOffset(64); //Zero or Tare the scale. Average over 64 readings.
  Serial.print(F("New zero offset: "));
  Serial.println(myScale.getZeroOffset());

  Serial.println(F("Place known weight on scale. Press a key when weight is in place and stable."));
  while (Serial.available()) Serial.read(); //Clear anything in RX buffer
  while (Serial.available() == 0) delay(10); //Wait for user to press key

  Serial.print(F("Please enter the weight, without units, currently sitting on the scale (for example '4.25'): "));
  while (Serial.available()) Serial.read(); //Clear anything in RX buffer
  while (Serial.available() == 0) delay(10); //Wait for user to press key

  //Read user input
  float weightOnScale = Serial.parseFloat();
  Serial.println();

  myScale.calculateCalibrationFactor(weightOnScale, 64); //Tell the library how much weight is currently on it and set the new calibration factor
  Serial.print(F("New cal factor: "));
  Serial.println(myScale.getCalibrationFactor(), 2);

  Serial.print(F("New Scale Reading: "));
  Serial.println(myScale.getWeight(), 2);

  recordSystemSettings(); //Commit these values to EEPROM
}

//Record the current system settings to EEPROM
void recordSystemSettings(void)
{
  //Get various values from the library and commit them to NVM
  EEPROM.put(LOCATION_CALIBRATION_FACTOR, myScale.getCalibrationFactor());
  EEPROM.put(LOCATION_ZERO_OFFSET, myScale.getZeroOffset());
}

//Reads the current system settings from EEPROM
//If anything looks weird, reset setting to default value

void readSystemSettings(void) //REQUIRED to read an already-calibrated system settings (zero offset and calibration factor)
{
  float settingCalibrationFactor; //Value used to convert the load cell reading to lbs or kg
  long settingZeroOffset; //Zero value that is found when scale is tared

  //Look up the calibration factor
  EEPROM.get(LOCATION_CALIBRATION_FACTOR, settingCalibrationFactor);
  if (settingCalibrationFactor == 0xFFFFFFFF)
  {
    settingCalibrationFactor = 0; //Default to 0
    EEPROM.put(LOCATION_CALIBRATION_FACTOR, settingCalibrationFactor);
  }

  //Look up the zero tare point
  EEPROM.get(LOCATION_ZERO_OFFSET, settingZeroOffset);
  if (settingZeroOffset == 0xFFFFFFFF)
  {
    settingZeroOffset = 1000L; //Default to 1000 so we don't get inf
    EEPROM.put(LOCATION_ZERO_OFFSET, settingZeroOffset);
  }

  //Pass these values to the library
  myScale.setCalibrationFactor(settingCalibrationFactor);
  myScale.setZeroOffset(settingZeroOffset);

  settingsDetected = true; //Assume for the moment that there are good cal values
  if (settingCalibrationFactor < 0.1 || settingZeroOffset == 1000)
    settingsDetected = false; //Defaults detected. Prompt user to cal scale.
}
