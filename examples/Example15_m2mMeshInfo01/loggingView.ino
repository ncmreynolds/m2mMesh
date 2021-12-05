void loggingView()
{    
    if(drawWholeUi)
    {
      moveToXy(80,24);
      eraseScreen();
      drawTopLine();
      if(logAllNodes)
      {
        printAtXy(30,3,F("Logging - all nodes"));
      }
      else
      {
        uint8_t originatorMac[6]; //temporary MAC address
        m2mMesh.macAddress(nodeToLog,originatorMac);
        char titleString[] = "Logging node 000/00:00:00:00:00:00 only";
        sprintf(titleString,"Logging node %03d/%02x:%02x:%02x:%02x:%02x:%02x only",nodeToLog,originatorMac[0],originatorMac[1],originatorMac[2],originatorMac[3],originatorMac[4],originatorMac[5]);
        printAtXy(20,3,titleString);
      }
      moveToXy(1,24);
      Serial.print(F("[Space] pause | "));
      Serial.print(F("[+]/[-] log next/previous node | "));
      Serial.print(F("[Tab] view | "));
      Serial.print(F("[c] channel"));
      setScrollWindow(4, 23);
      drawWholeUi = false;
    }
}
