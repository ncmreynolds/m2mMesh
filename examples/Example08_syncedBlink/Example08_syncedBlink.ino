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

#if defined(ARDUINO_ESP8266_WEMOS_D1MINI) || defined(ARDUINO_ESP8266_WEMOS_D1MINIPRO) || defined(ARDUINO_Pocket32)
#define LED_ON LOW
#define LED_OFF HIGH
#else
#define LED_ON HIGH
#define LED_OFF LOW
#endif

//Include the mesh library
//The mesh library will include the ESP8266 Wifi and ESP-Now libraries so you don't need to add them separately

#include <m2mMesh.h>

//Create a new mesh object, by default it supports 32 nodes but will dynamically re-allocate memory if more appear. Re-allocating memory is inefficient so if you know how many nodes you have specifiy it

uint32_t lastBlink;
uint32_t blinkInterval = 1000;
bool ledState = false;
bool synced = false;

void setup() {
  Serial.begin(115200);
  delay(1000);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN,LED_OFF);
  //m2mMesh.enableDebugging(Serial,m2mMesh.MESH_UI_LOG_ALL_RECEIVED_PACKETS | m2mMesh.MESH_UI_LOG_ALL_SENT_PACKETS | m2mMesh.MESH_UI_LOG_INFORMATION | m2mMesh.MESH_UI_LOG_WARNINGS | m2mMesh.MESH_UI_LOG_ERRORS | m2mMesh.MESH_UI_LOG_PEER_MANAGEMENT | m2mMesh.MESH_UI_LOG_NHS_RECEIVED );
  //m2mMesh.enableDebugging(Serial, m2mMesh.MESH_UI_LOG_INFORMATION | m2mMesh.MESH_UI_LOG_WARNINGS | m2mMesh.MESH_UI_LOG_ERRORS | m2mMesh.MESH_UI_LOG_PEER_MANAGEMENT | m2mMesh.MESH_UI_LOG_NHS_RECEIVED );
  if(m2mMesh.begin())
  {
    Serial.print("\n\nMesh started on channel:");
    Serial.println(WiFi.channel());
  }
  else
  {
    Serial.println("\n\nMesh failed to start");
  }
}

void loop() {
  m2mMesh.housekeeping();
  if(m2mMesh.synced())   //Flash the LED once synced
  {
    if(synced == false)
    {
      synced = true;
      Serial.println("Mesh uptime synced");
    }
    if(m2mMesh.syncedMillis() - lastBlink > blinkInterval)
    {
      lastBlink = m2mMesh.syncedMillis();
      if(ledState == true)  //Make the on cycle happen on 'even' intervals
      {
        lastBlink = lastBlink - lastBlink%(blinkInterval*2);
      }
      if(ledState == false)
      {
        digitalWrite(LED_BUILTIN, LED_ON);
        ledState = true;
        printUptime(lastBlink);
      }
      else
      {
        digitalWrite(LED_BUILTIN, LED_OFF);
        ledState = false;
      }
    }
  }
  else
  {
    if(m2mMesh.joined() == true && ledState == false)  //Turn on the LED when joining the mesh
    {
      digitalWrite(LED_BUILTIN, LED_ON);
      ledState = true;
      Serial.println("Joined mesh");
    }
    else if(m2mMesh.joined() == false && ledState == true) //Turn off the LED when leaving the mesh
    {
      digitalWrite(LED_BUILTIN, LED_OFF);
      ledState = false;
      Serial.println("Left mesh");
    }
  }
}

void printUptime(uint32_t uptime)
{
  uint8_t deciseconds = (uptime/10) % 100;
  uint8_t seconds = (uptime/1000) % 60;
  uint8_t minutes = (uptime/60000) % 60;
  uint8_t hours = (uptime/3600000) % 24;
  uint8_t days = (uptime/86400000) % 60;
  Serial.printf("Uptime:%02ud:%02uh:%02um:%02u.%02us\r\n",days,hours,minutes,seconds,deciseconds);
}
