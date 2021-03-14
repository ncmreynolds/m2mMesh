#ifndef m2mMesh_cpp
#define m2mMesh_cpp
#include "m2mMesh.h"

//Wrapper hack to allow the use of callbacks to the ESP-Now C library from the C++ class

m2mMesh* m2mMeshPointer = nullptr;	//A pointer to 'this' eventually

#ifdef ESP8266
void espNowSendCallbackWrapper(uint8_t* a, uint8_t b)
#elif defined(ESP32)
void espNowSendCallbackWrapper(const uint8_t *a, esp_now_send_status_t b)
#endif
{
    if (m2mMeshPointer)
	{
        m2mMeshPointer->espNowSendCallback(a,b);
	}
}

#ifdef ESP8266
void espNowReceiveCallbackWrapper(uint8_t *a, uint8_t *b, uint8_t c)
#elif defined(ESP32)
void espNowReceiveCallbackWrapper(const uint8_t *a, const uint8_t *b, int32_t c)
#endif
{
    if (m2mMeshPointer)
	{
        m2mMeshPointer->espNowReceiveCallback(a,b,c);
	}
}


//Constructor function
ICACHE_FLASH_ATTR m2mMesh::m2mMesh()
{
	//Set the pointer for the C callback wrapper hack
	m2mMeshPointer = this;
}
//Destructor function
ICACHE_FLASH_ATTR m2mMesh::~m2mMesh()
{
	//Do nothing
}

//Overloaded function for starting the mesh
void ICACHE_FLASH_ATTR m2mMesh::begin()
{
	begin(_maxNumberOfOriginators);
}

void ICACHE_FLASH_ATTR m2mMesh::begin(const uint8_t i)
{
	_maxNumberOfOriginators = i;
	//Allocate a block of memory for the originator table, a minimum of one
	if(_maxNumberOfOriginators > 0)
	{
		_originator = new originatorInfo[_maxNumberOfOriginators];
	}
	else
	{
		_originator = new originatorInfo[1];
	}
	//ESP8266/ESP8285 and ESP32 need different handling and have subtly different APIs
	#if defined(ESP8266)
	if (_debugEnabled == true && _loggingLevel & MESH_UI_LOG_INFORMATION)
	{
		_debugStream->println();
		_debugStream->print(F("running on ESP8266/ESP8285"));
	}
	#elif defined(ESP32)
	if (_debugEnabled == true && _loggingLevel & MESH_UI_LOG_INFORMATION)
	{
		_debugStream->println();
		_debugStream->print(F("m2mMesh running on ESP32"));
	}
	#endif
	if (_debugEnabled == true && _loggingLevel & MESH_UI_LOG_INFORMATION)
	{
		if(_maxNumberOfOriginators > 0)
		{
			_debugStream->printf("\r\nm2mMesh started with capacity for %d nodes",_maxNumberOfOriginators);
		}
		else
		{
			_debugStream->print(F("\r\nm2mMesh started with automatic allocation of memory. Stability may be impacted."));
		}
	}
	#if defined(ESP32)
	if(WiFi.getMode() == WIFI_OFF)
	{
		WiFi.mode(WIFI_STA);									//By default the ESP32 starts with WiFi disabled and if the application hasn't enabled it there will be an exception, crash & reboot
		WiFi.disconnect();										//Disconnect, if connected
	}
	#endif
	WiFi.macAddress(_localMacAddress);							//Retrieve the local MAC address, which may need tidying up to get the base MAC address
	#if defined(ESP8266)
	_localMacAddress[0] = _localMacAddress[0] & B11111101;		//Strip the variable bits out of the MAC address first octet, which are set in AP mode
	#elif defined(ESP32)
	if(WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA)
	{
		_localMacAddress[5] = _localMacAddress[5] - 1;			//Decrement the last octet of the MAC address, it is incremented in AP mode
	}
	#endif
	_initESPNow();												//Initialise ESP-NOW, the function handles the differences between ESP8266/ESP8285 and ESP32
	_currentInterval[ELP_PACKET_TYPE] = ELP_DEFAULT_INTERVAL;	//Packet sending intervals, per packet type which may be adjust later to reduce congestion
	_currentInterval[OGM_PACKET_TYPE] = OGM_DEFAULT_INTERVAL;
	_currentInterval[NHS_PACKET_TYPE] = NHS_DEFAULT_INTERVAL;
	_currentInterval[USR_PACKET_TYPE] = USR_DEFAULT_INTERVAL;
	_currentInterval[FSP_PACKET_TYPE] = FSP_DEFAULT_INTERVAL;
	_lastSent[ELP_PACKET_TYPE] = 1000ul - ELP_DEFAULT_INTERVAL;	//Offset sending times slightly between the different protocols
	_lastSent[OGM_PACKET_TYPE] = 3000ul - OGM_DEFAULT_INTERVAL;	//Send the first packets sooner than the default interval
	_lastSent[NHS_PACKET_TYPE] = 5000ul - NHS_DEFAULT_INTERVAL;	//would cause
	_lastSent[USR_PACKET_TYPE] = 7000ul - USR_DEFAULT_INTERVAL;
	_lastSent[FSP_PACKET_TYPE] = 9000ul - FSP_DEFAULT_INTERVAL;
	_currentTtl[ELP_PACKET_TYPE] = ELP_DEFAULT_TTL;				//TTLs, per packet type which are unlikely to change
	_currentTtl[OGM_PACKET_TYPE] = OGM_DEFAULT_TTL;
	_currentTtl[NHS_PACKET_TYPE] = NHS_DEFAULT_TTL;
	_currentTtl[USR_PACKET_TYPE] = USR_DEFAULT_TTL;
	_currentTtl[FSP_PACKET_TYPE] = FSP_DEFAULT_TTL;
	//Check the state of the AP
	if(WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA)
	{
		_softAPstate == true;
	}
}

void ICACHE_FLASH_ATTR m2mMesh::begin(const uint8_t i, const uint8_t c)
{
	WiFi.channel(c);		//Set the receive channel
	_currentChannel = c;	//Set the send channel
	begin(i);				//Begin for i nodes
}

void ICACHE_FLASH_ATTR m2mMesh::end()
{
	//Remove callback functions
	//Free memory
}
//Enable debugging on a stream, usually Serial but let's not assume that. This version accepts the default log level
void m2mMesh::enableDebugging(Stream &debugStream)
{
	enableDebugging(debugStream,_loggingLevel);
}
//Enable debugging on a stream, usually Serial but let's not assume that
void m2mMesh::enableDebugging(Stream &debugStream, uint32_t level)
{
	_debugStream = &debugStream;	//Set the stream used for debugging
	_debugEnabled = true;			//Flag that debugging is enabled
	_loggingLevel = level;			//Set the logging level
	if(_loggingLevel & MESH_UI_LOG_INFORMATION)
	{
		_debugStream->println();
		_debugStream->print(F("m2mMesh debugging enabled"));	//Announce this joyous event
	}
}
//Enable debugging on a stream, after being paused
void m2mMesh::enableDebugging(uint32_t level)
{
	if(_debugStream != nullptr)
	{
		_loggingLevel = level;			//Set the logging level
		_debugEnabled = true;			//Flag that debugging is enabled
		if(_loggingLevel & MESH_UI_LOG_INFORMATION)
		{
			_debugStream->println();
			_debugStream->print(F("m2mMesh debugging enabled"));	//Announce this joyous event
		}
	}
}
//Enable debugging on a stream, after being paused
void m2mMesh::enableDebugging()
{
	if(_debugStream != nullptr)
	{
		_debugEnabled = true;
		if(_loggingLevel & MESH_UI_LOG_INFORMATION)
		{
			_debugStream->println();
			_debugStream->print(F("m2mMesh debugging enabled"));	//Announce this joyous event
		}
	}
}
//Disable debugging
void m2mMesh::disableDebugging()
{
	if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_INFORMATION)
	{
		_debugStream->println();
		_debugStream->print(F("m2mMesh debugging disabled"));	//Warn that debugging is disabled
	}
	_debugEnabled = false;			//Flag that debugging is enabled
}

void m2mMesh::nodeToLog(uint8_t id)	//Sets the node to log
{
	_nodeToLog = id;
}
void m2mMesh::logAllNodes()	//Sets logging to all nodes
{
	_nodeToLog = MESH_ORIGINATOR_NOT_FOUND;
}


void m2mMesh::_initESPNow()
{
	//Check if the ESP-NOW initialization was successful, ESP8286/ESP8285 have different return results
	#if defined(ESP8266)
	uint8_t result = esp_now_init();
	#elif defined(ESP32)
	esp_err_t result = esp_now_init();
	#endif
	if (_debugEnabled == true && _loggingLevel & MESH_UI_LOG_INFORMATION)
	{
		_debugStream->println();
		_debugStream->print(F("m2mMesh initialising ESP-NOW "));
	}
	if (result != ESP_OK)
	{
		if (_debugEnabled == true && _loggingLevel & MESH_UI_LOG_INFORMATION)
		{
			_debugStream->print(F("- failed, restarting in 3s"));
		}
		delay(3000ul);
		ESP.restart();
	}
	else if (_debugEnabled == true && _loggingLevel & MESH_UI_LOG_INFORMATION)
	{
		_debugStream->print(F("- success"));
	}
	//The ESP8266/ESP8285 require a 'role' to be set, which is vaguely analogous to the ifidx on ESP32. Without this you cannot send packets.
	#if defined(ESP8266)
	esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
	#endif
	//Packets are processed with callback functions, which are called via wrappers from the C++ class
	//Register the callback routines needed to process ESP-Now traffic, using the wrapper functions
	esp_now_register_send_cb(espNowSendCallbackWrapper);
	esp_now_register_recv_cb(espNowReceiveCallbackWrapper);
	// Add the broadcast MAC address as an ESP-NOW peer. This is used for any message that needs to 'flood' the network. Without this node discovery will not work.
	if (_debugEnabled == true && _loggingLevel & MESH_UI_LOG_INFORMATION)
	{
		_debugStream->println();
		_debugStream->print(F("m2mMesh adding broadcast MAC address as a 'peer' to enable mesh discovery "));
	}
	#if defined(ESP8266)
	result = esp_now_add_peer(_broadcastMacAddress, ESP_NOW_ROLE_COMBO, _currentChannel, NULL, 0);
	#elif defined(ESP32)
	//Create a temporary peer info object
	esp_now_peer_info_t peerInfo;
	memcpy(peerInfo.peer_addr, _broadcastMacAddress, 6);
	//Peer info HAS to include an ifidx depending on mode (AP/STA) of the ESP32. If this changes later, it needs to change in the peer info too.
	if(WiFi.getMode() == WIFI_STA)
	{
		peerInfo.ifidx = WIFI_IF_STA;
	}
	else
	{
		peerInfo.ifidx = WIFI_IF_AP;
	}
	peerInfo.channel = _currentChannel;  
	peerInfo.encrypt = false;
	result = esp_now_add_peer(&peerInfo);
	#endif
	if (result != ESP_OK)
	{
		if (_debugEnabled == true && _loggingLevel & MESH_UI_LOG_INFORMATION)
		{
			_debugStream->print(F("- failed, restarting in 3s"));
		}
		delay(3000ul);
		ESP.restart();
	}
	else if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_INFORMATION)
	{
		_debugStream->print(F("- success"));
	}
}

#ifdef ESP8266
void ICACHE_FLASH_ATTR m2mMesh::espNowSendCallback(uint8_t* macAddress, uint8_t status)
#elif defined(ESP32)
void ICACHE_FLASH_ATTR m2mMesh::espNowSendCallback(const uint8_t* macAddress, esp_now_send_status_t status)
#endif
{
	//Don't HAVE to do anything in the send callback
	/*if(_debugEnabled == true)
	{
		if(status == ESP_OK)
		{
			_debugStream->printf("Packet sent to %02x:%02x:%02x:%02x:%02x:%02x",macAddress[0],macAddress[1],macAddress[2],macAddress[3],macAddress[4],macAddress[5]);
		}
	}*/
}

#ifdef ESP8266
void ICACHE_FLASH_ATTR m2mMesh::espNowReceiveCallback(uint8_t *macAddress, uint8_t *data, uint8_t length)
#elif defined(ESP32)
void ICACHE_FLASH_ATTR m2mMesh::espNowReceiveCallback(const uint8_t *macAddress, const uint8_t *data, int32_t length)
#endif
{
	if(_receiveBuffer.length == 0)	//The buffer is empty
	{
		//Mark the timestamp
		_receiveBuffer.timestamp = millis();
		//Copy the packet into a packet buffer
		memcpy (_receiveBuffer.macAddress, macAddress, 6);	//Copy MAC address
		if(length > 250)	//This is unlikely to be excessive, but should avoid a crash
		{
			length = 250;
		}
		memcpy (_receiveBuffer.data, data, length);
		_receiveBuffer.length = length;
		_rxPackets++;
	}
	else
	{
		_droppedRxPackets++;	//Increase the count of dropped packets
	}
	if(_activityLedEnabled)
	{
		_blinkActivityLed();	//Blinky blinky
	}
}

#if defined(ESP8266)
uint8_t ICACHE_FLASH_ATTR m2mMesh::_sendPacket(packetBuffer &packet)
#elif defined(ESP32)
esp_err_t ICACHE_FLASH_ATTR m2mMesh::_sendPacket(packetBuffer &packet)
#endif
{
	#if defined(ESP8266)
	uint8_t returnValue = esp_now_send(packet.macAddress, packet.data, packet.length);	//Send packet on ESP8266/ESP8285
	#elif defined(ESP32)
	esp_err_t returnValue = esp_now_send(packet.macAddress, packet.data, packet.length);	//Send packet on ESP32
	#endif
	if(returnValue == ESP_OK)
	{
		_txPackets++;	//Update the packet stats
	}
	else
	{
		_droppedTxPackets;	//Update the packet stats
	}
	if(_activityLedEnabled)
	{
		_blinkActivityLed(); //Blinky blinky
	}
	return(returnValue);
}

void ICACHE_FLASH_ATTR m2mMesh::housekeeping()
{
	//Process received packets first, if available
	if(_receiveBuffer.length > 0)	//There's something waiting to process
	{
		_processPacket(_receiveBuffer);
		_activityOcurred = true;
	}
	if (_serviceFlags & PROTOCOL_ELP_SEND && millis() - (_lastSent[ELP_PACKET_TYPE] - random(ANTI_COLLISION_JITTER)) > _currentInterval[ELP_PACKET_TYPE]) 	//Apply some jitter to the send times
	{
		if(_sendElp(_sendBuffer))
		{
			_lastSent[ELP_PACKET_TYPE] = millis();
		}
		else
		{
			_lastSent[ELP_PACKET_TYPE] += random(50,100);	//Cause a retry in 50-100ms
		}
		_activityOcurred = true;
	}
	else if (_serviceFlags & PROTOCOL_OGM_SEND && millis() - (_lastSent[OGM_PACKET_TYPE] - random(ANTI_COLLISION_JITTER)) > _currentInterval[OGM_PACKET_TYPE])
	{
		if(_sendOgm(_sendBuffer))
		{
			_lastSent[OGM_PACKET_TYPE] = millis();
		}
		else
		{
			_lastSent[OGM_PACKET_TYPE] += random(50,100);	//Cause a retry in 50-100ms
		}
		_activityOcurred = true;
	}
	else if (_serviceFlags & PROTOCOL_NHS_SEND && millis() - (_lastSent[NHS_PACKET_TYPE] - random(ANTI_COLLISION_JITTER)) > _currentInterval[NHS_PACKET_TYPE])
	{
		//Check the availability of the time server
		/*if(not(_actingAsTimeServer) && currentMeshTimeServer != MESH_ORIGINATOR_NOT_FOUND && not dataIsValid(currentMeshTimeServer,NHS_PACKET_TYPE))
		{
			//Choose a new time server if it's offline
			chooseNewTimeServer();
		}*/
		if(_sendNhs(_sendBuffer))
		{
			_lastSent[NHS_PACKET_TYPE] = millis();
		}
		else
		{
			_lastSent[NHS_PACKET_TYPE] += random(50,100);	//Cause a retry in 50-100ms
		}
		_activityOcurred = true;
	}
	if(_activityOcurred || millis() - _lastHousekeeping > _housekeepingInterval)	//Need to refresh the stats and routes
	{
		//Update the broad mesh statistics and fingerprint
		_numberOfActiveNeighbours = 0;
		_numberOfReachableOriginators = 0;
		//Copy in the local MAC address to the mesh MAC address
		memcpy(_meshMacAddress, _localMacAddress, 6);
		for(uint8_t originatorId = 0; originatorId < _numberOfOriginators; originatorId++)
		{
			if(_dataIsValid(originatorId,ELP_PACKET_TYPE))
			{
				_numberOfActiveNeighbours++;
			}
			if(_dataIsValid(originatorId,OGM_PACKET_TYPE))
			{
				_numberOfReachableOriginators++;
				_meshMacAddress[0] = _meshMacAddress[0] ^ _originator[originatorId].macAddress[0];	//XOR the originator MAC address
				_meshMacAddress[1] = _meshMacAddress[1] ^ _originator[originatorId].macAddress[1];	//XOR the originator MAC address
				_meshMacAddress[2] = _meshMacAddress[2] ^ _originator[originatorId].macAddress[2];	//XOR the originator MAC address
				_meshMacAddress[3] = _meshMacAddress[3] ^ _originator[originatorId].macAddress[3];	//XOR the originator MAC address
				_meshMacAddress[4] = _meshMacAddress[4] ^ _originator[originatorId].macAddress[4];	//XOR the originator MAC address
				_meshMacAddress[5] = _meshMacAddress[5] ^ _originator[originatorId].macAddress[5];	//XOR the originator MAC address
			}
			if(millis() - _originator[originatorId].ogmEchoLastConfirmed > _currentInterval[OGM_PACKET_TYPE])	//We have missed an OGM echo
			{
				_originator[originatorId].ogmEchoes = _originator[originatorId].ogmEchoes >> 1;					//Right shift the OGM Echo receipt confirmation bitmask, which will make it worse
				_calculateLtq(originatorId);																	//Recalculate the Local Transmission quality
				_originator[originatorId].ogmEchoLastConfirmed = millis();										//Record the time of the last missed OGM echo
			}
			if(millis() - _originator[originatorId].ogmReceiptLastConfirmed > _originator[originatorId].interval[OGM_PACKET_TYPE])	//We have missed an OGM
			{
				_originator[originatorId].ogmReceived = _originator[originatorId].ogmReceived >> 1;				//Right shift the OGM receipt confirmation bitmask, which will make it worse
				_calculateLtq(originatorId);																	//Recalculate the Local Transmission quality
				_originator[originatorId].ogmReceiptLastConfirmed = millis();									//Record the time of the last missed OGM
			}
			
		}
		_activityOcurred = false;
		_lastHousekeeping = millis();
	}
	//If the activity LED is lit, check if it's time to turn it off
	if(_activityLedEnabled && _activityLedState == true && millis() - _activityLedTimer > ACTIVITY_LED_BLINK_TIME)
	{
		digitalWrite(_activityLedPin,not _activityLedOnState);
		_activityLedState = false;
	}
}

