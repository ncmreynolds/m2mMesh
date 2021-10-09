/**
 * ESP-NOW mesh network test sketch
 * 
 * Author: Nick Reynolds
 * License: LGPL v2.1
 * 
 */
 
#include <m2mMesh.h>

void meshCallback(meshEvent event)
{
  if(event == meshEvent::joined)  //This station has gone online
  {
    Serial.println("Joined mesh");
    return;
  }
  else if(event == meshEvent::left) //This station has gone offline
  {
    Serial.println("Left mesh");
    return;
  }
  else if(event == meshEvent::stable) //All the nodes have the same view of the mesh
  {
    Serial.println("Mesh is stable");
    return;
  }
  else if(event == meshEvent::changing) //At least one node has changed
  {
    Serial.println("Mesh changing");
    return;
  }
  else if(event == meshEvent::synced) //Uptime synced with the mesh
  {
    Serial.println("Mesh synced");
    return;
  }
  else if(event == meshEvent::message)  //Message received
  {
    Serial.println("Mesh message received");
    m2mMesh.markMessageRead();
    return;
  }
  return;
}


void setup() {
  Serial.begin(115200);
  delay(1000);
  //m2mMesh.enableDebugging(Serial,m2mMesh.MESH_UI_LOG_ALL_RECEIVED_PACKETS | m2mMesh.MESH_UI_LOG_ALL_SENT_PACKETS | m2mMesh.MESH_UI_LOG_INFORMATION | m2mMesh.MESH_UI_LOG_WARNINGS | m2mMesh.MESH_UI_LOG_ERRORS | m2mMesh.MESH_UI_LOG_PEER_MANAGEMENT | m2mMesh.MESH_UI_LOG_NHS_RECEIVED );
  m2mMesh.setCallback(meshCallback);  //Set the callback function
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
}
