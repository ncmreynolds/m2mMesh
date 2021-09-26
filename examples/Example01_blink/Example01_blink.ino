/**
 * ESP-NOW mesh network test sketch
 * 
 * Author: Nick Reynolds
 * License: LGPL v2.1
 * 
 */
 
#ifndef LED_BUILTIN
#define LED_BUILTIN 1
#endif

//Include the mesh library
//The mesh library will include the ESP8266 Wifi and ESP-Now libraries so you don't need to add them separately

#include <m2mMesh.h>

//Create a new mesh object, by default it supports 32 nodes but will dynamically re-allocate memory if more appear. Re-allocating memory is inefficient so if you know how many nodes you have specifiy it

m2mMesh mesh;
uint32_t lastBlink;
uint32_t blinkInterval = 1000;
bool ledState = false;

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  mesh.begin();
}

void loop() {
  mesh.housekeeping();
  if(mesh.synced() && mesh.syncedMillis() - lastBlink > blinkInterval)
  {
    if(ledState == false)
    {
      digitalWrite(LED_BUILTIN, HIGH);
      Serial.println("Blink");
      ledState = true;
    }
    else
    {
      digitalWrite(LED_BUILTIN, LOW);
      ledState = false;
    }
    lastBlink = mesh.syncedMillis();
    //lastBlink = lastBlink - lastBlink%250;
  }
}