void ICACHE_FLASH_ATTR m2mMesh::_processPacket(packetBuffer &packet)
{
	if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_ALL_RECEIVED_PACKETS)
	{
		_debugStream->println();
		_debugStream->print(F("m2mMesh Received from "));
		_debugPacket(packet);
	}
	//Start processing the packet
	if(packet.data[1] == MESH_PROTOCOL_VERSION)	//Check the protocol version is valid
	{
		uint8_t routerId = MESH_ORIGINATOR_NOT_FOUND;
		uint8_t originatorId = MESH_ORIGINATOR_NOT_FOUND;
		uint8_t destinationId = MESH_ORIGINATOR_NOT_FOUND;
		bool isEcho = false;
		//Handle the incoming MAC address. The ESP8266 and ESP32 both vary their MAC address differently depending on whether in STA or AP mode
		/*if(packet.macAddress[0] & B00000010) && packet.macAddress[1] == packet.data[9] && packet.macAddress[2] == packet.data[10] && packet.macAddress[3] == packet.data[11] && packet.macAddress[4] == packet.data[12] && packet.macAddress[5] == packet.data[13])
		{*/
			//This is an ESP8266 in AP mode
			packet.macAddress[0] = packet.macAddress[0] & B11111101;	//Remove the variable bit for ESP8266/ESP8285
		/*}
		else*/ if(packet.macAddress[0] == packet.data[8] && packet.macAddress[1] == packet.data[9] && packet.macAddress[2] == packet.data[10] && packet.macAddress[3] == packet.data[11] && packet.macAddress[4] == packet.data[12] && packet.macAddress[5] == packet.data[13] + 1)
		{
			//This is an ESP32 in AP mode
			packet.macAddress[5] = packet.macAddress[5] - 1;			//Decrement the last octet for ESP32
		}
		if(not _isLocalMacAddress(packet.macAddress))
		{
			routerId = _originatorIdFromMac(packet.macAddress);
			if(routerId == MESH_ORIGINATOR_NOT_FOUND)
			{
				if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_PEER_MANAGEMENT)
				{
					_debugStream->print(F("\r\nm2mMesh RTR "));
				}
				routerId = _addOriginator(packet.macAddress,_currentChannel);
			}
		}
		uint8_t originatorMacAddress[6];
		memcpy(&originatorMacAddress[0], &packet.data[8], 6);
		//The originator may be a completely new device, so look for it but ignore any echoes of this node's traffic
		if(_isLocalMacAddress(originatorMacAddress))
		{
			isEcho = true;
		}
		else
		{
			originatorId = _originatorIdFromMac(originatorMacAddress);
			if(originatorId == MESH_ORIGINATOR_NOT_FOUND)
			{
				if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_PEER_MANAGEMENT)
				{
					_debugStream->print(F("\r\nm2mMesh SRC "));
				}
				originatorId = _addOriginator(originatorMacAddress,_currentChannel);
			}
		}
		//memcpy(originatorMacAddress, packet.data[8], 6);			//Retrieve the originator address
		uint8_t destinationMacAddress[6];
		uint8_t packetIndex = 14;
		if(not packet.data[3] & SEND_TO_ALL_NODES)	//If there's a destination, grab it
		{
			memcpy(&destinationMacAddress[0], &packet.data[packetIndex], 6);
			packetIndex+=6;
			//The destination may be a completely new device, so look for it but ignore any echoes of this node's traffic
			if(not _isLocalMacAddress(destinationMacAddress))
			{
				destinationId = _originatorIdFromMac(destinationMacAddress);
				if(destinationId == MESH_ORIGINATOR_NOT_FOUND)
				{
					if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_PEER_MANAGEMENT)
					{
						_debugStream->print(F("\r\nm2mMesh DST "));
					}
					destinationId = _addOriginator(destinationMacAddress,_currentChannel);
				}
			}
		}
		//At this point we know the router, originator and destination and can check for echoes, which are never processed further
		if(isEcho)
		{
			if(_isLocalMacAddress(originatorMacAddress))						//Process OGM 'echoes'
			{
				if(routerId != MESH_ORIGINATOR_NOT_FOUND && packet.data[0] == OGM_PACKET_TYPE)
				{
					_originator[routerId].ogmEchoes = _originator[routerId].ogmEchoes >> 1;			//Right shift the OGM Echo receipt confirmation bitmask, which will make it worse
					_originator[routerId].ogmEchoes = _originator[routerId].ogmEchoes | 0x8000;		//Set the most significant bit of the OGM Echo receipt confirmation bitmask, which MAY make it 'better'
					_originator[routerId].ogmEchoLastConfirmed = millis();							//Record when this happened, so the node can spot 'missed' echoes
					_calculateLtq(routerId);														//Recalculate Local Transmission Quality
					if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_OGM_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || routerId == _nodeToLog))
					{
						_debugStream->printf("\r\nm2mMesh OGM ECHO R:%02x:%02x:%02x:%02x:%02x:%02x O:%02x:%02x:%02x:%02x:%02x:%02x TTL:%d HOP:%d LEN:%d",packet.macAddress[0],packet.macAddress[1],packet.macAddress[2],packet.macAddress[3],packet.macAddress[4],packet.macAddress[5],_originator[originatorId].macAddress[0],_originator[originatorId].macAddress[2],_originator[originatorId].macAddress[2],_originator[originatorId].macAddress[3],_originator[originatorId].macAddress[4],_originator[originatorId].macAddress[5],packet.data[2],packet.data[20],packet.length);
					}
				}
			}
		}
		else
		{
			//Now check the sequence number is sensible or the protection window is disabled
			union unsignedLongToBytes packetSequenceNumber;
			packetSequenceNumber.b[0] = packet.data[4];
			packetSequenceNumber.b[1] = packet.data[5];
			packetSequenceNumber.b[2] = packet.data[6];
			packetSequenceNumber.b[3] = packet.data[7];
			if(packetSequenceNumber.value > _originator[originatorId].lastSequenceNumber || _originator[originatorId].sequenceNumberProtectionWindowActive == false)
			{
				//The sequence number was valid, or old enough to prompt a reset of the protection window, continue processing but enable protection window again
				if(_originator[originatorId].sequenceNumberProtectionWindowActive == false)
				{
					//Re-enable the sequence number protection window
					_originator[originatorId].sequenceNumberProtectionWindowActive = true;
					if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_WARNINGS && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || routerId == _nodeToLog || originatorId == _nodeToLog))
					{
						_debugStream->printf("\r\nm2mMesh %02x:%02x:%02x:%02x:%02x:%02x sequence number protection enabled",originatorMacAddress[0],originatorMacAddress[1],originatorMacAddress[2],originatorMacAddress[3],originatorMacAddress[4],originatorMacAddress[5]);
					}
				}
				//Update the sequence number
				_originator[originatorId].lastSequenceNumber = packetSequenceNumber.value;
				//Refresh first/last seen data
				_originator[originatorId].lastSeen[packet.data[0]] = millis();
				//Update the OGM receipt window if it's OGM
				if(packet.data[0] == OGM_PACKET_TYPE && originatorId == routerId)								//This is an OGM direct from a node
				{
					_originator[originatorId].ogmReceived = _originator[originatorId].ogmReceived >> 1;			//Right shift the receipt confirmation bitmask
					_originator[originatorId].ogmReceived = _originator[originatorId].ogmReceived | 0x8000;		//Set the most significant bit of the receipt confirmation bitmask, which MAY make it 'better'
					_originator[originatorId].ogmReceiptLastConfirmed = millis();								//Record when this direct OGM was confirmed
					_calculateLtq(originatorId);																//Recalculate Local Transmission Quality
				}
				//Retrieve the interval
				unsignedLongToBytes packetInterval;
				packetInterval.b[0] = packet.data[packetIndex++];
				packetInterval.b[1] = packet.data[packetIndex++];
				packetInterval.b[2] = packet.data[packetIndex++];
				packetInterval.b[3] = packet.data[packetIndex++];
				if(packetInterval.value != _originator[originatorId].interval[packet.data[0]])
				{
					if(_debugEnabled == true && _originator[originatorId].interval[packet.data[0]] > 0 && _loggingLevel & MESH_UI_LOG_INFORMATION && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || routerId == _nodeToLog || originatorId == _nodeToLog))
					{
						char packetTypeDescription[] = "UNK";
						_packetTypeDescription(packetTypeDescription,packet.data[0]);
						_debugStream->printf("\r\nm2mMesh %s originator %02x:%02x:%02x:%02x:%02x:%02x changed interval from %d to %d",packetTypeDescription,packet.data[8],packet.data[9],packet.data[10],packet.data[11],packet.data[12],packet.data[13],_originator[originatorId].interval[packet.data[0]],packetInterval.value);
					}
					_originator[originatorId].interval[packet.data[0]] = packetInterval.value;
				}
				//Consider the packet for processing so check the destination. Either it is has to be a flood or have this node as the destination
				if((packet.data[3] & SEND_TO_ALL_NODES) || ((not(packet.data[3] & SEND_TO_ALL_NODES)) && _isLocalMacAddress(destinationMacAddress)))
				{
					if(packet.data[0] == ELP_PACKET_TYPE && _serviceFlags & PROTOCOL_ELP_RECEIVE)	//This is an ELP packet
					{
						_processElp(routerId, originatorId, packet);								//Process the contents of the ELP packet
					}
					else if(packet.data[0] == OGM_PACKET_TYPE)							//This is an OGM packet
					{
						if(_serviceFlags & PROTOCOL_OGM_RECEIVE)								//Consider processing OGM
						{
							_processOgm(routerId, originatorId, packet);
						}
					}
					else if(packet.data[0] == NHS_PACKET_TYPE && _serviceFlags & PROTOCOL_NHS_RECEIVE)	//This is a NHS packet
					{
						_processNhs(routerId, originatorId, packet);									//Process the contents of the NHS packet
					}
					else if(packet.data[0] == USR_PACKET_TYPE && _serviceFlags & PROTOCOL_USR_RECEIVE)	//This is a USR packet
					{
						_processUsr(routerId, originatorId, packet);									//Process the contents of the USR packet
					}
					else if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_WARNINGS && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || routerId == _nodeToLog || originatorId == _nodeToLog))
					{
						_debugStream->printf("\r\nm2mMesh WARNING: unknown type %d from %02x:%02x:%02x:%02x:%02x:%02x",packet.data[0],packet.macAddress[0],packet.macAddress[1],packet.macAddress[2],packet.macAddress[3],packet.macAddress[4],packet.macAddress[5]);
					}
				}
				//Consider a packet for forwarding, it may already have been changed in earlier processing
				//The TTL must be >0 and either a flood or NOT have this node at its source or destination
				if(packet.data[2] > 0 && ((packet.data[3] & SEND_TO_ALL_NODES) || (not _isLocalMacAddress(destinationMacAddress))))
				{
					//Reduce the TTL, regardless whether we end up forwarding or not
					packet.data[2]--;
					bool doTheForward = false;
					bool logTheForward = false;
					if(packet.data[0] == ELP_PACKET_TYPE && _serviceFlags & PROTOCOL_ELP_FORWARD)		//This is an ELP packet
					{
						if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_ELP_FORWARDING && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || routerId == _nodeToLog || originatorId == _nodeToLog))
						{
							logTheForward = true;
						}
						doTheForward = true;
					}
					else if(packet.data[0] == OGM_PACKET_TYPE && _serviceFlags & PROTOCOL_OGM_FORWARD)	//This is an OGM packet
					{
						if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_OGM_FORWARDING && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || routerId == _nodeToLog || originatorId == _nodeToLog))
						{
							logTheForward = true;
						}
						doTheForward = true;
					}
					else if(packet.data[0] == NHS_PACKET_TYPE && _serviceFlags & PROTOCOL_NHS_FORWARD)		//This is a NHS packet
					{
						if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NHS_FORWARDING && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || routerId == _nodeToLog || originatorId == _nodeToLog))
						{
							logTheForward = true;
						}
						doTheForward = true;
					}
					else if(packet.data[0] == USR_PACKET_TYPE && _serviceFlags & PROTOCOL_USR_FORWARD)	//This is a USR packet
					{
						if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_USR_FORWARDING && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || routerId == _nodeToLog || originatorId == _nodeToLog))
						{
							logTheForward = true;
						}
						doTheForward = true;
					}
					if(doTheForward == true)	//Forward is OK, we simply use the receive buffer and send from there to avoid copying data
					{
						if(packet.data[3] & SEND_TO_ALL_NODES)
						{
							//This is a flood, send it to all neighbours
							memcpy(&packet.macAddress[0], &_broadcastMacAddress[0], 6);
						}
						else
						{
							//This has a specific destination, send it to the best next hop, if known
							if(_originator[destinationId].selectedRouter == MESH_ORIGINATOR_NOT_FOUND)
							{
								memcpy(&packet.macAddress[0], &_broadcastMacAddress[0], 6);	//Fall back to using a broadcast
							}
							else
							{
								memcpy(&packet.macAddress[0], &_originator[_originator[destinationId].selectedRouter].macAddress[0], 6);
							}
						}

						#if defined(ESP8266)
						uint8_t result = _sendPacket(packet);	//Send packet on ESP8266/ESP8285
						#elif defined(ESP32)
						esp_err_t result = _sendPacket(packet);	//Send packet on ESP32
						#endif
						if(result == ESP_OK && logTheForward == true)
						{
							char packetTypeDescription[] = "UNK";
							_packetTypeDescription(packetTypeDescription,packet.data[0]);
							_debugStream->printf("\r\nm2mMesh %s FWD R:%02x:%02x:%02x:%02x:%02x:%02x O:%02x:%02x:%02x:%02x:%02x:%02x TTL:%d",packetTypeDescription,packet.macAddress[0],packet.macAddress[1],packet.macAddress[2],packet.macAddress[3],packet.macAddress[4],packet.macAddress[5],_originator[originatorId].macAddress[0],_originator[originatorId].macAddress[1],_originator[originatorId].macAddress[2],_originator[originatorId].macAddress[3],_originator[originatorId].macAddress[4],_originator[originatorId].macAddress[5],packet.data[2]);
						}
						else if(logTheForward == true)
						{
							char packetTypeDescription[] = "UNK";
							_packetTypeDescription(packetTypeDescription,packet.data[0]);
							_debugStream->printf("\r\nm2mMesh %s FWD R:%02x:%02x:%02x:%02x:%02x:%02x O:%02x:%02x:%02x:%02x:%02x:%02x TTL:%d failed",packetTypeDescription,packet.macAddress[0],packet.macAddress[1],packet.macAddress[2],packet.macAddress[3],packet.macAddress[4],packet.macAddress[5],_originator[originatorId].macAddress[0],_originator[originatorId].macAddress[1],_originator[originatorId].macAddress[2],_originator[originatorId].macAddress[3],_originator[originatorId].macAddress[4],_originator[originatorId].macAddress[5],packet.data[2]);
						}
					}
				}
			}
			else if(_originator[originatorId].lastSequenceNumber - packetSequenceNumber.value > SEQUENCE_NUMBER_MAX_AGE)
			{
				//If it's a very old sequence number disable the protection window to allow the next packet to reset the sequence number
				//this logic is here to handle a reboot, but ignore a single stray weird sequence number
				_originator[originatorId].sequenceNumberProtectionWindowActive = false;
				if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_WARNINGS && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || routerId == _nodeToLog || originatorId == _nodeToLog))
				{
					_debugStream->printf("\r\nm2mMesh %02x:%02x:%02x:%02x:%02x:%02x sequence number protection disabled, possible reboot",originatorMacAddress[0],originatorMacAddress[1],originatorMacAddress[2],originatorMacAddress[3],originatorMacAddress[4],originatorMacAddress[5]);
				}
			}
		}
	}
	else if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_WARNINGS)	//This is the wrong protocol version
	{
		_debugStream->printf("\r\nm2mMesh WARNING: incorrect protocol version %d from %02x:%02x:%02x:%02x:%02x:%02x",packet.data[1],packet.macAddress[0],packet.macAddress[1],packet.macAddress[2],packet.macAddress[3],packet.macAddress[4],packet.macAddress[5]);
	}
	packet.length = 0;	//Mark the buffer empty
}

