/**
 * ESP-NOW mesh network test sketch
 * 
 * Author: Nick Reynolds
 * License: Apache License v2
 */


//Include the mesh library
//The mesh library will include the ESP8266 Wifi and ESP-Now libraries so you don't need to add them separately

#include <m2mMesh.h>

uint32_t lastSend = 0;
uint32_t sendInterval = 30000;
uint8_t minimumArrayLength = 2;
uint8_t maximumArrayLength = 16;

void setup()
{
  Serial.begin(115200);
  m2mMesh.enableDebugging(Serial,m2mMesh.MESH_UI_LOG_INFORMATION | m2mMesh.MESH_UI_LOG_WARNINGS | m2mMesh.MESH_UI_LOG_ERRORS | m2mMesh.MESH_UI_LOG_PEER_MANAGEMENT | m2mMesh.MESH_UI_LOG_USR_RECEIVED);
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
          Serial.printf("Sending data to node %02u",destinationId);
        }
        else
        {
          Serial.printf("Failed to add destination node %02u",destinationId);
        }
      }
      else
      {
        Serial.print("Sending data to all nodes");
      }
      Serial.printf(" %u bytes payload available\r\n",m2mMesh.payloadLeft());
      uint8_t numberOfFields = random(4,13);
      //uint8_t numberOfFields = 12;
      Serial.printf("\tAdding %u random fields\r\n",numberOfFields);
      for(uint8_t count=0; count<numberOfFields;count++)
      {
        //switch(count){
        switch (random(0,12)){
          case 0: //uint8_t
            switch(random(0,2)){
              case 0: //Single
                addBool();
              break;
              case 1: //Array
                addBoolArray();
              break;
            }
          break;
          case 1: //uint8_t
            switch(random(0,2)){
              case 0: //Single
                addUint8_t();
              break;
              case 1: //Array
                addUint8_tArray();
              break;
            }
          break;
          case 2: //int8_t
            addInt8_t();
          break;
          case 3: //uint16_t
            switch(random(0,2)){
              case 0: //Single
                addUint16_t();
              break;
              case 1: //Array
                addUint16_tArray();
              break;
            }
          break;
          case 4: //int16_t
            switch(random(0,2)){
              case 0: //Single
                addInt16_t();
              break;
              case 1: //Array
                addInt16_tArray();
              break;
            }
          break;
          case 5: //uint32_t
            switch(random(0,2)){
              case 0: //Single
                addUint32_t();
              break;
              case 1: //Array
                addUint32_tArray();
              break;
            }
          break;
          case 6: //int32_t
            switch(random(0,2)){
              case 0: //Single
                addInt32_t();
              break;
              case 1: //Array
                addInt32_tArray();
              break;
            }
          break;
          case 7: //float
            switch(random(0,2)){
              case 0: //Single
                addFloat();
              break;
              case 1: //Array
                addFloatArray();
              break;
            }
          break;
          case 8: //double
            switch(random(0,2)){
              case 0: //Single
                addDouble();
              break;
              case 1: //Array
                addDoubleArray();
              break;
            }
          break;
          case 9: //char
            switch(random(0,2)){
              case 0: //Single
                addChar();
              break;
              case 1: //Array
                addCharArray();
              break;
            }
          break;
          case 10: //char array
            addStr();
          break;
          case 11: //String
            addString();
          break;
        }
      }
      if(m2mMesh.send())
      {
        Serial.println("Sent successfully");
      }
      else
      {
        Serial.println("Sending failed");
      }
    }
  }
}
