/**
 * ESP-NOW mesh network test sketch
 * 
 * Author: Nick Reynolds
 * License: Apache License v2
 */

#include <m2mMesh.h>

char nodeName[] = "setNodeName.ino";
uint8_t numberOfNodes = 0;
uint8_t numberOfReachableNodes = 0;
uint32_t timeOfLastNodeList = 0;
uint32_t nodeListInterval = 60000;
bool meshJoined = false;

void setup()
{
  Serial.begin(115200);
  m2mMesh.enableDebugging(Serial,m2mMesh.MESH_UI_LOG_INFORMATION | m2mMesh.MESH_UI_LOG_WARNINGS | m2mMesh.MESH_UI_LOG_ERRORS | m2mMesh.MESH_UI_LOG_NODE_MANAGEMENT | m2mMesh.MESH_UI_LOG_PEER_MANAGEMENT);
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
    if(numberOfNodes != m2mMesh.numberOfNodes())
    {
      numberOfNodes = m2mMesh.numberOfNodes();
      timeOfLastNodeList = millis();
      listNodes();
    }
    else if(numberOfReachableNodes != m2mMesh.numberOfReachableNodes())
    {
      numberOfReachableNodes = m2mMesh.numberOfReachableNodes();
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
  if(m2mMesh.messageWaiting())
  {
    m2mMesh.markMessageRead();  //Simply trash any inbound application messages
    Serial.println("Received message");
  }
}

void listNodes()
{
  Serial.printf("Number of nodes:%02u, %02u reachable \r\n", numberOfNodes, numberOfReachableNodes);
  uint8_t macaddress[6];
  for(uint8_t node=0; node < numberOfNodes; node++)
  {
    m2mMesh.macAddress(node,macaddress);
    if(m2mMesh.nodeNameIsSet(node))
    {
      Serial.printf("\tNode:%02u\t%s\tMAC address:%02x%02x%02x%02x%02x%02x\tNode name:%s\r\n",node,m2mMesh.nodeIsReachableNode(node) == true ? "Up  ":"Down",macaddress[0],macaddress[1],macaddress[2],macaddress[3],macaddress[4],macaddress[5],m2mMesh.getNodeName(node));
    }
    else
    {
      Serial.printf("\tNode:%02u\t%s\tMAC address:%02x%02x%02x%02x%02x%02x\tNode name:<Unknown>\r\n",node,m2mMesh.nodeIsReachableNode(node) == true ? "Up  ":"Down",macaddress[0],macaddress[1],macaddress[2],macaddress[3],macaddress[4],macaddress[5]);
    }
  }
}
