/**
 * ESP-NOW mesh network test sketch
 * 
 * Author: Nick Reynolds
 * License: Apache License v2
 * 
 * See README.md for information
 * 
 */

#include <m2mMesh.h>

uint32_t lastSend = 0;
uint32_t sendInterval = 30000;
bool joinedMesh = false;
uint8_t numberOfNodes = 0;
uint8_t numberOfReachableNodes = 0;
uint8_t destinationId = 0;
uint32_t successes = 0;
uint32_t failures = 0;

char destinationNode[] = "setNodeName.ino";

void setup()
{
  Serial.begin(115200);
  delay(1000);
  m2mMesh.setNodeName("useNodeName.ino");
  m2mMesh.enableDebugging(Serial,m2mMesh.MESH_UI_LOG_INFORMATION | m2mMesh.MESH_UI_LOG_WARNINGS | m2mMesh.MESH_UI_LOG_ERRORS | m2mMesh.MESH_UI_LOG_NODE_MANAGEMENT | m2mMesh.MESH_UI_LOG_PEER_MANAGEMENT);
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

void loop()
{
  m2mMesh.housekeeping();
  if(joinedMesh == false && m2mMesh.joined() == true)
  {
    joinedMesh = true;
    Serial.println("Joined mesh, sending data to named node");
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
      if(m2mMesh.destination(destinationNode))
      {
        uint8_t macaddress[6];
        m2mMesh.macAddress(destinationNode,macaddress);
        Serial.printf("Sending message to node:%02u MAC address:%02x:%02x:%02x:%02x:%02x:%02x Node name:\"%s\"",destinationId,macaddress[0],macaddress[1],macaddress[2],macaddress[3],macaddress[4],macaddress[5],destinationNode);
        Serial.printf(" %u bytes payload available\r\n",m2mMesh.payloadLeft());
        String message = "Hello " + String(destinationNode);
        if(m2mMesh.add(message))
        {
          if(m2mMesh.send())
          {
            Serial.println("Message \"" + message + "\" sent successfully");
            successes++;
          }
          else
          {
            Serial.println("Sending failed");
            failures++;
          }
          Serial.println(String(successes) + " successes, " + String(failures) + " failures");
        }
        else
        {
          Serial.println("Unable to add field to message");
          failures++;
        }
      }
      else
      {
        Serial.printf("Failed to set destination as %s\r\n",destinationNode);
        failures++;
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
