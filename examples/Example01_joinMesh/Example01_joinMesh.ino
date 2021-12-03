/**
 * ESP-NOW mesh network test sketch
 * 
 * Author: Nick Reynolds
 * License: LGPL v2.1
 *
 * See README.md for information
 * 
 */

#if defined(LED_BUILTIN)
#if defined(ARDUINO_ESP8266_WEMOS_D1MINI) || defined(ARDUINO_ESP8266_WEMOS_D1MINIPRO) || defined(ARDUINO_Pocket32)
#define LED_ON LOW
#define LED_OFF HIGH
#else
#define LED_ON HIGH
#define LED_OFF LOW
#endif
#endif

#include <m2mMesh.h>
bool joinedMesh = false;
uint8_t numberOfNodes = 0;
uint8_t numberOfReachableNodes = 0;

void setup() {
  #if defined(LED_BUILTIN)
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN,LED_OFF);
  #endif
  Serial.begin(115200);
  //m2mMesh.enableDebugging(Serial,m2mMesh.MESH_UI_LOG_INFORMATION | m2mMesh.MESH_UI_LOG_WARNINGS | m2mMesh.MESH_UI_LOG_ERRORS | m2mMesh.MESH_UI_LOG_NODE_MANAGEMENT | m2mMesh.MESH_UI_LOG_OGM_RECEIVED |  m2mMesh.MESH_UI_LOG_OGM_SEND | m2mMesh.MESH_UI_LOG_OGM_FORWARDING);
  delay(1000);
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
  if(joinedMesh == false && m2mMesh.joined() == true)
  {
    joinedMesh = true;
    #if defined(LED_BUILTIN)
    digitalWrite(LED_BUILTIN,LED_ON);
    #endif
    Serial.println("Joined mesh");
  }
  else if(joinedMesh == true && m2mMesh.joined() == false)
  {
    joinedMesh = false;
    #if defined(LED_BUILTIN)
    digitalWrite(LED_BUILTIN,LED_OFF);
    #endif
    Serial.println("Left mesh");
  }
  else if(numberOfNodes != m2mMesh.numberOfNodes() || numberOfReachableNodes != m2mMesh.numberOfReachableNodes())
  {
    numberOfNodes = m2mMesh.numberOfNodes();
    numberOfReachableNodes = m2mMesh.numberOfReachableNodes();
    Serial.print("Number of nodes:");
    Serial.print(numberOfNodes);
    Serial.print(", reachable nodes:");
    Serial.println(numberOfReachableNodes);
  }
  if(m2mMesh.messageWaiting())
  {
    m2mMesh.markMessageRead();
  }
}
