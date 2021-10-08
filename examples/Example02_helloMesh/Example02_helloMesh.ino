/**
 * ESP-NOW mesh network test sketch
 * 
 * Author: Nick Reynolds
 * License: LGPL v2.1
 * 
 */

#include <m2mMesh.h>
bool joinedMesh = false;
uint8_t numberOfNodes = 0;
uint8_t numberOfReachableNodes = 0;
uint32_t lastSend = 0;
uint32_t sendInterval = 30000;
String messageToSend = "Hello Mesh";

void setup() {
  Serial.begin(115200);
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
    Serial.println("Joined mesh");
  }
  else if(joinedMesh == true && m2mMesh.joined() == false)
  {
    joinedMesh = false;
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
  if(millis() - lastSend > sendInterval)
  {
    lastSend = millis();
    if(m2mMesh.joined())
    {
      if(m2mMesh.add(messageToSend))
      {
        if(m2mMesh.send())
        {
          Serial.print("Message \"");
          Serial.print(messageToSend);
          Serial.println("\" sent");
        }
        else
        {
          Serial.println("Failed to send message");
          m2mMesh.clearMessage();
        }
      }
      else
      {
        Serial.print("Unable to add\"");
        Serial.print(messageToSend);
        Serial.println("\" to message");
      }
    }
  }
  if(m2mMesh.messageWaiting())
  {
    if(m2mMesh.nextDataType() == m2mMesh.USR_DATA_STRING)
    {
      Serial.print("Received message:\"");
      Serial.print(m2mMesh.retrieveString());
      Serial.print("\" from node:");
      Serial.println(m2mMesh.sourceId());
    }
    m2mMesh.markMessageRead();
  }
}
