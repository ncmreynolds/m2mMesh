void addBool()
{
  bool dataToAdd;
  if(random(0,255)>127)
  {
    dataToAdd = true;
  }
  else
  {
    dataToAdd = false;
  }
  if(m2mMesh.add(dataToAdd))
  {
    if(dataToAdd == true)
    {
      Serial.printf("\tAdded bool data:true %u bytes payload left\r\n", m2mMesh.payloadLeft());
    }
    else
    {
      Serial.printf("\tAdded bool data:false %u bytes payload left\r\n", m2mMesh.payloadLeft());
    }
  }
  else
  {
    if(dataToAdd == true)
    {
      Serial.println("\tFailed to add bool data:true");
    }
    else
    {
      Serial.println("\tFailed to add bool data:false");
    }
  }
}
void addBoolArray()
{
  uint8_t arrayLength = random(minimumArrayLength,maximumArrayLength);
  bool dataToAdd[arrayLength];
  for(uint8_t index = 0; index < arrayLength; index++)
  {
    if(random(0,255)>127)
    {
      dataToAdd[index] = true;
    }
    else
    {
      dataToAdd[index] = false;
    }
  }
  if(m2mMesh.add(dataToAdd, arrayLength))
  {
    Serial.printf("\tAdded bool[%u] array data {",arrayLength);
    printArray(dataToAdd, arrayLength);
    Serial.printf("} %u bytes payload left\r\n", m2mMesh.payloadLeft());
  }
  else
  {
    Serial.printf("\tFailed to add bool array data\r\n");
  }
}
void addUint8_t()
{
  uint8_t dataToAdd = random(0,255);
  if(m2mMesh.add(dataToAdd))
  {
    Serial.printf("\tAdded uint8_t data:%u %u bytes payload left\r\n",dataToAdd, m2mMesh.payloadLeft());
  }
  else
  {
    Serial.printf("\tFailed to add uint8_t data:%u\r\n",dataToAdd);
  }
}
void addUint8_tArray()
{
  uint8_t arrayLength = random(minimumArrayLength,maximumArrayLength);
  uint8_t dataToAdd[arrayLength];
  for(uint8_t index = 0; index < arrayLength; index++)
  {
    dataToAdd[index] = index;
  }
  if(m2mMesh.add(dataToAdd, arrayLength))
  {
    Serial.printf("\tAdded uint8_t[%u] array data {",arrayLength);
    printArray(dataToAdd, arrayLength);
    Serial.printf("} %u bytes payload left\r\n", m2mMesh.payloadLeft());
  }
  else
  {
    Serial.printf("\tFailed to add uint8_t array data\r\n");
  }
}
void addInt8_t()
{
  int8_t dataToAdd = random(-127,127);
  if(m2mMesh.add(dataToAdd))
  {
    Serial.printf("\tAdded int8_t data:%i %u bytes payload left\r\n",dataToAdd, m2mMesh.payloadLeft());
  }
  else
  {
    Serial.printf("\tFailed to add int8_t data:%i\r\n",dataToAdd);
  }
}
void addInt8_tArray()
{
  uint8_t arrayLength = random(minimumArrayLength,maximumArrayLength);
  int8_t dataToAdd[arrayLength];
  for(uint8_t index = 0; index < arrayLength; index++)
  {
    dataToAdd[index] = -index;
  }
  if(m2mMesh.add(dataToAdd, arrayLength))
  {
    Serial.printf("\tAdded int8_t[%u] array data {",arrayLength);
    printArray(dataToAdd, arrayLength);
    Serial.printf("} %u bytes payload left\r\n", m2mMesh.payloadLeft());
  }
  else
  {
    Serial.printf("\tFailed to add uint16_t array data\r\n");
  }
}
void addUint16_t()
{
  uint16_t dataToAdd = random(0,65535);
  if(m2mMesh.add(dataToAdd))
  {
    Serial.printf("\tAdded uint16_t data:%u %u bytes payload left\r\n",dataToAdd, m2mMesh.payloadLeft());
  }
  else
  {
    Serial.printf("\tFailed to add uint16_t data:%u\r\n",dataToAdd);
  }
}
void addUint16_tArray()
{
  uint8_t arrayLength = random(minimumArrayLength,maximumArrayLength);
  uint16_t dataToAdd[arrayLength];
  for(uint8_t index = 0; index < arrayLength; index++)
  {
    dataToAdd[index] = index;
  }
  if(m2mMesh.add(dataToAdd, arrayLength))
  {
    Serial.printf("\tAdded uint16_t[%u] array data {",arrayLength);
    printArray(dataToAdd, arrayLength);
    Serial.printf("} %u bytes payload left\r\n", m2mMesh.payloadLeft());
  }
  else
  {
    Serial.printf("\tFailed to add uint16_t array data\r\n");
  }
}
void addInt16_t()
{
  int16_t dataToAdd = random(-32767,32767);
  if(m2mMesh.add(dataToAdd))
  {
    Serial.printf("\tAdded int16_t data:%i %u bytes payload left\r\n",dataToAdd, m2mMesh.payloadLeft());
  }
  else
  {
    Serial.printf("\tFailed to add int16_t data:%i\r\n",dataToAdd);
  }
}
void addInt16_tArray()
{
  uint8_t arrayLength = random(minimumArrayLength,maximumArrayLength);
  int16_t dataToAdd[arrayLength];
  for(uint8_t index = 0; index < arrayLength; index++)
  {
    dataToAdd[index] = -index;
  }
  if(m2mMesh.add(dataToAdd, arrayLength))
  {
    Serial.printf("\tAdded int16_t[%u] array data {",arrayLength);
    printArray(dataToAdd, arrayLength);
    Serial.printf("} %u bytes payload left\r\n", m2mMesh.payloadLeft());
  }
  else
  {
    Serial.printf("\tFailed to add int16_t array data\r\n");
  }
}
void addUint32_t()
{
  uint32_t dataToAdd = (random(0,65535)*32768+random(0,65535));
  if(m2mMesh.add(dataToAdd))
  {
    Serial.printf("\tAdded uint32_t data:%u %u bytes payload left\r\n",dataToAdd, m2mMesh.payloadLeft());
  }
  else
  {
    Serial.printf("\tFailed to add uint32_t data:%u\r\n",dataToAdd);
  }
}
void addUint32_tArray()
{
  uint8_t arrayLength = random(minimumArrayLength,maximumArrayLength);
  uint32_t dataToAdd[arrayLength];
  for(uint8_t index = 0; index < arrayLength; index++)
  {
    dataToAdd[index] = index;
  }
  if(m2mMesh.add(dataToAdd, arrayLength))
  {
    Serial.printf("\tAdded uint32_t[%u] array data {",arrayLength);
    printArray(dataToAdd, arrayLength);
    Serial.printf("} %u bytes payload left\r\n", m2mMesh.payloadLeft());
  }
  else
  {
    Serial.printf("\tFailed to add uint32_t array data\r\n");
  }
}
void addInt32_t()
{
  int32_t dataToAdd = (random(-32767,32768))*32768+random(-32767,32768);
  if(m2mMesh.add(dataToAdd))
  {
    Serial.printf("\tAdded int32_t data:%i %u bytes payload left\r\n",dataToAdd, m2mMesh.payloadLeft());
  }
  else
  {
    Serial.printf("\tFailed to add int32_t data:%i\r\n",dataToAdd);
  }
}
void addInt32_tArray()
{
  uint8_t arrayLength = random(minimumArrayLength,maximumArrayLength);
  int32_t dataToAdd[arrayLength];
  for(uint8_t index = 0; index < arrayLength; index++)
  {
    dataToAdd[index] = -index;
  }
  if(m2mMesh.add(dataToAdd, arrayLength))
  {
    Serial.printf("\tAdded int32_t[%u] array data {",arrayLength);
    printArray(dataToAdd, arrayLength);
    Serial.printf("} %u bytes payload left\r\n", m2mMesh.payloadLeft());
  }
  else
  {
    Serial.printf("\tFailed to add int32_t array data\r\n");
  }
}
void addUint64_t()
{
  uint32_t dataToAdd = (random(0,65535)*32768+random(0,65535));
  if(m2mMesh.add(dataToAdd))
  {
    Serial.printf("\tAdded uint64_t data:%u %u bytes payload left\r\n",dataToAdd, m2mMesh.payloadLeft());
  }
  else
  {
    Serial.printf("\tFailed to add uint64_t data:%u\r\n",dataToAdd);
  }
}
void addUint64_tArray()
{
  uint8_t arrayLength = random(minimumArrayLength,maximumArrayLength);
  uint64_t dataToAdd[arrayLength];
  for(uint8_t index = 0; index < arrayLength; index++)
  {
    dataToAdd[index] = index;
  }
  if(m2mMesh.add(dataToAdd, arrayLength))
  {
    Serial.printf("\tAdded uint64_t[%u] array data {",arrayLength);
    printArray(dataToAdd, arrayLength);
    Serial.printf("} %u bytes payload left\r\n", m2mMesh.payloadLeft());
  }
  else
  {
    Serial.printf("\tFailed to add uint64_t array data\r\n");
  }
}
void addInt64_t()
{
  int64_t dataToAdd = (random(-32767,32768))*32768+random(-32767,32768);
  if(m2mMesh.add(dataToAdd))
  {
    Serial.printf("\tAdded int64_t data:%i %u bytes payload left\r\n",dataToAdd, m2mMesh.payloadLeft());
  }
  else
  {
    Serial.printf("\tFailed to add int64_t data:%i\r\n",dataToAdd);
  }
}
void addInt64_tArray()
{
  uint8_t arrayLength = random(minimumArrayLength,maximumArrayLength);
  int64_t dataToAdd[arrayLength];
  for(uint8_t index = 0; index < arrayLength; index++)
  {
    dataToAdd[index] = -index;
  }
  if(m2mMesh.add(dataToAdd, arrayLength))
  {
    Serial.printf("\tAdded int64_t[%u] array data {",arrayLength);
    printArray(dataToAdd, arrayLength);
    Serial.printf("} %u bytes payload left\r\n", m2mMesh.payloadLeft());
  }
  else
  {
    Serial.printf("\tFailed to add int32_t array data\r\n");
  }
}
void addFloat()
{
  float dataToAdd = (random(-32767,32768))*3.1415927;
  if(m2mMesh.add(dataToAdd))
  {
    Serial.printf("\tAdded float data:%f %u bytes payload left\r\n",dataToAdd, m2mMesh.payloadLeft());
  }
  else
  {
    Serial.printf("\tFailed to add float data:%f\r\n",dataToAdd);
  }
}
void addFloatArray()
{
  uint8_t arrayLength = random(minimumArrayLength,maximumArrayLength);
  float dataToAdd[arrayLength];
  for(uint8_t index = 0; index < arrayLength; index++)
  {
    dataToAdd[index] = float(index)/10.0;
  }
  if(m2mMesh.add(dataToAdd, arrayLength))
  {
    Serial.printf("\tAdded float[%u] array data {",arrayLength);
    printArray(dataToAdd, arrayLength);
    Serial.printf("} %u bytes payload left\r\n", m2mMesh.payloadLeft());
  }
  else
  {
    Serial.printf("\tFailed to add float array data\r\n");
  }
}
void addDouble()
{
  double dataToAdd = (random(-32767,32768))*3.1415927;
  if(m2mMesh.add(dataToAdd))
  {
    Serial.printf("\tAdded double data:%f %u bytes payload left\r\n",dataToAdd, m2mMesh.payloadLeft());
  }
  else
  {
    Serial.printf("\tFailed to add double data:%f\r\n",dataToAdd);
  }
}
void addDoubleArray()
{
  uint8_t arrayLength = random(minimumArrayLength,maximumArrayLength);
  double dataToAdd[arrayLength];
  for(uint8_t index = 0; index < arrayLength; index++)
  {
    dataToAdd[index] = double(index)/10.0;
  }
  if(m2mMesh.add(dataToAdd, arrayLength))
  {
    Serial.printf("\tAdded double[%u] array data {",arrayLength);
    printArray(dataToAdd, arrayLength);
    Serial.printf("} %u bytes payload left\r\n", m2mMesh.payloadLeft());
  }
  else
  {
    Serial.printf("\tFailed to add double array data\r\n");
  }
}
void addChar()
{
  char dataToAdd = char(random(32,127));
  if(m2mMesh.add(dataToAdd))
  {
    Serial.printf("\tAdded char data:\"%c\" %u bytes payload left\r\n",dataToAdd, m2mMesh.payloadLeft());
  }
  else
  {
    Serial.printf("\tFailed to add char data:\"%c\"\r\n",dataToAdd);
  }
}
void addCharArray()
{
  uint8_t arrayLength = random(minimumArrayLength,maximumArrayLength);
  char dataToAdd[arrayLength];
  for(uint8_t index = 0; index < arrayLength; index++)
  {
    dataToAdd[index] = randomCharacter();
  }
  if(m2mMesh.add(dataToAdd,arrayLength))
  {
    Serial.printf("\tAdded char[%u] array data {",arrayLength);
    printArray(dataToAdd, arrayLength);
    Serial.printf("} %u bytes payload left\r\n", m2mMesh.payloadLeft());
  }
  else
  {
    Serial.printf("\tFailed to add array data:\"%s\"\r\n",dataToAdd);
  }
}
void addString()
{
  String dataToAdd = "Test Arduino String";
  if(m2mMesh.add(dataToAdd))
  {
    Serial.printf("\tAdded String data:\"%s\" %u bytes payload left\r\n",(dataToAdd).c_str(), m2mMesh.payloadLeft());
  }
  else
  {
    Serial.printf("\tFailed to add String data:\"%s\"\r\n",(dataToAdd).c_str());
  }
}
void addStr()
{
  char dataToAdd[] = "Test C string (char array)";
  if(m2mMesh.add(dataToAdd))
  {
    Serial.printf("\tAdded C string data[%u]:\"%s\" %u bytes payload left\r\n",strlen(dataToAdd),dataToAdd, m2mMesh.payloadLeft());
  }
  else
  {
    Serial.printf("\tFailed to add C string data[%u]:\"%s\"\r\n",strlen(dataToAdd),dataToAdd);
  }
}
char randomCharacter()
{
  switch(random(0,3))
  {
    case 0:
      return(char(random(65,91)));
    break;
    case 1:
      return(char(random(97,123)));
    break;
    case 2:
      return(char(random(48,58)));
    break;
  }
}
