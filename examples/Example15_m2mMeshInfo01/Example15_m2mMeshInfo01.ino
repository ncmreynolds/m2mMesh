/*
 * This sketch shows information about the mesh over the serial console with an ncurses style interface
 * 
 * It was primarily written to help troubleshoot problems during development of the m2mMesh library
 * but can be useful for looking behind the scenes when developing your own applications with it
 * 
 * By default the device running this also participates in the mesh routing but that can be switched off
 * 
 */

#include <m2mMesh.h>

#if defined(ARDUINO_ESP8266_WEMOS_D1MINI)
ADC_MODE(ADC_VCC);
#endif

const uint8_t UI_STATUS = 0;
const uint8_t UI_ORIGINATORS = 1;
const uint8_t UI_PROTOCOLS = 2;
const uint8_t UI_LOG_CONFIG = 3;
const uint8_t UI_LOGS = 4;
const uint8_t LOG_LINE_LENGTH = 80;
const uint8_t TX_POWER_RED_THRESHOLD = 16;
const uint8_t TX_POWER_YELLOW_THRESHOLD  = 8;

uint8_t currentUiView = UI_STATUS;
bool drawWholeUi = true;
bool loggingPaused = false;
uint8_t periodicUIrefresh = 0;

bool sequenceNumberChanged = false;
uint32_t sequenceNumber = 0;

bool stable = false;
bool stableChanged = false;

uint8_t numberOfActiveNeighbours = 0;
bool numberOfActiveNeighboursChanged = false;

uint8_t numberOfNodes = 0;
bool numberOfNodesChanged = false;

uint8_t numberOfReachableNodes = 0;
bool numberOfReachableNodesChanged = false;

bool currentlyViewedOriginatorChanged = false;

bool scanIntervalChanged = false;
bool currentSSIDchanged = false;
bool currentChannelChanged = false;
uint32_t channelLastChanged = 0;
uint8_t currentChannel = 1;

bool supplyVoltageChanged = false;
uint32_t uiRedrawTimer;
bool currentTxPowerChanged = false;
uint8_t currentlyViewedOriginator = 0;
bool servicesChanged = false;

bool loggingLevelChanged = false;
uint32_t loggingLevel = 0;
uint8_t nodeToLog = 0;
bool logAllNodes = true;

uint32_t initialFreeHeap = ESP.getFreeHeap();    //Used to get a percentage value for free heap

uint8_t maxNeighboursOnPage = 20;
uint32_t lastRedraw = 0;
uint32_t redrawInterval = 250;

void drawUi();
void addLog(char *);
void popupBox(String);
void setScrollWindow(uint8_t, uint8_t);
void hideCursor();
void eraseScreen();
void eraseLine();
void moveToXy(uint8_t, uint8_t);
void inverseOn();
void inverseOff();
void printAtXy(uint8_t, uint8_t, String);
void printUptimeAtXy(uint8_t, uint8_t, uint32_t);
void drawTitle(String);
void printCentred(uint8_t, String);
void packetType(char *, uint8_t);
void statusView();
void protocolsView();
void loggingLevelView();
void loggingView();
void originatorView();
void inputHandling();

void ICACHE_FLASH_ATTR setup(){
  char localNodeName[] = "Analyser-00000000";
  #if defined (ESP8266)
  sprintf(localNodeName,"Analyser-%08x",ESP.getChipId());
  #elif defined (ESP32)
  sprintf(localNodeName,"Analyser-%08x",getChipId());
  #endif
  m2mMesh.setNodeName(localNodeName);
  Serial.begin(115200);
  WiFi.disconnect();
  WiFi.persistent(false);
  drawUi();
  Serial.write(17); //Send an XON to unlock an XOFFed terminal
  //m2mMesh.enableDebugging(Serial);
  //m2mMesh.enableDebugging(Serial,m2mMesh.MESH_UI_LOG_INFORMATION | m2mMesh.MESH_UI_LOG_WARNINGS | m2mMesh.MESH_UI_LOG_ERRORS | m2mMesh.MESH_UI_LOG_PEER_MANAGEMENT | 
  //  m2mMesh.MESH_UI_LOG_ELP_RECEIVED | m2mMesh.MESH_UI_LOG_OGM_RECEIVED | m2mMesh.MESH_UI_LOG_NHS_RECEIVED |  m2mMesh.MESH_UI_LOG_USR_RECEIVED);
  //m2mMesh.enableDebugging(Serial,m2mMesh.MESH_UI_LOG_INFORMATION | m2mMesh.MESH_UI_LOG_WARNINGS | m2mMesh.MESH_UI_LOG_ERRORS | m2mMesh.MESH_UI_LOG_PEER_MANAGEMENT | m2mMesh.MESH_UI_LOG_OGM_RECEIVED | m2mMesh.MESH_UI_LOG_NHS_RECEIVED |  m2mMesh.MESH_UI_LOG_USR_RECEIVED | m2mMesh.MESH_UI_LOG_OGM_SEND | m2mMesh.MESH_UI_LOG_NHS_SEND | m2mMesh.MESH_UI_LOG_OGM_FORWARDING );
  m2mMesh.enableDebugging(Serial,m2mMesh.MESH_UI_LOG_INFORMATION | m2mMesh.MESH_UI_LOG_WARNINGS | m2mMesh.MESH_UI_LOG_ERRORS | m2mMesh.MESH_UI_LOG_PEER_MANAGEMENT | m2mMesh.MESH_UI_LOG_NHS_RECEIVED);
  loggingLevel = m2mMesh.loggingLevel();  
  //m2mMesh.begin(16,11);  //Begin the ESP-Now mesh with default arguments  
  m2mMesh.begin();  //Begin the ESP-Now mesh with default arguments  
  #if defined(ARDUINO_ESP8266_WEMOS_D1MINI) || (ARDUINO_Pocket32)
  m2mMesh.enableActivityLed(LED_BUILTIN,LOW);  //Enable the activity LED on the pin LED_BUILTIN. The LED is lit when the pin is LOW
  #endif
  currentChannel = WiFi.channel();
}

