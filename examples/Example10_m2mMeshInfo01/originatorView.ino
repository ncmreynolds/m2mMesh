void originatorView()
{
  if(drawWholeUi)
  {
    moveToXy(80,24);
    eraseScreen();
    hideCursor();
    if(mesh.numberOfOriginators() == 0)
    {
      popupBox("No originators to display");
    }
    else
    {
      //Which device is it?
      moveToXy(1,1);
      Serial.printf("Originator:%03d/%03d",currentlyViewedOriginator+1,numberOfOriginators);
      //Does it have a name?
      printAtXy(20,1,F("Node name:"));
      //ID and MAC address
      moveToXy(3,3);
      uint8_t originatorMac[6]; //temporary MAC address
      mesh.macAddress(currentlyViewedOriginator,originatorMac); //retrieve MAC address
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
      printAtXy(3,16,F("OGM calculated TQ:"));
      printAtXy(3,17,F("OGM selected Router:"));
      printAtXy(3,18,F("OGM selected Router TQ:"));
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
  if(numberOfOriginators > 0)
  {
    if(currentlyViewedOriginatorChanged || drawWholeUi)
    {
      //Which one is this?
      moveToXy(12,1);
      Serial.printf("%03d/%03d",currentlyViewedOriginator+1,numberOfOriginators);
      //ID and MAC address
      moveToXy(14,3);
      uint8_t tempMac[6];
      mesh.macAddress(currentlyViewedOriginator,tempMac);
      Serial.printf("%02x/%02x:%02x:%02x:%02x:%02x:%02x",currentlyViewedOriginator,tempMac[0],tempMac[1],tempMac[2],tempMac[3],tempMac[4],tempMac[5]);
      //Does it have a name?
      moveToXy(30,1);
      if(mesh.nodeNameIsSet(currentlyViewedOriginator))
      {
        Serial.printf("%-50s",mesh.getNodeName(currentlyViewedOriginator));
      }
      else
      {
        Serial.print(F("Not known/Unset                                    "));
      }
      currentlyViewedOriginatorChanged = false;
    }
    //NHS interval
    if(mesh.nhsIsValid(currentlyViewedOriginator))
    {
      printUptimeAtXy(56,4,mesh.nhsInterval(currentlyViewedOriginator));
      Serial.print(F("           "));
    }
    else
    {
      moveToXy(56,4);
      Serial.print(F("Unknown    "));
    }
    //Sequence number
    moveToXy(19,4);
    Serial.printf("%08x",mesh.lastSequenceNumber(currentlyViewedOriginator));
    //Error stats
    if(mesh.nhsIsValid(currentlyViewedOriginator))
    {
      moveToXy(13,5);
      Serial.printf("%6d/%-6d(%d%%)",mesh.droppedRxPackets(currentlyViewedOriginator),mesh.rxPackets(currentlyViewedOriginator),(mesh.droppedRxPackets(currentlyViewedOriginator)*100)/(mesh.droppedRxPackets(currentlyViewedOriginator)+mesh.rxPackets(currentlyViewedOriginator)));
      moveToXy(13,6);
      Serial.printf("%6d/%-6d(%d%%)",mesh.droppedTxPackets(currentlyViewedOriginator),mesh.txPackets(currentlyViewedOriginator),(mesh.droppedTxPackets(currentlyViewedOriginator)*100)/(mesh.droppedTxPackets(currentlyViewedOriginator)+mesh.txPackets(currentlyViewedOriginator)));
    }
    else
    {
      printAtXy(13,5,"                  ");
      printAtXy(13,6,"                  ");
    }
    //NHS last seen
    if(mesh.nhsIsValid(currentlyViewedOriginator))
    {
      printUptimeAtXy(57,5,millis() - mesh.nhsLastSeen(currentlyViewedOriginator));
      Serial.print(F(" ago       "));
    }
    else
    {
      moveToXy(57,5);
      Serial.print(F("Never       "));
    }
    //Uptime
    if(mesh.elpIsValid(currentlyViewedOriginator) && mesh.nhsIsValid(currentlyViewedOriginator))
    {
      //Show the expected uptime
      printUptimeAtXy(50,6,mesh.expectedUptime(currentlyViewedOriginator));
    }
    else
    {
      printAtXy(50,6,F("Unknown    "));
    }
    //Acting as time server
    moveToXy(56,7);
    if(mesh.nhsIsValid(currentlyViewedOriginator))
    {
      if(mesh.actingAsTimeServer(currentlyViewedOriginator))
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
    if(mesh.currentMeshTimeServer() == currentlyViewedOriginator)
    {
      Serial.print(F("Yes"));
    }
    else
    {
      Serial.print(F("No "));
    }
    //Free heap
    moveToXy(53,9);
    if(mesh.nhsIsValid(currentlyViewedOriginator) && mesh.initialFreeHeap(currentlyViewedOriginator)>0)
    {
      Serial.printf("%05d/%05d(%02d%%)",mesh.currentFreeHeap(currentlyViewedOriginator),mesh.initialFreeHeap(currentlyViewedOriginator),100ul*mesh.currentFreeHeap(currentlyViewedOriginator)/mesh.initialFreeHeap(currentlyViewedOriginator));
    }
    else
    {
      Serial.print(F("Unknown         "));
    }
    //ELP interval
    if(mesh.elpIsValid(currentlyViewedOriginator))
    {
      printUptimeAtXy(16,9,mesh.elpInterval(currentlyViewedOriginator));
      Serial.print(F("           "));
    }
    else
    {
      moveToXy(16,9);
      Serial.print(F("Unknown    "));
    }
    // Supply voltage
    moveToXy(58,10);
    if(mesh.nhsIsValid(currentlyViewedOriginator))
    {
      Serial.printf("%.2fv   ",mesh.supplyVoltage(currentlyViewedOriginator));
    }
    else
    {
      Serial.print(F("Unknown    "));
    }
    //ELP last seen
    if(mesh.elpLastSeen(currentlyViewedOriginator) > 0)
    {
      printUptimeAtXy(17,10,millis() - mesh.elpLastSeen(currentlyViewedOriginator));
      Serial.print(F(" ago      "));
    }
    else
    {
      moveToXy(17,10);
      Serial.print(F("Never              "));
    }
    //Active peers and mesh size
    moveToXy(61,11);
    if(mesh.nhsIsValid(currentlyViewedOriginator))
    {
      Serial.printf("%03d/%03d ",mesh.numberOfActiveNeighbours(currentlyViewedOriginator),mesh.numberOfOriginators(currentlyViewedOriginator));
    }
    else
    {
      Serial.print(F("Unknown"));
    }
    //SoftAp state
    moveToXy(50,12);
    if(mesh.nhsIsValid(currentlyViewedOriginator))
    {
      /*if(mesh.softApState(currentlyViewedOriginator) == true)
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
    if(mesh.nhsIsValid(currentlyViewedOriginator))
    {
      Serial.printf("%02d     ",mesh.currentTxPower(currentlyViewedOriginator));
    }
    else
    {
      Serial.print(F("Unknown"));
    }
    //OGM interval
    if(mesh.ogmIsValid(currentlyViewedOriginator))
    {
      printUptimeAtXy(16,13,mesh.ogmInterval(currentlyViewedOriginator));
      Serial.print(F("           "));
    }
    else
    {
      moveToXy(16,13);
      Serial.print(F("Unknown    "));
    }
    //ELP up?
    if(mesh.elpIsValid(currentlyViewedOriginator))
    {
      printAtXy(7,8,F("Up  "));
    }
    else
    {
      printAtXy(7,8,F("Down"));
    }
    //OGM last seen
    if(mesh.ogmLastSeen(currentlyViewedOriginator)>0)
    {
      printUptimeAtXy(17,14,millis() - mesh.ogmLastSeen(currentlyViewedOriginator));
      Serial.print(F(" ago      "));
    }
    else
    {
      moveToXy(17,14);
      Serial.print(F("Never              "));
    }
    //OGM up?
    if(mesh.ogmIsValid(currentlyViewedOriginator))
    {
      printAtXy(7,12,F("Up  "));
    }
    else
    {
      printAtXy(7,12,F("Down"));
    }
    //OGM GTQ
    moveToXy(21,16);
    Serial.printf("%04x",mesh.localTransmissionQuality(currentlyViewedOriginator));
    //NHS up?
    if(mesh.nhsIsValid(currentlyViewedOriginator))
    {
      printAtXy(47,3,F("Up  "));
    }
    else
    {
      printAtXy(47,3,F("Down"));
    }   
    //OGM selected router
    moveToXy(23,17);
    if(mesh.validRoute(currentlyViewedOriginator))
    {
      Serial.printf("%02x     ",mesh.selectedRouter(currentlyViewedOriginator));
      //OGM TQ
      moveToXy(26,18);
      Serial.printf("%04x   ",mesh.globalTransmissionQuality(currentlyViewedOriginator));
    }
    else
    {
      Serial.print(F("Unknown"));
      //OGM TQ
      moveToXy(26,18);
      Serial.print(F("Unknown"));
    }
    //OGMs received
    moveToXy(17,19);
    Serial.printf("%04x",mesh.ogmReceived(currentlyViewedOriginator));
    //OGMs echoes
    moveToXy(15,20);
    Serial.printf("%04x",mesh.ogmEchoes(currentlyViewedOriginator));
  }
  drawWholeUi = false;
}