/*
 * This is the ELP packet format, inspired by https://www.open-mesh.org/projects/batman-adv/wiki/ELP
 * 
 * If we are sharing neighbours, then set the third octet to numberOfPeers and put their addresses
 * after the Originator address. Interval is used for guesses at packet loss.
 * 
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 | Packet Type   |    Version    |      TTL      |    Flags      |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |                       Sequence Number                         |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |                     Originator Address                        |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |      Originator Address       |         Interval              |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |        Interval (cont)        | Num Neighbors | Neighbour 1   |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |                     Neighbour 1 address  (cont)               |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 | Neigbour 1     |    Neighbour 2 address                       |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |    Neighbour 2 address                        |  More...      |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

//Overloaded functions for variant calls


bool ICACHE_FLASH_ATTR m2mMesh::_sendElp(packetBuffer &packet)
{
	if(_serviceFlags & PROTOCOL_ELP_INCLUDE_PEERS)
	{
		return(_sendElp(true,_currentTtl[ELP_PACKET_TYPE], packet));
	}
	else
	{
		return(_sendElp(false,_currentTtl[ELP_PACKET_TYPE], packet));
	}
}


bool ICACHE_FLASH_ATTR m2mMesh::_sendElp(bool includeNeighbours,packetBuffer &packet)
{
	return(_sendElp(includeNeighbours,_currentTtl[ELP_PACKET_TYPE], packet));
}

bool ICACHE_FLASH_ATTR m2mMesh::_sendElp(uint8_t elpTtl,packetBuffer &packet)
{
	if(_serviceFlags & PROTOCOL_ELP_INCLUDE_PEERS)
	{
		return(_sendElp(true, elpTtl, packet));
	}
	else
	{
		return(_sendElp(false, elpTtl, packet));
	}
}


bool ICACHE_FLASH_ATTR m2mMesh::_sendElp(bool includeNeighbours,uint8_t elpTtl,packetBuffer &packet)
{
	packet.data[0] = ELP_PACKET_TYPE;					//Add the ELP packet type
	packet.data[1] = MESH_PROTOCOL_VERSION;			//Add the mesh protocol version
	packet.data[2] = elpTtl;							//Add the TTL, usually 0 for ELP
	if(includeNeighbours)										//Modify the flags if neighbours are included
	{
		//Set the includes peers flag
		packet.data[3] = packet.data[3] | ELP_FLAGS_INCLUDES_PEERS;
	}
	else
	{
		packet.data[3] = ELP_DEFAULT_FLAGS;			//Add the default flags
	}
	union unsignedLongToBytes sequenceNumber;				//Use a union to pack the sequence number
	sequenceNumber.value = _sequenceNumber;
	packet.data[4] = sequenceNumber.b[0];	//Add the sequence number
	packet.data[5] = sequenceNumber.b[1];
	packet.data[6] = sequenceNumber.b[2];
	packet.data[7] = sequenceNumber.b[3];
	memcpy(&packet.data[8], &_localMacAddress[0], 6);	//Add the source address
	union unsignedLongToBytes interval;						//Use a union to pack the number of neighbours
	interval.value = _currentInterval[ELP_PACKET_TYPE];
	packet.data[14] = interval.b[0];		//Add the interval
	packet.data[15] = interval.b[1];
	packet.data[16] = interval.b[2];
	packet.data[17] = interval.b[3];
	packet.length = 18;								//This will iterate through filling in the data if necessary
	if(includeNeighbours)									//If adding neighbours, iterate through adding them
	{
		packet.data[packet.length++] = 0;			//Start with zero neighbours
		//Only add active peers, but we need to iterate the whole list
		for(uint8_t i = 0; i<_numberOfOriginators; i++)
		{
			if(_dataIsValid(i, ELP_PACKET_TYPE) && packet.length + 6 < ESP_NOW_MAX_PACKET_SIZE) //Don't overflow the packet
			{
				//This originator is an active neighbour so include it
				memcpy(&packet.data[packet.length], &_originator[i].macAddress[0], 6);
				packet.length+=6;
				packet.data[18]++;						//Increment the number of neighbours
			}
		}
	}
	while(packet.length < ESP_NOW_MIN_PACKET_SIZE)		//If the packet is still not big enough, pad it
	{
		packet.data[packet.length++] = 0x00;				//Fill any spare space with nonsense
	}
	memcpy(&packet.macAddress[0], &_broadcastMacAddress[0], 6);	//Set the destination MAC address
	#if defined(ESP8266)
	uint8_t result = _sendPacket(packet);				//Send packet on ESP8266/ESP8285
	#elif defined(ESP32)
	esp_err_t result = _sendPacket(packet);			//Send packet on ESP32
	#endif
	if(result == ESP_OK && _debugEnabled == true)
	{
		if(_loggingLevel & MESH_UI_LOG_ELP_SEND)
		{
			_debugStream->print(F("\r\nm2mMesh ELP SND "));
			if(includeNeighbours)
			{
				_debugStream->printf("TTL:%02d FLG:%02x SEQ:%08x LEN:%d NBR:%d",packet.data[2],packet.data[3],sequenceNumber.value,packet.length,_numberOfActiveNeighbours);
			}
			else
			{
				_debugStream->printf("TTL:%02d FLG:%02x SEQ:%08x LEN:%d",packet.data[2],packet.data[3],sequenceNumber.value,packet.length);
			}
		}
		if(_loggingLevel & MESH_UI_LOG_ALL_SENT_PACKETS)
		{
			_debugStream->print(F("\r\nm2mMesh Sent "));
			_debugPacket(packet);
		}
	}
	else if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_ELP_SEND)
	{
		_debugStream->print(F("\r\nm2mMesh ELP SND fail"));
	}
	_sequenceNumber++;
	if(result == ESP_OK)
	{
		return(true);
	}
	else
	{
		return(false);
	}
}

void ICACHE_FLASH_ATTR m2mMesh::_processElp(uint8_t routerId, uint8_t originatorId, packetBuffer &packet)
{
	if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_ELP_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || routerId == _nodeToLog || originatorId == _nodeToLog))
	{
		if(packet.data[3] & ELP_FLAGS_INCLUDES_PEERS && packet.data[18]>0)
		{
			_debugStream->printf("\r\nm2mMesh ELP RCV R:%02x:%02x:%02x:%02x:%02x:%02x O:%02x:%02x:%02x:%02x:%02x:%02x TTL:%d NBRS:%02d LEN:%d",packet.macAddress[0],packet.macAddress[1],packet.macAddress[2],packet.macAddress[3],packet.macAddress[4],packet.macAddress[5],_originator[originatorId].macAddress[0],_originator[originatorId].macAddress[2],_originator[originatorId].macAddress[2],_originator[originatorId].macAddress[3],_originator[originatorId].macAddress[4],_originator[originatorId].macAddress[5],packet.data[2],packet.data[18],packet.length);
		}
		else
		{
			_debugStream->printf("\r\nm2mMesh ELP RCV R:%02x:%02x:%02x:%02x:%02x:%02x O:%02x:%02x:%02x:%02x:%02x:%02x TTL:%d LEN:%d",packet.macAddress[0],packet.macAddress[1],packet.macAddress[2],packet.macAddress[3],packet.macAddress[4],packet.macAddress[5],_originator[originatorId].macAddress[0],_originator[originatorId].macAddress[2],_originator[originatorId].macAddress[2],_originator[originatorId].macAddress[3],_originator[originatorId].macAddress[4],_originator[originatorId].macAddress[5],packet.data[2],packet.length);
		}
	}
	if(packet.data[3] & ELP_FLAGS_INCLUDES_PEERS && packet.data[18]>0)	//No further processing is necessary unless the ELP contains neighbours
	{
		uint8_t sharedNeighbours = packet.data[18];
		for(uint8_t neighbour = 0; neighbour<sharedNeighbours; neighbour++)
		{
			if(_isLocalMacAddress(&packet.data[19+neighbour*6]))	//Ignore this node
			{
				if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_ELP_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || routerId == _nodeToLog || originatorId == _nodeToLog)) //Print this neighbour
				{
					_debugStream->printf("\r\nm2mMesh ELP neighbour %02x:%02x:%02x:%02x:%02x:%02x - this node",packet.data[19+neighbour*6],packet.data[20+neighbour*6],packet.data[21+neighbour*6],packet.data[22+neighbour*6],packet.data[23+neighbour*6],packet.data[24+neighbour*6]);
				}
			}
			else
			{
				//It is not this node, so look at adding if necessery
				uint8_t neighbourId = _originatorIdFromMac(&packet.data[19+neighbour*6]);
				if(neighbourId == MESH_ORIGINATOR_NOT_FOUND)
				{
					//This is a new originator
					if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_ELP_RECEIVED) //Print this neighbour and show it's new
					{
						_debugStream->printf("\r\nm2mMesh ELP neighbour %02x:%02x:%02x:%02x:%02x:%02x - new node",packet.data[19+neighbour*6],packet.data[20+neighbour*6],packet.data[21+neighbour*6],packet.data[22+neighbour*6],packet.data[23+neighbour*6],packet.data[24+neighbour*6]);
					}
					if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_PEER_MANAGEMENT)
					{
						_debugStream->print(F("\r\nm2mMesh ELP "));
					}
					neighbourId = _addOriginator(&packet.data[19+neighbour*6],_currentChannel);
				}
				else
				{
					//This is an existing originator
					if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_ELP_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || routerId == _nodeToLog || originatorId == _nodeToLog))
					{
						_debugStream->printf("\r\nm2mMesh ELP neighbour %02x:%02x:%02x:%02x:%02x:%02x",packet.data[19+neighbour*6],packet.data[20+neighbour*6],packet.data[21+neighbour*6],packet.data[22+neighbour*6],packet.data[23+neighbour*6],packet.data[24+neighbour*6]);
					}
				}
			}
		}
	}
}



/*
 * This is the OGM packet format, inspired by https://www.open-mesh.org/projects/batman-adv/wiki/Ogmv2
 * 
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 | Packet Type   |    Version    |      TTL      |   Flags       |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |                       Sequence Number                         |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |                     Originator Address                        |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |      Originator Address       |         Interval              |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |        Interval (cont)        |   Transmission Quality        |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * 
 */

bool ICACHE_FLASH_ATTR m2mMesh::_sendOgm(packetBuffer &packet)
{
	//Use unions to pack the non-8-bit values
	union unsignedLongToBytes sequenceNumber;
	sequenceNumber.value = _sequenceNumber;
	union unsignedIntToBytes tQ;
	union unsignedLongToBytes interval;
	interval.value = _currentInterval[OGM_PACKET_TYPE];
	tQ.value = 65535;
	packet.data[0] = OGM_PACKET_TYPE;
	packet.data[1] = MESH_PROTOCOL_VERSION;
	packet.data[2] = _currentTtl[OGM_PACKET_TYPE];
	packet.data[3] = OGM_DEFAULT_FLAGS;
	packet.data[4] = sequenceNumber.b[0];
	packet.data[5] = sequenceNumber.b[1];
	packet.data[6] = sequenceNumber.b[2];
	packet.data[7] = sequenceNumber.b[3];
	packet.data[8] = _localMacAddress[0];
	packet.data[9] = _localMacAddress[1];
	packet.data[10] = _localMacAddress[2];
	packet.data[11] = _localMacAddress[3];
	packet.data[12] = _localMacAddress[4];
	packet.data[13] = _localMacAddress[5];
	packet.data[14] = interval.b[0];
	packet.data[15] = interval.b[1];
	packet.data[16] = interval.b[2];
	packet.data[17] = interval.b[3];
	packet.data[18] = tQ.b[0];
	packet.data[19] = tQ.b[1];
	packet.data[20] = 0;
	packet.length = 21;
	while(packet.length < ESP_NOW_MIN_PACKET_SIZE)
	{
		packet.data[packet.length++] = 0x00;
	}
	memcpy(&packet.macAddress[0], &_broadcastMacAddress[0], 6);	//Set the destination MAC address
	#if defined(ESP8266)
	uint8_t result = _sendPacket(packet);	//Send packet on ESP8266/ESP8285
	#elif defined(ESP32)
	esp_err_t result = _sendPacket(packet);	//Send packet on ESP32
	#endif
	if(result == ESP_OK && _debugEnabled == true)
	{
		if(_loggingLevel & MESH_UI_LOG_OGM_SEND)
		{
			_debugStream->printf("\r\nm2mMesh OGM SND TTL:%02d FLG:%02x SEQ:%08x LEN:%d",packet.data[2],packet.data[3],_sequenceNumber,packet.length);
		}
		if(_loggingLevel & MESH_UI_LOG_ALL_SENT_PACKETS)
		{
			_debugStream->println();
			_debugStream->print(F("m2mMesh Sent "));
			_debugPacket(packet);
		}
	}
	else if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_OGM_SEND)
	{
		_debugStream->printf("\r\nm2mMesh OGM SND failed TTL:%02d Flags:%02x Seq:%08x LEN:%d",packet.data[2],packet.data[3],_sequenceNumber,packet.length);
	}
	_sequenceNumber++;
	if(result == ESP_OK)
	{
		return(true);
	}
	else
	{
		return(false);
	}
}

void ICACHE_FLASH_ATTR m2mMesh::_processOgm(uint8_t routerId, uint8_t originatorId, packetBuffer &packet)
{
	if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_OGM_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || routerId == _nodeToLog || originatorId == _nodeToLog))
	{
		_debugStream->printf("\r\nm2mMesh OGM RCV R:%02x:%02x:%02x:%02x:%02x:%02x O:%02x:%02x:%02x:%02x:%02x:%02x TTL:%d HOP:%d LEN:%d",packet.macAddress[0],packet.macAddress[1],packet.macAddress[2],packet.macAddress[3],packet.macAddress[4],packet.macAddress[5],_originator[originatorId].macAddress[0],_originator[originatorId].macAddress[2],_originator[originatorId].macAddress[2],_originator[originatorId].macAddress[3],_originator[originatorId].macAddress[4],_originator[originatorId].macAddress[5],packet.data[2],packet.data[20],packet.length);
	}
	union unsignedIntToBytes tq;				//Retrieve the transmission quality in the packet
	tq.b[0] = packet.data[18];
	tq.b[1] = packet.data[19];
	if(routerId != originatorId)
	{
		uint16_t currentGtq = _originator[originatorId].gtq;		//Store current TQ for later comparison
		//Compare routerId with originatorID to see if this is direct or forwarded
		if(routerId != _originator[originatorId].selectedRouter)
		{
			if(tq.value > currentGtq)
			{
				//This is a better indirect route than we already have
				_originator[originatorId].selectedRouter = routerId;
				_originator[originatorId].gtq = tq.value;
				_meshLastChanged = millis();
				if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_OGM_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || routerId == _nodeToLog || originatorId == _nodeToLog))
				{
					_debugStream->printf("\r\nm2mMesh OGM R:%02x%02x%02x%02x%02x%02x selected for O:%02x%02x%02x%02x%02x%02x TQ:%04x",packet.macAddress[0],packet.macAddress[1],packet.macAddress[2],packet.macAddress[3],packet.macAddress[4],packet.macAddress[5],packet.data[8],packet.data[9],packet.data[10],packet.data[11],packet.data[12],packet.data[13],tq.value);
				}
			}
			else
			{
				//This does not change the routing
				if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_OGM_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || routerId == _nodeToLog || originatorId == _nodeToLog))
				{
					_debugStream->printf("\r\nm2mMesh OGM R:%02x%02x%02x%02x%02x%02x for O:%02x%02x%02x%02x%02x%02x inferior TQ:%04x",packet.macAddress[0],packet.macAddress[1],packet.macAddress[2],packet.macAddress[3],packet.macAddress[4],packet.macAddress[5],packet.data[8],packet.data[9],packet.data[10],packet.data[11],packet.data[12],packet.data[13],tq.value);
				}
			}
		}
		else if(routerId == _originator[originatorId].selectedRouter)
		{
			if(tq.value != currentGtq)
			{
				//Update the TQ of the existing route
				_originator[originatorId].gtq = tq.value;
				//This does not change the routing
				if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_OGM_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || routerId == _nodeToLog || originatorId == _nodeToLog))
				{
					_debugStream->printf("\r\nm2mMesh OGM R:%02x%02x%02x%02x%02x%02x for O:%02x%02x%02x%02x%02x%02x update TQ:%04x",packet.macAddress[0],packet.macAddress[1],packet.macAddress[2],packet.macAddress[3],packet.macAddress[4],packet.macAddress[5],packet.data[8],packet.data[9],packet.data[10],packet.data[11],packet.data[12],packet.data[13],tq.value);
				}
			}
			//This does not change the routing
			else if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_OGM_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || routerId == _nodeToLog || originatorId == _nodeToLog))
			{
				_debugStream->printf("\r\nm2mMesh OGM R:%02x%02x%02x%02x%02x%02x for O:%02x%02x%02x%02x%02x%02x TQ:%04x",packet.data[2],packet.data[3],packet.macAddress[0],packet.macAddress[1],packet.macAddress[2],packet.macAddress[3],packet.macAddress[4],packet.macAddress[5],packet.data[8],packet.data[9],packet.data[10],packet.data[11],packet.data[12],packet.data[13],tq.value);
			}
		}
		//Update the stats on reachable nodes
		if(currentGtq == 0 && _originator[originatorId].gtq > 0)
		{
			_originatorHasBecomeRoutable(originatorId);
		}
		else if(currentGtq > 0 && _originator[originatorId].gtq == 0)
		{
			_originatorHasBecomeUnroutable(originatorId);
		}
	}
	if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_OGM_RECEIVED && packet.data[20]>0 && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || routerId == _nodeToLog || originatorId == _nodeToLog))
	{
		for(uint8_t id = 0; id<packet.data[20]; id++)
		{
			_debugStream->printf("\r\nm2mMesh OGM forwarding chain %02x:%02x:%02x:%02x:%02x:%02x",packet.data[21+id*6],packet.data[22+id*6],packet.data[23+id*6],packet.data[24+id*6],packet.data[25+id*6],packet.data[26+id*6]);
		}
	}
	//Consider the packet for forwarding, whether or not it was selected as router or not
	if(packet.data[2] > 0)
	{
		//If our route back to the originator is worse than the GTQ in the packet, reduce GTQ to the LTQ value
		if(_originator[routerId].ltq < tq.value)
		{
			tq.value = _originator[routerId].ltq;
		}
		//Impose an arbitrary hop penalty on GTQ and only forward if GTQ is still > 0
		if(tq.value > OGM_HOP_PENALTY)
		{
			tq.value = tq.value - OGM_HOP_PENALTY;
			//Update GTQ in the packet
			packet.data[18] = tq.b[0];
			packet.data[19] = tq.b[1];
		}
		else if(routerId != originatorId)
		{
			//Reduce TTL to 0 on low TQ packets so they are NOT forwarded beyond first hop
			//First hop MUST be allowed so TQ can be calculated from zero
			packet.data[2] = 0;
		}
		if(packet.data[2] > 0)	//Packet will be forwarded despite penalties
		{
			if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_OGM_FORWARDING && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || routerId == _nodeToLog || originatorId == _nodeToLog))
			{
				_debugStream->printf("\r\nm2mMesh OGM hop penalty applied, TQ now %02x", tq.value);
			}
			if(21+(packet.data[20]+1)*6 < ESP_NOW_MAX_PACKET_SIZE)	//Add in this node's address to the forwarding chain if there's space.
			{
				memcpy (&packet.data[21+packet.data[20]*6], &_localMacAddress, 6);			
				packet.data[20]++;
				if(21+packet.data[20]*6>packet.length)
				{
					packet.length=21+packet.data[20]*6;
				}
				if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_OGM_FORWARDING && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || routerId == _nodeToLog || originatorId == _nodeToLog))
				{
					_debugStream->print(F("\r\nm2mMesh OGM added this node to forwarding chain"));
				}
			}
		}
	}
}

