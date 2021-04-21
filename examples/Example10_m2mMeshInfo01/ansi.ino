/*
 * This file contains a number of functions for handling ANSI control codes
 * graphics colours and so on. It's a bit quick and dirty and not a proper
 * library of any kind.
 * 
 * 
 */

void ICACHE_FLASH_ATTR hideCursor()
{
  Serial.printf("\033[?25l");
}

void ICACHE_FLASH_ATTR eraseScreen()
{
  Serial.printf("\033[1J");
}

void ICACHE_FLASH_ATTR eraseLine()
{
  Serial.printf("\033[2K");
}

void ICACHE_FLASH_ATTR moveToXy(uint8_t x, uint8_t y)
{
  Serial.printf("\033[");
  Serial.print(y,DEC);
  Serial.write(';');
  Serial.print(x, DEC);
  Serial.write('H');
}

void ICACHE_FLASH_ATTR inverseOn()
{
  Serial.printf("\033[");
  Serial.print(7,DEC);
  Serial.write('m');
}

void ICACHE_FLASH_ATTR inverseOff()
{
  Serial.printf("\033[");
  Serial.print(27,DEC);
  Serial.write('m');
}


void ICACHE_FLASH_ATTR printAtXy(uint8_t x, uint8_t y, String textToPrint)
{
  moveToXy(x,y);
  Serial.print(textToPrint);
}

void ICACHE_FLASH_ATTR printUptimeAtXy(uint8_t x, uint8_t y, uint32_t uptime)
{
  uint8_t seconds = (uptime/   1000ul)%60;
  uint8_t minutes = (uptime/  60000ul)%60;
  uint8_t hours =   (uptime/3600000ul);
  moveToXy(x,y);
  if(hours>0)
  {
    Serial.printf("%03dh%02dm%02ds",hours,minutes,seconds);
  }
  else if(minutes>0)
  {
    Serial.printf("%02dm%02ds",minutes,seconds);
  }
  else
  {
    Serial.printf("%02ds",seconds);
  }
}

void ICACHE_FLASH_ATTR setScrollWindow(uint8_t top, uint8_t bottom)
{
  Serial.printf("\033[%03d;%03dr",top,bottom);
}
void ICACHE_FLASH_ATTR popupBox(String text)
{
  uint8_t len = text.length();
  moveToXy(39-(len/2),12);
  Serial.write('|');
  Serial.print(text);
  Serial.write('|');
  moveToXy(39-(len/2),11);
  Serial.write('O');
  for(uint8_t i=0;i<len;i++)
  {
    Serial.write('-');
  }
  Serial.write('O');
  moveToXy(39-(len/2),13);
  Serial.write('O');
  for(uint8_t i=0;i<len;i++)
  {
    Serial.write('-');
  }
  Serial.write('O');
}
void ICACHE_FLASH_ATTR drawTitle(String text)
{
  moveToXy(40-(text.length()/2),1);
  Serial.print(text);
}
void ICACHE_FLASH_ATTR printCentred(uint8_t row,String text)
{
  moveToXy(40-(text.length()/2),row);
  Serial.print(text);
}
void ICACHE_FLASH_ATTR packetType(char *desc, uint8_t type)
{
  switch (type)
  {
    case 0x00:
      desc[0] = 'E';
      desc[1] = 'L';
      desc[2] = 'P';
    break;
    case 0x01:
      desc[0] = 'O';
      desc[1] = 'G';
      desc[2] = 'M';
    break;
    case 0x02:
      desc[0] = 'N';
      desc[1] = 'H';
      desc[2] = 'S';
    break;
    case 0x03:
      desc[0] = 'U';
      desc[1] = 'S';
      desc[2] = 'R';
    break;
  }
}
