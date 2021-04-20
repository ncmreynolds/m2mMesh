void loggingView()
{    
    if(drawWholeUi)
    {
      moveToXy(80,24);
      eraseScreen();
      //moveToXy(36,1);
      if(logAllNodes)
      {
        drawTitle(F("Logging - all nodes"));
      }
      else
      {
        uint8_t originatorMac[6]; //temporary MAC address
        mesh.macAddress(nodeToLog,originatorMac);
        char titleString[] = "Logging node 000/00:00:00:00:00:00 only";
        sprintf(titleString,"Logging node %03d/%02x:%02x:%02x:%02x:%02x:%02x only",nodeToLog,originatorMac[0],originatorMac[1],originatorMac[2],originatorMac[3],originatorMac[4],originatorMac[5]);
        drawTitle(titleString);
      }
      moveToXy(1,24);
      inverseOn();Serial.print('P');inverseOff();Serial.print("ause logging | ");
      inverseOn();Serial.print('l');inverseOff();
      if(logAllNodes)
      {
        Serial.print("og specific node | ");
      }
      else
      {
        Serial.print("og all nodes | ");
        inverseOn();Serial.print('+');inverseOff();inverseOn();Serial.print('/');Serial.print('-');inverseOff();Serial.print(" Next/Previous originator|");
      }
      Serial.print("Change ");inverseOn();Serial.print('v');inverseOff();Serial.print("iew");
      setScrollWindow(2, 23);
      drawWholeUi = false;
    }
}