void ICACHE_FLASH_ATTR m2mMesh::_originatorHasBecomeRoutable(uint8_t originatorId)
{
	//Originator has become reachable
	_numberOfReachableOriginators++;
	_meshLastChanged = millis();
	if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_PEER_MANAGEMENT && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || originatorId == _nodeToLog))
	{
		_debugStream->printf("\r\nm2mMesh OGM %02x%02x%02x%02x%02x%02x has become reachable",_originator[originatorId].macAddress[0],_originator[originatorId].macAddress[1],_originator[originatorId].macAddress[2],_originator[originatorId].macAddress[3],_originator[originatorId].macAddress[4],_originator[originatorId].macAddress[5]);
	}
}

void ICACHE_FLASH_ATTR m2mMesh::_originatorHasBecomeUnroutable(uint8_t originatorId)
{
	//Peer has become unreachable
	if(_numberOfReachableOriginators>0)
	{
		_numberOfReachableOriginators--;
	}
	_originator[originatorId].gtq = 0;
	_originator[originatorId].selectedRouter = MESH_ORIGINATOR_NOT_FOUND;
	_meshLastChanged = millis();
	if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_PEER_MANAGEMENT && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || originatorId == _nodeToLog))
	{
		_debugStream->printf("\r\nm2mMesh OGM %02x%02x%02x%02x%02x%02x has become unreachable",_originator[originatorId].macAddress[0],_originator[originatorId].macAddress[1],_originator[originatorId].macAddress[2],_originator[originatorId].macAddress[3],_originator[originatorId].macAddress[4],_originator[originatorId].macAddress[5]);
	}
}


/*
 * This is the Node Health Status (NHS) packet format, which I've added to the protocol for checking node/network health
 * it can also do a slow spread of previously seen originators in the event bits get cut off
 * 
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 | Packet Type   |    Version    |      TTL      |     Flags     |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |                       Sequence Number                         |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |                     Originator Address                        |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |  (cont'd) Originator Address  |            Interval           |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |       Interval (cont'd)       |             Uptime            |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |             Uptime (cont'd)   |           Free heap           |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |            Free Heap (cont'd) |      Initial Free Heap        |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |    Initial Free Heap (cont'd) |    Peers      |    Tx Power   |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |            Mesh Time (opt)    |                                
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * 
 */

bool ICACHE_FLASH_ATTR m2mMesh::_sendNhs(packetBuffer &packet)
{
	/*if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NHS_SEND)
	{
		_debugStream->print(F("\r\nm2mMesh building NHS packet "));
	}*/	
	packet.data[0] = NHS_PACKET_TYPE;
	packet.data[1] = MESH_PROTOCOL_VERSION;
	packet.data[2] = _currentTtl[NHS_PACKET_TYPE];
	packet.data[3] = NHS_DEFAULT_FLAGS;
	if(_softAPstate)
	{
		//Flags the state of the SoftAp
		packet.data[3] = packet.data[3] | NHS_FLAGS_SOFTAP_ON;
	}
	//Use unions to pack the non-8-bit values
	union unsignedLongToBytes temp;
	temp.value = _sequenceNumber;
	packet.data[4] = temp.b[0];
	packet.data[5] = temp.b[1];
	packet.data[6] = temp.b[2];
	packet.data[7] = temp.b[3];
	//Origin address
	packet.data[8] = _localMacAddress[0];
	packet.data[9] = _localMacAddress[1];
	packet.data[10] = _localMacAddress[2];
	packet.data[11] = _localMacAddress[3];
	packet.data[12] = _localMacAddress[4];
	packet.data[13] = _localMacAddress[5];
	union unsignedLongToBytes interval;
	interval.value = _currentInterval[NHS_PACKET_TYPE];
	packet.data[14] = interval.b[0];
	packet.data[15] = interval.b[1];
	packet.data[16] = interval.b[2];
	packet.data[17] = interval.b[3];
	temp.value = millis();								//Add the current millis() for 'uptime'
	packet.data[18] = temp.b[0];
	packet.data[19] = temp.b[1];
	packet.data[20] = temp.b[2];
	packet.data[21] = temp.b[3];
	temp.value = uint32_t(ESP.getFreeHeap());			//Add the current free heap
	packet.data[22] = temp.b[0];
	packet.data[23] = temp.b[1];
	packet.data[24] = temp.b[2];
	packet.data[25] = temp.b[3];
	temp.value = _initialFreeHeap;						//Add the initial free heap
	packet.data[26] = temp.b[0];
	packet.data[27] = temp.b[1];
	packet.data[28] = temp.b[2];
	packet.data[29] = temp.b[3];
	temp.value = _rxPackets;							//Add the RX packets
	packet.data[30] = temp.b[0];
	packet.data[31] = temp.b[1];
	packet.data[32] = temp.b[2];
	packet.data[33] = temp.b[3];
	temp.value = _droppedRxPackets;						//Add the dropped RX packets
	packet.data[34] = temp.b[0];
	packet.data[35] = temp.b[1];
	packet.data[36] = temp.b[2];
	packet.data[37] = temp.b[3];
	temp.value = _txPackets;							//Add the TX packets
	packet.data[38] = temp.b[0];
	packet.data[39] = temp.b[1];
	packet.data[40] = temp.b[2];
	packet.data[41] = temp.b[3];
	temp.value = _droppedTxPackets;						//Add the dropped TX packets
	packet.data[42] = temp.b[0];
	packet.data[43] = temp.b[1];
	packet.data[44] = temp.b[2];
	packet.data[45] = temp.b[3];
	packet.data[46] = _numberOfActiveNeighbours;		//Add the number of activeNeighbours
	packet.data[47] = _numberOfOriginators;				//Add the number of originators
	packet.data[48] = _meshMacAddress[0];				//Add the mesh MAC address
	packet.data[49] = _meshMacAddress[1];				//Add the mesh MAC address
	packet.data[50] = _meshMacAddress[2];				//Add the mesh MAC address
	packet.data[51] = _meshMacAddress[3];				//Add the mesh MAC address
	packet.data[52] = _meshMacAddress[4];				//Add the mesh MAC address
	packet.data[53] = _meshMacAddress[5];				//Add the mesh MAC address
	packet.data[54] = uint8_t(_currentTxPower*4);			//Add the TxPower
	packet.length = 55;									//Use this to build the rest of the packet
	if(_serviceFlags & PROTOCOL_NHS_SUPPLY_VOLTAGE)				//Add the power supply voltage
	{
		packet.data[3] = packet.data[3] | NHS_FLAGS_INCLUDES_VCC;
		union floatToBytes supplyV;
		supplyV.value = supplyVoltage();
		packet.data[packet.length++] = supplyV.b[0];
		packet.data[packet.length++] = supplyV.b[1];
		packet.data[packet.length++] = supplyV.b[2];
		packet.data[packet.length++] = supplyV.b[3];
	}
	if(_serviceFlags & PROTOCOL_NHS_TIME_SERVER && _actingAsTimeServer)	//Add the mesh time
	{
		packet.data[3] = packet.data[3] | NHS_FLAGS_TIMESERVER;
		temp.value = time();
		packet.data[packet.length++] = temp.b[0];
		packet.data[packet.length++] = temp.b[1];
		packet.data[packet.length++] = temp.b[2];
		packet.data[packet.length++] = temp.b[3];
	}
	if(nodeNameIsSet())																	//Insert the node name, if set
	{
		packet.data[3] = packet.data[3] | NHS_FLAGS_NODE_NAME_SET;			//Mark this in the flags
		packet.length += strlen(_nodeName) + 1;									//Node name includes the length and a null termination!
		packet.data[packet.length++] = strlen(_nodeName);						//Include the length
		memcpy (&packet.data[packet.length], _nodeName, strlen(_nodeName));	//Copy in the node name
		packet.length+=strlen(_nodeName);											//Advance the index
		packet.data[packet.length-1] = 0;										//Ensure it's null terminated, the buffer WILL have had values in from before
	}
	uint8_t originatorCountIndex = packet.length++;									//Store the count of included originators here
	if(_serviceFlags & PROTOCOL_NHS_INCLUDE_ORIGINATORS && _numberOfOriginators>0)		//This packet will include all the nodes in the mesh
	{
		packet.data[3] = packet.data[3] | NHS_FLAGS_INCLUDES_ORIGINATORS;
		packet.data[originatorCountIndex] = 0;
		uint8_t randomOffset = random(_numberOfOriginators);							//Add a random offset so it's not always the same originators in the same order
		for(uint8_t originatorId = 0; originatorId<_numberOfOriginators && packet.length + 8 < ESP_NOW_MAX_PACKET_SIZE; originatorId++)
		{
			uint8_t originatorToSend = (originatorId+randomOffset)%_numberOfOriginators;
			//Insert the Originator MAC address and LTQ
			packet.data[packet.length++] = _originator[originatorToSend].macAddress[0];
			packet.data[packet.length++] = _originator[originatorToSend].macAddress[1];
			packet.data[packet.length++] = _originator[originatorToSend].macAddress[2];
			packet.data[packet.length++] = _originator[originatorToSend].macAddress[3];
			packet.data[packet.length++] = _originator[originatorToSend].macAddress[4];
			packet.data[packet.length++] = _originator[originatorToSend].macAddress[5];
			unsignedIntToBytes packetLtq;
			packetLtq.value = _originator[originatorToSend].ltq;
			packet.data[packet.length++] = packetLtq.b[0];
			packet.data[packet.length++] = packetLtq.b[1];
			packet.data[originatorCountIndex]++;
			/*if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NHS_SEND)
			{
				_debugStream->printf("\r\nm2mMesh NHS included %02x:%02x:%02x:%02x:%02x:%02x TQ:%02x",_originator[originatorToSend].macAddress[0],_originator[originatorToSend].macAddress[1],_originator[originatorToSend].macAddress[2],_originator[originatorToSend].macAddress[3],_originator[originatorToSend].macAddress[4],_originator[originatorToSend].macAddress[5],_originator[originatorToSend].ltq);
			}*/
		}
		/*if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NHS_SEND)
		{
			_debugStream->printf("\r\nm2mMesh NHS included %d originators",packet.data[originatorCountIndex]);
		}*/
	}
	while(packet.length < ESP_NOW_MIN_PACKET_SIZE)		//Fill any spare space with nonsense
	{
		packet.data[packet.length++] = 0x00;
	}
	memcpy(&packet.macAddress[0], &_broadcastMacAddress[0], 6);	//Set the destination MAC address
	if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NHS_SEND)
	{
		_debugStream->print(F("\r\nm2mMesh NHS SND"));
	}	
	#if defined(ESP8266)
	uint8_t result = _sendPacket(packet);	//Send packet on ESP8266/ESP8285
	#elif defined(ESP32)
	esp_err_t result = _sendPacket(packet);	//Send packet on ESP32
	#endif
	if(result == ESP_OK && _debugEnabled == true)
	{
		if(_loggingLevel & MESH_UI_LOG_NHS_SEND)
		{
			_debugStream->printf(" TTL:%02d FLG:%02x SEQ:%08x LEN:%d",packet.data[2],packet.data[3],_sequenceNumber, packet.length);
			if(_actingAsTimeServer)
			{
				char uptime[] = "00h00m00s";
				_friendlyUptime(temp.value,uptime);
				_debugStream->printf(" TIME:%s",uptime);
			}
			if(_serviceFlags & PROTOCOL_NHS_INCLUDE_ORIGINATORS && packet.data[originatorCountIndex]>0)
			{
				_debugStream->printf(" ORG:%d",packet.data[originatorCountIndex]);
			}
		}
		if(_loggingLevel & MESH_UI_LOG_ALL_SENT_PACKETS)
		{
			_debugStream->print(F("\r\nm2mMesh Sent "));
			_debugPacket(packet);
		}
	}
	else if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NHS_SEND)
	{
		_debugStream->print(F(" failed"));
	}
	_sequenceNumber++;
	if(result == ESP_OK)
	{
		return(true);
	}
	else
	{
		return(false);
	}
}

