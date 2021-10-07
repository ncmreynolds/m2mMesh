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
