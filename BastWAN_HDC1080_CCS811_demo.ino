/**************************************************************************************
  This is an example for HDC1080 Humidity and Temperature Sensor breakout booard
  plus CCS811 digital gas sensor packaged as CJMCU-8118
  Adapted from ClosedCube_HDC1080 & SparkFunCCS811 libraries
  Hardware connections for Arduino Uno:
  VDD to 3.3V DC
  SCL to SCL
  SDA to SDA
  GND to common ground
  WAK to 5
  INT to 6
  MIT License

**************************************************************************************/

#include <Wire.h>
#include <ClosedCube_HDC1080.h>
// Click here to get the library: http://librarymanager/All#ClosedCube_HDC1080
#include <SparkFunCCS811.h>
// Click here to get the library: http://librarymanager/All#SparkFun_CCS811

// #define CCS811_ADDR 0x5B // Default I2C Address
#define CCS811_ADDR 0x5A // Alternate I2C Address
// BastWAN_I2C_Scanner found it at 0x5A

#define PIN_NOT_WAKE 5
#define PIN_NOT_INT 6

CCS811 myCCS811(CCS811_ADDR);
ClosedCube_HDC1080 hdc1080;
#define hdc1080_waitout 3000
double t0;

void setup() {
  Serial.begin(115200);
  Serial.flush();
  delay(3000);
  Serial.println("\n\nClosedCube HDC1080 Arduino Test");
  Wire.begin(SDA, SCL);
  Wire.setClock(100000);
  // Default settings:
  //  - Heater off
  //  - 14 bit Temperature and Humidity Measurement Resolutions
  hdc1080.begin(0x40);
  Serial.print("Manufacturer ID=0x");
  Serial.println(hdc1080.readManufacturerId(), HEX); // 0x5449 ID of Texas Instruments
  Serial.print("Device ID=0x");
  Serial.println(hdc1080.readDeviceId(), HEX); // 0x1050 ID of the device
  printSerialNumber();

  // This begins the CCS811 sensor and prints error status of .beginWithStatus()
  CCS811Core::CCS811_Status_e returnCode = myCCS811.beginWithStatus();
  Serial.print("CCS811 begin exited with: ");
  // Pass the error code to a function to print the results
  Serial.println(myCCS811.statusString(returnCode));
  // This sets the mode to 60 second reads, and prints returned error status.
  returnCode = myCCS811.setDriveMode(2);
  Serial.print("Mode request exited with: ");
  Serial.println(myCCS811.statusString(returnCode));
  // Configure and enable the interrupt line,
  // then print error status
  pinMode(PIN_NOT_INT, INPUT_PULLUP);
  returnCode = myCCS811.enableInterrupts();
  Serial.print("Interrupt configuration exited with: ");
  Serial.println(myCCS811.statusString(returnCode));
  // Configure the wake line
  pinMode(PIN_NOT_WAKE, OUTPUT);
  digitalWrite(PIN_NOT_WAKE, 1); // Start asleep

  t0 = millis();
}

void loop() {
  if (millis() - t0 > hdc1080_waitout) {
    Serial.print("T=");
    Serial.print(hdc1080.readTemperature());
    Serial.print("C, RH=");
    Serial.print(hdc1080.readHumidity());
    Serial.println("%");
    t0 = millis();
  }
  // Look for interrupt request from CCS811
  if (digitalRead(PIN_NOT_INT) == 0) {
    // Wake up the CCS811 logic engine
    digitalWrite(PIN_NOT_WAKE, 0);
    // Need to wait at least 50 µs
    delay(1);
    // Interrupt signal caught, so cause the CCS811 to run its algorithm
    myCCS811.readAlgorithmResults(); // Calling this function updates the global tVOC and CO2 variables
    Serial.print("CO2[");
    Serial.print(myCCS811.getCO2());
    Serial.print("] tVOC[");
    Serial.print(myCCS811.getTVOC());
    Serial.print("] millis[");
    Serial.print(millis());
    Serial.print("]");
    Serial.println();
    // Now put the CCS811's logic engine to sleep
    digitalWrite(PIN_NOT_WAKE, 1);
    // Need to be asleep for at least 20 µs
    delay(1);
  }
  delay(1); // cycle kinda fast
}

void printSerialNumber() {
  Serial.print("Device Serial Number=");
  HDC1080_SerialNumber sernum = hdc1080.readSerialNumber();
  char format[12];
  sprintf(format, "%02X-%04X-%04X", sernum.serialFirst, sernum.serialMid, sernum.serialLast);
  Serial.println(format);
}

// printSensorError gets, clears, then prints the errors
// saved within the error register.
void printSensorError() {
  uint8_t error = myCCS811.getErrorRegister();
  if (error == 0xFF) {
    // comm error
    Serial.println("Failed to get ERROR_ID register.");
  } else {
    Serial.print("Error: ");
    if (error & 1 << 5)
      Serial.print("HeaterSupply");
    if (error & 1 << 4)
      Serial.print("HeaterFault");
    if (error & 1 << 3)
      Serial.print("MaxResistance");
    if (error & 1 << 2)
      Serial.print("MeasModeInvalid");
    if (error & 1 << 1)
      Serial.print("ReadRegInvalid");
    if (error & 1 << 0)
      Serial.print("MsgInvalid");
    Serial.println();
  }
}
