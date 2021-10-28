/**
 * ESP-NOW mesh network test sketch
 * 
 * Author: Nick Reynolds
 * License: Apache License v2
 */

#include <m2mMesh.h>

char nodeName[17];
uint8_t numberOfNodes = 0;
uint32_t timeOfLastNodeList = 0;
uint32_t nodeListInterval = 60000;
bool meshJoined = false;

void setup()
{
  Serial.begin(115200);
  //Set the Node name from the ESP chip ID
  #ifdef ESP8266
  sprintf(nodeName, "MeshNode%08x",ESP.getChipId());
  #elif defined (ESP32)
  sprintf(nodeName, "MeshNode%08x",getChipId());
  #endif
  if(m2mMesh.setNodeName(nodeName))
  {
    Serial.print("\n\nMesh node name:");
    Serial.println(nodeName);
  }
  else
  {
    Serial.println("Failed to set node name");
  }
  if(m2mMesh.begin())
  {
    Serial.print("Mesh started on channel:");
    Serial.println(WiFi.channel());
  }
  else
  {
    Serial.println("Mesh failed to start");
  }
}

void loop()
{
  m2mMesh.housekeeping();
  if(m2mMesh.joined())
  {
    if(meshJoined == false)
    {
      meshJoined = true;
      Serial.println("Joined mesh");
    }
    if(numberOfNodes != m2mMesh.numberOfReachableNodes())
    {
      numberOfNodes = m2mMesh.numberOfReachableNodes();
      timeOfLastNodeList = millis();
      listNodes();
    }
    else if(numberOfNodes>0 && millis()-timeOfLastNodeList>nodeListInterval)
    {
      timeOfLastNodeList = millis();
      listNodes();
    }
  }
  else if(meshJoined == true)
  {
    meshJoined = false;
    Serial.println("Left mesh");
  }
}

void listNodes()
{
  Serial.printf("Number of reachable nodes:%02u\r\n",numberOfNodes);
  uint8_t macaddress[6];
  for(uint8_t node=0; node < numberOfNodes; node++)
  {
    m2mMesh.macAddress(node,macaddress);
    if(m2mMesh.nodeNameIsSet(node))
    {
      Serial.printf("\tNode:%02u\tMAC address:%02x%02x%02x%02x%02x%02x\tNode name:%s\r\n",node,macaddress[0],macaddress[1],macaddress[2],macaddress[3],macaddress[4],macaddress[5],m2mMesh.getNodeName(node));
    }
    else
    {
      Serial.printf("\tNode:%02u\tMAC address:%02x%02x%02x%02x%02x%02x\tNode name:<Unknown>\r\n",node,macaddress[0],macaddress[1],macaddress[2],macaddress[3],macaddress[4],macaddress[5]);
    }
  }
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
