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
void setup()
{
  Serial.begin(115200);
  delay(1000);
  //m2mMesh.enableDebugging(Serial,m2mMesh.MESH_UI_LOG_INFORMATION | m2mMesh.MESH_UI_LOG_WARNINGS | m2mMesh.MESH_UI_LOG_ERRORS | m2mMesh.MESH_UI_LOG_NODE_MANAGEMENT | m2mMesh.MESH_UI_LOG_NHS_RECEIVED);
  m2mMesh.enableDebugging(Serial,m2mMesh.MESH_UI_LOG_INFORMATION | m2mMesh.MESH_UI_LOG_WARNINGS | m2mMesh.MESH_UI_LOG_ERRORS | m2mMesh.MESH_UI_LOG_NODE_MANAGEMENT | m2mMesh.MESH_UI_LOG_PEER_MANAGEMENT);
  //m2mMesh.enableDebugging(Serial,m2mMesh.MESH_UI_LOG_INFORMATION | m2mMesh.MESH_UI_LOG_WARNINGS | m2mMesh.MESH_UI_LOG_ERRORS | m2mMesh.MESH_UI_LOG_NODE_MANAGEMENT);
  //m2mMesh.enableDebugging(Serial,m2mMesh.MESH_UI_LOG_NODE_MANAGEMENT);
  m2mMesh.setNodeName("Data display node");
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
    Serial.println("Joined mesh, sending data every node, one at a time");
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
      //uint8_t destinationId = random(0,m2mMesh.numberOfNodes() + 1);  //Select the destination randomly, with a chance of flooding
      if(destinationId < numberOfNodes)                                 //Not adding a destination means the message will be flooded to ALL nodes
      {
        if(m2mMesh.destination(destinationId))
        {
          uint8_t macaddress[6];
          m2mMesh.macAddress(destinationId,macaddress);
          if(m2mMesh.nodeNameIsSet(destinationId))
          {
            Serial.printf("Sending message to node:%02u MAC address:%02x:%02x:%02x:%02x:%02x:%02x Node name:%s",destinationId,macaddress[0],macaddress[1],macaddress[2],macaddress[3],macaddress[4],macaddress[5],m2mMesh.getNodeName(destinationId));
          }
          else
          {
            Serial.printf("Sending message to node:%02u MAC address:%02x:%02x:%02x:%02x:%02x:%02x Node name:<Unknown>",destinationId,macaddress[0],macaddress[1],macaddress[2],macaddress[3],macaddress[4],macaddress[5]);
          }
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
      String message;
      if(destinationId < m2mMesh.numberOfNodes())
      {
        message = "Hello node " + String(destinationId);
      }
      else
      {
        message = "Hello all";
      }
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
      }
      destinationId++;
      if(destinationId > numberOfNodes)
      {
        destinationId = 0;
      }
    }
  }
}