void loop() {
  if(millis() - lastRedraw > redrawInterval) {
    if(numberOfActiveNeighbours != m2mMesh.numberOfActiveNeighbours())
    {
      numberOfActiveNeighbours = m2mMesh.numberOfActiveNeighbours();
      numberOfActiveNeighboursChanged = true;
    }
    if(numberOfReachableNodes != m2mMesh.numberOfReachableNodes())
    {
      numberOfReachableNodes = m2mMesh.numberOfReachableNodes();
      numberOfReachableNodesChanged = true;
    }
    if(numberOfNodes != m2mMesh.numberOfNodes())
    {
      numberOfNodes = m2mMesh.numberOfNodes();
      numberOfNodesChanged = true;
    }
    if(sequenceNumber != m2mMesh.sequenceNumber())
    {
      sequenceNumber = m2mMesh.sequenceNumber();
      sequenceNumberChanged = true;
    }
    if(stable != m2mMesh.stable())
    {
      stable = m2mMesh.stable();
      stableChanged = true;
    }
    if(loggingLevel != m2mMesh.loggingLevel())
    {
      loggingLevel = m2mMesh.loggingLevel();
      loggingLevelChanged = true;
    }
    inputHandling();
    drawUi();
    lastRedraw = millis();
  }
  m2mMesh.housekeeping();  //This needs to run regularly otherwise the mesh will fail
  if(m2mMesh.messageWaiting())
  {
    m2mMesh.markMessageRead();  //Simply trash any inbound application messages
  }
  if(channelLastChanged - millis() > 5000)
  {
    changeChannel();
    currentChannelChanged = false;
  }
}

void ICACHE_FLASH_ATTR friendlyUptime(uint32_t uptime, char * formattedUptime)
{
  uint8_t seconds = (uptime/   1000ul)%60;
  uint8_t minutes = (uptime/  60000ul)%60;
  uint8_t hours =   (uptime/3600000ul);
  sprintf(formattedUptime,"%02dh%02dm%02ds",hours,minutes,seconds);
}


void ICACHE_FLASH_ATTR drawUi()
{
  if(currentUiView == UI_PROTOCOLS)
  {
    protocolsView();
  }
  else if(currentUiView == UI_LOGS)
  {
    loggingView();
  }
  else if(currentUiView == UI_LOG_CONFIG)
  {
    loggingLevelView();
  }
  else if(currentUiView == UI_ORIGINATORS)
  {
    originatorView();
  }
  else if(currentUiView == UI_STATUS)
  {
    statusView();
  }
}

void ICACHE_FLASH_ATTR addLog(char *logEntry)
{
  if(not loggingPaused)
  {
    //Change the 'scroll region' to lines 14-23 of the screen
    //Serial.printf ("\033[14,23r");
    if(currentUiView == UI_STATUS)
    {
      char uptime[] = "00h00m00s";
      friendlyUptime(millis(),uptime);
    moveToXy(80,23);
    Serial.println();
      Serial.print(uptime);
      Serial.print(':');
      Serial.print(logEntry);
    }
    else if(currentUiView == UI_LOGS)
    {
      char uptime[] = "00h00m00s";
      friendlyUptime(millis(),uptime);
    moveToXy(80,23);
    Serial.println();
      Serial.print(uptime);
      Serial.print(':');
      Serial.print(logEntry);
    }
  }
}

void changeChannel()
{
  WiFi.channel(currentChannel);
  WiFi.mode(WIFI_AP);
  WiFi.softAP("asdfadfghasdfg", "6v5457sfx1vb8frgth", currentChannel, true, 0);
  delay(500);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
}

#if defined (ESP32)
uint32_t getChipId()
{
  uint32_t chipId = 0;
  for(int i=0; i<17; i=i+8)
  {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  return(chipId);
}
#endif
