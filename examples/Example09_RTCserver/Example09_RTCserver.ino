/**
 * ESP-NOW mesh network test sketch
 * 
 * Author: Nick Reynolds
 * License: LGPL v2.1
 * 
 */
 
#include "credentials.h"  //Used to contain my own personal Wi-Fi credentials but not pushed to GitHub, comment this line out and use your own below
#ifndef WIFI_SSID
  #define WIFI_SSID "test"
  #define WIFI_PSK "testpresharedkey"
#endif

#include <m2mMesh.h>
bool joinedMesh = false;
uint8_t numberOfNodes = 0;
uint8_t numberOfReachableNodes = 0;
char ntpServer[] = "pool.ntp.org";
char timeZone[] = "GMTGMT-1,M3.4.0/01,M10.4.0/02";
char timeAndDate[] = "--/--/---- --:--:--";
uint32_t lastClockTick = 0;
bool rtcValid = false;

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
  WiFi.begin(WIFI_SSID,WIFI_PSK);
  Serial.print("Connecting to Wi-Fi");
  uint8_t retries = 120;
  while(WiFi.status() != WL_CONNECTED && retries-- > 0)
  {
    Serial.print('.');
    delay(500);
  }
  if(WiFi.status() == WL_CONNECTED)
  {
    Serial.println("Connected, configuring NTP");
    Serial.print("NTP server:");
    Serial.println(ntpServer);
    Serial.print("Timezone:");
    Serial.println(timeZone);
    m2mMesh.setNtpServer(ntpServer);
    m2mMesh.setTimeZone(timeZone);
  }
  else
  {
    Serial.println("Failed");
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
}

void printTimeAndDate(const time_t timestamp)
{
  strftime(timeAndDate, strlen(timeAndDate)+1, "%d/%m/%Y %H:%M:%S", localtime(&timestamp));
  Serial.println(timeAndDate);
}
