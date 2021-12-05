void originatorView()
{
  if(drawWholeUi)
  {
    moveToXy(80,24);
    eraseScreen();
    hideCursor();
    drawTopLine();
    if(m2mMesh.numberOfNodes() == 0)
    {
      popupBox("No originators to display");
    }
    else
    {
      //Which device is it?
      moveToXy(1,4);
      Serial.printf("Originator:%03d/%03d",currentlyViewedOriginator+1,numberOfNodes);
      //Does it have a name?
      printAtXy(20,4,F("Node name:"));
      //ID and MAC address
      moveToXy(3,5);
      uint8_t originatorMac[6]; //temporary MAC address
      m2mMesh.macAddress(currentlyViewedOriginator,originatorMac); //retrieve MAC address
      Serial.printf("ID/MACaddr:%02x/%02x:%02x:%02x:%02x:%02x:%02x",currentlyViewedOriginator,originatorMac[0],originatorMac[1],originatorMac[2],originatorMac[3],originatorMac[4],originatorMac[5]);
      printAtXy(3,6, F("Sequence number:"));
      printAtXy(3,7, F("RX errors:"));
      printAtXy(3,8, F("TX errors:"));

      printAtXy(3,9, F("ESP-Now peer:"));
      printAtXy(3,10,F("ESP-Now peering last used:"));
      printAtXy(3,11,F("ESP-Now peers:"));
      
      printAtXy(3,12, F("ELP:"));
      printAtXy(3,13,F("ELP interval:"));
      printAtXy(3,14,F("ELP last seen:"));

      printAtXy(3,15,F("OGM:"));
      printAtXy(3,16,F("OGM interval:"));
      printAtXy(3,17,F("OGM last seen:"));
      printAtXy(3,18,F("OGM local transmission quality :"));
      printAtXy(3,19,F("OGM global transmission quality:"));
      printAtXy(3,20,F("OGM best router:"));
      printAtXy(3,21,F("OGMs received:"));
      printAtXy(3,22,F("OGMs echoes:"));


      printAtXy(43,5,F("NHS:"));
      printAtXy(43,6,F("NHS interval:"));
      printAtXy(43,7,F("NHS last seen:"));
      printAtXy(43,8,F("Uptime:"));
      printAtXy(43,9,F("Sending time:"));
      printAtXy(43,10,F("Time server:"));
      printAtXy(43,11,F("Free Heap:"));
      //printAtXy(43,10,F("Supply voltage:"));
      printAtXy(43,13,F("Active Neighbours:"));
      //printAtXy(43,12,F("SoftAp:"));
      printAtXy(43,15,F("Tx Power:"));

      // Supply voltage

      moveToXy(1,24);
      eraseLine();
      Serial.print(F("[+]/[-] next/previous node | "));
      Serial.print(F("[Tab] view"));
    }
  }
  if(numberOfNodes > 0)
  {
    if(currentlyViewedOriginatorChanged || drawWholeUi)
    {
      //Which one is this?
      moveToXy(12,4);
      Serial.printf("%03d/%03d",currentlyViewedOriginator+1,numberOfNodes);
      //ID and MAC address
      moveToXy(14,5);
      uint8_t tempMac[6];
      m2mMesh.macAddress(currentlyViewedOriginator,tempMac);
      Serial.printf("%02x/%02x:%02x:%02x:%02x:%02x:%02x",currentlyViewedOriginator,tempMac[0],tempMac[1],tempMac[2],tempMac[3],tempMac[4],tempMac[5]);
      //Does it have a name?
      moveToXy(30,4);
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
      printUptimeAtXy(56,6,m2mMesh.nhsInterval(currentlyViewedOriginator));
      Serial.print(F("           "));
    }
    else
    {
      moveToXy(56,6);
      Serial.print(F("Unknown    "));
    }
    //Sequence number
    moveToXy(19,6);
    Serial.printf("%08x",m2mMesh.lastSequenceNumber(currentlyViewedOriginator));
    //Error stats
    if(m2mMesh.nhsIsValid(currentlyViewedOriginator))
    {
      moveToXy(13,7);
      Serial.printf("%6d/%-6d(%d%%)",m2mMesh.droppedRxPackets(currentlyViewedOriginator),m2mMesh.rxPackets(currentlyViewedOriginator),(m2mMesh.droppedRxPackets(currentlyViewedOriginator)*100)/(m2mMesh.droppedRxPackets(currentlyViewedOriginator)+m2mMesh.rxPackets(currentlyViewedOriginator)));
      moveToXy(13,8);
      Serial.printf("%6d/%-6d(%d%%)",m2mMesh.droppedTxPackets(currentlyViewedOriginator),m2mMesh.txPackets(currentlyViewedOriginator),(m2mMesh.droppedTxPackets(currentlyViewedOriginator)*100)/(m2mMesh.droppedTxPackets(currentlyViewedOriginator)+m2mMesh.txPackets(currentlyViewedOriginator)));
    }
    else
    {
      printAtXy(13,7,"                  ");
      printAtXy(13,8,"                  ");
    }
    //NHS last seen
    if(m2mMesh.nhsIsValid(currentlyViewedOriginator))
    {
      printElapsedTimeAtXy(57,7,millis() - m2mMesh.nhsLastSeen(currentlyViewedOriginator));
    }
    else
    {
      moveToXy(57,7);
      Serial.print(F("Never       "));
    }
    //Uptime
    if(m2mMesh.elpIsValid(currentlyViewedOriginator) && m2mMesh.nhsIsValid(currentlyViewedOriginator))
    {
      //Show the expected uptime
      printUptimeAtXy(50,8,m2mMesh.expectedUptime(currentlyViewedOriginator));
    }
    else
    {
      printAtXy(50,8,F("Unknown    "));
    }
    //Acting as time server
    moveToXy(56,9);
    if(m2mMesh.nhsIsValid(currentlyViewedOriginator))
    {
      if(m2mMesh.actingAsSyncServer(currentlyViewedOriginator))
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
    moveToXy(55,10);
    if(m2mMesh.currentMeshSyncServer() == currentlyViewedOriginator)
    {
      Serial.print(F("Yes"));
    }
    else
    {
      Serial.print(F("No "));
    }
    //Free heap
    moveToXy(53,11);
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
      printUptimeAtXy(16,13,m2mMesh.elpInterval(currentlyViewedOriginator));
      Serial.print(F("           "));
    }
    else
    {
      moveToXy(16,13);
      Serial.print(F("Unknown    "));
    }
    // Supply voltage
    /*moveToXy(58,10);
    if(m2mMesh.nhsIsValid(currentlyViewedOriginator))
    {
      Serial.printf("%.2fv   ",m2mMesh.supplyVoltage(currentlyViewedOriginator));
    }
    else
    {
      Serial.print(F("Unknown    "));
    }*/
    //Active peers and mesh size
    moveToXy(61,13);
    if(m2mMesh.nhsIsValid(currentlyViewedOriginator))
    {
      Serial.printf("%03d/%03d ",m2mMesh.numberOfActiveNeighbours(currentlyViewedOriginator),m2mMesh.numberOfNodes(currentlyViewedOriginator));
    }
    else
    {
      Serial.print(F("Unknown"));
    }

    //ELP last seen
    if(m2mMesh.elpLastSeen(currentlyViewedOriginator) > 0)
    {
      printElapsedTimeAtXy(17,14,millis() - m2mMesh.elpLastSeen(currentlyViewedOriginator));
    }
    else
    {
      moveToXy(17,14);
      Serial.print(F("Never              "));
    }

    //Tx Power
    moveToXy(52,15);
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
      printUptimeAtXy(16,16,m2mMesh.ogmInterval(currentlyViewedOriginator));
      Serial.print(F("           "));
    }
    else
    {
      moveToXy(16,16);
      Serial.print(F("Unknown    "));
    }
    //ELP up?
    if(m2mMesh.elpIsValid(currentlyViewedOriginator))
    {
      printAtXy(7,12,F("Up  "));
    }
    else
    {
      printAtXy(7,12,F("Down"));
    }

    //ESP-Now
    switch(m2mMesh.espnowPeer(currentlyViewedOriginator))
    {
      case 0:
        printAtXy(16,9,F("No                      "));
      break;
      case 1:
        printAtXy(16,9,F("Local only              "));
      break;
      case 3:
        printAtXy(16,9,F("Local only (expired)    "));
      break;
      case 4:
        printAtXy(16,9,F("Remote only             "));
      break;
      case 12:
        printAtXy(16,9,F("Remote only (expired)   "));
      break;
      case 5:
        printAtXy(16,9,F("Local & Remote          "));
      break;
      case 7:
        printAtXy(16,9,F("Local (expired) & Remote"));
      break;
      case 13:
        printAtXy(16,9,F("Local & Remote (expired)"));
      break;
      case 15:
        printAtXy(16,9,F("Both expired            "));
      break;
      default:
        printAtXy(16,9,F("?                       "));
      break;
    }
    if(m2mMesh.espnowPeeringLastUsed(currentlyViewedOriginator) > 0)
    {
      printElapsedTimeAtXy(29,10,millis() - m2mMesh.espnowPeeringLastUsed(currentlyViewedOriginator));
    }
    else
    {
      printAtXy(29,10,"Never     ");
    }
    if(m2mMesh.elpIsValid(currentlyViewedOriginator))
    {
      moveToXy(17,11);
      Serial.printf("%u/%u expired    ",m2mMesh.numberOfPeers(currentlyViewedOriginator),m2mMesh.numberOfExpiredPeers(currentlyViewedOriginator));
    }
    //OGM last seen
    if(m2mMesh.ogmLastSeen(currentlyViewedOriginator)>0)
    {
      printElapsedTimeAtXy(17,17,millis() - m2mMesh.ogmLastSeen(currentlyViewedOriginator));
    }
    else
    {
      moveToXy(17,17);
      Serial.print(F("Never              "));
    }
    //OGM up?
    if(m2mMesh.ogmIsValid(currentlyViewedOriginator))
    {
      printAtXy(7,15,F("Up  "));
    }
    else
    {
      printAtXy(7,15,F("Down"));
    }
    //OGM GTQ
    moveToXy(35,18);
    Serial.printf("%04x",m2mMesh.localTransmissionQuality(currentlyViewedOriginator));
    //NHS up?
    if(m2mMesh.nhsIsValid(currentlyViewedOriginator))
    {
      printAtXy(47,5,F("Up  "));
    }
    else
    {
      printAtXy(47,5,F("Down"));
    }   
    //OGM selected router
    if(m2mMesh.validRoute(currentlyViewedOriginator))
    {
      //OGM TQ
      moveToXy(35,19);
      Serial.printf("%04x",m2mMesh.globalTransmissionQuality(currentlyViewedOriginator));
      moveToXy(19,20);
      uint8_t routerMac[6]; //temporary MAC address
      m2mMesh.macAddress(m2mMesh.selectedRouter(currentlyViewedOriginator),routerMac); //retrieve MAC address
      Serial.printf("%02x/%02x:%02x:%02x:%02x:%02x:%02x",m2mMesh.selectedRouter(currentlyViewedOriginator), routerMac[0], routerMac[1], routerMac[2], routerMac[3], routerMac[4], routerMac[5]);
    }
    else
    {
      moveToXy(35,19);
      Serial.print(F("0000"));
      //OGM TQ
      moveToXy(19,20);
      Serial.print(F("None                "));
    }
    //OGMs received
    moveToXy(17,21);
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
    moveToXy(17,22);
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
