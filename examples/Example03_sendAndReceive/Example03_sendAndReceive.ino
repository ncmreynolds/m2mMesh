/**
 * ESP-NOW mesh network test sketch
 * 
 * Author: Nick Reynolds
 * License: Apache License v2
 */

//Set the ADC on the ESP8266 to read its supply voltage directly. This must be done before 'setup' in a sketch. Comment out if you want to put a connection that reads the battery voltage.

#ifdef ESP8266
ADC_MODE(ADC_VCC);
#endif

//Include the mesh library
//The mesh library will include the ESP8266 Wifi and ESP-Now libraries so you don't need to add them separately

#include <m2mMesh.h>

char nodeName[]="MeshNode00000000";

//Create a new mesh object, by default it supports 32 nodes but will dynamically re-allocate memory if more appear. Re-allocating memory is inefficient so if you know how many nodes you have specifiy it

m2mMesh mesh;

void setup()
{
  //Set the Node name from the ESP chip ID
  #ifdef ESP8266
  sprintf(nodeName, "MeshNode%08x",ESP.getChipId());
  #elif defined (ESP32)
  sprintf(nodeName, "MeshNode%08x",getChipId());
  #endif
  mesh.setNodeName(nodeName);
  //Need to start ESP-Now early in the sketch or it is unstable
  mesh.begin();
}

void loop()
{
  mesh.housekeeping();
}
#if defined (ESP32)
uint32_t getChipId()
{
  uint32_t chipId = 0;
  for(int i=0; i<17; i=i+8)
  {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  return(chipId);
}
#endif
