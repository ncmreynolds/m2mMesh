void statusView()
{
  if(drawWholeUi)
  {
    moveToXy(80,24);
    eraseScreen();
    hideCursor();

    moveToXy(1,1);
    if(mesh.nodeNameIsSet())
    {
      Serial.print(mesh.getNodeName());
    }
    else
    {
      Serial.print(WiFi.macAddress());
    }
    moveToXy(19,1);
    Serial.print(F("Up:"));
    moveToXy(33,1);
    Serial.print(F("Time:"));
    moveToXy(51,1);
    Serial.print(F("Ch:"));
    moveToXy(57,1);
    Serial.print(F("Tx:"));
    moveToXy(65,1);
    Serial.print(F("Fl:"));
    moveToXy(73,1);
    Serial.print(F("Vcc:"));

    moveToXy(1,2);
    Serial.print(F("Neighbours:--- Mesh:---/---"));
    moveToXy(30,2);
    Serial.print(F("Seq:"));
    moveToXy(43,2);
    Serial.print(F("Err:"));
    moveToXy(60,2);
    Serial.print(F("Heap:"));

    moveToXy(1,3);
    Serial.print(F("ID"));
    moveToXy(9,3);
    Serial.print(F("MAC"));
    moveToXy(17,3);
    Serial.print(F("Flgs  GTQ  LTQ   Seq"));
    moveToXy(42,3);
    Serial.print(F("ID"));
    moveToXy(50,3);
    Serial.print(F("MAC"));
    moveToXy(58,3);
    Serial.print(F("Flgs  GTQ  LTQ   Seq"));
    moveToXy(1,14);
    Serial.print(F("Board:"));
    #if defined (ARDUINO_ESP8266_WEMOS_D1MINI)
    Serial.print(F("WeMos D1 mini"));
    #elif defined (ARDUINO_ESP8266_WEMOS_D1MINIPRO)
    Serial.print(F("WeMos D1 mini pro"));
    #elif defined (ARDUINO_ESP8266_GENERIC)
    Serial.print(F("ESP-01S"));
    #elif defined (ARDUINO_ESP8266_ESP01)
    Serial.print(F("ESP8285-M1"));
    #elif defined(ARDUINO_ESP32S2_DEV)
    Serial.print(F("ESP32S2 Saola-1R"));
    #else
    Serial.print(' ');
    Serial.print(ARDUINO_BOARD);
    Serial.print(F("Unknown"));
    #endif
    if(logAllNodes)
    {
      printCentred(14,"Logging (all nodes)");
    }
    else
    {
      uint8_t originatorMac[6]; //temporary MAC address
      mesh.macAddress(nodeToLog,originatorMac);
      char titleString[36];
      sprintf(titleString,"Logging (%03d/%02x:%02x:%02x:%02x:%02x:%02x only)",nodeToLog,originatorMac[0],originatorMac[1],originatorMac[2],originatorMac[3],originatorMac[4],originatorMac[5]);
      printCentred(14,titleString);
    }
    moveToXy(1,24);
    inverseOn();Serial.print('r');inverseOff();Serial.print(F("eboot | "));
    inverseOn();Serial.print('+');inverseOff();inverseOn();Serial.print('/');Serial.print('-');inverseOff();Serial.print(F(" Tx power | "));
    inverseOn();Serial.print('P');inverseOff();Serial.print(F("ause logging | "));
    Serial.print(F("Change "));inverseOn();Serial.print('v');inverseOff();Serial.print(F("iew | "));
    Serial.print(F("Switch "));inverseOn();Serial.print('C');inverseOff();Serial.print(F("hannel"));
  }
  //Uptime
  {
    printUptimeAtXy(22,1,millis());
  }
  //Mesh time
  printUptimeAtXy(38,1,mesh.syncedMillis());
  if(mesh.actingAsTimeServer())
  {
    Serial.print('*');
  }
  else if(abs(mesh.meshTimeDrift()) > 1000)
  {
    Serial.print('D');
  }
  else
  {
    Serial.print(' ');
  }
  //Reachable Originators
  if(numberOfReachableOriginatorsChanged || drawWholeUi)
  {
    moveToXy(21,2);
    Serial.printf("%03d",mesh.numberOfReachableOriginators());
    numberOfReachableOriginatorsChanged = false;
  }
  //Originators
  if(numberOfOriginatorsChanged || drawWholeUi)
  {
    moveToXy(25,2);
    Serial.printf("%03d",mesh.numberOfOriginators());
    numberOfOriginatorsChanged = false;
  }
  //Mesh stability
  if(meshIsStableChanged ||  drawWholeUi)
  {
    moveToXy(28,2);
    if(mesh.meshIsStable())
    {
      Serial.print('S');
    }
    else
    {
      Serial.print('U');
    }
  }
  //Sequence number
  if(sequenceNumberChanged  || drawWholeUi)
  {
    moveToXy(34,2);
    Serial.printf("%08x",sequenceNumber);
    sequenceNumberChanged = false;
  }
  //RX/TX errors
  moveToXy(47,2);
  if(mesh.rxPackets() + mesh.droppedRxPackets() > 0)
  {
    Serial.printf("%02d%%Rx",(mesh.droppedRxPackets()*100)/(mesh.rxPackets() + mesh.droppedRxPackets()));
  }
  else
  {
    Serial.print(F("00%Rx"));
  }
  Serial.print('/');
  if(mesh.txPackets() + mesh.droppedTxPackets() > 0)
  {
    Serial.printf("%02d%%Tx",(mesh.droppedTxPackets()*100)/(mesh.txPackets() + mesh.droppedTxPackets()));
  }
  else
  {
    Serial.print(F("00%Tx"));
  }
  //Free Heap
  {
    moveToXy(65,2);
    Serial.printf("%05d/%05d(%02d%%)",ESP.getFreeHeap(),initialFreeHeap,(100ul*ESP.getFreeHeap()/initialFreeHeap));
  }
  //Neighbour table
  if(numberOfActiveNeighboursChanged || drawWholeUi)
  {
    moveToXy(12,2);
    Serial.printf("%03d",numberOfActiveNeighbours);
    numberOfActiveNeighboursChanged = false;
    {
      uint8_t neighbour = 0;
      for(uint8_t originatorId = 0; originatorId < mesh.numberOfOriginators(); originatorId++)
      {
        if(mesh.elpIsValid(originatorId) || mesh.ogmIsValid(originatorId))
        {
          char flags[4] = {'-','-','-','-'};
          if(mesh.actingAsRTCServer(originatorId))
          {
            flags[0] = 'R';
          }
          else
          {
            flags[0] = '-';
          }
          if(mesh.actingAsTimeServer(originatorId))
          {
            flags[1] = 'T';
          }
          else
          {
            flags[1] = '-';
          }
          if(mesh.nodeNameIsSet(originatorId))
          {
            flags[2] = 'N';
          }
          else
          {
            flags[2] = '-';
          }
          //Move the cursor to the right spot in the table
          moveToXy((neighbour/10)*42,4+neighbour%10);
          uint8_t tempMac[6];
          mesh.macAddress(originatorId,tempMac);
          Serial.printf("%02d %02x%02x%02x%02x%02x%02x %c%c%c%c ---- ---- %08x",originatorId,tempMac[0],tempMac[1],tempMac[2],tempMac[3],tempMac[4],tempMac[5],flags[0],flags[1],flags[2],flags[3],mesh.lastSequenceNumber(originatorId));
          //GTQ
          moveToXy(22+(neighbour/10)*41,4+neighbour%10);
          if(mesh.globalTransmissionQuality(originatorId) > 0)
          {
            Serial.printf("%04x",mesh.globalTransmissionQuality(originatorId));
          }
          else
          {
            Serial.print(F("----"));
          }
          //LTQ 
          moveToXy(27+(neighbour/10)*41,4+neighbour%10);
          if(mesh.localTransmissionQuality(originatorId) > 0)
          {
            Serial.printf("%04x",mesh.localTransmissionQuality(originatorId));
          }
          else
          {
            Serial.print(F("----"));
          }        
          neighbour++;
        }
      }
      //Now all the empty slots
      for(uint8_t i = neighbour;i<maxNeighboursOnPage;i++)
      {
        //Move the cursor to the right spot in the table
        moveToXy((i/10)*42,4+i%10);
        Serial.print(F("-- ------------ ---- ---- ---- --------"));
      }
    }
  }
  else
  {
    //Draw only what's changed
    uint8_t neighbour = 0;
    for(uint8_t originatorId = 0; originatorId < mesh.numberOfOriginators(); originatorId++)
    {
      if(mesh.elpIsValid(originatorId) || mesh.ogmIsValid(originatorId))
      {
        //Peer flags
        //if(originator[originatorId].flagsChanged)
        {
          char flags[4] = {'-','-','-','-'};
          if(mesh.actingAsRTCServer(originatorId))
          {
            flags[0] = 'R';
          }
          else
          {
            flags[0] = '-';
          }
          if(mesh.actingAsTimeServer(originatorId))
          {
            flags[1] = 'T';
          }
          else
          {
            flags[1] = '-';
          }
          if(mesh.nodeNameIsSet(originatorId))
          {
            flags[2] = 'N';
          }
          else
          {
            flags[2] = '-';
          }
          //Move the cursor to the right spot in the table
          moveToXy(17+(neighbour/10)*41,4+neighbour%10);
          Serial.printf("%c%c%c%c",flags[0],flags[1],flags[2],flags[3]);
          //originator[originatorId].flagsChanged = false;
        }
        //GTQ
        moveToXy(22+(neighbour/10)*41,4+neighbour%10);
        if(mesh.globalTransmissionQuality(originatorId) > 0)
        {
          Serial.printf("%04x",mesh.globalTransmissionQuality(originatorId));
        }
        else
        {
          Serial.print(F("----"));
        }
        //LTQ
        moveToXy(27+(neighbour/10)*41,4+neighbour%10);
        if(mesh.localTransmissionQuality(originatorId) > 0)
        {
          Serial.printf("%04x",mesh.localTransmissionQuality(originatorId));
        }
        else
        {
          Serial.print(F("----"));
        }        
        //Sequence number
        //if(originator[originatorId].sequenceNumberChanged)
        {
          moveToXy(32+(neighbour/10)*41,4+neighbour%10);
          //We may not know the sequence number
          if(mesh.lastSequenceNumber(originatorId)>0 && (mesh.elpIsValid(originatorId) || mesh.ogmIsValid(originatorId)))
          {
            Serial.printf("%08x",mesh.lastSequenceNumber(originatorId));
          }
          else
          {
            Serial.print(F("????????"));
          }
          //originator[originatorId].sequenceNumberChanged = false;
        }
        neighbour++;
      }
    }
  }
  //Input Vcc
  if(supplyVoltageChanged || drawWholeUi)
  {
    moveToXy(77,0);
    Serial.printf("%01.1fv",mesh.supplyVoltage());
  }
  if(currentChannelChanged || drawWholeUi)
  {
    moveToXy(54,0);
    Serial.printf("%02d",currentChannel);
    currentChannelChanged = false;
  }
  //Tx power
  if(currentTxPowerChanged || drawWholeUi)
  {
    moveToXy(60,1);
    Serial.printf("%02.1f",mesh.currentTxPower()*4);
    //Tx Power floor
    moveToXy(68,1);
    Serial.printf("%02.1f",mesh.txPowerFloor()*4);
  }
  setScrollWindow(15, 23);  //Set the logging scroll window
  moveToXy(1,23);           //Put the logging point at the bottom of the scroll window
  if(drawWholeUi)
  {
    drawWholeUi = false;
  }
}