void ICACHE_FLASH_ATTR m2mMesh::_processNhs(uint8_t routerId, uint8_t originatorId, packetBuffer &packet)
{
	if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NHS_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || routerId == _nodeToLog || originatorId == _nodeToLog))
	{
		_debugStream->printf("\r\nm2mMesh NHS R:%02x:%02x:%02x:%02x:%02x:%02x O:%02x:%02x:%02x:%02x:%02x:%02x TTL:%d Length:%d",packet.macAddress[0],packet.macAddress[1],packet.macAddress[2],packet.macAddress[3],packet.macAddress[4],packet.macAddress[5],_originator[originatorId].macAddress[0],_originator[originatorId].macAddress[2],_originator[originatorId].macAddress[2],_originator[originatorId].macAddress[3],_originator[originatorId].macAddress[4],_originator[originatorId].macAddress[5],packet.data[2],packet.length);
	}
	if(originatorId != MESH_ORIGINATOR_NOT_FOUND)	//Update details about the originator
	{
		//Extract the flags
		_originator[originatorId].flags = packet.data[3];
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NHS_RECEIVED && _originator[originatorId].flags & NHS_FLAGS_SOFTAP_ON && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || routerId == _nodeToLog || originatorId == _nodeToLog))
		{
			_debugStream->print(F(" SoftAP is on"));
		}
		//Extract the uptime
		union unsignedLongToBytes tempUint32;
		tempUint32.b[0] = packet.data[18];
		tempUint32.b[1] = packet.data[19];
		tempUint32.b[2] = packet.data[20];
		tempUint32.b[3] = packet.data[21];
		_originator[originatorId].uptime = tempUint32.value;
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NHS_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || routerId == _nodeToLog || originatorId == _nodeToLog))
		{
			_debugStream->printf("\r\nm2mMesh NHS Uptime %dms",tempUint32.value);
		}
		//Extract the current free Heap
		tempUint32.b[0] = packet.data[22];
		tempUint32.b[1] = packet.data[23];
		tempUint32.b[2] = packet.data[24];
		tempUint32.b[3] = packet.data[25];
		_originator[originatorId].currentFreeHeap = tempUint32.value;
		//Extract the initial free heap
		tempUint32.b[0] = packet.data[26];
		tempUint32.b[1] = packet.data[27];
		tempUint32.b[2] = packet.data[28];
		tempUint32.b[3] = packet.data[29];
		_originator[originatorId].initialFreeHeap = tempUint32.value;
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NHS_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || routerId == _nodeToLog || originatorId == _nodeToLog))
		{
			_debugStream->printf("\r\nm2mMesh NHS Current Free Heap %d/%d",_originator[originatorId].currentFreeHeap,_originator[originatorId].initialFreeHeap);
		}
		//Extract the packet statistics
		tempUint32.b[0] = packet.data[30];
		tempUint32.b[1] = packet.data[31];
		tempUint32.b[2] = packet.data[32];
		tempUint32.b[3] = packet.data[33];
		_originator[originatorId].rxPackets = tempUint32.value;
		tempUint32.b[0] = packet.data[34];
		tempUint32.b[1] = packet.data[35];
		tempUint32.b[2] = packet.data[36];
		tempUint32.b[3] = packet.data[37];
		_originator[originatorId].droppedRxPackets = tempUint32.value;
		tempUint32.b[0] = packet.data[38];
		tempUint32.b[1] = packet.data[39];
		tempUint32.b[2] = packet.data[40];
		tempUint32.b[3] = packet.data[41];
		_originator[originatorId].txPackets = tempUint32.value;
		tempUint32.b[0] = packet.data[42];
		tempUint32.b[1] = packet.data[43];
		tempUint32.b[2] = packet.data[44];
		tempUint32.b[3] = packet.data[45];
		_originator[originatorId].droppedTxPackets = tempUint32.value;
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NHS_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || routerId == _nodeToLog || originatorId == _nodeToLog))
		{
			_debugStream->printf("\r\nm2mMesh NHS dropped packets %d/%dRX %d/%dTX",_originator[originatorId].droppedRxPackets,_originator[originatorId].rxPackets,_originator[originatorId].txPackets,_originator[originatorId].droppedTxPackets);
		}
		//Extract the number of active neighbours
		_originator[originatorId].numberOfActiveNeighbours = packet.data[46];
		//Extract the number of originators
		_originator[originatorId].numberOfOriginators = packet.data[47];
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NHS_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || routerId == _nodeToLog || originatorId == _nodeToLog))
		{
			_debugStream->printf("\r\nm2mMesh NHS Active neighbours %d/%d MAC:%02x:%02x:%02x:%02x:%02x:%02x",_originator[originatorId].numberOfActiveNeighbours,_originator[originatorId].numberOfOriginators,packet.data[48],packet.data[49],packet.data[50],packet.data[51],packet.data[52],packet.data[53]);
		}
		
		//Check the consistency of the network
		if(_originator[originatorId].numberOfOriginators != _numberOfOriginators)
		{
			_meshLastChanged = millis();
			if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NHS_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || routerId == _nodeToLog || originatorId == _nodeToLog))
			{
				_debugStream->print(" - differs");
			}
		}
		else if(packet.data[48] != _meshMacAddress[0] | packet.data[49] != _meshMacAddress[1] | packet.data[50] != _meshMacAddress[2] | packet.data[51] != _meshMacAddress[3] | packet.data[52] != _meshMacAddress[4] | packet.data[53] != _meshMacAddress[5])
		{
			_meshLastChanged = millis();
			if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NHS_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || routerId == _nodeToLog || originatorId == _nodeToLog))
			{
				_debugStream->print(" - differs");
			}
		}
		//Extract the Tx power
		_originator[originatorId].currentTxPower = float(packet.data[54])/4;
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NHS_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || routerId == _nodeToLog || originatorId == _nodeToLog))
		{
			_debugStream->printf("\r\nm2mMesh NHS Current TX power %f",4*_originator[originatorId].currentTxPower);
		}
		uint8_t receivedPacketIndex = 55;		//Use this to index through the packet
		//Extract the supply voltage, if included
		if(packet.data[3] & NHS_FLAGS_INCLUDES_VCC)
		{
			//Packet includes information about the supply voltage
			union floatToBytes nhsSupplyVoltage;
			nhsSupplyVoltage.b[0] = packet.data[receivedPacketIndex++];
			nhsSupplyVoltage.b[1] = packet.data[receivedPacketIndex++];
			nhsSupplyVoltage.b[2] = packet.data[receivedPacketIndex++];
			nhsSupplyVoltage.b[3] = packet.data[receivedPacketIndex++];
			_originator[originatorId].supplyVoltage = nhsSupplyVoltage.value;
			if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NHS_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || routerId == _nodeToLog || originatorId == _nodeToLog))
			{
				_debugStream->printf("\r\nm2mMesh NHS Supply voltage %fV",nhsSupplyVoltage.value);
			}
		}
		if(_currentMeshTimeServer == originatorId && not (packet.data[3] & NHS_FLAGS_TIMESERVER))	//Check if the time server has shut down
		{
			_chooseNewTimeServer(); //Choose a new time server now
		}
		else if(packet.data[3] & NHS_FLAGS_TIMESERVER)		//Check for included time
		{
			//The NHS packet is flagged as being from an active time server, which sends mesh time separately from uptime
			//To help with accuracy only updates with the least number of hops are accepted
			tempUint32.b[0] = packet.data[receivedPacketIndex++];
			tempUint32.b[1] = packet.data[receivedPacketIndex++];
			tempUint32.b[2] = packet.data[receivedPacketIndex++];
			tempUint32.b[3] = packet.data[receivedPacketIndex++];
			_updateMeshTime(tempUint32.value,originatorId);
			if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NHS_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || routerId == _nodeToLog || originatorId == _nodeToLog))
			{
				_debugStream->printf("\r\nm2mMesh NHS Mesh time %dms",tempUint32.value);
			}
		}
		//Look for a node name
		if(packet.data[3] & NHS_FLAGS_NODE_NAME_SET)
		{
			uint8_t nodeNameLength = packet.data[receivedPacketIndex++];
			char temp[nodeNameLength+1];
			memcpy (temp, &packet.data[receivedPacketIndex], nodeNameLength);
			temp[nodeNameLength]=0;
			if(_originator[originatorId].nodeName !=nullptr && String(_originator[originatorId].nodeName) != String(temp)) //Node name has changed, free the memory
			{
				if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NHS_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || routerId == _nodeToLog || originatorId == _nodeToLog))
				{
					_debugStream->printf("\r\nm2mMesh NHS node name len=%d '%s' changed from '%s'!",nodeNameLength,temp,_originator[originatorId].nodeName);
				}
				//New node name, delete the previously allocated memory and reallocate
				delete[] _originator[originatorId].nodeName;
			}
			if(_originator[originatorId].nodeName == nullptr)	//Name not set
			{
				//Node name not set
				if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NHS_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || routerId == _nodeToLog || originatorId == _nodeToLog))
				{
					_debugStream->printf("\r\nm2mMesh NHS node name len=%d '%s'",nodeNameLength,temp);
				}
				//Allocate memory
				_originator[originatorId].nodeName = new char[nodeNameLength+1];
				if(_originator[originatorId].nodeName)
				{
					//Copy the node name into the newly allocated memory
					memcpy(_originator[originatorId].nodeName, &packet.data[receivedPacketIndex], nodeNameLength);
					//Terminate the string with a null
					_originator[originatorId].nodeName[nodeNameLength]=0;
				}
				else
				{
					if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NHS_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || routerId == _nodeToLog || originatorId == _nodeToLog))
					{
						_debugStream->print(F("\r\nm2mMesh NHS Unable to store node name, not enough memory"));
					}
				}
			}
			//Advance the packet index past the name
			receivedPacketIndex+=nodeNameLength;
		}
		//Extract originator information, if included
		if(packet.data[3] & NHS_FLAGS_INCLUDES_ORIGINATORS)
		{
			uint8_t originatorCount = packet.data[receivedPacketIndex++];	//Retrieve the number of originators in the end of the packet
			if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NHS_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || routerId == _nodeToLog || originatorId == _nodeToLog))
			{
				_debugStream->printf("\r\nm2mMesh NHS contains %d originators",originatorCount);
			}
			for(uint8_t i = 0; i<originatorCount && receivedPacketIndex + 8 < ESP_NOW_MAX_PACKET_SIZE; i++) //The node will have filled in as many as it can in the space in the packet
			{
				unsignedIntToBytes packetLtq;
				packetLtq.b[0] = packet.data[receivedPacketIndex+6];
				packetLtq.b[1] = packet.data[receivedPacketIndex+7];
				if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NHS_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || routerId == _nodeToLog || originatorId == _nodeToLog))
				{
					_debugStream->printf("\r\nm2mMesh NHS originator data %02x:%02x:%02x:%02x:%02x:%02x TQ:%02x",packet.data[receivedPacketIndex],packet.data[receivedPacketIndex+1],packet.data[receivedPacketIndex+2],packet.data[receivedPacketIndex+3],packet.data[receivedPacketIndex+4],packet.data[receivedPacketIndex+5],packetLtq);
				}
				if(not _isLocalMacAddress(&packet.data[receivedPacketIndex]))
				{
					//The included originator MIGHT also be a completely new device, so look for it.
					uint8_t includedOriginatorId = _originatorIdFromMac(&packet.data[receivedPacketIndex]);
					if(includedOriginatorId == MESH_ORIGINATOR_NOT_FOUND)
					{
						if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NHS_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || routerId == _nodeToLog || originatorId == _nodeToLog))
						{
							_debugStream->print(F(" - new"));
						}
						//This device has never been seen, create a new originator
						if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_PEER_MANAGEMENT)
						{
							_debugStream->print(F("\r\nm2mMesh NHS "));
						}
						includedOriginatorId = _addOriginator(&packet.data[receivedPacketIndex],_currentChannel);
					}
				}
				else if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NHS_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || routerId == _nodeToLog || originatorId == _nodeToLog))
				{
					_debugStream->print(F(" - this node"));
				}
				receivedPacketIndex+=8;
			}
		}
	}
}

void ICACHE_FLASH_ATTR m2mMesh::_processUsr(uint8_t routerId, uint8_t originatorId, packetBuffer &packet)
{
	if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_USR_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || routerId == _nodeToLog || originatorId == _nodeToLog))
	{
		_debugStream->printf("\r\nm2mMesh USR R:%02x:%02x:%02x:%02x:%02x:%02x O:%02x:%02x:%02x:%02x:%02x:%02x TTL:%d Length:%d",packet.macAddress[0],packet.macAddress[1],packet.macAddress[2],packet.macAddress[3],packet.macAddress[4],packet.macAddress[5],_originator[originatorId].macAddress[0],_originator[originatorId].macAddress[2],_originator[originatorId].macAddress[2],_originator[originatorId].macAddress[3],_originator[originatorId].macAddress[4],_originator[originatorId].macAddress[5],packet.data[2],packet.length);
	}
	if(not _userPacketReceived)
	{
		_userPacketReceived = true;									//Mark that there is packet waiting for the user application and copy it to the buffer for retrieval
		memcpy (_receivedUserPacket, packet.data, packet.length);
		_receivedUserPacketLength = packet.length;
		uint8_t receivedPacketIndex = 18;
		if(not packet.data[3] & SEND_TO_ALL_NODES)	//If there's a destination, skip past
		{
			receivedPacketIndex+=6;
		}
		_receivedUserPacketIndex = receivedPacketIndex;							//Record where we are in reading the packet for later data retrieval
		_receivedUserPacketIndex++;
		_receivedUserPacketFieldCounter = packet.data[receivedPacketIndex++];	//Retrieve the number of fields in the packet
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_USR_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || routerId == _nodeToLog || originatorId == _nodeToLog))
		{
			_debugStream->printf("\r\nm2mMesh USR packet contains %d fields",_receivedUserPacketFieldCounter);
			uint8_t field = 0;
			while(receivedPacketIndex < _receivedUserPacketLength && field < _receivedUserPacketFieldCounter)
			{
				if(packet.data[receivedPacketIndex] == USR_DATA_UINT8_T)
				{
					receivedPacketIndex++;
					_debugStream->printf("\r\nm2mMesh USR data field %d uint8_t %d",field++,packet.data[receivedPacketIndex++]);
				}
				else if(packet.data[receivedPacketIndex] == USR_DATA_UINT16_T)
				{
					receivedPacketIndex++;
					unsignedIntToBytes temp;
					temp.b[0] = packet.data[receivedPacketIndex++];
					temp.b[1] = packet.data[receivedPacketIndex++];
					_debugStream->printf("\r\nm2mMesh USR data field %d uint16_t %d",field++,temp.value);
				}
				else if(packet.data[receivedPacketIndex] == USR_DATA_UINT32_T)
				{
					receivedPacketIndex++;
					unsignedLongToBytes temp;
					temp.b[0] = packet.data[receivedPacketIndex++];
					temp.b[1] = packet.data[receivedPacketIndex++];
					temp.b[2] = packet.data[receivedPacketIndex++];
					temp.b[3] = packet.data[receivedPacketIndex++];
					_debugStream->printf("\r\nm2mMesh USR data field %d uint32_t %d",field++,temp.value);
				}
				else if(packet.data[receivedPacketIndex] == USR_DATA_INT8_T)
				{
					receivedPacketIndex++;
					_debugStream->printf(" \r\nm2mMesh USR data field %d int8_t %d",field++,int8_t(packet.data[receivedPacketIndex++]));
				}
				else if(packet.data[receivedPacketIndex] == USR_DATA_INT16_T)
				{
					receivedPacketIndex++;
					intToBytes temp;
					temp.b[0] = packet.data[receivedPacketIndex++];
					temp.b[1] = packet.data[receivedPacketIndex++];
					_debugStream->printf(" \r\nm2mMesh USR data field %d int16_t %d",field++,temp.value);
				}
				else if(packet.data[receivedPacketIndex] == USR_DATA_INT32_T)
				{
					receivedPacketIndex++;
					longToBytes temp;
					temp.b[0] = packet.data[receivedPacketIndex++];
					temp.b[1] = packet.data[receivedPacketIndex++];
					temp.b[2] = packet.data[receivedPacketIndex++];
					temp.b[3] = packet.data[receivedPacketIndex++];
					_debugStream->printf(" \r\nm2mMesh USR data field %d int32_t %d",field++,temp.value);
				}
				else if(packet.data[receivedPacketIndex] == USR_DATA_FLOAT)
				{
					receivedPacketIndex++;
					floatToBytes temp;
					temp.b[0] = packet.data[receivedPacketIndex++];
					temp.b[1] = packet.data[receivedPacketIndex++];
					temp.b[2] = packet.data[receivedPacketIndex++];
					temp.b[3] = packet.data[receivedPacketIndex++];
					_debugStream->printf(" \r\nm2mMesh USR data field %d float %f",field++,temp.value);
				}
				else if(packet.data[receivedPacketIndex] == USR_DATA_CHAR)
				{
					receivedPacketIndex++;
					_debugStream->printf(" \r\nm2mMesh USR data field %d char '%c'",field++,char(packet.data[receivedPacketIndex++]));
				}
				else if(packet.data[receivedPacketIndex] == USR_DATA_STRING)
				{
					receivedPacketIndex++;
					uint8_t userDataStringLength = packet.data[receivedPacketIndex++];
					char userDataString[userDataStringLength+1];
					for(uint8_t i = 0; i<userDataStringLength ; i++)
					{
						userDataString[i] = char(packet.data[receivedPacketIndex++]);
					}
					userDataString[userDataStringLength] = char(0);
					_debugStream->printf(" \r\nm2mMesh USR data field %d String len=%d '%s'",field++,userDataStringLength,&userDataString);
				}
				else if(packet.data[receivedPacketIndex] == USR_DATA_CHAR_ARRAY)
				{
					receivedPacketIndex++;
					uint8_t userDataStringLength = packet.data[receivedPacketIndex++];
					char userDataString[userDataStringLength+1];
					for(uint8_t i = 0; i<userDataStringLength ; i++)
					{
						userDataString[i] = char(packet.data[receivedPacketIndex++]);
					}
					userDataString[userDataStringLength] = char(0);
					_debugStream->printf(" \r\nm2mMesh USR data field %d char array len=%d '%s'",field++,userDataStringLength,&userDataString);
				}
				else
				{
					_debugStream->printf(" \r\nm2mMesh USR data field %d unknown type %d, stopping decode ",field++,packet.data[receivedPacketIndex]);
					while(receivedPacketIndex < packet.length)
					{
						_debugStream->printf("%d/%02x ",packet.data[receivedPacketIndex],packet.data[receivedPacketIndex++]);
					}
				}
			}
		}
		packet.length = 0;						//Mark the packet buffer as read, but really we've just copied it

	}
	else
	{
		_droppedRxPackets++;	//Increase the dropped packet count even though the radio dropped it
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_WARNINGS)
		{
			_debugStream->print(F("\r\nm2mMesh Previous USR message not read, packet dropped"));
		}
	}

}


//Update the mesh time from the time server. This does NOT allowance for latency across the mesh, so nodes will only be roughly in sync
void ICACHE_FLASH_ATTR m2mMesh::_updateMeshTime(uint32_t newMeshTime, uint8_t originatorId)
{
  //It's a tie, which is not uncommon when handing over between two servers
  if(time() == newMeshTime)
  {
    //If we have better uptime we win a tie, it is unlikely nodes will tie on real uptime unless they share a common PSU
    if(millis() > expectedUptime(originatorId))
    {
      //Become the time server
      _becomeTimeServer();
    }
    else
    {
      //Concede the time server and set the mesh time
      _setMeshTime(newMeshTime, originatorId);
    }
  }
  //A higher mesh uptime becomes the reference clock, if the reference clock wanders <1s closer also update it
  else if(newMeshTime != time() && (newMeshTime > time() || time() - newMeshTime < 1000ul))
  {
    //Set the mesh time
    _setMeshTime(newMeshTime, originatorId);
  }
  else if(newMeshTime < millis())
  {
    //Become the time server
    _becomeTimeServer();
  }
}

void ICACHE_FLASH_ATTR m2mMesh::_setMeshTime(uint32_t newMeshTime, uint8_t originatorId)
{
  //Stop being a time server if we currently are
  if(_actingAsTimeServer == true)
  {
    _actingAsTimeServer = false;
  }
  //Temporarily record the old time offset
  int32_t oldMeshTimeOffset = _meshTimeOffset;
  _meshTimeOffset = newMeshTime - millis();
  //Track clock drift
  if(abs(oldMeshTimeOffset - _meshTimeOffset)<1000)
  {
    //track cumlative drift
    _meshTimeDrift += oldMeshTimeOffset - _meshTimeOffset;
  }
  else
  {
    //It was a big jump, so clear drift
    _meshTimeDrift = 0;
  }
  if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NHS_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || originatorId == _nodeToLog))
  {
    if(originatorId != _currentMeshTimeServer)
    {
      _debugStream->printf("\r\nm2mMesh NHS %02x:%02x:%02x:%02x:%02x:%02x is now the time server",_originator[originatorId].macAddress[0],_originator[originatorId].macAddress[1],_originator[originatorId].macAddress[2],_originator[originatorId].macAddress[3],_originator[originatorId].macAddress[4],_originator[originatorId].macAddress[5]);
    }
    if(oldMeshTimeOffset > _meshTimeOffset)
    {
      _debugStream->printf("\r\nm2mMesh NHS time offset -%dms",oldMeshTimeOffset - _meshTimeOffset);
    }
    else if(oldMeshTimeOffset != _meshTimeOffset && _meshTimeOffset - oldMeshTimeOffset < 5000)
    {
      _debugStream->printf("\r\nm2mMesh NHS time offset +%dms",_meshTimeOffset - oldMeshTimeOffset);
    }
    else if(oldMeshTimeOffset != _meshTimeOffset)
    {
      char uptime[] = "00h00m00s";
      _friendlyUptime(millis() + _meshTimeOffset,uptime);
      _debugStream->printf("\r\nm2mMesh NHS mesh time set to %s",uptime);
    }
  }
  _currentMeshTimeServer = originatorId;
}


