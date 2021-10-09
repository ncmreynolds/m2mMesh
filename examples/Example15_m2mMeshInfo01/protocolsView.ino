void protocolsView()
{
    if(drawWholeUi)
    {
      moveToXy(80,24);
      eraseScreen();
      moveToXy(36,1);
      drawTitle(F("Protocols"));
      printAtXy(3,3, F("F - PROTOCOL_USR_FORWARD -------------- "));
      printAtXy(3,4, F("E - PROTOCOL_USR_RECEIVE -------------- "));
      printAtXy(3,5, F("D - PROTOCOL_USR_SEND ----------------- "));
      printAtXy(3,6, F("C - PROTOCOL_NHS_SUPPLY_VOLTAGE ------- "));
      printAtXy(3,7, F("B - PROTOCOL_NHS_TIME_SERVER ---------- "));
      printAtXy(3,8, F("A - PROTOCOL_NHS_INCLUDE_ORIGINATORS -- "));
      printAtXy(3,9, F("9 - PROTOCOL_NHS_FORWARD -------------- "));
      printAtXy(3,10,F("8 - PROTOCOL_NHS_RECEIVE -------------- "));
      printAtXy(3,11,F("7 - PROTOCOL_NHS_SEND ----------------- "));
      printAtXy(3,12,F("6 - PROTOCOL_OGM_FORWARD -------------- "));
      printAtXy(3,13,F("5 - PROTOCOL_OGM_RECEIVE -------------- "));
      printAtXy(3,14,F("4 - PROTOCOL_OGM_SEND ----------------- "));
      printAtXy(3,15,F("3 - PROTOCOL_ELP_INCLUDE_PEERS -------- "));
      printAtXy(3,16,F("2 - PROTOCOL_ELP_FORWARD -------------- "));
      printAtXy(3,17,F("1 - PROTOCOL_ELP_RECEIVE -------------- "));
      printAtXy(3,18,F("0 - PROTOCOL_ELP_SEND ----------------- "));
      moveToXy(1,24);
      inverseOn();Serial.print('0');inverseOff();inverseOn();Serial.print('-');Serial.print('f');inverseOff();Serial.print(F(" toggle service | "));
      Serial.print(F("Change "));inverseOn();Serial.print('v');inverseOff();Serial.print(F("iew"));
    }
    if(drawWholeUi || servicesChanged)
    {
      servicesChanged = false;
      drawWholeUi = false;
      for(int i = 0; i<16 ;i++)
      {
        moveToXy(43,18-i);
        if(m2mMesh.serviceFlags() & uint16_t(pow(2, i)))
        {
          Serial.print(F("Enabled"));
        }
        else
        {
          Serial.print(F("       "));
        }
      }
    }
}
