void loggingLevelView()
{
    if(drawWholeUi)
    {
      moveToXy(80,24);
      eraseScreen();
      drawTopLine();
      printAtXy(30,4,"Logging configuration");
      printAtXy(39, 7, F("NODE TO LOG: "));

      printAtXy(39, 9,F("V - ALL_SENT_PACKETS ------ "));
      printAtXy(39,10,F("U - ALL_RECEIVED_PACKETS -- "));
      printAtXy(39,11,F("T - WIFI_POWER_MANAGEMENT - "));
      printAtXy(39,12,F("S - PACKET ROUTING -------- "));
      printAtXy(39,13,F("R - BUFFER_MANAGEMENT------ "));
      printAtXy(39,14,F("Q - ESP-NOW EVENTS -------- "));
      printAtXy(39,15,F("P - NODE MANAGEMENT ------- "));
      printAtXy(39,16,F("O - PEER MANAGEMENT ------- "));

      printAtXy(39,17,F("N - BTP_SEND -------------- "));
      printAtXy(39,18,F("M - BTP_RECEIVED ---------- "));
      printAtXy(39,19,F("L - BTP_FORWARDING -------- "));

      printAtXy(39,20,F("K - USER_DATA_SEND -------- "));
      printAtXy(39,21,F("J - USER_DATA_RECEIVED ---- "));
      printAtXy(39,22,F("I - USER_DATA_FORWARDING -- "));

      printAtXy(3 , 5,F("H - TRC_SEND -------------  "));
      printAtXy(3 , 6,F("G - TRC_RECEIVED ---------  "));
      printAtXy(3 , 7,F("F - TRC_FORWARDING -------  "));

      printAtXy(3 , 8,F("E - PRP_SEND -------------  "));
      printAtXy(3 , 9,F("D - PRP_RECEIVED ---------  "));
      printAtXy(3 ,10,F("C - PRP_FORWARDING -------  "));

      printAtXy(3 ,11,F("B - NHS_SEND -------------  "));
      printAtXy(3 ,12,F("A - NHS_RECEIVED ---------  "));
      printAtXy(3 ,13,F("9 - NHS_FORWARDING -------  "));

      printAtXy(3 ,14,F("8 - OGM_SEND -------------  "));
      printAtXy(3 ,15,F("7 - OGM_RECEIVED ---------  "));
      printAtXy(3 ,16,F("6 - OGM_FORWARDING -------  "));

      printAtXy(3 ,17,F("5 - ELP_SEND -------------  "));
      printAtXy(3 ,18,F("4 - ELP_RECEIVED ---------  "));
      printAtXy(3 ,19,F("3 - ELP_FORWARDING -------  "));

      printAtXy(3 ,20,F("2 - INFORMATION-----------  "));
      printAtXy(3 ,21,F("1 - WARNINGS -------------  "));
      printAtXy(3 ,22,F("0 - ERRORS ---------------  "));
      moveToXy(1,24);
      Serial.print(F("[0]-[v] toggle logging | "));
      Serial.print(F("[+]/[-] log next/previous node | "));
      Serial.print(F("[Tab] view"));
    }
    if(drawWholeUi || loggingLevelChanged == true)
    {
      loggingLevelChanged = false;
      drawWholeUi = false;
      if(logAllNodes)
      {
        printAtXy(51,7, F("All                 "));
      }
      else
      {
        uint8_t originatorMac[6]; //temporary MAC address
        m2mMesh.macAddress(nodeToLog,originatorMac); //retrieve MAC address
        moveToXy(51,7);
        Serial.printf("%02x/%02x:%02x:%02x:%02x:%02x:%02x",nodeToLog,originatorMac[0],originatorMac[1],originatorMac[2],originatorMac[3],originatorMac[4],originatorMac[5]);
      }
      for(uint8_t i = 0; i<32 ;i++)
      {
        if(i<18)
        {
          moveToXy(31,22-i);
        }
        else
        {
          moveToXy(67,22-(i%18));
        }
        if(loggingLevel & uint32_t(pow(2, i)))
        {
          Serial.print(F("Logging"));
        }
        else
        {
          Serial.print(F("       "));
        }
      }
    }
}