//Choose a new time server
void ICACHE_FLASH_ATTR m2mMesh::_chooseNewTimeServer()
{
  //The current time server has gone away, so set it as unknown and don't reselect it
  uint8_t previousMeshTimeServer = _currentMeshTimeServer;
  _currentMeshTimeServer = MESH_ORIGINATOR_NOT_FOUND;
  uint32_t longestUp = 0;
  //Iterate through the mesh looking for the likely new time server
  for(uint8_t originatorId = 0; originatorId < _numberOfOriginators; originatorId++)
  {
    if(originatorId != previousMeshTimeServer && _dataIsValid(originatorId,NHS_PACKET_TYPE) && expectedUptime(originatorId) > millis() && expectedUptime(originatorId) > longestUp)
    {
      //This originator is active and up longer than us, we HOPE it is running as a time server
      _currentMeshTimeServer = originatorId;
    }
  }
  //This node has the highest uptime, so become the time server
  if(_currentMeshTimeServer == MESH_ORIGINATOR_NOT_FOUND)
  {
    _becomeTimeServer();
    if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_WARNINGS)
    {
      _debugStream->print(F("Time server has gone offline, taking over time server role"));
    }
  }
}

//Become the time server
void ICACHE_FLASH_ATTR m2mMesh::_becomeTimeServer()
{
  _actingAsTimeServer = true;
  _currentMeshTimeServer = MESH_ORIGINATOR_NOT_FOUND; 
}



#if defined(ESP8266)
void m2mMesh::_errorDescription(uint8_t result)
#elif defined(ESP32)
void m2mMesh::_errorDescription(esp_err_t result)
#endif
{
	if (result == ESP_OK)
	{
		_debugStream->print(F("Success"));
	}
	else if (result == ESP_ERR_ESPNOW_NOT_INIT)
	{
		_debugStream->println(F("ESPNOW is not initialized"));
	}
	else if (result == ESP_ERR_ESPNOW_ARG)
	{
		_debugStream->print(F("Invalid argument"));
	}
	else if (result == ESP_ERR_ESPNOW_INTERNAL)
	{
		_debugStream->print(F("Internal error"));
	}
	else if (result == ESP_ERR_ESPNOW_NO_MEM)
	{
		_debugStream->print(F("Out of memory"));
	}
	else if (result == ESP_ERR_ESPNOW_NOT_FOUND)
	{
		_debugStream->print(F("Peer is not found"));
	}
	else if (result == ESP_ERR_ESPNOW_IF)
	{
		_debugStream->print(F("Current WiFi interface doesn’t match that of peer"));
	}
	else
	{
		_debugStream->print(F("Unknown error"));
	}
}

void m2mMesh::_friendlyUptime(uint32_t uptime, char * formattedUptime)
{
  uint8_t seconds = (uptime/   1000ul)%60;
  uint8_t minutes = (uptime/  60000ul)%60;
  uint8_t hours =   (uptime/3600000ul);
  sprintf(formattedUptime,"%02dh%02dm%02ds",hours,minutes,seconds);
}

#ifdef ESP8266
void m2mMesh::_debugPacket(packetBuffer &packet)
#elif defined(ESP32)
void m2mMesh::_debugPacket(packetBuffer &packet)
#endif
{
	_debugStream->printf("%02x:%02x:%02x:%02x:%02x:%02x %d bytes\r\nm2mMesh Type:",packet.macAddress[0],packet.macAddress[1],packet.macAddress[2],packet.macAddress[3],packet.macAddress[4],packet.macAddress[5],packet.length);
	if(packet.data[0] == ELP_PACKET_TYPE)
	{
		_debugStream->print(F("ELP"));
	}
	else if(packet.data[0] == OGM_PACKET_TYPE)
	{
		_debugStream->print(F("OGM"));
	}
	else if(packet.data[0] == NHS_PACKET_TYPE)
	{
		_debugStream->print(F("NHS"));
	}
	else if(packet.data[0] == USR_PACKET_TYPE)
	{
		_debugStream->print(F("USR"));
	}
	else
	{
		_debugStream->print(F("Unknown"));
	}
	_debugStream->printf(" Version:%d",packet.data[1]);
	_debugStream->printf(" TTL:%d",packet.data[2]);
	_debugStream->printf(" Flags:%d",packet.data[3]);
	union unsignedLongToBytes packetSequenceNumber;
	packetSequenceNumber.b[0] = packet.data[4];
	packetSequenceNumber.b[1] = packet.data[5];
	packetSequenceNumber.b[2] = packet.data[6];
	packetSequenceNumber.b[3] = packet.data[7];
	_debugStream->printf(" Sequence number:%d\r\n",packetSequenceNumber.value);
	_debugStream->printf("m2mMesh Src:%02x:%02x:%02x:%02x:%02x:%02x",packet.data[8],packet.data[9],packet.data[10],packet.data[11],packet.data[12],packet.data[13]);
	uint8_t packetIndex = 14;
	if(packet.data[3] & SEND_TO_ALL_NODES)
	{
		_debugStream->print(" Dst:ALL");
	}
	else
	{
		_debugStream->printf(" Dst:%02x:%02x:%02x:%02x:%02x:%02x",packet.data[packetIndex++],packet.data[packetIndex++],packet.data[packetIndex++],packet.data[packetIndex++],packet.data[packetIndex++],packet.data[packetIndex++]);
	}
	unsignedLongToBytes packetInterval;
	packetInterval.b[0] = packet.data[packetIndex++];
	packetInterval.b[1] = packet.data[packetIndex++];
	packetInterval.b[2] = packet.data[packetIndex++];
	packetInterval.b[3] = packet.data[packetIndex++];
	_debugStream->print(" Interval:" + String(packetInterval.value));
	while(packetIndex + 7 < packet.length)
	{
		_debugStream->printf("\r\nm2mMesh Data:%02x %02x %02x %02x %02x %02x %02x %02x",packet.data[packetIndex++],packet.data[packetIndex++],packet.data[packetIndex++],packet.data[packetIndex++],packet.data[packetIndex++],packet.data[packetIndex++],packet.data[packetIndex++],packet.data[packetIndex++]);
	}
	if(packetIndex < packet.length - 6)
	{
		_debugStream->printf("\r\nm2mMesh Data:%02x %02x %02x %02x %02x %02x %02x --",packet.data[packetIndex++],packet.data[packetIndex++],packet.data[packetIndex++],packet.data[packetIndex++],packet.data[packetIndex++],packet.data[packetIndex++],packet.data[packetIndex++]);
	}
	else if(packetIndex < packet.length - 5)
	{
		_debugStream->printf("\r\nm2mMesh Data:%02x %02x %02x %02x %02x %02x -- --",packet.data[packetIndex++],packet.data[packetIndex++],packet.data[packetIndex++],packet.data[packetIndex++],packet.data[packetIndex++],packet.data[packetIndex++]);
	}
	else if(packetIndex < packet.length - 4)
	{
		_debugStream->printf("\r\nm2mMesh Data:%02x %02x %02x %02x %02x -- -- --",packet.data[packetIndex++],packet.data[packetIndex++],packet.data[packetIndex++],packet.data[packetIndex++],packet.data[packetIndex++]);
	}
	else if(packetIndex < packet.length - 3)
	{
		_debugStream->printf("\r\nm2mMesh Data:%02x %02x %02x %02x -- -- -- --",packet.data[packetIndex++],packet.data[packetIndex++],packet.data[packetIndex++],packet.data[packetIndex++]);
	}
	else if(packetIndex < packet.length - 2)
	{
		_debugStream->printf("\r\nm2mMesh Data:%02x %02x %02x -- -- -- -- --",packet.data[packetIndex++],packet.data[packetIndex++],packet.data[packetIndex++]);
	}
	else if(packetIndex < packet.length - 1)
	{
		_debugStream->printf("\r\nm2mMesh Data:%02x %02x -- -- -- -- -- --",packet.data[packetIndex++],packet.data[packetIndex++]);
	}
	else if(packetIndex < packet.length)
	{
		_debugStream->printf("\r\nm2mMesh Data:%02x -- -- -- -- -- -- --",packet.data[packetIndex++]);
	}
}

float ICACHE_FLASH_ATTR m2mMesh::supplyVoltage()
{
	#if defined(ESP8266)
		uint16_t v = ESP.getVcc();
		#if defined(ARDUINO_ESP8266_WEMOS_D1MINI)
			return((float)v/918.0f);
		#elif defined(ARDUINO_ESP8266_WEMOS_D1MINIPRO)
			return((float)v/918.0f);
		#elif defined(ARDUINO_ESP8266_GENERIC)
			return((float)v/1000.0f);
		#else
			return((float)v/1000.0f);
		#endif
	#elif defined(ESP32)
		return(3.3f);
	#endif
}

//Mesh time is just an offset of the node uptime to make it broadly the same as the longest up node
uint32_t ICACHE_FLASH_ATTR m2mMesh::time()
{
  return(millis() + _meshTimeOffset);
}

void ICACHE_FLASH_ATTR m2mMesh::enableStatusLed(uint8_t pin, bool onState)
{
	_statusLedPin = pin;
	pinMode(_statusLedPin,OUTPUT);
	digitalWrite(_statusLedPin, onState);
	_statusLedOnState = onState;
	_statusLedState = true;
	_statusLedEnabled = true;
}

void ICACHE_FLASH_ATTR m2mMesh::enableActivityLed(uint8_t pin, bool onState)
{
	_activityLedPin = pin;
	pinMode(_activityLedPin,OUTPUT);
	digitalWrite(_activityLedPin,not onState);
	_activityLedOnState = onState;
	_activityLedState = false;
	_activityLedEnabled = true;
}

//Blink the activity LED
void ICACHE_FLASH_ATTR m2mMesh::_blinkActivityLed()
{
  if(_activityLedState == false)
  {
    digitalWrite(_activityLedPin,_activityLedOnState);
    _activityLedState = true;
    _activityLedTimer = millis();
  }
}

//Check if a given MAC address is the local one
bool ICACHE_FLASH_ATTR m2mMesh::_isLocalMacAddress(uint8_t *mac)
{
  if(_localMacAddress[0] == mac[0] && _localMacAddress[1] == mac[1] && _localMacAddress[2] == mac[2] && _localMacAddress[3] == mac[3] && _localMacAddress[4] == mac[4] && _localMacAddress[5] == mac[5])
  {
    return (true);
  }
  return (false);
}


//Given a pointer to a MAC address this returns which originator ID it is, MESH_ORIGINATOR_NOT_FOUND is a failure
uint8_t ICACHE_FLASH_ATTR m2mMesh::_originatorIdFromMac(uint8_t *mac)
{
  uint8_t originatorId = MESH_ORIGINATOR_NOT_FOUND;
  for(uint8_t i = 0; i<_numberOfOriginators;i++)
  {
    if(mac[0] == _originator[i].macAddress[0] && mac[1] == _originator[i].macAddress[1] && mac[2] == _originator[i].macAddress[2] && mac[3] == _originator[i].macAddress[3] && mac[4] == _originator[i].macAddress[4] && mac[5] == _originator[i].macAddress[5])
    {
      originatorId = i;
      return(originatorId);
    }
  }
  return(originatorId);
}

uint8_t ICACHE_FLASH_ATTR m2mMesh::_originatorIdFromMac(const uint8_t mac0, const uint8_t mac1, const uint8_t mac2, const uint8_t mac3, const uint8_t mac4, const uint8_t mac5)
{
	uint8_t macAddress[6] = {mac0, mac1, mac2, mac3, mac4, mac5};
	return(_originatorIdFromMac(macAddress));
}

//Adds an originator record
uint8_t ICACHE_FLASH_ATTR m2mMesh::_addOriginator(uint8_t* mac,uint8_t originatorChannel)
{
	if(_numberOfOriginators<MESH_NO_MORE_ORIGINATORS_LEFT && _numberOfOriginators<_maxNumberOfOriginators)
	{
		//memcpy(originator[numberOfOriginators].macAddress[0],mac,6);
		_originator[_numberOfOriginators].macAddress[0] = mac[0];
		_originator[_numberOfOriginators].macAddress[1] = mac[1];
		_originator[_numberOfOriginators].macAddress[2] = mac[2];
		_originator[_numberOfOriginators].macAddress[3] = mac[3];
		_originator[_numberOfOriginators].macAddress[4] = mac[4];
		_originator[_numberOfOriginators].macAddress[5] = mac[5];
		_originator[_numberOfOriginators].channel = originatorChannel;
		_originator[_numberOfOriginators].selectedRouter = MESH_ORIGINATOR_NOT_FOUND;
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_PEER_MANAGEMENT)
		{
			_debugStream->printf("%02x:%02x:%02x:%02x:%02x:%02x added id=%d",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5],_numberOfOriginators);
		}
		_numberOfOriginators++;
		_meshLastChanged = millis();
		return(_numberOfOriginators - 1);
	}
	else if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_PEER_MANAGEMENT)
	{
		_debugStream->println();
		_debugStream->printf("%02x:%02x:%02x:%02x:%02x:%02x could not be added",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	}
	_meshLastChanged = millis();
	return(MESH_NO_MORE_ORIGINATORS_LEFT);
}

void ICACHE_FLASH_ATTR m2mMesh::_calculateLtq(uint8_t originatorId)
{
	if(_originator[originatorId].ogmEchoes == 0 || _originator[originatorId].ogmReceived == 0)
	{
		_originator[originatorId].ltq = 0;
	}
	else if(_originator[originatorId].ogmEchoes >= _originator[originatorId].ogmReceived)
	{
		_originator[originatorId].ltq = 0xffff;
	}
	else
	{
		_originator[originatorId].ltq = uint16_t(((uint32_t(_originator[originatorId].ogmEchoes) << 16 ) / uint32_t(_originator[originatorId].ogmReceived)));
	}
}

void ICACHE_FLASH_ATTR m2mMesh::_packetTypeDescription(char *desc, uint8_t type)
{
	switch (type)
	{
		case 0x00:
			desc[0] = 'E';
			desc[1] = 'L';
			desc[2] = 'P';
		break;
		case 0x01:
			desc[0] = 'O';
			desc[1] = 'G';
			desc[2] = 'M';
		break;
		case 0x02:
			desc[0] = 'N';
			desc[1] = 'H';
			desc[2] = 'S';
		break;
		case 0x04:
			desc[0] = 'U';
			desc[1] = 'S';
			desc[2] = 'R';
		break;
	}
}

bool ICACHE_FLASH_ATTR m2mMesh::_dataIsValid(uint8_t originatorId, uint8_t dataType)
{
  if(_originator[originatorId].lastSeen[dataType] == 0)
  {
    //We've had no packets at all
    return(false);
  }
  else if((millis() - _originator[originatorId].lastSeen[dataType]) < 2 * _originator[originatorId].interval[dataType])
  {
	//There has been some traffic of this type recently enough to consider it valid
    return(true);
  }
  else
  {
    //We've dropped two or more packets recently
    return(false);
  }
}

//Return the local node name
uint8_t * ICACHE_FLASH_ATTR m2mMesh::getMeshAddress()
{
	return(_localMacAddress);
}


//Return the local node name
char * ICACHE_FLASH_ATTR m2mMesh::getNodeName()
{
	return(_nodeName);
}
//Return the node name for another node
char * ICACHE_FLASH_ATTR m2mMesh::getNodeName(uint8_t originatorId)
{
	return(_originator[originatorId].nodeName);
}

bool ICACHE_FLASH_ATTR m2mMesh::joined()
{
	if(_numberOfActiveNeighbours > 0)
	{
		return(true);
	}
	else
	{
		return(false);
	}
}


bool ICACHE_FLASH_ATTR m2mMesh::setNodeName(const char *newName)
{
	if(newName != nullptr)	//Check for garbage in
	{
		//If the name is already set, delete it and free up the memory
		if(nodeNameIsSet())
		{
			//return(false);
			delete[] _nodeName;
		}
		//Allocate memory for the name, adding one for the null character
		_nodeName = new char[strlen(newName)+1];
		if(_nodeName)
		{
			//Copy the node name into the newly allocated memory
			memcpy(_nodeName, newName, strlen(newName) + 1);
			if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_INFORMATION)
			{
				_debugStream->printf("\r\nNode name set to '%s'",&newName);
			}
			return(true);
		}
		else
		{
			#ifdef MESH_UI
			if(loggingLevel & MESH_UI_LOG_WARNINGS)
			{
				addStringLog(F("Unable to set node name, not enough memory"));
			}
			#endif
			return(false);
		}
	}
	else
	{
		return(false);
	}
}

bool ICACHE_FLASH_ATTR m2mMesh::setNodeName(String newName)
{
	uint8_t stringLength = newName.length() + 1;
	char tempArray[stringLength+1];
	newName.toCharArray(tempArray, stringLength);
	return(setNodeName(tempArray));
}

