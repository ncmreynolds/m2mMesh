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
bool joinedMesh = false;
uint8_t numberOfNodes = 0;
uint8_t numberOfReachableNodes = 0;

uint32_t lastSend = 0;
uint32_t sendInterval = 30000;
uint8_t minimumArrayLength = 2;
uint8_t maximumArrayLength = 16;

void setup()
{
  Serial.begin(115200);
  m2mMesh.setNodeName("Data flooding node");
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
    Serial.println("Joined mesh, flooding random data types to the whole mesh");
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
      Serial.printf("Flooding data to all nodes, %u bytes payload available\r\n",m2mMesh.payloadLeft());
      uint8_t numberOfFields = random(4,13);
      Serial.printf("\tAdding %u random fields\r\n",numberOfFields);
      for(uint8_t count=0; count<numberOfFields;count++)
      {
        switch (random(0,14)){
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
          case 7: //uint64_t
            switch(random(0,2)){
              case 0: //Single
                addUint64_t();
              break;
              case 1: //Array
                addUint64_tArray();
              break;
            }
          break;
          case 8: //int64_t
            switch(random(0,2)){
              case 0: //Single
                addInt64_t();
              break;
              case 1: //Array
                addInt64_tArray();
              break;
            }
          break;
          case 9: //float
            switch(random(0,2)){
              case 0: //Single
                addFloat();
              break;
              case 1: //Array
                addFloatArray();
              break;
            }
          break;
          case 10: //double
            switch(random(0,2)){
              case 0: //Single
                addDouble();
              break;
              case 1: //Array
                addDoubleArray();
              break;
            }
          break;
          case 11: //char
            switch(random(0,2)){
              case 0: //Single
                addChar();
              break;
              case 1: //Array
                addCharArray();
              break;
            }
          break;
          case 12: //C string
            addStr();
          break;
          case 13: //String
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
  if(m2mMesh.messageWaiting())
  {
    m2mMesh.markMessageRead();
  }
}
