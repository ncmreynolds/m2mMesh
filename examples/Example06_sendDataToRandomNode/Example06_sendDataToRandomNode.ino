/**
 * ESP-NOW mesh network test sketch
 * 
 * Author: Nick Reynolds
 * License: Apache License v2
 */

#include <m2mMesh.h>

uint32_t lastSend = 0;
uint32_t sendInterval = 30000;

void setup()
{
  Serial.begin(115200);
  m2mMesh.begin();
}

void loop()
{
  m2mMesh.housekeeping();
  if(millis() - lastSend > sendInterval)
  {
    lastSend = millis();
    if(m2mMesh.joined())
    {
      uint8_t destinationId = random(0,m2mMesh.numberOfOriginators() + 1);  //Select the destination randomly, with a chance of flooding
      if(destinationId < m2mMesh.numberOfOriginators())                     //Not adding a destination means the message will be flooded to ALL nodes
      {
        if(m2mMesh.destination(destinationId))
        {
          Serial.printf("Sending message to node %02u",destinationId);
        }
        else
        {
          Serial.printf("Failed to add destination node %02u",destinationId);
        }
      }
      else
      {
        Serial.print("Sending message to all nodes");
      }
      Serial.printf(" %u bytes payload available\r\n",m2mMesh.payloadLeft());
      if(m2mMesh.add("Hello there"))
      {
        if(m2mMesh.send())
        {
          Serial.println("Sent successfully");
        }
        else
        {
          Serial.println("Sending failed");
        }
      }
      else
      {
        Serial.println("Unable to add field to message");
      }
    }
  }
}