//Functions for adding data to a packet before sending. These are overloaded for each type you can send
//They are all similar, using a union to encode data, a single byte to say which type the data is and incrementing an index as they go
/*
 * This is the USR packet format, inspired by https://www.open-mesh.org/projects/batman-adv/wiki/Ogmv2
 * 
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 | Packet Type   |    Version    |      TTL      |  Data size    |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |                       Sequence Number                         |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |                     Originator Address                        |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |      Originator Address       |           Interval            |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |        Interval (cont)        |     Destination Address       |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |                      Destination Address                      |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |                           User Data                           |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |                        User Data (cont'd)                     |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |                        User Data (cont'd)                     |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * 
 */
 
 void ICACHE_FLASH_ATTR m2mMesh::_buildUserPacketHeader()
 {
	_buildUserPacketHeader(MESH_ORIGINATOR_NOT_FOUND);
 }

void ICACHE_FLASH_ATTR m2mMesh::_buildUserPacketHeader(uint8_t destId)
{
	if(not _buildingUserPacket)
	{
		_userPacket[0] = USR_PACKET_TYPE;
		_userPacket[1] = MESH_PROTOCOL_VERSION;
		_userPacket[2] = _currentTtl[USR_PACKET_TYPE];
		_userPacket[3] = USR_DEFAULT_FLAGS;
		//The sequence number is put in at time of sending
		//Origin address
		_userPacket[8] = _localMacAddress[0];
		_userPacket[9] = _localMacAddress[1];
		_userPacket[10] = _localMacAddress[2];
		_userPacket[11] = _localMacAddress[3];
		_userPacket[12] = _localMacAddress[4];
		_userPacket[13] = _localMacAddress[5];
		//From here the packet specification is flexible, so use an incrementing index
		_userPacketIndex = 14;
		if(destId != MESH_ORIGINATOR_NOT_FOUND)
		{
			_userPacket[3] = _userPacket[3] & ~SEND_TO_ALL_NODES;
			_userPacket[_userPacketIndex++] = _originator[destId].macAddress[0];
			_userPacket[_userPacketIndex++] = _originator[destId].macAddress[1];
			_userPacket[_userPacketIndex++] = _originator[destId].macAddress[2];
			_userPacket[_userPacketIndex++] = _originator[destId].macAddress[3];
			_userPacket[_userPacketIndex++] = _originator[destId].macAddress[4];
			_userPacket[_userPacketIndex++] = _originator[destId].macAddress[5];
		}
		//Interval
		union unsignedLongToBytes usrInterval;
		usrInterval.value = _currentInterval[USR_PACKET_TYPE];
		_userPacket[_userPacketIndex++] = usrInterval.b[0];
		_userPacket[_userPacketIndex++] = usrInterval.b[1];
		_userPacket[_userPacketIndex++] = usrInterval.b[2];
		_userPacket[_userPacketIndex++] = usrInterval.b[3];
		//Number of fields, record where in the packet it is stored
		_userPacketFieldCounterIndex = _userPacketIndex;
		_userPacket[_userPacketIndex++] = 0;
		_buildingUserPacket = true;
	}
}

void ICACHE_FLASH_ATTR m2mMesh::_buildUserPacketHeader(const uint8_t mac0, const uint8_t mac1, const uint8_t mac2, const uint8_t mac3, const uint8_t mac4, const uint8_t mac5)
{
	_buildUserPacketHeader(_originatorIdFromMac(mac0, mac1, mac2, mac3, mac4, mac5));
}

uint8_t ICACHE_FLASH_ATTR m2mMesh::payloadLeft()
{
	_buildUserPacketHeader();
	return(USR_MAX_PACKET_SIZE - _userPacketIndex);
}

//Functions for setting destination on a packet before sending. These are overloaded to make it easier to set a destination.
//They must be called before adding data to a packet

void ICACHE_FLASH_ATTR m2mMesh::destination(const uint8_t mac0, const uint8_t mac1, const uint8_t mac2, const uint8_t mac3, const uint8_t mac4, const uint8_t mac5)	//Add a destination MAC address
{
	_buildUserPacketHeader(mac0, mac1, mac2, mac3, mac4, mac5);	//Add the MAC address to the packet
}


/*bool ICACHE_FLASH_ATTR m2mMesh::destination(uint8_t destId)
{
	if(not buildingUserPacket)
	{
		if(destId<numberOfOriginators)
		{
			buildUserPacketHeader(destId);
			return(true);
		}
	}
	return(false);
}
bool ICACHE_FLASH_ATTR m2mMesh::destination(char* dest)
{
	if(not buildingUserPacket)
	{
		buildUserPacketHeader(MESH_ORIGINATOR_NOT_FOUND);
		return(true);
	}
	else
	{
		return(false);
	}
}
bool ICACHE_FLASH_ATTR m2mMesh::destination(String dest)
{	
	if(not buildingUserPacket)
	{
		//Need to handle various cases here
		for(int i=0; i<numberOfOriginators;i++)
		{
			if(dest == String(originator[i].nodeName))
			{
				buildUserPacketHeader(i);
				return(true);
			}
		}
		return(false);
	}
	else
	{
		return(false);
	}
}*/

bool ICACHE_FLASH_ATTR m2mMesh::add(const uint8_t dataToAdd)
{
	_buildUserPacketHeader();
	if(_userPacketIndex + sizeof(uint8_t) < USR_MAX_PACKET_SIZE)
	{
		_userPacket[_userPacketIndex++] = USR_DATA_UINT8_T;
		_userPacket[_userPacketIndex++] = dataToAdd;
		//Increment the field counter
		_userPacket[_userPacketFieldCounterIndex] = _userPacket[_userPacketFieldCounterIndex] + 1;
		return(true);
	}
	else
	{
		return(false);
	}
}
bool ICACHE_FLASH_ATTR m2mMesh::add(const uint16_t dataToAdd)
{
	_buildUserPacketHeader();
	if(_userPacketIndex + sizeof(uint16_t) < USR_MAX_PACKET_SIZE)
	{
		_userPacket[_userPacketIndex++] = USR_DATA_UINT16_T;
		unsignedIntToBytes temp;
		temp.value = dataToAdd;
		_userPacket[_userPacketIndex++] = temp.b[0];
		_userPacket[_userPacketIndex++] = temp.b[1];
		//Increment the field counter
		_userPacket[_userPacketFieldCounterIndex] = _userPacket[_userPacketFieldCounterIndex] + 1;
		return(true);
	}
	else
	{
		return(false);
	}
}
bool ICACHE_FLASH_ATTR m2mMesh::add(const uint32_t dataToAdd)
{
	_buildUserPacketHeader();
	if(_userPacketIndex + sizeof(uint32_t) < USR_MAX_PACKET_SIZE)
	{
		_userPacket[_userPacketIndex++] = USR_DATA_UINT32_T;
		unsignedLongToBytes temp;
		temp.value = dataToAdd;
		_userPacket[_userPacketIndex++] = temp.b[0];
		_userPacket[_userPacketIndex++] = temp.b[1];
		_userPacket[_userPacketIndex++] = temp.b[2];
		_userPacket[_userPacketIndex++] = temp.b[3];
		//Increment the field counter
		_userPacket[_userPacketFieldCounterIndex] = _userPacket[_userPacketFieldCounterIndex] + 1;
		return(true);
	}
	else
	{
		return(false);
	}
}
bool ICACHE_FLASH_ATTR m2mMesh::add(const int8_t dataToAdd)
{
	_buildUserPacketHeader();
	if(_userPacketIndex + sizeof(int8_t) < USR_MAX_PACKET_SIZE)
	{
		_userPacket[_userPacketIndex++] = USR_DATA_INT8_T;
		_userPacket[_userPacketIndex++] = uint8_t(dataToAdd);
		//Increment the field counter
		_userPacket[_userPacketFieldCounterIndex] = _userPacket[_userPacketFieldCounterIndex] + 1;
		return(true);
	}
	else
	{
		return(false);
	}
}
bool ICACHE_FLASH_ATTR m2mMesh::add(const int16_t dataToAdd)
{
	_buildUserPacketHeader();
	if(_userPacketIndex + sizeof(int16_t) < USR_MAX_PACKET_SIZE)
	{
		_userPacket[_userPacketIndex++] = USR_DATA_INT16_T;
		intToBytes temp;
		temp.value = dataToAdd;
		_userPacket[_userPacketIndex++] = temp.b[0];
		_userPacket[_userPacketIndex++] = temp.b[1];
		//Increment the field counter
		_userPacket[_userPacketFieldCounterIndex] = _userPacket[_userPacketFieldCounterIndex] + 1;
		return(true);
	}
	else
	{
		return(false);
	}
}
bool ICACHE_FLASH_ATTR m2mMesh::add(const int32_t dataToAdd)
{
	_buildUserPacketHeader();
	if(_userPacketIndex + sizeof(int32_t) < USR_MAX_PACKET_SIZE)
	{
		_userPacket[_userPacketIndex++] = USR_DATA_INT32_T;
		longToBytes temp;
		temp.value = dataToAdd;
		_userPacket[_userPacketIndex++] = temp.b[0];
		_userPacket[_userPacketIndex++] = temp.b[1];
		_userPacket[_userPacketIndex++] = temp.b[2];
		_userPacket[_userPacketIndex++] = temp.b[3];
		//Increment the field counter
		_userPacket[_userPacketFieldCounterIndex] = _userPacket[_userPacketFieldCounterIndex] + 1;
		return(true);
	}
	else
	{
		return(false);
	}
}
bool ICACHE_FLASH_ATTR m2mMesh::add(const char dataToAdd)
{
	_buildUserPacketHeader();
	if(_userPacketIndex + sizeof(char) < USR_MAX_PACKET_SIZE)
	{
		_userPacket[_userPacketIndex++] = USR_DATA_CHAR;
		_userPacket[_userPacketIndex++] = uint8_t(dataToAdd);
		//Increment the field counter
		_userPacket[_userPacketFieldCounterIndex] = _userPacket[_userPacketFieldCounterIndex] + 1;
		return(true);
	}
	else
	{
		return(false);
	}
}
bool ICACHE_FLASH_ATTR m2mMesh::add(const float dataToAdd)
{
	_buildUserPacketHeader();
	if(_userPacketIndex + sizeof(float) < USR_MAX_PACKET_SIZE)
	{
		_userPacket[_userPacketIndex++] = USR_DATA_FLOAT;
		floatToBytes temp;
		temp.value = dataToAdd;
		_userPacket[_userPacketIndex++] = temp.b[0];
		_userPacket[_userPacketIndex++] = temp.b[1];
		_userPacket[_userPacketIndex++] = temp.b[2];
		_userPacket[_userPacketIndex++] = temp.b[3];
		//Increment the field counter
		_userPacket[_userPacketFieldCounterIndex] = _userPacket[_userPacketFieldCounterIndex] + 1;
		return(true);
	}
	else
	{
		return(false);
	}
}
bool ICACHE_FLASH_ATTR m2mMesh::add(String dataToAdd)
{
	_buildUserPacketHeader();
	if(_userPacketIndex + dataToAdd.length() + 1 < USR_MAX_PACKET_SIZE)
	{
		_userPacket[_userPacketIndex++] = USR_DATA_STRING;
		_userPacket[_userPacketIndex++] = dataToAdd.length();
		for(int i = 0 ; i < dataToAdd.length() ; i++)
		{
			_userPacket[_userPacketIndex++] = dataToAdd[i];
		}
		//Increment the field counter
		_userPacket[_userPacketFieldCounterIndex] = _userPacket[_userPacketFieldCounterIndex] + 1;
		return(true);
	}
	else
	{
		return(false);
	}
}

bool ICACHE_FLASH_ATTR m2mMesh::add(char *dataToAdd)
{
	_buildUserPacketHeader();
	if(_userPacketIndex + strlen(dataToAdd) + 1 < USR_MAX_PACKET_SIZE)
	{
		_userPacket[_userPacketIndex++] = USR_DATA_CHAR_ARRAY;
		_userPacket[_userPacketIndex++] = strlen(dataToAdd);
		for(int i = 0 ; i < strlen(dataToAdd) ; i++)
		{
			_userPacket[_userPacketIndex++] = dataToAdd[i];
		}
		//Increment the field counter
		_userPacket[_userPacketFieldCounterIndex] = _userPacket[_userPacketFieldCounterIndex] + 1;
		return(true);
	}
	else
	{
		return(false);
	}
}

bool ICACHE_FLASH_ATTR m2mMesh::add(uint8_t *dataToAdd, uint8_t amountOfData)
{
	_buildUserPacketHeader();
	if(_userPacketIndex + amountOfData + 1 < USR_MAX_PACKET_SIZE)
	{
		_userPacket[_userPacketIndex++] = USR_DATA_UINT8_T_ARRAY;
		_userPacket[_userPacketIndex++] = amountOfData;
		for(int i = 0 ; i < amountOfData ; i++)
		{
			_userPacket[_userPacketIndex++] = dataToAdd[i];
		}
		//Increment the field counter
		_userPacket[_userPacketFieldCounterIndex] = _userPacket[_userPacketFieldCounterIndex] + 1;
		return(true);
	}
	else
	{
		return(false);
	}
}

bool ICACHE_FLASH_ATTR m2mMesh::send()
{
	//Pad the packet out to the minimum size with garbage
	while(_userPacketIndex < ESP_NOW_MIN_PACKET_SIZE)
	{
		_userPacket[_userPacketIndex++] = 0x00;
	}
	//Use unions to pack the non-8-bit values
	//Sequence number is added immediately before sending
	union unsignedLongToBytes temp;
	temp.value = _sequenceNumber++;
	_userPacket[4] = temp.b[0];
	_userPacket[5] = temp.b[1];
	_userPacket[6] = temp.b[2];
	_userPacket[7] = temp.b[3];
	if (_debugEnabled == true && _loggingLevel & MESH_UI_LOG_ALL_SENT_PACKETS)
	{
		_debugStream->printf("\r\nm2mMesh Sending packet type %02x version %02x",_userPacket[0],_userPacket[1]);
	}
	if(esp_now_send(_broadcastMacAddress, _userPacket, _userPacketIndex) == ESP_OK)
	{
		if(_activityLedEnabled)
		{
			_blinkActivityLed();
		}
		if (_debugEnabled == true && _loggingLevel & MESH_UI_LOG_USR_SEND)
		{
			_debugStream->printf("\r\nm2mMesh USR SND O:%02x%02x%02x%02x%02x%02x TTL:%02d Flags:%02x",_userPacket[8],_userPacket[9],_userPacket[10],_userPacket[11],_userPacket[12],_userPacket[13],_userPacket[2],_userPacket[3]);
		}
		_userPacketIndex = 0;
		_buildingUserPacket = false;
		return(true);
	}
	else
	{
		_userPacketIndex = 0;
		_buildingUserPacket = false;
		return(false);
	}
}

void ICACHE_FLASH_ATTR m2mMesh::clear()	//Clear the message without sending
{
	_userPacketIndex = 0;
	_buildingUserPacket = false;
}

//Functions for retrieving data from a message in a user sketch

bool ICACHE_FLASH_ATTR m2mMesh::messageWaiting()
{
	if(_userPacketReceived)
	{
		return(true);
	}
	return(false);
}

uint8_t ICACHE_FLASH_ATTR m2mMesh::messageSize()
{
	return(_receivedUserPacketLength);
}

void ICACHE_FLASH_ATTR m2mMesh::markMessageRead()
{
	_userPacketReceived = false;
	_receivedUserPacketIndex = ESP_NOW_MAX_PACKET_SIZE;
	_receivedUserPacketFieldCounter = 0;
}

uint8_t ICACHE_FLASH_ATTR m2mMesh::sourceId()
{
	return(_originatorIdFromMac(&_receivedUserPacket[8]));
}

bool ICACHE_FLASH_ATTR m2mMesh::sourceMacAddress(uint8_t *macAddressArray)
{
	//if(_userPacketReceived)
	if(true)
	{
		macAddressArray[0] = _receivedUserPacket[8];
		macAddressArray[1] = _receivedUserPacket[9];
		macAddressArray[2] = _receivedUserPacket[10];
		macAddressArray[3] = _receivedUserPacket[11];
		macAddressArray[4] = _receivedUserPacket[12];
		macAddressArray[5] = _receivedUserPacket[13];
		return(true);
	}
	else
	{
		return(false);
	}
}

uint8_t ICACHE_FLASH_ATTR m2mMesh::nextDataType()
{
	if(dataAvailable())
	{
		return(_receivedUserPacket[_receivedUserPacketIndex]);
	}
	else
	{
		//This is disincentive to read past the end of the message, but won't actually stop you retrieveing dummy values
		return(USR_DATA_UNAVAILABLE);
	}
}

bool ICACHE_FLASH_ATTR m2mMesh::dataAvailable()
{
	if(_receivedUserPacketFieldCounter > 0 && _receivedUserPacketIndex < ESP_NOW_MAX_PACKET_SIZE)
	{
		return(true);
	}
	return(false);
}

uint8_t ICACHE_FLASH_ATTR m2mMesh::retrieveDataLength()
{
	if(dataAvailable())
	{
		if(_receivedUserPacket[_receivedUserPacketIndex] == USR_DATA_UINT8_T_ARRAY)
		{
			//uint8_t array has no null terminator
			return(_receivedUserPacket[_receivedUserPacketIndex+1]);
		}
		else
		{
			//char array and String have a null terminator
			return(_receivedUserPacket[_receivedUserPacketIndex+1]+1);
		}
	}
	else
	{
		//This is disincentive to read past the end of the message, but won't actually stop you retrieveing dummy values
		return(0);
	}
}


