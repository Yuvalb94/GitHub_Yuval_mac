

// EEPROM explained:
// EEPROM is a sort of internal storage of the arduino (non-volatile memory).
// It has 1024 bytes (in UNO) for storage and can endure up to 100,000 write/erase instances.
// For the calibration of the scale we use this storage, in order to store the calibration factor and zero offset of each scale.
// here are some modules and their meaning:
// LOCATION_CALIBRATION_FACTOR(port) - used to define the location of each port's calibration factor
// LOCATION_ZERO_OFFSET(port) - used to define the location of each port's zero offset
//    The locations are based on the storage requirements of the data with extra space taken for future alterations, total 20 bytes:
//      - Each port takes the first 20 spots after the 20*portNumber
//      - First 4 bytes for float (calibration factor)
//      - 4 bytes extra space
//      - 4 bytes for long (zero offset)
//      - 8 bytes extra space
// EEPROM.get(adress, variable) - receives the data stored in (adress) to (variabe)
// EEPROM.put(adress, data) - sets a new value (data) to the defined adress (adress)
// myScale.calculateCalibrationFactor()
// myScale.calculateZeroOffset()
// myScale.getCalibrationFactor()
// myScale.getZeroOffset()
// myScale.setCalibrationFactor()
// myScale.setZeroOffset()

#include <Wire.h>
#include <SparkFun_I2C_Mux_Arduino_Library.h>
#include <SparkFun_Qwiic_Scale_NAU7802_Arduino_Library.h>
#include <EEPROM.h> //Needed to record user settings

QWIICMUX myMux;
NAU7802 myScale;

#define LOCATION_CALIBRATION_FACTOR(port) (port * 20) // Each scale requires 20 bytes. Float, requires 4 bytes
#define LOCATION_ZERO_OFFSET(port) (LOCATION_CALIBRATION_FACTOR(port) + 8) // //Must be more than 4 away from previous spot. Long, requires 4 bytes of EEPROM
bool settingsDetected = false; //Used to prompt user to calibrate their scale

const int numScales = 8; // Number of Qwiic Scale units connected
int activeScales[numScales]; //define an array of (numScales) capacity, each value of type int
int numActiveScales = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  myMux.begin();
  if (!myMux.begin()) {
    Serial.println("Mux not detected. Freezing...");
    while (1);
  }
  Serial.println("Mux detected");

  for (int i = 0; i < numScales; ++i) {
    myMux.enablePort(i);
    myScale.begin();
    delay(1000);

    if (!myScale.begin()) {
      Serial.println("Scale not detected on port " + String(i));
    } 
    else {
      Serial.println("Scale detected on port " + String(i));
      activeScales[numActiveScales++] = i; //append the index (i) of the active scale to the array, and increase the numActiveScales by 1.

      myScale.setSampleRate(NAU7802_SPS_320); //Increase to max sample rate
      myScale.calibrateAFE(); //Re-cal analog front end when we change gain, sample rate, or channel 

      readSystemSettings(i); //Load zeroOffset and calibrationFactor from EEPROM

      Serial.print("Zero offset: ");
      Serial.println(myScale.getZeroOffset());
      Serial.print("Calibration factor: ");
      Serial.println(myScale.getCalibrationFactor());
    }
    myMux.disablePort(i);
  }

  // myMux.setPort(0); // In the end of the for loop, return to the first port number (0).
}

void loop() {
  for (int i = 0; i < numActiveScales; ++i) {
    int scalePort = activeScales[i];
    // myMux.setPort(scalePort);
    myMux.enablePort(scalePort);
    
    if (myScale.available() == true) {
      
      readSystemSettings(scalePort);
      if(settingsDetected == false)
      {
        Serial.print("\tScale ");
        Serial.print(scalePort);
        Serial.print(" not calibrated. Press 'c'...");
        delay(5000);
        // while(1); //waiting for user response

      }
      float weight = myScale.getWeight();
      Serial.print("Scale " + String(scalePort));
      Serial.print(" reading: ");
      Serial.print(weight, 2);
      Serial.print(".\t");

    
    } else {
      Serial.print("Scale " + String(scalePort));
      Serial.println(" is not available at the moment");
    }
    myMux.disablePort(scalePort);
    delay(10);
  
  // Check for tare or calibration requests from the user
  if (Serial.available())
    {
    byte incoming = Serial.read();

    if (incoming == 't') //Tare the scale
      myScale.calculateZeroOffset();
    else if (incoming == 'c') //Calibrate
      {
      calibrateScale();
      }
    } 
  }
  Serial.println(" ");
  myMux.setPort(0);
  delay(100);
}

