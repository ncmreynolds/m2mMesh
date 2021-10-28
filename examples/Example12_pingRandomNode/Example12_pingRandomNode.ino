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

uint32_t lastPing = 0;
uint32_t pingInterval = 1000;
uint32_t timeout = 5000;
bool joinedMesh = false;
uint8_t numberOfNodes = 0;
uint8_t numberOfReachableNodes = 0;
bool waitingForResponse = false;
uint32_t successes = 0;
uint32_t failures = 0;

void setup()
{
  Serial.begin(115200);
  delay(1000);
  //m2mMesh.enableDebugging(Serial,m2mMesh.MESH_UI_LOG_INFORMATION | m2mMesh.MESH_UI_LOG_WARNINGS | m2mMesh.MESH_UI_LOG_ERRORS | m2mMesh.MESH_UI_LOG_NODE_MANAGEMENT | m2mMesh.MESH_UI_LOG_NHS_RECEIVED);
  //m2mMesh.enableDebugging(Serial,m2mMesh.MESH_UI_LOG_INFORMATION | m2mMesh.MESH_UI_LOG_WARNINGS | m2mMesh.MESH_UI_LOG_ERRORS | m2mMesh.MESH_UI_LOG_NODE_MANAGEMENT | m2mMesh.MESH_UI_LOG_ESP_NOW_EVENTS | m2mMesh.MESH_UI_LOG_USR_SEND | m2mMesh.MESH_UI_LOG_USR_RECEIVED | m2mMesh.MESH_UI_LOG_USR_FORWARDING);
  //m2mMesh.enableDebugging(Serial,m2mMesh.MESH_UI_LOG_NODE_MANAGEMENT);
  //m2mMesh.enableDebugging(Serial,m2mMesh.MESH_UI_LOG_ALL_RECEIVED_PACKETS);
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
  if(waitingForResponse == false)
  {
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
    if(millis() - lastPing > pingInterval)
    {
      if(m2mMesh.joined())
      {
        uint8_t destinationId = random(0,m2mMesh.numberOfNodes());    //Select the destination randomly
        uint8_t macaddress[6];
        m2mMesh.macAddress(destinationId,macaddress);
        if(m2mMesh.nodeNameIsSet(destinationId))
        {
          Serial.printf("Pinging node:%02u MAC address:%02x:%02x:%02x:%02x:%02x:%02x Node name:\"%s\"",destinationId,macaddress[0],macaddress[1],macaddress[2],macaddress[3],macaddress[4],macaddress[5],m2mMesh.getNodeName(destinationId));
        }
        else
        {
          Serial.printf("Pinging node:%02u MAC address:%02x:%02x:%02x:%02x:%02x:%02x Node name:<Unknown>",destinationId,macaddress[0],macaddress[1],macaddress[2],macaddress[3],macaddress[4],macaddress[5]);
        }
        if(m2mMesh.ping(destinationId))
        {
          waitingForResponse = true;
        }
        else
        {
          lastPing = millis();
          Serial.printf(" failed, code:%02x \"%s\"\r\n",m2mMesh.lastError(),m2mMesh.lastErrorDescription());
          m2mMesh.clearLastError();
        }
      }
    }
  }
  else if(millis() - lastPing > timeout)
  {
    failures++;
    Serial.println(" timed out");
    waitingForResponse = false;
    Serial.printf("Success rate %u/%u\r\n",successes,successes+failures);
    lastPing = millis();
  }
  if(m2mMesh.pingResponse())
  {
    lastPing = millis();
    successes++;
    uint8_t hops = m2mMesh.pingHops();
    uint8_t hopMacAddress[6];
    if(hops > 1)
    {
      Serial.printf(" success RTT:%ums %u hops\r\n",m2mMesh.pingTime());
      for(uint8_t hop = 0; hop < hops; hop++)
      {
        if(m2mMesh.retrievePingHop(hop,hopMacAddress))
        {
          Serial.printf("\tHop %02u %02x:%02x:%02x:%02x:%02x:%02x\r\n",hop,hopMacAddress[0],hopMacAddress[1],hopMacAddress[2],hopMacAddress[3],hopMacAddress[4],hopMacAddress[5]);
        }
      }
    }
    else
    {
      Serial.printf(" success RTT:%ums 1 hop\r\n",m2mMesh.pingTime(),hops);
      if(m2mMesh.retrievePingHop(0,hopMacAddress))
      {
        Serial.printf("\tHop 00 %02x:%02x:%02x:%02x:%02x:%02x\r\n",hopMacAddress[0],hopMacAddress[1],hopMacAddress[2],hopMacAddress[3],hopMacAddress[4],hopMacAddress[5]);
      }
    }
    m2mMesh.markPingRead();
    waitingForResponse = false;
    Serial.printf("Success rate %u/%u\r\n",successes,successes+failures);
  }
  if(m2mMesh.messageWaiting())
  {
    m2mMesh.markMessageRead();
  }
}