uint8_t ICACHE_FLASH_ATTR m2mMesh::retrieveUint8_t()
{
	if(not dataAvailable())
	{
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_WARNINGS)
		{
			_debugStream->print("\r\nm2mMesh tried to read beyond end of packet");
		}
		//Return a dummy value if nothing available
		return(0x00);
	}
	else
	{
		//Step past the 'type'
		_receivedUserPacketIndex++;
		//Decrement the count of data fields
		_receivedUserPacketFieldCounter--;
		if(not dataAvailable())
		{
			markMessageRead();
		}
		return(uint8_t(_receivedUserPacket[_receivedUserPacketIndex++]));
	}
}
uint16_t ICACHE_FLASH_ATTR m2mMesh::retrieveUint16_t()
{
	if(not dataAvailable())
	{
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_WARNINGS)
		{
			_debugStream->print("\r\nm2mMesh tried to read beyond end of packet");
		}
		//Return a dummy value if nothing available
		return(0x0000);
	}
	else
	{
		//Step past the 'type'
		_receivedUserPacketIndex++;
		//Decrement the count of data fields
		_receivedUserPacketFieldCounter--;
		unsignedIntToBytes temp;
		temp.b[0] = _receivedUserPacket[_receivedUserPacketIndex++];
		temp.b[1] = _receivedUserPacket[_receivedUserPacketIndex++];
		if(not dataAvailable())
		{
			markMessageRead();
		}
		return(temp.value);
	}
}
uint32_t ICACHE_FLASH_ATTR m2mMesh::retrieveUint32_t()
{
	if(not dataAvailable())
	{
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_WARNINGS)
		{
			_debugStream->print("\r\nm2mMesh tried to read beyond end of packet");
		}
		//Return a dummy value if nothing available
		return(0x00000000);
	}
	else
	{
		//Step past the 'type'
		_receivedUserPacketIndex++;
		//Decrement the count of data fields
		_receivedUserPacketFieldCounter--;
		unsignedLongToBytes temp;
		temp.b[0] = _receivedUserPacket[_receivedUserPacketIndex++];
		temp.b[1] = _receivedUserPacket[_receivedUserPacketIndex++];
		temp.b[2] = _receivedUserPacket[_receivedUserPacketIndex++];
		temp.b[3] = _receivedUserPacket[_receivedUserPacketIndex++];
		if(not dataAvailable())
		{
			markMessageRead();
		}
		return(temp.value);
	}
}
int8_t ICACHE_FLASH_ATTR m2mMesh::retrieveInt8_t()
{
	if(not dataAvailable())
	{
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_WARNINGS)
		{
			_debugStream->print("\r\nm2mMesh tried to read beyond end of packet");
		}
		//Return a dummy value if nothing available
		return(0x00);
	}
	else
	{
		//Step past the 'type'
		_receivedUserPacketIndex++;
		//Decrement the count of data fields
		_receivedUserPacketFieldCounter--;
		if(not dataAvailable())
		{
			markMessageRead();
		}
		return(int8_t(_receivedUserPacket[_receivedUserPacketIndex++]));
	}
}
int16_t ICACHE_FLASH_ATTR m2mMesh::retrieveInt16_t()
{
	if(not dataAvailable())
	{
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_WARNINGS)
		{
			_debugStream->print("\r\nm2mMesh tried to read beyond end of packet");
		}
		//Return a dummy value if nothing available
		return(0x0000);
	}
	else
	{
		//Step past the 'type'
		_receivedUserPacketIndex++;
		//Decrement the count of data fields
		_receivedUserPacketFieldCounter--;
		intToBytes temp;
		temp.b[0] = _receivedUserPacket[_receivedUserPacketIndex++];
		temp.b[1] = _receivedUserPacket[_receivedUserPacketIndex++];
		if(not dataAvailable())
		{
			markMessageRead();
		}
		return(temp.value);
	}
}
int32_t ICACHE_FLASH_ATTR m2mMesh::retrieveInt32_t()
{
	if(not dataAvailable())
	{
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_WARNINGS)
		{
			_debugStream->print("\r\nm2mMesh tried to read beyond end of packet");
		}
		//Return a dummy value if nothing available
		return(0x00000000);
	}
	else
	{
		//Step past the 'type'
		_receivedUserPacketIndex++;
		//Decrement the count of data fields
		_receivedUserPacketFieldCounter--;
		longToBytes temp;
		temp.b[0] = _receivedUserPacket[_receivedUserPacketIndex++];
		temp.b[1] = _receivedUserPacket[_receivedUserPacketIndex++];
		temp.b[2] = _receivedUserPacket[_receivedUserPacketIndex++];
		temp.b[3] = _receivedUserPacket[_receivedUserPacketIndex++];
		if(not dataAvailable())
		{
			markMessageRead();
		}
		return(temp.value);
	}
}
float ICACHE_FLASH_ATTR m2mMesh::retrieveFloat()
{
	if(not dataAvailable())
	{
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_WARNINGS)
		{
			_debugStream->print("\r\nm2mMesh tried to read beyond end of packet");
		}
		//Return a dummy value if nothing available
		return(0.0f);
	}
	else
	{
		//Step past the 'type'
		_receivedUserPacketIndex++;
		//Decrement the count of data fields
		_receivedUserPacketFieldCounter--;
		floatToBytes temp;
		temp.b[0] = _receivedUserPacket[_receivedUserPacketIndex++];
		temp.b[1] = _receivedUserPacket[_receivedUserPacketIndex++];
		temp.b[2] = _receivedUserPacket[_receivedUserPacketIndex++];
		temp.b[3] = _receivedUserPacket[_receivedUserPacketIndex++];
		if(not dataAvailable())
		{
			markMessageRead();
		}
		return(temp.value);
	}
}
char ICACHE_FLASH_ATTR m2mMesh::retrieveChar()
{
	if(not dataAvailable())
	{
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_WARNINGS)
		{
			_debugStream->print("\r\nm2mMesh tried to read beyond end of packet");
		}
		//Return a dummy value if nothing available
		return(' ');
	}
	else
	{
		//Step past the 'type'
		_receivedUserPacketIndex++;
		//Decrement the count of data fields
		_receivedUserPacketFieldCounter--;
		return(char(_receivedUserPacket[_receivedUserPacketIndex++]));
	}
}
String ICACHE_FLASH_ATTR m2mMesh::retrieveString()
{
	if(not dataAvailable())
	{
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_WARNINGS)
		{
			_debugStream->print("\r\nm2mMesh tried to read beyond end of packet");
		}
		//Return a dummy value if nothing available
		return("Nothing");
	}
	else
	{
		//Step past the 'type'
		_receivedUserPacketIndex++;
		//Decrement the count of data fields
		_receivedUserPacketFieldCounter--;
		uint8_t stringLength = _receivedUserPacket[_receivedUserPacketIndex++];
		String tempString;
		for(uint8_t i = 0; i<stringLength ; i++)
		{
			tempString += char(_receivedUserPacket[_receivedUserPacketIndex++]);
		}
		if(not dataAvailable())
		{
			markMessageRead();
		}
		return(tempString);
	}
}
void ICACHE_FLASH_ATTR m2mMesh::retrieveCharArray(char *data)
{
	if(not dataAvailable())
	{
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_WARNINGS)
		{
			_debugStream->print("\r\nm2mMesh tried to read beyond end of packet");
		}
		//Return a dummy value if nothing available
	}
	else
	{
		//Step past the 'type'
		_receivedUserPacketIndex++;
		//Decrement the count of data fields
		_receivedUserPacketFieldCounter--;
		uint8_t dataLength = _receivedUserPacket[_receivedUserPacketIndex++];
		memcpy(data,&_receivedUserPacket[_receivedUserPacketIndex],dataLength);
		data[dataLength]=0;
		_receivedUserPacketIndex+=dataLength;
		if(not dataAvailable())
		{
			markMessageRead();
		}
	}
}

void ICACHE_FLASH_ATTR m2mMesh::retrieveUint8_tArray(uint8_t *data)
{
	if(not dataAvailable())
	{
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_WARNINGS)
		{
			_debugStream->print("\r\nm2mMesh tried to read beyond end of packet");
		}
		//Return a dummy value if nothing available
	}
	else
	{
		//Step past the 'type'
		_receivedUserPacketIndex++;
		//Decrement the count of data fields
		_receivedUserPacketFieldCounter--;
		uint8_t dataLength = _receivedUserPacket[_receivedUserPacketIndex++];
		memcpy(data,&_receivedUserPacket[_receivedUserPacketIndex],dataLength);
		_receivedUserPacketIndex+=dataLength;
		if(not dataAvailable())
		{
			markMessageRead();
		}
	}
}


/* Public functions used for the 'network analyser' sketch
 *
 * If building a LARGE application that does no need them they can
 * be omitted by commenting out the #define in m2mMesh.h
 *
 */
#ifdef m2mMeshExtraInfoFuntions
void ICACHE_FLASH_ATTR m2mMesh::macAddress(uint8_t id,uint8_t *array)
{
	array[0] = _originator[id].macAddress[0];
	array[1] = _originator[id].macAddress[1];
	array[2] = _originator[id].macAddress[2];
	array[3] = _originator[id].macAddress[3];
	array[4] = _originator[id].macAddress[4];
	array[5] = _originator[id].macAddress[5];
}
uint8_t ICACHE_FLASH_ATTR m2mMesh::maxNumberOfOriginators()
{
	return(_maxNumberOfOriginators);
}
uint8_t ICACHE_FLASH_ATTR m2mMesh::numberOfOriginators()
{
	return(_numberOfOriginators);
}
uint8_t ICACHE_FLASH_ATTR m2mMesh::numberOfOriginators(uint8_t id)
{
	return(_originator[id].numberOfOriginators);
}
uint8_t ICACHE_FLASH_ATTR m2mMesh::numberOfReachableOriginators()
{
	return(_numberOfReachableOriginators);
}
uint8_t ICACHE_FLASH_ATTR m2mMesh::numberOfActiveNeighbours()
{
	return(_numberOfActiveNeighbours);
}
uint8_t ICACHE_FLASH_ATTR m2mMesh::numberOfActiveNeighbours(uint8_t id)
{
	return(_originator[id].numberOfActiveNeighbours);
}
bool ICACHE_FLASH_ATTR m2mMesh::elpIsValid(uint8_t id)
{
	return(_dataIsValid(id,ELP_PACKET_TYPE));
}
bool ICACHE_FLASH_ATTR m2mMesh::ogmIsValid(uint8_t id)
{
	return(_dataIsValid(id,OGM_PACKET_TYPE));
}
bool ICACHE_FLASH_ATTR m2mMesh::nhsIsValid(uint8_t id)
{
	return(_dataIsValid(id,NHS_PACKET_TYPE));
}
bool ICACHE_FLASH_ATTR m2mMesh::validRoute(uint8_t id)
{
	return(_originator[id].selectedRouter != MESH_ORIGINATOR_NOT_FOUND);
}
uint8_t ICACHE_FLASH_ATTR m2mMesh::selectedRouter(uint8_t id)
{
	return(_originator[id].selectedRouter);
}
uint32_t ICACHE_FLASH_ATTR m2mMesh::ogmReceived(uint8_t id)
{
	return(_originator[id].ogmReceived);
}
uint32_t ICACHE_FLASH_ATTR m2mMesh::ogmEchoes(uint8_t id)
{
	return(_originator[id].ogmEchoes);
}
uint32_t ICACHE_FLASH_ATTR m2mMesh::elpInterval(uint8_t id)
{
	return(_originator[id].interval[ELP_PACKET_TYPE]);
}
uint32_t ICACHE_FLASH_ATTR m2mMesh::ogmInterval(uint8_t id)
{
	return(_originator[id].interval[OGM_PACKET_TYPE]);
}
uint32_t ICACHE_FLASH_ATTR m2mMesh::nhsInterval(uint8_t id)
{
	return(_originator[id].interval[NHS_PACKET_TYPE]);
}
uint32_t ICACHE_FLASH_ATTR m2mMesh::elpLastSeen(uint8_t id)
{
	return(_originator[id].lastSeen[ELP_PACKET_TYPE]);
}
uint32_t ICACHE_FLASH_ATTR m2mMesh::ogmLastSeen(uint8_t id)
{
	return(_originator[id].lastSeen[OGM_PACKET_TYPE]);
}
uint32_t ICACHE_FLASH_ATTR m2mMesh::nhsLastSeen(uint8_t id)
{
	return(_originator[id].lastSeen[NHS_PACKET_TYPE]);
}
bool ICACHE_FLASH_ATTR m2mMesh::actingAsTimeServer()
{
	return(_actingAsTimeServer);
}
bool ICACHE_FLASH_ATTR m2mMesh::actingAsTimeServer(uint8_t id)
{
	return(_originator[id].flags & NHS_FLAGS_TIMESERVER);
}
uint32_t ICACHE_FLASH_ATTR m2mMesh::initialFreeHeap(uint8_t id)
{
	return(_originator[id].initialFreeHeap);
}
uint32_t ICACHE_FLASH_ATTR m2mMesh::currentFreeHeap(uint8_t id)
{
	return(_originator[id].currentFreeHeap);
}
uint32_t ICACHE_FLASH_ATTR m2mMesh::sequenceNumber()
{
	return(_sequenceNumber);
}
uint32_t ICACHE_FLASH_ATTR m2mMesh::lastSequenceNumber(uint8_t id)
{
	return(_originator[id].lastSequenceNumber);
}
float ICACHE_FLASH_ATTR m2mMesh::supplyVoltage(uint8_t id)
{
	return(_originator[id].supplyVoltage);
}
uint8_t ICACHE_FLASH_ATTR m2mMesh::flags(uint8_t id)
{
	return(_originator[id].flags);
}
bool ICACHE_FLASH_ATTR m2mMesh::softApState()
{
	return(_softAPstate);
}
bool ICACHE_FLASH_ATTR m2mMesh::softApState(uint8_t id)
{
	if(_originator[id].flags & NHS_FLAGS_SOFTAP_ON)
	{
		return(true);
	}
	else
	{
		return(false);
	}
}
uint32_t ICACHE_FLASH_ATTR m2mMesh::localTransmissionQuality(uint8_t id)
{
	return(_originator[id].ltq);
}
uint32_t ICACHE_FLASH_ATTR m2mMesh::globalTransmissionQuality(uint8_t id)
{
	return(_originator[id].gtq);
}
uint16_t ICACHE_FLASH_ATTR m2mMesh::serviceFlags()
{
	return(_serviceFlags);
}
void ICACHE_FLASH_ATTR m2mMesh::setServiceFlags(uint16_t flags)
{
	_serviceFlags = flags;
}
float  ICACHE_FLASH_ATTR m2mMesh::currentTxPower()
{
	return(_currentTxPower);
}
float  ICACHE_FLASH_ATTR m2mMesh::currentTxPower(uint8_t id)
{
	return(_originator[id].currentTxPower);
}
float ICACHE_FLASH_ATTR m2mMesh::txPowerFloor()
{
	return(_txPowerFloor);
}
bool ICACHE_FLASH_ATTR m2mMesh::nodeNameIsSet()
{
	if(_nodeName != nullptr)
	{
		return(true);
	}
	else
	{
		return(false);
	}
}
bool ICACHE_FLASH_ATTR m2mMesh::nodeNameIsSet(uint8_t id)
{
	if(_originator[id].nodeName != nullptr)
	{
		return(true);
	}
	else
	{
		return(false);
	}
}
uint32_t ICACHE_FLASH_ATTR m2mMesh::expectedUptime(uint8_t originatorId) //Returns the current uptime of a node, on the assumption it has continued to run uninterrupted since we last heard from it
{
  return(_originator[originatorId].uptime + (millis() - _originator[originatorId].lastSeen[NHS_PACKET_TYPE]));
}
uint8_t ICACHE_FLASH_ATTR m2mMesh::currentMeshTimeServer()
{
	return(_currentMeshTimeServer);
}
int32_t ICACHE_FLASH_ATTR m2mMesh::meshTimeDrift()
{
	return(_meshTimeDrift);
}
bool ICACHE_FLASH_ATTR m2mMesh::meshIsStable()
{
	if(millis() - _meshLastChanged > _currentInterval[NHS_PACKET_TYPE])
	{
		return (true);
	}
	else
	{
		return (false);
	}
}
uint32_t ICACHE_FLASH_ATTR m2mMesh::loggingLevel()
{
	return(_loggingLevel);
}
void ICACHE_FLASH_ATTR m2mMesh::setLoggingLevel(uint32_t level)
{
	_loggingLevel = level;
}
uint32_t ICACHE_FLASH_ATTR m2mMesh::rxPackets()
{
	return(_rxPackets);
}
uint32_t ICACHE_FLASH_ATTR m2mMesh::rxPackets(uint8_t id)
{
	return(_originator[id].rxPackets);
}
uint32_t ICACHE_FLASH_ATTR m2mMesh::txPackets()
{
	return(_txPackets);
}
uint32_t ICACHE_FLASH_ATTR m2mMesh::txPackets(uint8_t id)
{
	return(_originator[id].txPackets);
}
uint32_t ICACHE_FLASH_ATTR m2mMesh::droppedRxPackets()
{
	return(_droppedRxPackets);
}
uint32_t ICACHE_FLASH_ATTR m2mMesh::droppedRxPackets(uint8_t id)
{
	return(_originator[id].droppedRxPackets);
}
uint32_t ICACHE_FLASH_ATTR m2mMesh::droppedTxPackets()
{
	return(_droppedTxPackets);
}
uint32_t ICACHE_FLASH_ATTR m2mMesh::droppedTxPackets(uint8_t id)
{
	return(_originator[id].droppedTxPackets);
}
#endif
#endif