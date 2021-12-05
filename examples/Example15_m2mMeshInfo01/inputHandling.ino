void inputHandling()
{
  if (Serial.available())
  {
    char keypress = Serial.read();
    switch (keypress)
    {
      case char(0x0d):
        drawWholeUi = true; //Enter pressed
      break;
      case char(0):
        //Unregister the callback so we no longer process mesh packets
        esp_now_unregister_recv_cb();
        //This is the 'break' character
        popupBox("Rebooting");
        delay(2000);
        ESP.restart();
      break;
      case ' ':
        if(currentUiView == UI_STATUS || currentUiView == UI_LOGS)
        {
          loggingPaused = not loggingPaused;
          if(loggingPaused)
          {
            m2mMesh.disableDebugging();
          }
          else
          {
            m2mMesh.enableDebugging();
          }
        }
      break;
      case char(9): //Tab
        //Change logging view
        currentUiView++;
        currentUiView = currentUiView%5;
        if(currentUiView == UI_STATUS)
        {
          popupBox("Status display, press [Tab] to change");
          delay(1000);
          m2mMesh.enableDebugging();
        }
        else if(currentUiView == UI_ORIGINATORS)
        {
          popupBox("Originator display, press [Tab] to change");
          delay(1000);
          m2mMesh.disableDebugging();
        }
        else if(currentUiView == UI_PROTOCOLS)
        {
          popupBox("Service configuration display, press [Tab] to change");
          delay(1000);
          m2mMesh.disableDebugging();
        }
        else if(currentUiView == UI_LOGS)
        {
          popupBox("Logging display, press [Tab] to change");
          delay(1000);
          m2mMesh.enableDebugging();
        }
        else if(currentUiView == UI_LOG_CONFIG)
        {
          popupBox("Logging configuration, press [Tab] to change");
          delay(1000);
          m2mMesh.disableDebugging();
        }
        drawWholeUi = true;
    break;
    case '+':
      if(currentUiView == UI_ORIGINATORS)
      {
        if(m2mMesh.numberOfNodes() > 0 && currentlyViewedOriginator < m2mMesh.numberOfNodes() - 1)
        {
          currentlyViewedOriginator++;
        }
        else
        {
          currentlyViewedOriginator = 0;
        }
        currentlyViewedOriginatorChanged = true;
      }
      else if(currentUiView == UI_LOGS || currentUiView == UI_LOG_CONFIG)
      {
        if(logAllNodes == false)
        {
          if(m2mMesh.numberOfNodes() > 0 && nodeToLog < m2mMesh.numberOfNodes() - 1)
          {
            nodeToLog++;
            m2mMesh.nodeToLog(nodeToLog);
          }
          else
          {
            nodeToLog = 0;
            logAllNodes = true;
            m2mMesh.logAllNodes();
          }
        }
        else
        {
          nodeToLog = 0;
          logAllNodes = false;
          m2mMesh.nodeToLog(nodeToLog);
        }
        drawWholeUi = true;
      }
      else
      {
        //increaseTxPower();
      }
    break;
    case '-':
      if(currentUiView == UI_ORIGINATORS)
      {
        if(currentlyViewedOriginator > 0)
        {
          currentlyViewedOriginator--;
        }
        else
        {
          currentlyViewedOriginator = m2mMesh.numberOfNodes() - 1;
        }
        currentlyViewedOriginatorChanged = true;
      }
      else if(currentUiView == UI_LOGS || currentUiView == UI_LOG_CONFIG)
      {
        if(logAllNodes == false)
        {
          if(nodeToLog > 0)
          {
            nodeToLog--;
            m2mMesh.nodeToLog(nodeToLog);
          }
          else
          {
            logAllNodes = true;
            m2mMesh.logAllNodes();
          }
        }
        else
        {
          nodeToLog = m2mMesh.numberOfNodes() - 1;
          logAllNodes = false;
          m2mMesh.nodeToLog(nodeToLog);
        }
        drawWholeUi = true;
      }
      else
      {
        //reduceTxPower();
      }
    break;
    case 'v':
      if(currentUiView == UI_LOG_CONFIG)
      {
        m2mMesh.setLoggingLevel(m2mMesh.loggingLevel() ^ m2mMesh.MESH_UI_LOG_ALL_SENT_PACKETS);
        loggingLevelChanged = true;
      }
    break;
    case 'u':
      if(currentUiView == UI_LOG_CONFIG)
      {
        m2mMesh.setLoggingLevel(m2mMesh.loggingLevel() ^ m2mMesh.MESH_UI_LOG_ALL_RECEIVED_PACKETS);
        loggingLevelChanged = true;
      }
    break;
    case 't':
      if(currentUiView == UI_LOG_CONFIG)
      {
        m2mMesh.setLoggingLevel(m2mMesh.loggingLevel() ^ m2mMesh.MESH_UI_LOG_WIFI_POWER_MANAGEMENT);
        loggingLevelChanged = true;
      }
    break;
    case 's':
      if(currentUiView == UI_LOG_CONFIG)
      {
        m2mMesh.setLoggingLevel(m2mMesh.loggingLevel() ^ m2mMesh.MESH_UI_LOG_PACKET_ROUTING);
        loggingLevelChanged = true;
      }
    break;
    case 'r':
      if(currentUiView == UI_LOG_CONFIG)
      {
        m2mMesh.setLoggingLevel(m2mMesh.loggingLevel() ^ m2mMesh.MESH_UI_LOG_BUFFER_MANAGEMENT);
        loggingLevelChanged = true;
      }
      else if(currentUiView == UI_STATUS)
      {
        //Unregister the callback so we no longer process mesh packets
        esp_now_unregister_recv_cb();
        //This is the 'break' character
        popupBox("Rebooting");
        delay(2000);
        ESP.restart();
      }
    break;
    case 'q':
      if(currentUiView == UI_LOG_CONFIG)
      {
        m2mMesh.setLoggingLevel(m2mMesh.loggingLevel() ^ m2mMesh.MESH_UI_LOG_ESP_NOW_EVENTS);
        loggingLevelChanged = true;
      }
    break;
    case 'p':
      if(currentUiView == UI_LOG_CONFIG)
      {
        m2mMesh.setLoggingLevel(m2mMesh.loggingLevel() ^ m2mMesh.MESH_UI_LOG_NODE_MANAGEMENT);
        loggingLevelChanged = true;
      }
    break;
    case 'o':
      if(currentUiView == UI_LOG_CONFIG)
      {
        m2mMesh.setLoggingLevel(m2mMesh.loggingLevel() ^ m2mMesh.MESH_UI_LOG_PEER_MANAGEMENT);
        loggingLevelChanged = true;
      }
    break;
    case 'n':
      if(currentUiView == UI_LOG_CONFIG)
      {
        m2mMesh.setLoggingLevel(m2mMesh.loggingLevel() ^ m2mMesh.MESH_UI_LOG_BTP_SEND);
        loggingLevelChanged = true;
      }
    break;
    case 'm':
      if(currentUiView == UI_LOG_CONFIG)
      {
        m2mMesh.setLoggingLevel(m2mMesh.loggingLevel() ^ m2mMesh.MESH_UI_LOG_BTP_RECEIVED);
        loggingLevelChanged = true;
      }
    break;
    case 'l':
      if(currentUiView == UI_LOG_CONFIG)
      {
        m2mMesh.setLoggingLevel(m2mMesh.loggingLevel() ^ m2mMesh.MESH_UI_LOG_BTP_FORWARDING);
        loggingLevelChanged = true;
      }
      else if(currentUiView == UI_ORIGINATORS)
      {
        if(numberOfNodes > 0)
        {
          logAllNodes = not logAllNodes;
          if(logAllNodes)
          {
            m2mMesh.logAllNodes();
          }
          else
          {
            nodeToLog = currentlyViewedOriginator;
            m2mMesh.nodeToLog(currentlyViewedOriginator);
          }
          drawWholeUi = true;
        }
      }
    break;
    case 'k':
      if(currentUiView == UI_LOG_CONFIG)
      {
        m2mMesh.setLoggingLevel(m2mMesh.loggingLevel() ^ m2mMesh.MESH_UI_LOG_USR_SEND);
        loggingLevelChanged = true;
      }
    break;
    case 'j':
      if(currentUiView == UI_LOG_CONFIG)
      {
        m2mMesh.setLoggingLevel(m2mMesh.loggingLevel() ^ m2mMesh.MESH_UI_LOG_USR_RECEIVED);
        loggingLevelChanged = true;
      }
    break;
    case 'i':
      if(currentUiView == UI_LOG_CONFIG)
      {
        m2mMesh.setLoggingLevel(m2mMesh.loggingLevel() ^ m2mMesh.MESH_UI_LOG_USR_FORWARDING);
        loggingLevelChanged = true;
      }
    break;
    case 'h':
      if(currentUiView == UI_LOG_CONFIG)
      {
        m2mMesh.setLoggingLevel(m2mMesh.loggingLevel() ^ m2mMesh.MESH_UI_LOG_TRC_SEND);
        loggingLevelChanged = true;
      }
    break;
    case 'g':
      if(currentUiView == UI_LOG_CONFIG)
      {
        m2mMesh.setLoggingLevel(m2mMesh.loggingLevel() ^ m2mMesh.MESH_UI_LOG_TRC_RECEIVED);
        loggingLevelChanged = true;
      }
    break;
    case 'f':
      if(currentUiView == UI_LOG_CONFIG)
      {
        m2mMesh.setLoggingLevel(m2mMesh.loggingLevel() ^ m2mMesh.MESH_UI_LOG_TRC_FORWARDING);
        loggingLevelChanged = true;
      }
      else if(currentUiView == UI_PROTOCOLS)
      {
        m2mMesh.setServiceFlags(m2mMesh.serviceFlags() ^ m2mMesh.PROTOCOL_TRACE_RESPONDER);
        servicesChanged = true;
      }
    break;
    case 'e':
      if(currentUiView == UI_LOG_CONFIG)
      {
        m2mMesh.setLoggingLevel(m2mMesh.loggingLevel() ^ m2mMesh.MESH_UI_LOG_PRP_SEND);
        loggingLevelChanged = true;
      }
      else if(currentUiView == UI_PROTOCOLS)
      {
        m2mMesh.setServiceFlags(m2mMesh.serviceFlags() ^ m2mMesh.PROTOCOL_USR_FORWARD);
        servicesChanged = true;
      }
    break;
    case 'd':
      if(currentUiView == UI_LOG_CONFIG)
      {
        m2mMesh.setLoggingLevel(m2mMesh.loggingLevel() ^ m2mMesh.MESH_UI_LOG_PRP_RECEIVED);
        loggingLevelChanged = true;
      }
      else if(currentUiView == UI_PROTOCOLS)
      {
        m2mMesh.setServiceFlags(m2mMesh.serviceFlags() ^ m2mMesh.PROTOCOL_USR_RECEIVE);
        servicesChanged = true;
      }
    break;
    case 'c':
      if(currentUiView == UI_LOG_CONFIG)
      {
        m2mMesh.setLoggingLevel(m2mMesh.loggingLevel() ^ m2mMesh.MESH_UI_LOG_PRP_FORWARDING);
        loggingLevelChanged = true;
      }
      else if(currentUiView == UI_PROTOCOLS)
      {
        m2mMesh.setServiceFlags(m2mMesh.serviceFlags() ^ m2mMesh.PROTOCOL_USR_SEND);
        servicesChanged = true;
      }
      else if(currentUiView == UI_STATUS || currentUiView == UI_LOGS)
      {
        if(currentChannel == 1)
        {
          currentChannel = 6;
        }
        else if(currentChannel == 6)
        {
          currentChannel = 11;
        }
        else if(currentChannel == 11)
        {
          currentChannel = 1;
        }
        channelLastChanged = millis();
        currentChannelChanged = true;
        addStringLog("Channel: " + String(currentChannel));
        drawUi();
      }
    break;
    case 'b':
      if(currentUiView == UI_LOG_CONFIG)
      {
        m2mMesh.setLoggingLevel(m2mMesh.loggingLevel() ^ m2mMesh.MESH_UI_LOG_NHS_SEND);
        loggingLevelChanged = true;
      }
      else if(currentUiView == UI_PROTOCOLS)
      {
        m2mMesh.setServiceFlags(m2mMesh.serviceFlags() ^ m2mMesh.PROTOCOL_NHS_TIME_SERVER);
        servicesChanged = true;
      }
    break;
    case 'a':
      if(currentUiView == UI_LOG_CONFIG)
      {
        m2mMesh.setLoggingLevel(m2mMesh.loggingLevel() ^ m2mMesh.MESH_UI_LOG_NHS_RECEIVED);
        loggingLevelChanged = true;
      }
      else if(currentUiView == UI_PROTOCOLS)
      {
        m2mMesh.setServiceFlags(m2mMesh.serviceFlags() ^ m2mMesh.PROTOCOL_NHS_INCLUDE_ORIGINATORS);
        servicesChanged = true;
      }
    break;
    case '9':
      if(currentUiView == UI_LOG_CONFIG)
      {
        m2mMesh.setLoggingLevel(m2mMesh.loggingLevel() ^ m2mMesh.MESH_UI_LOG_NHS_FORWARDING);
        loggingLevelChanged = true;
      }
      else if(currentUiView == UI_PROTOCOLS)
      {
        m2mMesh.setServiceFlags(m2mMesh.serviceFlags() ^ m2mMesh.PROTOCOL_NHS_FORWARD);
        servicesChanged = true;
      }
    break;
    case '8':
      if(currentUiView == UI_LOG_CONFIG)
      {
        m2mMesh.setLoggingLevel(m2mMesh.loggingLevel() ^ m2mMesh.MESH_UI_LOG_OGM_SEND);
        loggingLevelChanged = true;
      }
      else if(currentUiView == UI_PROTOCOLS)
      {
        m2mMesh.setServiceFlags(m2mMesh.serviceFlags() ^ m2mMesh.PROTOCOL_NHS_RECEIVE);
        servicesChanged = true;
      }
    break;
      case '7':
      if(currentUiView == UI_LOG_CONFIG)
      {
        m2mMesh.setLoggingLevel(m2mMesh.loggingLevel() ^ m2mMesh.MESH_UI_LOG_OGM_RECEIVED);
        loggingLevelChanged = true;
      }
      else if(currentUiView == UI_PROTOCOLS)
      {
        m2mMesh.setServiceFlags(m2mMesh.serviceFlags() ^ m2mMesh.PROTOCOL_NHS_SEND);
        servicesChanged = true;
      }
    break;
    case '6':
      if(currentUiView == UI_LOG_CONFIG)
      {
        m2mMesh.setLoggingLevel(m2mMesh.loggingLevel() ^ m2mMesh.MESH_UI_LOG_OGM_FORWARDING);
        loggingLevelChanged = true;
      }
      else if(currentUiView == UI_PROTOCOLS)
      {
        m2mMesh.setServiceFlags(m2mMesh.serviceFlags() ^ m2mMesh.PROTOCOL_OGM_FORWARD);
        servicesChanged = true;
      }
    break;
    case '5':
      if(currentUiView == UI_LOG_CONFIG)
      {
        m2mMesh.setLoggingLevel(m2mMesh.loggingLevel() ^ m2mMesh.MESH_UI_LOG_ELP_SEND);
        loggingLevelChanged = true;
      }
      else if(currentUiView == UI_PROTOCOLS)
      {
        m2mMesh.setServiceFlags(m2mMesh.serviceFlags() ^ m2mMesh.PROTOCOL_OGM_RECEIVE);
        servicesChanged = true;
      }
    break;
    case '4':
      if(currentUiView == UI_LOG_CONFIG)
      {
        m2mMesh.setLoggingLevel(m2mMesh.loggingLevel() ^ m2mMesh.MESH_UI_LOG_ELP_RECEIVED);
        loggingLevelChanged = true;
      }
      else if(currentUiView == UI_PROTOCOLS)
      {
        m2mMesh.setServiceFlags(m2mMesh.serviceFlags() ^ m2mMesh.PROTOCOL_OGM_SEND);
        servicesChanged = true;
      }
    break;
    case '3':
      if(currentUiView == UI_LOG_CONFIG)
      {
        m2mMesh.setLoggingLevel(m2mMesh.loggingLevel() ^ m2mMesh.MESH_UI_LOG_ELP_FORWARDING);
        loggingLevelChanged = true;
      }
      else if(currentUiView == UI_PROTOCOLS)
      {
        m2mMesh.setServiceFlags(m2mMesh.serviceFlags() ^ m2mMesh.PROTOCOL_ELP_INCLUDE_PEERS);
        servicesChanged = true;
      }
    break;
    case '2':
      if(currentUiView == UI_LOG_CONFIG)
      {
        m2mMesh.setLoggingLevel(m2mMesh.loggingLevel() ^ m2mMesh.MESH_UI_LOG_INFORMATION);
        loggingLevelChanged = true;
      }
      else if(currentUiView == UI_PROTOCOLS)
      {
        m2mMesh.setServiceFlags(m2mMesh.serviceFlags() ^ m2mMesh.PROTOCOL_ELP_FORWARD);
        servicesChanged = true;
      }
    break;
    case '1':
      if(currentUiView == UI_LOG_CONFIG)
      {
        m2mMesh.setLoggingLevel(m2mMesh.loggingLevel() ^ m2mMesh.MESH_UI_LOG_WARNINGS);
        loggingLevelChanged = true;
      }
      else if(currentUiView == UI_PROTOCOLS)
      {
        m2mMesh.setServiceFlags(m2mMesh.serviceFlags() ^ m2mMesh.PROTOCOL_ELP_RECEIVE);
        servicesChanged = true;
      }
    break;
    case '0':
      if(currentUiView == UI_LOG_CONFIG)
      {
        m2mMesh.setLoggingLevel(m2mMesh.loggingLevel() ^ m2mMesh.MESH_UI_LOG_ERRORS);
        loggingLevelChanged = true;
      }
      else if(currentUiView == UI_PROTOCOLS)
      {
        m2mMesh.setServiceFlags(m2mMesh.serviceFlags() ^ m2mMesh.PROTOCOL_ELP_SEND);
        servicesChanged = true;
      }
    break;
    default:
    char logEntry[LOG_LINE_LENGTH];
    if(int(keypress)>31 && int(keypress)<128)
    {
      sprintf(logEntry, "Unknown choice '%c'",keypress);
    }
    else
    {
      sprintf(logEntry, "Unknown control character %02x",int(keypress));
    }
    addLog(logEntry);
    }
  }
}
