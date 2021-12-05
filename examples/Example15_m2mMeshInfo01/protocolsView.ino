void protocolsView()
{
    if(drawWholeUi)
    {
      moveToXy(80,24);
      eraseScreen();
      drawTopLine();
      printAtXy(35,4, F("Protocols"));
      printAtXy(3,7,F  ("F - PROTOCOL_PING_RESPONDER ----------- "));
      printAtXy(3,8,F  ("E - PROTOCOL_USR_FORWARD -------------- "));
      printAtXy(3,9,F  ("D - PROTOCOL_USR_RECEIVE -------------- "));
      printAtXy(3,10,F ("C - PROTOCOL_USR_SEND ----------------- "));
      printAtXy(3,11,F ("B - PROTOCOL_NHS_TIME_SERVER ---------- "));
      printAtXy(3,12,F ("A - PROTOCOL_NHS_INCLUDE_ORIGINATORS -- "));
      printAtXy(3,13,F ("9 - PROTOCOL_NHS_FORWARD -------------- "));
      printAtXy(3,14,F ("8 - PROTOCOL_NHS_RECEIVE -------------- "));
      printAtXy(3,15,F ("7 - PROTOCOL_NHS_SEND ----------------- "));
      printAtXy(3,16,F ("6 - PROTOCOL_OGM_FORWARD -------------- "));
      printAtXy(3,17,F ("5 - PROTOCOL_OGM_RECEIVE -------------- "));
      printAtXy(3,18,F ("4 - PROTOCOL_OGM_SEND ----------------- "));
      printAtXy(3,19,F ("3 - PROTOCOL_ELP_INCLUDE_PEERS -------- "));
      printAtXy(3,20,F ("2 - PROTOCOL_ELP_FORWARD -------------- "));
      printAtXy(3,21,F ("1 - PROTOCOL_ELP_RECEIVE -------------- "));
      printAtXy(3,22,F ("0 - PROTOCOL_ELP_SEND ----------------- "));
      moveToXy(1,24);
      Serial.print(F("[0]-[f] toggle service | "));
      Serial.print(F("[Tab] view"));
    }
    if(drawWholeUi || servicesChanged)
    {
      servicesChanged = false;
      drawWholeUi = false;
      for(int i = 0; i<16 ;i++)
      {
        moveToXy(43,22-i);
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
