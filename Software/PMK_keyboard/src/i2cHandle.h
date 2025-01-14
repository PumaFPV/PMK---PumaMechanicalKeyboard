#ifndef I2CHANDLE_H
#define I2CHANDLE_H

#include "variables.h"


uint8_t* scanI2c()
{
  byte error, address;
  int nDevices;
  uint8_t* slaveAddress = new uint8_t(8);    // Doesnt expect more than 8 slaves on the bus
  Serial.println("Scanning...");

  nDevices = 0;
  for(address = 1; address < 127; address++ ) 
  {
    //Serial.printf("Current address tested: %i\r\n", address);
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if(error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if(address<16) 
      Serial.print("0");
      Serial.print(address,HEX);
      Serial.println("  !");
      slaveAddress[nDevices] = address;
      nDevices++;
    }
    else if(error==4) 
    {
      Serial.print("Unknow error at address 0x");
      if(address<16) 
      Serial.print("0");
      Serial.println(address,HEX);
    }    
  }
  if(nDevices == 0)
  {
    Serial.println("No I2C devices found\n");
  }
  else
  {
    Serial.println("done\n");
  }
  return slaveAddress;
}

void checkSideModule()
{
  //Check for side module presence
  uint8_t error;

  Wire.beginTransmission(LDC1612_ADDRESS);
  error = Wire.endTransmission(); // Returns 0 if device is found

  if(error == 0)
  {
    //SpaceMouse is present
    Serial.printf("Detected space mouse\r\n");
    spacemouseIsPresent = 1;
  }

  Wire.beginTransmission(TRACKPAD_ADDRESS);
  error = Wire.endTransmission();

  if (error == 0 && !spacemouseIsPresent)
  {
    // Trackpad is present
    Serial.printf("Detected trackpad\r\n");
    trackpadIsPresent = 1;
  }
  
}

#endif