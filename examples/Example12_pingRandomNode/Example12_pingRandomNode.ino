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
uint32_t sendInterval = 10000;
uint32_t timeout = 5000;
bool joinedMesh = false;
uint8_t numberOfNodes = 0;
uint8_t numberOfReachableNodes = 0;
bool pingSent = false;

void setup()
{
  Serial.begin(115200);
  delay(1000);
  //m2mMesh.enableDebugging(Serial,m2mMesh.MESH_UI_LOG_INFORMATION | m2mMesh.MESH_UI_LOG_WARNINGS | m2mMesh.MESH_UI_LOG_ERRORS | m2mMesh.MESH_UI_LOG_PEER_MANAGEMENT | m2mMesh.MESH_UI_LOG_NHS_RECEIVED);
  //m2mMesh.enableDebugging(Serial,m2mMesh.MESH_UI_LOG_INFORMATION | m2mMesh.MESH_UI_LOG_WARNINGS | m2mMesh.MESH_UI_LOG_ERRORS | m2mMesh.MESH_UI_LOG_USR_SEND | m2mMesh.MESH_UI_LOG_USR_RECEIVED | m2mMesh.MESH_UI_LOG_USR_FORWARDING);
  //m2mMesh.enableDebugging(Serial,m2mMesh.MESH_UI_LOG_PEER_MANAGEMENT);
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
      uint8_t destinationId = random(0,m2mMesh.numberOfNodes());    //Select the destination randomly
      if(m2mMesh.ping(destinationId))
      {
        Serial.printf("Ping node %02u",destinationId);
        pingSent = true;
      }
      else
      {
        Serial.printf("Ping node %02u failed, unreachable\r\n",destinationId);
      }
    }
  }
  if(pingSent == true && millis() - lastSend > timeout)
  {
    Serial.println(" timed out");
    pingSent = false;
  }
  if(m2mMesh.pingResponse())
  {
    if(m2mMesh.pingHops() > 1)
    {
      Serial.printf(" success %ums %u hops\r\n",m2mMesh.pingTime(),m2mMesh.pingHops());
    }
    else
    {
      Serial.printf(" success %ums 1 hop\r\n",m2mMesh.pingTime(),m2mMesh.pingHops());
    }
    m2mMesh.markPingRead();
    pingSent = false;
  }
}