//This function checks if a given port number is within the active scales array
//It is used inside the calibrateScale function.
bool isActiveScale(int port) {
  for (int i = 0; i < numActiveScales; ++i) {
    if (activeScales[i] == port) {
      return true; // The port is an active scale
    }
  }
  return false; // The port is not an active scale
}
//Weight sensor Calibration and EEPROM(non-vlatile memory of arduino) stuff

//Gives user the ability to set a known weight on the scale and calculate a calibration factor
void calibrateScale()
{
  int selectedPort = -1; // Initialize selectedPort with an invalid value
  
  while (selectedPort < 0 || selectedPort >= numScales || !isActiveScale(selectedPort)) {
    Serial.println("Enter scale port number to calibrate (0-7): ");
    while (Serial.available()) Serial.read(); //Clear anything in RX buffer
    while (Serial.available() == 0) delay(10); //Wait for user to press key
    selectedPort = Serial.parseInt();
    
    // Check if the input is within the valid range and corresponds to an active scale
    if (selectedPort < 0 || selectedPort >= numScales || !isActiveScale(selectedPort)) {
      Serial.println("Invalid scale port number. Please enter a number between 0 and 7 for an active scale.");
    }
  }

  // at this point, selectedPort is a valid port number. Proceeding with calibration process
  myMux.setPort(selectedPort);
  Serial.println();
  Serial.println("Starting calibration for scale on port " + String(selectedPort));
  
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

  myScale.calculateCalibrationFactor(weightOnScale, 64); //Tell the library how much weight is currently on it
  Serial.print("Scale " + selectedPort);
  Serial.print(" Calibrated. New cal factor: ");
  Serial.println(myScale.getCalibrationFactor(), 2);

  Serial.print(F("New Scale Reading: "));
  Serial.println(myScale.getWeight(), 2);

  recordSystemSettings(selectedPort); //Commit these values to EEPROM

}

//Record the current system settings to EEPROM
void recordSystemSettings(int scalePort)
{
  //Get various values from the library and commit them to NVM
  EEPROM.put(LOCATION_CALIBRATION_FACTOR(scalePort), myScale.getCalibrationFactor());
  EEPROM.put(LOCATION_ZERO_OFFSET(scalePort), myScale.getZeroOffset());
}

//Reads the current system settings from EEPROM
//If anything looks weird, reset setting to default value

void readSystemSettings(int scalePort) //REQUIRED to read an already-calibrated system settings (zero offset and calibration factor)
{
  float settingCalibrationFactor; //Value used to convert the load cell reading to lbs or kg
  long settingZeroOffset; //Zero value that is found when scale is tared

  //Look up the calibration factor
  EEPROM.get(LOCATION_CALIBRATION_FACTOR(scalePort), settingCalibrationFactor);
  if (settingCalibrationFactor == 0xFFFFFFFF)
  {
    settingCalibrationFactor = 0; //Default to 0
    EEPROM.put(LOCATION_CALIBRATION_FACTOR(scalePort), settingCalibrationFactor);
  }

  //Look up the zero tare point
  EEPROM.get(LOCATION_ZERO_OFFSET(scalePort), settingZeroOffset);
  if (settingZeroOffset == 0xFFFFFFFF)
  {
    settingZeroOffset = 1000L; //Default to 1000 so we don't get inf
    EEPROM.put(LOCATION_ZERO_OFFSET(scalePort), settingZeroOffset);
  }

  //Pass these values to the library
  myScale.setCalibrationFactor(settingCalibrationFactor);
  myScale.setZeroOffset(settingZeroOffset);

  settingsDetected = true; //Assume for the moment that there are good cal values
  if (settingCalibrationFactor < 0.1 || settingZeroOffset == 1000)
    settingsDetected = false; //Defaults detected. Prompt user to cal scale.
}
