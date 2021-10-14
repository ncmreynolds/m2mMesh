void originatorView()
{
  if(drawWholeUi)
  {
    moveToXy(80,24);
    eraseScreen();
    hideCursor();
    if(m2mMesh.numberOfNodes() == 0)
    {
      popupBox("No originators to display");
    }
    else
    {
      //Which device is it?
      moveToXy(1,1);
      Serial.printf("Originator:%03d/%03d",currentlyViewedOriginator+1,numberOfNodes);
      //Does it have a name?
      printAtXy(20,1,F("Node name:"));
      //ID and MAC address
      moveToXy(3,3);
      uint8_t originatorMac[6]; //temporary MAC address
      m2mMesh.macAddress(currentlyViewedOriginator,originatorMac); //retrieve MAC address
      Serial.printf("ID/MACaddr:%02x/%02x:%02x:%02x:%02x:%02x:%02x",currentlyViewedOriginator,originatorMac[0],originatorMac[1],originatorMac[2],originatorMac[3],originatorMac[4],originatorMac[5]);
      printAtXy(3,4,F("Sequence number:"));
      printAtXy(3,5,F("RX errors:"));
      printAtXy(3,6,F("TX errors:"));

      printAtXy(3,8,F("ELP:"));
      printAtXy(3,9,F("ELP interval:"));
      printAtXy(3,10,F("ELP last seen:"));

      printAtXy(3,12,F("OGM:"));
      printAtXy(3,13,F("OGM interval:"));
      printAtXy(3,14,F("OGM last seen:"));
      printAtXy(3,16,F("OGM calculated LTQ:"));
      printAtXy(3,17,F("OGM selected Router:"));
      printAtXy(3,18,F("OGM selected Router GTQ:"));
      printAtXy(3,19,F("OGMs received:"));
      printAtXy(3,20,F("OGMs echoes:"));


      printAtXy(43,3,F("NHS:"));
      printAtXy(43,4,F("NHS interval:"));
      printAtXy(43,5,F("NHS last seen:"));
      printAtXy(43,6,F("Uptime:"));
      printAtXy(43,7,F("Sending time:"));
      printAtXy(43,8,F("Time server:"));
      printAtXy(43,9,F("Free Heap:"));
      printAtXy(43,10,F("Supply voltage:"));
      printAtXy(43,11,F("Active Neighbours:"));
      //printAtXy(43,12,F("SoftAp:"));
      printAtXy(43,13,F("Tx Power:"));

      // Supply voltage

      moveToXy(1,24);
      eraseLine();
      inverseOn();Serial.print('+');inverseOff();inverseOn();Serial.print('/');Serial.print('-');inverseOff();Serial.print(F(" Next/Previous originator | "));
      inverseOn();Serial.print('l');inverseOff();Serial.print(F("og this node only | "));
      Serial.print(F("Change "));inverseOn();Serial.print('v');inverseOff();Serial.print(F("iew"));
    }
  }
  if(numberOfNodes > 0)
  {
    if(currentlyViewedOriginatorChanged || drawWholeUi)
    {
      //Which one is this?
      moveToXy(12,1);
      Serial.printf("%03d/%03d",currentlyViewedOriginator+1,numberOfNodes);
      //ID and MAC address
      moveToXy(14,3);
      uint8_t tempMac[6];
      m2mMesh.macAddress(currentlyViewedOriginator,tempMac);
      Serial.printf("%02x/%02x:%02x:%02x:%02x:%02x:%02x",currentlyViewedOriginator,tempMac[0],tempMac[1],tempMac[2],tempMac[3],tempMac[4],tempMac[5]);
      //Does it have a name?
      moveToXy(30,1);
      if(m2mMesh.nodeNameIsSet(currentlyViewedOriginator))
      {
        Serial.printf("%-50s",m2mMesh.getNodeName(currentlyViewedOriginator));
      }
      else
      {
        Serial.print(F("Not known/Unset                                    "));
      }
      currentlyViewedOriginatorChanged = false;
    }
    //NHS interval
    if(m2mMesh.nhsIsValid(currentlyViewedOriginator))
    {
      printUptimeAtXy(56,4,m2mMesh.nhsInterval(currentlyViewedOriginator));
      Serial.print(F("           "));
    }
    else
    {
      moveToXy(56,4);
      Serial.print(F("Unknown    "));
    }
    //Sequence number
    moveToXy(19,4);
    Serial.printf("%08x",m2mMesh.lastSequenceNumber(currentlyViewedOriginator));
    //Error stats
    if(m2mMesh.nhsIsValid(currentlyViewedOriginator))
    {
      moveToXy(13,5);
      Serial.printf("%6d/%-6d(%d%%)",m2mMesh.droppedRxPackets(currentlyViewedOriginator),m2mMesh.rxPackets(currentlyViewedOriginator),(m2mMesh.droppedRxPackets(currentlyViewedOriginator)*100)/(m2mMesh.droppedRxPackets(currentlyViewedOriginator)+m2mMesh.rxPackets(currentlyViewedOriginator)));
      moveToXy(13,6);
      Serial.printf("%6d/%-6d(%d%%)",m2mMesh.droppedTxPackets(currentlyViewedOriginator),m2mMesh.txPackets(currentlyViewedOriginator),(m2mMesh.droppedTxPackets(currentlyViewedOriginator)*100)/(m2mMesh.droppedTxPackets(currentlyViewedOriginator)+m2mMesh.txPackets(currentlyViewedOriginator)));
    }
    else
    {
      printAtXy(13,5,"                  ");
      printAtXy(13,6,"                  ");
    }
    //NHS last seen
    if(m2mMesh.nhsIsValid(currentlyViewedOriginator))
    {
      printUptimeAtXy(57,5,millis() - m2mMesh.nhsLastSeen(currentlyViewedOriginator));
      Serial.print(F(" ago       "));
    }
    else
    {
      moveToXy(57,5);
      Serial.print(F("Never       "));
    }
    //Uptime
    if(m2mMesh.elpIsValid(currentlyViewedOriginator) && m2mMesh.nhsIsValid(currentlyViewedOriginator))
    {
      //Show the expected uptime
      printUptimeAtXy(50,6,m2mMesh.expectedUptime(currentlyViewedOriginator));
    }
    else
    {
      printAtXy(50,6,F("Unknown    "));
    }
    //Acting as time server
    moveToXy(56,7);
    if(m2mMesh.nhsIsValid(currentlyViewedOriginator))
    {
      if(m2mMesh.actingAsTimeServer(currentlyViewedOriginator))
      {
        Serial.print(F("Yes    "));
      }
      else
      {
        Serial.print(F("No     "));
      }
    }
    else
    {
      Serial.print(F("Unknown"));
    }
    //Is this the time server
    moveToXy(55,8);
    if(m2mMesh.currentMeshTimeServer() == currentlyViewedOriginator)
    {
      Serial.print(F("Yes"));
    }
    else
    {
      Serial.print(F("No "));
    }
    //Free heap
    moveToXy(53,9);
    if(m2mMesh.nhsIsValid(currentlyViewedOriginator) && m2mMesh.initialFreeHeap(currentlyViewedOriginator)>0)
    {
      Serial.printf("%05d/%05d(%02d%%)",m2mMesh.currentFreeHeap(currentlyViewedOriginator),m2mMesh.initialFreeHeap(currentlyViewedOriginator),100ul*m2mMesh.currentFreeHeap(currentlyViewedOriginator)/m2mMesh.initialFreeHeap(currentlyViewedOriginator));
    }
    else
    {
      Serial.print(F("Unknown         "));
    }
    //ELP interval
    if(m2mMesh.elpIsValid(currentlyViewedOriginator))
    {
      printUptimeAtXy(16,9,m2mMesh.elpInterval(currentlyViewedOriginator));
      Serial.print(F("           "));
    }
    else
    {
      moveToXy(16,9);
      Serial.print(F("Unknown    "));
    }
    // Supply voltage
    moveToXy(58,10);
    if(m2mMesh.nhsIsValid(currentlyViewedOriginator))
    {
      Serial.printf("%.2fv   ",m2mMesh.supplyVoltage(currentlyViewedOriginator));
    }
    else
    {
      Serial.print(F("Unknown    "));
    }
    //ELP last seen
    if(m2mMesh.elpLastSeen(currentlyViewedOriginator) > 0)
    {
      printUptimeAtXy(17,10,millis() - m2mMesh.elpLastSeen(currentlyViewedOriginator));
      Serial.print(F(" ago      "));
    }
    else
    {
      moveToXy(17,10);
      Serial.print(F("Never              "));
    }
    //Active peers and mesh size
    moveToXy(61,11);
    if(m2mMesh.nhsIsValid(currentlyViewedOriginator))
    {
      Serial.printf("%03d/%03d ",m2mMesh.numberOfActiveNeighbours(currentlyViewedOriginator),m2mMesh.numberOfNodes(currentlyViewedOriginator));
    }
    else
    {
      Serial.print(F("Unknown"));
    }
    //SoftAp state
    moveToXy(50,12);
    if(m2mMesh.nhsIsValid(currentlyViewedOriginator))
    {
      /*if(m2mMesh.softApState(currentlyViewedOriginator) == true)
      {
        Serial.print(F("On     "));
      }
      else
      {
        Serial.print(F("Off    "));
      }*/
    }
    else
    {
      Serial.print(F("Unknown"));
    }
    //Tx Power
    moveToXy(52,13);
    if(m2mMesh.nhsIsValid(currentlyViewedOriginator))
    {
      Serial.printf("%02d     ",m2mMesh.currentTxPower(currentlyViewedOriginator));
    }
    else
    {
      Serial.print(F("Unknown"));
    }
    //OGM interval
    if(m2mMesh.ogmIsValid(currentlyViewedOriginator))
    {
      printUptimeAtXy(16,13,m2mMesh.ogmInterval(currentlyViewedOriginator));
      Serial.print(F("           "));
    }
    else
    {
      moveToXy(16,13);
      Serial.print(F("Unknown    "));
    }
    //ELP up?
    if(m2mMesh.elpIsValid(currentlyViewedOriginator))
    {
      printAtXy(7,8,F("Up  "));
    }
    else
    {
      printAtXy(7,8,F("Down"));
    }
    //OGM last seen
    if(m2mMesh.ogmLastSeen(currentlyViewedOriginator)>0)
    {
      printUptimeAtXy(17,14,millis() - m2mMesh.ogmLastSeen(currentlyViewedOriginator));
      Serial.print(F(" ago      "));
    }
    else
    {
      moveToXy(17,14);
      Serial.print(F("Never              "));
    }
    //OGM up?
    if(m2mMesh.ogmIsValid(currentlyViewedOriginator))
    {
      printAtXy(7,12,F("Up  "));
    }
    else
    {
      printAtXy(7,12,F("Down"));
    }
    //OGM GTQ
    moveToXy(22,16);
    Serial.printf("%04x",m2mMesh.localTransmissionQuality(currentlyViewedOriginator));
    //NHS up?
    if(m2mMesh.nhsIsValid(currentlyViewedOriginator))
    {
      printAtXy(47,3,F("Up  "));
    }
    else
    {
      printAtXy(47,3,F("Down"));
    }   
    //OGM selected router
    moveToXy(23,17);
    if(m2mMesh.validRoute(currentlyViewedOriginator))
    {
      Serial.printf("%02x     ",m2mMesh.selectedRouter(currentlyViewedOriginator));
      //OGM TQ
      moveToXy(27,18);
      Serial.printf("%04x   ",m2mMesh.globalTransmissionQuality(currentlyViewedOriginator));
    }
    else
    {
      Serial.print(F("Unknown"));
      //OGM TQ
      moveToXy(27,18);
      Serial.print(F("Unknown"));
    }
    //OGMs received
    moveToXy(17,19);
    Serial.printf("%04x ",m2mMesh.ogmReceived(currentlyViewedOriginator));
    for(int8_t bit = 15; bit >= 0; bit--)
    {
      if(m2mMesh.ogmReceived(currentlyViewedOriginator) & uint16_t(pow(2,bit)))
      {
        Serial.print('1');
      }
      else
      {
        Serial.print('0');
      }
    }
    //OGMs echoes
    moveToXy(17,20);
    Serial.printf("%04x ",m2mMesh.ogmEchoes(currentlyViewedOriginator));
    for(int8_t bit = 15; bit >= 0; bit--)
    {
      if(m2mMesh.ogmEchoes(currentlyViewedOriginator) & uint16_t(pow(2,bit)))
      {
        Serial.print('1');
      }
      else
      {
        Serial.print('0');
      }
    }
  }
  drawWholeUi = false;
}
