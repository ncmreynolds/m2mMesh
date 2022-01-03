/**
 * ESP-NOW mesh network test sketch
 * 
 * Author: Nick Reynolds
 * License: LGPL v2.1
 * 
 * See README.md for information
 * 
 */
 
#include <m2mMesh.h>
bool joinedMesh = false;
uint8_t numberOfNodes = 0;
uint8_t numberOfReachableNodes = 0;
char timeAndDate[] = "--/--/---- --:--:--";
uint32_t lastClockTick = 0;
bool rtcValid = false;
bool synced = false;
bool rtcSynced = false;

void setup() {
  Serial.begin(115200);
  delay(1000);
  if(m2mMesh.begin())
  {
    Serial.print("\n\nMesh started on channel:");
    Serial.println(WiFi.channel());
  }
  else
  {
    Serial.println("\n\nMesh failed to start");
  }
}

void loop() {
  m2mMesh.housekeeping();
  if(joinedMesh == false && m2mMesh.joined() == true)
  {
    joinedMesh = true;
    Serial.println("Joined mesh");
  }
  else if(joinedMesh == true && m2mMesh.joined() == false)
  {
    joinedMesh = false;
    Serial.println("Left mesh");
  }
  else if(numberOfNodes != m2mMesh.numberOfNodes() || numberOfReachableNodes != m2mMesh.numberOfReachableNodes())
  {
    numberOfNodes = m2mMesh.numberOfNodes();
    numberOfReachableNodes = m2mMesh.numberOfReachableNodes();
    Serial.print("Number of nodes:");
    Serial.print(numberOfNodes);
    Serial.print(", reachable nodes:");
    Serial.println(numberOfReachableNodes);
  }
  if(m2mMesh.synced())
  {
    if(synced == false)
    {
      synced = true;
      Serial.println("Mesh uptime synced");
    }
  }
  if(m2mMesh.rtcSynced())
  {
    if(rtcSynced == false)
    {
      rtcSynced = true;
      if(synced == false)
      {
        Serial.println("RTC offset synced, waiting for uptime sync to set RTC");
      }
      else
      {
        Serial.println("RTC synced");
      }
    }
  }
  if(rtcValid == false && m2mMesh.rtcValid() == true)
  {
    rtcValid = true;
    Serial.println("RTC now valid");
  }
  if(rtcValid == true && millis() - lastClockTick > 1000)
  {
    lastClockTick = millis();
    printTimeAndDate(time(nullptr));
  }
  if(m2mMesh.messageWaiting())
  {
    if(m2mMesh.nextDataType() == m2mMesh.USR_DATA_STRING)
    {
      Serial.print("Received message:\"");
      Serial.print(m2mMesh.retrieveString());
      Serial.print("\" from node:");
      Serial.println(m2mMesh.sourceId());
    }
    m2mMesh.markMessageRead();
  }
}

void printTimeAndDate(const time_t timestamp)
{
  strftime(timeAndDate, strlen(timeAndDate)+1, "%d/%m/%Y %H:%M:%S", localtime(&timestamp));
  Serial.println(timeAndDate);
}
