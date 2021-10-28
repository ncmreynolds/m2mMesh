void displayMessage()
{
  while(m2mMesh.dataAvailable() > 0)
  {
    switch (m2mMesh.nextDataType()){
      case m2mMesh.USR_DATA_BOOL:
        if(m2mMesh.retrieveBool() == true)
        {
          Serial.println("\r\n\tRetrieved bool:true");
        }
        else
        {
          Serial.println("\r\n\tRetrieved bool:false");
        }
      break;
      case m2mMesh.USR_DATA_BOOL_ARRAY:
        retrieveBoolArray();
      break;
      case m2mMesh.USR_DATA_UINT8_T:
        Serial.printf("\r\n\tRetrieved uint8_t:%u",m2mMesh.retrieveUint8_t());
      break;
      case m2mMesh.USR_DATA_UINT8_T_ARRAY:
        retrieveUint8_tArray();
      break;
      case m2mMesh.USR_DATA_INT8_T:
        Serial.printf("\r\n\tRetrieved int8_t:%i",m2mMesh.retrieveInt8_t());
      break;
      case m2mMesh.USR_DATA_INT8_T_ARRAY:
        retrieveInt8_tArray();
      break;
      case m2mMesh.USR_DATA_UINT16_T:
        Serial.printf("\r\n\tRetrieved uint16_t:%u",m2mMesh.retrieveUint16_t());
      break;
      case m2mMesh.USR_DATA_UINT16_T_ARRAY:
        retrieveUint16_tArray();
      break;
      case m2mMesh.USR_DATA_INT16_T:
        Serial.printf("\r\n\tRetrieved int16_t:%i",m2mMesh.retrieveInt16_t());
      break;
      case m2mMesh.USR_DATA_INT16_T_ARRAY:
        retrieveInt16_tArray();
      break;
      case m2mMesh.USR_DATA_UINT32_T:
        Serial.printf("\r\n\tRetrieved uint32_t:%u",m2mMesh.retrieveUint32_t());
      break;
      case m2mMesh.USR_DATA_UINT32_T_ARRAY:
        retrieveUint32_tArray();
      break;
      case m2mMesh.USR_DATA_INT32_T:
        Serial.printf("\r\n\tRetrieved int32_t:%i",m2mMesh.retrieveInt32_t());
      break;
      case m2mMesh.USR_DATA_INT32_T_ARRAY:
        retrieveInt32_tArray();
      break;
      case m2mMesh.USR_DATA_UINT64_T:
        Serial.printf("\r\n\tRetrieved uint64_t:%u",m2mMesh.retrieveUint64_t());
      break;
      case m2mMesh.USR_DATA_UINT64_T_ARRAY:
        retrieveUint64_tArray();
      break;
      case m2mMesh.USR_DATA_INT64_T:
        Serial.printf("\r\n\tRetrieved int64_t:%i",m2mMesh.retrieveInt64_t());
      break;
      case m2mMesh.USR_DATA_INT64_T_ARRAY:
        retrieveInt64_tArray();
      break;
      case m2mMesh.USR_DATA_FLOAT:
        Serial.printf("\r\n\tRetrieved float:%f",m2mMesh.retrieveFloat());
      break;
      case m2mMesh.USR_DATA_FLOAT_ARRAY:
        retrieveFloatArray();
      break;
      case m2mMesh.USR_DATA_DOUBLE:
        Serial.printf("\r\n\tRetrieved double:%f",m2mMesh.retrieveDouble());
      break;
      case m2mMesh.USR_DATA_DOUBLE_ARRAY:
        retrieveDoubleArray();
      break;
      case m2mMesh.USR_DATA_CHAR:
        Serial.printf("\r\n\tRetrieved char:\"%c\"",m2mMesh.retrieveChar());
      break;
      case m2mMesh.USR_DATA_CHAR_ARRAY:
        retrieveCharArray();
      break;
      case m2mMesh.USR_DATA_STR:
        retrieveStr();
      break;
      case m2mMesh.USR_DATA_STRING:
        Serial.printf("\r\n\tRetrieved String:\"%s\"",(m2mMesh.retrieveString()).c_str());
      break;
      case m2mMesh.USR_DATA_UNAVAILABLE:
        Serial.println("\r\n\tData unavailable");
      break;
      default : //Just in case an unhandled data type arrives
        Serial.printf("\r\n\tUnhandled data type %02x",m2mMesh.nextDataType());
        m2mMesh.skipRetrieve();
      break;
    }
  }
}
void retrieveBoolArray()
{
  uint8_t dataLength = m2mMesh.retrieveDataLength();
  bool tempArray[dataLength];
  m2mMesh.retrieveArray(tempArray);
  Serial.printf("\r\n\tRetrieved bool[%u] array {",dataLength);
  printArray(tempArray, dataLength);
  Serial.print('}');
}
void retrieveUint8_tArray()
{
  uint8_t dataLength = m2mMesh.retrieveDataLength();
  uint8_t tempArray[dataLength];
  m2mMesh.retrieveArray(tempArray);
  Serial.printf("\r\n\tRetrieved uint8_t[%u] array {",dataLength);
  printArray(tempArray, dataLength);
  Serial.print('}');
}
void retrieveUint16_tArray()
{
  uint8_t dataLength = m2mMesh.retrieveDataLength();
  uint16_t tempArray[dataLength];
  m2mMesh.retrieveArray(tempArray);
  Serial.printf("\r\n\tRetrieved uint16_t[%u] array {",dataLength);
  printArray(tempArray, dataLength);
  Serial.print('}');
}
void retrieveUint32_tArray()
{
  uint8_t dataLength = m2mMesh.retrieveDataLength();
  uint32_t tempArray[dataLength];
  m2mMesh.retrieveArray(tempArray);
  Serial.printf("\r\n\tRetrieved uint32_t[%u] array {",dataLength);
  printArray(tempArray, dataLength);
  Serial.print('}');
}
void retrieveUint64_tArray()
{
  uint8_t dataLength = m2mMesh.retrieveDataLength();
  uint64_t tempArray[dataLength];
  m2mMesh.retrieveArray(tempArray);
  Serial.printf("\r\n\tRetrieved uint64_t[%u] array {",dataLength);
  printArray(tempArray, dataLength);
  Serial.print('}');
}
void retrieveInt8_tArray()
{
  uint8_t dataLength = m2mMesh.retrieveDataLength();
  int8_t tempArray[dataLength];
  m2mMesh.retrieveArray(tempArray);
  Serial.printf("\r\n\tRetrieved int8_t[%d] array {",dataLength);
  printArray(tempArray, dataLength);
  Serial.print('}');
}
void retrieveInt16_tArray()
{
  uint8_t dataLength = m2mMesh.retrieveDataLength();
  int16_t tempArray[dataLength];
  m2mMesh.retrieveArray(tempArray);
  Serial.printf("\r\n\tRetrieved int16_t[%d] array {",dataLength);
  printArray(tempArray, dataLength);
  Serial.print('}');
}
void retrieveInt32_tArray()
{
  uint8_t dataLength = m2mMesh.retrieveDataLength();
  int32_t tempArray[dataLength];
  m2mMesh.retrieveArray(tempArray);
  Serial.printf("\r\n\tRetrieved int32_t[%d] array {",dataLength);
  printArray(tempArray, dataLength);
  Serial.print('}');
}
void retrieveInt64_tArray()
{
  uint8_t dataLength = m2mMesh.retrieveDataLength();
  int64_t tempArray[dataLength];
  m2mMesh.retrieveArray(tempArray);
  Serial.printf("\r\n\tRetrieved uint64_t[%d] array {",dataLength);
  printArray(tempArray, dataLength);
  Serial.print('}');
}
void retrieveFloatArray()
{
  uint8_t dataLength = m2mMesh.retrieveDataLength();
  float tempArray[dataLength];
  m2mMesh.retrieveArray(tempArray);
  Serial.printf("\r\n\tRetrieved float[%d] array {",dataLength);
  printArray(tempArray, dataLength);
  Serial.print('}');
}
void retrieveDoubleArray()
{
  uint8_t dataLength = m2mMesh.retrieveDataLength();
  double tempArray[dataLength];
  m2mMesh.retrieveArray(tempArray);
  Serial.printf("\r\n\tRetrieved double[%d] array {",dataLength);
  printArray(tempArray, dataLength);
  Serial.print('}');
}
void retrieveCharArray()
{
  uint8_t dataLength = m2mMesh.retrieveDataLength();
  char tempArray[dataLength];
  m2mMesh.retrieveArray(tempArray);
  Serial.printf("\r\n\tRetrieved char[%d] array {",dataLength);
  printArray(tempArray, dataLength);
  Serial.print('}');
}
void retrieveStr()
{
  uint8_t dataLength = m2mMesh.retrieveDataLength();  //NB this length INCLUDES the null termination!
  char tempArray[dataLength];
  m2mMesh.retrieveStr(tempArray); //NB this will be forced to be null terminated!
  Serial.printf("\r\n\tRetrieved C string[%u]:\"%s\"",dataLength,tempArray);
}
void printArray(uint8_t* dataToPrint)
{
  uint8_t numberOfItems = sizeof(dataToPrint)/sizeof(dataToPrint[0]);
  for(uint8_t index = 0; index < numberOfItems; index++)
  {
    Serial.print(dataToPrint[index]);
    if(index < numberOfItems - 1)
    {
      Serial.print(' ');
    }
  }
}
template<typename arrayType>
void printArray(arrayType* dataToPrint, uint8_t arrayLength)
{
  for(uint8_t index = 0; index < arrayLength; index++)
  {
    Serial.print(dataToPrint[index]);
    if(index < arrayLength - 1)
    {
      Serial.print(' ');
    }
  }
}
