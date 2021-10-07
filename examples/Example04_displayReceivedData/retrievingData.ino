void retrieveBoolArray()
{
  uint8_t dataLength = m2mMesh.retrieveDataLength();
  bool tempArray[dataLength];
  m2mMesh.retrieveArray(tempArray);
  Serial.printf("\tRetrieved bool[%u] array {",dataLength);
  printArray(tempArray, dataLength);
  Serial.println('}');
}
void retrieveUint8_tArray()
{
  uint8_t dataLength = m2mMesh.retrieveDataLength();
  uint8_t tempArray[dataLength];
  m2mMesh.retrieveArray(tempArray);
  Serial.printf("\tRetrieved uint8_t[%u] array {",dataLength);
  printArray(tempArray, dataLength);
  Serial.println('}');
}
void retrieveUint16_tArray()
{
  uint8_t dataLength = m2mMesh.retrieveDataLength();
  uint16_t tempArray[dataLength];
  m2mMesh.retrieveArray(tempArray);
  Serial.printf("\tRetrieved uint16_t[%u] array {",dataLength);
  printArray(tempArray, dataLength);
  Serial.println('}');
}
void retrieveUint32_tArray()
{
  uint8_t dataLength = m2mMesh.retrieveDataLength();
  uint32_t tempArray[dataLength];
  m2mMesh.retrieveArray(tempArray);
  Serial.printf("\tRetrieved uint32_t[%u] array {",dataLength);
  printArray(tempArray, dataLength);
  Serial.println('}');
}
void retrieveUint64_tArray()
{
  uint8_t dataLength = m2mMesh.retrieveDataLength();
  uint64_t tempArray[dataLength];
  m2mMesh.retrieveArray(tempArray);
  Serial.printf("\tRetrieved uint64_t[%u] array {",dataLength);
  printArray(tempArray, dataLength);
  Serial.println('}');
}
void retrieveInt8_tArray()
{
  uint8_t dataLength = m2mMesh.retrieveDataLength();
  int8_t tempArray[dataLength];
  m2mMesh.retrieveArray(tempArray);
  Serial.printf("\tRetrieved int8_t[%d] array {",dataLength);
  printArray(tempArray, dataLength);
  Serial.println('}');
}
void retrieveInt16_tArray()
{
  uint8_t dataLength = m2mMesh.retrieveDataLength();
  int16_t tempArray[dataLength];
  m2mMesh.retrieveArray(tempArray);
  Serial.printf("\tRetrieved int16_t[%d] array {",dataLength);
  printArray(tempArray, dataLength);
  Serial.println('}');
}
void retrieveInt32_tArray()
{
  uint8_t dataLength = m2mMesh.retrieveDataLength();
  int32_t tempArray[dataLength];
  m2mMesh.retrieveArray(tempArray);
  Serial.printf("\tRetrieved int32_t[%d] array {",dataLength);
  printArray(tempArray, dataLength);
  Serial.println('}');
}
void retrieveInt64_tArray()
{
  uint8_t dataLength = m2mMesh.retrieveDataLength();
  int64_t tempArray[dataLength];
  m2mMesh.retrieveArray(tempArray);
  Serial.printf("\tRetrieved uint64_t[%d] array {",dataLength);
  printArray(tempArray, dataLength);
  Serial.println('}');
}
void retrieveFloatArray()
{
  uint8_t dataLength = m2mMesh.retrieveDataLength();
  float tempArray[dataLength];
  m2mMesh.retrieveArray(tempArray);
  Serial.printf("\tRetrieved float[%d] array {",dataLength);
  printArray(tempArray, dataLength);
  Serial.println('}');
}
void retrieveDoubleArray()
{
  uint8_t dataLength = m2mMesh.retrieveDataLength();
  double tempArray[dataLength];
  m2mMesh.retrieveArray(tempArray);
  Serial.printf("\tRetrieved double[%d] array {",dataLength);
  printArray(tempArray, dataLength);
  Serial.println('}');
}
void retrieveCharArray()
{
  uint8_t dataLength = m2mMesh.retrieveDataLength();
  char tempArray[dataLength];
  m2mMesh.retrieveArray(tempArray);
  Serial.printf("\tRetrieved char[%d] array {",dataLength);
  printArray(tempArray, dataLength);
  Serial.println('}');
}
void retrieveStr()
{
  uint8_t dataLength = m2mMesh.retrieveDataLength();  //NB this length INCLUDES the null termination!
  char tempArray[dataLength];
  m2mMesh.retrieveStr(tempArray); //NB this will be forced to be null terminated!
  Serial.printf("\tRetrieved C string[%u]:\"%s\"\r\n",dataLength,tempArray);
}
