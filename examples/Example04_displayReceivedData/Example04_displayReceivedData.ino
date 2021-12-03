/**
 * ESP-NOW mesh network test sketch
 * 
 * Author: Nick Reynolds
 * License: Apache License v2
 * 
 * See README.md for information
 * 
 */


//Include the mesh library
//The mesh library will include the ESP8266 Wifi and ESP-Now libraries so you don't need to add them separately

#include <m2mMesh.h>
bool joinedMesh = false;
uint8_t numberOfNodes = 0;
uint8_t numberOfReachableNodes = 0;

void setup()
{
  Serial.begin(115200);
  //m2mMesh.enableDebugging(Serial,m2mMesh.MESH_UI_LOG_INFORMATION | m2mMesh.MESH_UI_LOG_WARNINGS | m2mMesh.MESH_UI_LOG_ERRORS | m2mMesh.MESH_UI_LOG_NODE_MANAGEMENT | m2mMesh.MESH_UI_LOG_USR_RECEIVED);
  //m2mMesh.enableDebugging(Serial,m2mMesh.MESH_UI_LOG_INFORMATION | m2mMesh.MESH_UI_LOG_WARNINGS | m2mMesh.MESH_UI_LOG_ERRORS | m2mMesh.MESH_UI_LOG_NODE_MANAGEMENT | m2mMesh.MESH_UI_LOG_ESP_NOW_EVENTS | m2mMesh.MESH_UI_LOG_BUFFER_MANAGEMENT);
  //m2mMesh.enableDebugging(Serial,m2mMesh.MESH_UI_LOG_ALL_RECEIVED_PACKETS);
  //m2mMesh.enableDebugging(Serial, m2mMesh.MESH_UI_LOG_INFORMATION | m2mMesh.MESH_UI_LOG_WARNINGS | m2mMesh.MESH_UI_LOG_ERRORS| m2mMesh.MESH_UI_LOG_NODE_MANAGEMENT | m2mMesh.MESH_UI_LOG_ESP_NOW_EVENTS | m2mMesh.MESH_UI_LOG_BUFFER_MANAGEMENT);
  m2mMesh.enableDebugging(Serial, m2mMesh.MESH_UI_LOG_INFORMATION | m2mMesh.MESH_UI_LOG_WARNINGS | m2mMesh.MESH_UI_LOG_ERRORS| m2mMesh.MESH_UI_LOG_NODE_MANAGEMENT | m2mMesh.MESH_UI_LOG_ESP_NOW_EVENTS | m2mMesh.MESH_UI_LOG_PEER_MANAGEMENT);
  m2mMesh.setNodeName("Sequential hello node");
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
    Serial.println("Joined mesh, displaying received data");
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
  if(m2mMesh.messageWaiting())
  {
    uint8_t sourceId = m2mMesh.sourceId();
    uint8_t macaddress[6];
    m2mMesh.macAddress(sourceId,macaddress);
    if(m2mMesh.nodeNameIsSet(sourceId))
    {
      Serial.printf("New message from node:%02u MAC address:%02x:%02x:%02x:%02x:%02x:%02x Node name:%s with %u items of data\r\n",sourceId,macaddress[0],macaddress[1],macaddress[2],macaddress[3],macaddress[4],macaddress[5],m2mMesh.getNodeName(sourceId),m2mMesh.dataAvailable());
    }
    else
    {
      Serial.printf("New message from node:%02u MAC address:%02x:%02x:%02x:%02x:%02x:%02x Node name:<Unknown> with %u items of data\r\n",sourceId,macaddress[0],macaddress[1],macaddress[2],macaddress[3],macaddress[4],macaddress[5],m2mMesh.dataAvailable());
    }
    while(m2mMesh.dataAvailable() > 0)
    {
      switch (m2mMesh.nextDataType()){
        case m2mMesh.USR_DATA_BOOL:
          if(m2mMesh.retrieveBool() == true)
          {
            Serial.println("\tRetrieved bool:true");
          }
          else
          {
            Serial.println("\tRetrieved bool:false");
          }
        break;
        case m2mMesh.USR_DATA_BOOL_ARRAY:
          retrieveBoolArray();
        break;
        case m2mMesh.USR_DATA_UINT8_T:
          Serial.printf("\tRetrieved uint8_t:%u\r\n",m2mMesh.retrieveUint8_t());
        break;
        case m2mMesh.USR_DATA_UINT8_T_ARRAY:
          retrieveUint8_tArray();
        break;
        case m2mMesh.USR_DATA_INT8_T:
          Serial.printf("\tRetrieved int8_t:%i\r\n",m2mMesh.retrieveInt8_t());
        break;
        case m2mMesh.USR_DATA_INT8_T_ARRAY:
          retrieveInt8_tArray();
        break;
        case m2mMesh.USR_DATA_UINT16_T:
          Serial.printf("\tRetrieved uint16_t:%u\r\n",m2mMesh.retrieveUint16_t());
        break;
        case m2mMesh.USR_DATA_UINT16_T_ARRAY:
          retrieveUint16_tArray();
        break;
        case m2mMesh.USR_DATA_INT16_T:
          Serial.printf("\tRetrieved int16_t:%i\r\n",m2mMesh.retrieveInt16_t());
        break;
        case m2mMesh.USR_DATA_INT16_T_ARRAY:
          retrieveInt16_tArray();
        break;
        case m2mMesh.USR_DATA_UINT32_T:
          Serial.printf("\tRetrieved uint32_t:%u\r\n",m2mMesh.retrieveUint32_t());
        break;
        case m2mMesh.USR_DATA_UINT32_T_ARRAY:
          retrieveUint32_tArray();
        break;
        case m2mMesh.USR_DATA_INT32_T:
          Serial.printf("\tRetrieved int32_t:%i\r\n",m2mMesh.retrieveInt32_t());
        break;
        case m2mMesh.USR_DATA_INT32_T_ARRAY:
          retrieveInt32_tArray();
        break;
        case m2mMesh.USR_DATA_UINT64_T:
          Serial.printf("\tRetrieved uint64_t:%u\r\n",m2mMesh.retrieveUint64_t());
        break;
        case m2mMesh.USR_DATA_UINT64_T_ARRAY:
          retrieveUint64_tArray();
        break;
        case m2mMesh.USR_DATA_INT64_T:
          Serial.printf("\tRetrieved int64_t:%i\r\n",m2mMesh.retrieveInt64_t());
        break;
        case m2mMesh.USR_DATA_INT64_T_ARRAY:
          retrieveInt64_tArray();
        break;
        case m2mMesh.USR_DATA_FLOAT:
          Serial.printf("\tRetrieved float:%f\r\n",m2mMesh.retrieveFloat());
        break;
        case m2mMesh.USR_DATA_FLOAT_ARRAY:
          retrieveFloatArray();
        break;
        case m2mMesh.USR_DATA_DOUBLE:
          Serial.printf("\tRetrieved double:%f\r\n",m2mMesh.retrieveDouble());
        break;
        case m2mMesh.USR_DATA_DOUBLE_ARRAY:
          retrieveDoubleArray();
        break;
        case m2mMesh.USR_DATA_CHAR:
          Serial.printf("\tRetrieved char:\"%c\"\r\n",m2mMesh.retrieveChar());
        break;
        case m2mMesh.USR_DATA_CHAR_ARRAY:
          retrieveCharArray();
        break;
        case m2mMesh.USR_DATA_STR:
          retrieveStr();
        break;
        case m2mMesh.USR_DATA_STRING:
          Serial.printf("\tRetrieved String:\"%s\"\r\n",(m2mMesh.retrieveString()).c_str());
        break;
        case m2mMesh.USR_DATA_UNAVAILABLE:
          Serial.println("\tData unavailable");
        break;
        default : //Just in case an unhandled data type arrives
          Serial.printf("\tUnhandled data type %02x\r\n",m2mMesh.nextDataType());
          m2mMesh.skipRetrieve();
        break;
      }
    }
    m2mMesh.markMessageRead();
  }
}
