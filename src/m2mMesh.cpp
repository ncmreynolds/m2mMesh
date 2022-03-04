#ifndef m2mMesh_cpp
#define m2mMesh_cpp
#include "m2mMesh.h"

//Wrapper hack to allow the use of callbacks to the ESP-Now C library from the C++ class

m2mMeshClass* m2mMeshPointer = nullptr;	//A pointer to 'this' eventually

#ifdef ESP8266
void IRAM_ATTR espNowSendCallbackWrapper(uint8_t* a, uint8_t b)
#elif defined(ESP32)
void IRAM_ATTR espNowSendCallbackWrapper(const uint8_t *a, esp_now_send_status_t b)
#endif
{
    if (m2mMeshPointer)
	{
        m2mMeshPointer->espNowSendCallback(a,b);
	}
}

#ifdef ESP8266
void IRAM_ATTR espNowReceiveCallbackWrapper(uint8_t *a, uint8_t *b, uint8_t c)
#elif defined(ESP32)
void IRAM_ATTR espNowReceiveCallbackWrapper(const uint8_t *a, const uint8_t *b, int32_t c)
#endif
{
    if (m2mMeshPointer)
	{
        m2mMeshPointer->espNowReceiveCallback(a,b,c);
	}
}


//Constructor function
ICACHE_FLASH_ATTR m2mMeshClass::m2mMeshClass()
{
	//Set the pointer for the C callback wrapper hack
	m2mMeshPointer = this;
}
//Destructor function
ICACHE_FLASH_ATTR m2mMeshClass::~m2mMeshClass()
{
	//Do nothing
}

bool ICACHE_FLASH_ATTR m2mMeshClass::begin(const uint8_t max, const uint8_t channel)
{
	if(max == 0)
	{
		_maxNumberOfOriginators = 1;
		_allowMeshGrowth = true;
	}
	else
	{
		_maxNumberOfOriginators = max;
	}
	//Allocate a block of memory for the originator table, a minimum of one
	if(_maxNumberOfOriginators > 0)
	{
		_originator = new m2mMeshOriginatorInfo[_maxNumberOfOriginators];
	}
	else
	{
		_originator = new m2mMeshOriginatorInfo[1];
	}
	#ifdef m2mMeshIncludeDebugFeatures
	//ESP8266/ESP8285 and ESP32 need different handling and have subtly different APIs	
	if (_debugEnabled == true && _loggingLevel & MESH_UI_LOG_INFORMATION)
	{
		#if defined(ESP8266)
		_debugStream->print(F("\r\nMesh running on ESP8266/ESP8285"));
		#elif defined(ESP32)
		_debugStream->print(F("\r\nMesh running on ESP32"));
		#endif
		//Show the IDF version, if possible
		#ifdef ESP_IDF_VERSION_MAJOR
		_debugStream->print(F("\r\nIDF version:"));			
		#ifdef ESP_IDF_VERSION_MINOR
			_debugStream->print(ESP_IDF_VERSION_MAJOR);
			_debugStream->print('.');
			_debugStream->println(ESP_IDF_VERSION_MINOR);
		#else
			_debugStream->println(ESP_IDF_VERSION_MAJOR);
		#endif
		#endif
		_debugStream->printf_P(m2mMeshstartedwithcapacityfordnodes,_maxNumberOfOriginators);
		if(_allowMeshGrowth == true)
		{
			_debugStream->print(m2mMeshautomaticallysizingmeshStabilitymaybeimpacted);
		}
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
	_forwardingDelay = _localMacAddress[5] >> 4;				//Delay up to 255ms to reduce flood collisions
	_currentInterval[ELP_PACKET_TYPE] = ELP_FAST_INTERVAL;		//Packet sending intervals, per packet type which may be adjust later to reduce congestion
	_currentInterval[OGM_PACKET_TYPE] = OGM_FAST_INTERVAL;
	_currentInterval[NHS_PACKET_TYPE] = NHS_FAST_INTERVAL;
	_lastSent[ELP_PACKET_TYPE] = 5000ul - _currentInterval[ELP_PACKET_TYPE];	//Offset sending times slightly between the different protocols
	_lastSent[OGM_PACKET_TYPE] = 10000ul - _currentInterval[OGM_PACKET_TYPE];	//Send the first packets sooner than the default interval
	_lastSent[NHS_PACKET_TYPE] = 30000ul - _currentInterval[NHS_PACKET_TYPE];	//would cause
	_currentTtl[ELP_PACKET_TYPE] = ELP_DEFAULT_TTL;					//TTLs, per packet type which are unlikely to change
	_currentTtl[OGM_PACKET_TYPE] = OGM_DEFAULT_TTL;
	_currentTtl[NHS_PACKET_TYPE] = NHS_DEFAULT_TTL;
	_currentTtl[PRP_PACKET_TYPE] = PRP_DEFAULT_TTL;
	_currentTtl[TRACE_PACKET_TYPE] = TRACE_DEFAULT_TTL;
	_currentTtl[USR_PACKET_TYPE] = USR_DEFAULT_TTL;
	_currentTtl[BTP_PACKET_TYPE] = BTP_DEFAULT_TTL;
	_currentChannel = channel;										//Set the send channel
	WiFi.channel(_currentChannel);									//Set the receive channel
	return(_initESPNow());											//Initialise ESP-NOW, the function handles the API differences between ESP8266/ESP8285 and ESP32
}


void ICACHE_FLASH_ATTR m2mMeshClass::end()
{
	//Remove callback functions
	//Free memory
}

bool m2mMeshClass::_initESPNow()
{
	//Start WiFi if it isn't already started
	#if defined(ESP8266)
	if(WiFi.status() == 7)	//This seems to be the 'not started' status, which isn't documented in the ESP8266 core header files. If you don't start WiFi, no packets will be sent
	#elif defined(ESP32)
	if(WiFi.getMode() == WIFI_OFF)
	#endif
	{
		#ifdef m2mMeshIncludeDebugFeatures
		if (_debugEnabled == true && _loggingLevel & MESH_UI_LOG_INFORMATION)
		{
			_debugStream->print(m2mMeshinitialisingWiFi);
		}
		#endif
		#if defined(ESP8266)
		wl_status_t status = WiFi.begin();
		if(status == WL_IDLE_STATUS || status == WL_CONNECTED)
		{
			#ifdef m2mMeshIncludeDebugFeatures
			if (_debugEnabled == true && _loggingLevel & MESH_UI_LOG_INFORMATION)
			{
				_debugStream->print(m2mMeshsuccess);
			}
			#endif
		}
		else
		{
			#ifdef m2mMeshIncludeDebugFeatures
			if (_debugEnabled == true && _loggingLevel & MESH_UI_LOG_INFORMATION)
			{
				#if defined(ESP8266)
				_debugStream->print(m2mMeshfailed);
				#elif defined(ESP32)
				_debugStream->print(m2mMeshfailed_code_);
				_debugStream->print(esp_err_to_name(status));
				#endif
			}
			#endif
			return(false);
		}
		#elif defined(ESP32)
		WiFi.begin();							//Start the WiFi
		esp_err_t status = WiFi.mode(WIFI_STA);	//Annoyingly this errors, but then everything works, so can't check for success
		#endif
	}
	#ifdef m2mMeshIncludeDebugFeatures
	else if (_debugEnabled == true && _loggingLevel & MESH_UI_LOG_INFORMATION)
	{
		_debugStream->print(m2mMeshWiFiStatus);
		_debugStream->print(WiFi.status());
	}
	#endif
	//Check if the ESP-NOW initialization was successful, ESP8286/ESP8285 have different return results
	#if defined(ESP8266)
	uint8_t initResult = esp_now_init();
	#elif defined(ESP32)
	esp_err_t initResult = esp_now_init();
	#endif
	#ifdef m2mMeshIncludeDebugFeatures
	if (_debugEnabled == true && _loggingLevel & MESH_UI_LOG_INFORMATION)
	{
		_debugStream->print(m2mMeshinitialisingESPNOW);
	}
	#endif
	if (initResult != ESP_OK)	//Init failed
	{
		#ifdef m2mMeshIncludeDebugFeatures
		if (_debugEnabled == true && _loggingLevel & MESH_UI_LOG_INFORMATION)
		{
			_debugStream->print(m2mMeshfailed);
		}
		#endif
		return(false);
	}
	else
	{
		#ifdef m2mMeshIncludeDebugFeatures
		if (_debugEnabled == true && _loggingLevel & MESH_UI_LOG_INFORMATION)
		{
			_debugStream->print(m2mMeshsuccess);
		}
		#endif
		//The ESP8266/ESP8285 require a 'role' to be set, which is vaguely analogous to the ifidx on ESP32. Without this you cannot send packets.
		#if defined(ESP8266)
		esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
		#endif
		//Packets are processed with callback functions, which are called via wrappers from the C++ class
		//Register the callback routines needed to process ESP-Now traffic, using the wrapper functions
		esp_now_register_send_cb(espNowSendCallbackWrapper);
		esp_now_register_recv_cb(espNowReceiveCallbackWrapper);
		// Add the broadcast MAC address as an ESP-NOW peer. This is used for any message that needs to 'flood' the network. Without this node discovery will not work.
		#ifdef m2mMeshIncludeDebugFeatures
		if (_debugEnabled == true && _loggingLevel & MESH_UI_LOG_INFORMATION)
		{
			_debugStream->println();
			_debugStream->print(m2mMeshaddingbroadcastMACaddressasapeertoenablemeshdiscovery);
		}
		#endif
		return(_addPeer(_broadcastMacAddress, _currentChannel));
	}
	return(false);
}

#ifdef ESP8266
void IRAM_ATTR m2mMeshClass::espNowSendCallback(uint8_t* macAddress, uint8_t status)
#elif defined(ESP32)
void IRAM_ATTR m2mMeshClass::espNowSendCallback(const uint8_t* macAddress, esp_now_send_status_t status)
#endif
{
	//Don't HAVE to do anything in the send callback
	/*if(_debugEnabled == true)
	{
		if(status == ESP_OK)
		{
			_debugStream->printf_P(m2mMeshPacketsentto02x02x02x02x02x02x,macAddress[0],macAddress[1],macAddress[2],macAddress[3],macAddress[4],macAddress[5]);
		}
	}*/
	if(_waitingForSend == true)
	{
		_waitingForSend = false;
		if(status == ESP_OK)
		{
			_sendSuccess = true;
		}
		else
		{
			_sendSuccess = false;
		}
	}
	_sendSuccess = true;
}

#ifdef ESP8266
//void IRAM_ATTR m2mMeshClass::espNowReceiveCallback(uint8_t *macAddress, uint8_t *data, uint8_t length)
void ICACHE_FLASH_ATTR m2mMeshClass::espNowReceiveCallback(uint8_t *macAddress, uint8_t *data, uint8_t length)
#elif defined(ESP32)
//void IRAM_ATTR m2mMeshClass::espNowReceiveCallback(const uint8_t *macAddress, const uint8_t *data, int32_t length)
void ICACHE_FLASH_ATTR m2mMeshClass::espNowReceiveCallback(const uint8_t *macAddress, const uint8_t *data, int32_t length)
#endif
{
	//Immediately check the checksum to avoid processing invalid packets
	uint8_t checksum = 0;
	//for(uint8_t index = 2; index < length; index++)
	for(uint8_t index = m2mMeshPacketDataLengthIndex; index < data[m2mMeshPacketDataLengthIndex]; index++)
	{
		checksum+=data[index] >> 4;
		checksum+=data[index] & 0x0f;
	}
	checksum = checksum ^ _meshId;
	if(data[m2mMeshPacketChecksumIndex] != checksum)
	{
		#ifdef m2mMeshIncludeDebugFeatures
		if(_debugEnabled == true && _loggingLevel & m2mMesh.MESH_UI_LOG_ERRORS)
		{
			_debugStream->printf_P(m2mMeshchecksumInvalidreceived2xshouldbe2x, macAddress[0], macAddress[1], macAddress[2], macAddress[3], macAddress[4], macAddress[5], data[m2mMeshPacketChecksumIndex], checksum);
		}
		#endif
		return;
	}
	if(length > ESP_NOW_MAX_PACKET_SIZE)	//This is unlikely to be excessively long, but will should avoid a crash
	{
		return;
	}
	uint8_t routerMacAddress[6] = {0,0,0,0,0,0};	//Take a copy of the MAC address as it may get tweaked
	memcpy(&routerMacAddress, macAddress, 6);
	if(data[m2mMeshPacketTypeIndex] & SENDER_IS_SOFTAP)	//Sender MAC address needs tweaking if bit is set
	{
		if(routerMacAddress[0] & B00000010)	//This is an ESP8266, this is how they make the AP MAC address
		{
			routerMacAddress[0] = routerMacAddress[0] & B11111101;	//Remove the locally administered flag
		}
		else	//Other models increment the last byte
		{
			routerMacAddress[5] = routerMacAddress[5] - 1;	//Decrement the last octet
		}
	}
	uint8_t routerId = _originatorIdFromMac(routerMacAddress);	//Extract router ID
	if(routerId == MESH_ORIGINATOR_NOT_FOUND)
	{
		#ifdef m2mMeshIncludeDebugFeatures
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NODE_MANAGEMENT)
		{
			_debugStream->print(m2mMeshRTR);
		}
		#endif
		routerId = _addOriginator(routerMacAddress,_currentChannel);
		if(routerId == MESH_ORIGINATOR_NOT_FOUND)
		{
			return;	//Give up on this packet if its router can't be determined
		}
	}
	//Extract originator ID
	uint8_t originatorId = _originatorIdFromMac(&data[m2mMeshPacketOriginatorIndex]);
	if(originatorId == MESH_ORIGINATOR_NOT_FOUND)
	{
		#ifdef m2mMeshIncludeDebugFeatures
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NODE_MANAGEMENT)
		{
			_debugStream->print(F("\r\nSRC "));
		}
		#endif
		originatorId = _addOriginator(&data[m2mMeshPacketOriginatorIndex],_currentChannel);
		if(originatorId == MESH_ORIGINATOR_NOT_FOUND)
		{
			return;	//Give up on this packet if its originator can't be determined
		}
	}
	uint8_t _packetType = data[m2mMeshPacketTypeIndex] & 0x07;
	uint32_t sequenceNumber;
	memcpy(&sequenceNumber, &data[m2mMeshPacketSNIndex],sizeof(sequenceNumber));
	//The originator may be a completely new device, so look for it but ignore any echoes of this node's traffic
	if(originatorId == MESH_THIS_ORIGINATOR)	//Process 'echoes' where this node is the source
	{
		if(routerId != MESH_ORIGINATOR_NOT_FOUND && _packetType == OGM_PACKET_TYPE)	//Process potential OGM 'echoes' for LTQ calculation, all others are silently dropped
		{
			if(sequenceNumber == _lastOGMSequenceNumber)
			{
				_originator[routerId].ogmEchoes = _originator[routerId].ogmEchoes >> 1;		//Right shift the OGM Echo receipt confirmation bitmask, which will make it worse
				_originator[routerId].ogmEchoes = _originator[routerId].ogmEchoes | 0x8000;	//Set the most significant bit of the OGM Echo receipt confirmation bitmask, which MAY make it 'better'
				_originator[routerId].ogmEchoLastConfirmed = millis();						//Record when this happened, so the node can spot 'missed' echoes
				_calculateLtq(routerId);													//Recalculate Local Transmission Quality
				#ifdef m2mMeshIncludeDebugFeatures
				if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_OGM_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || routerId == _nodeToLog))
				{
					_debugStream->printf_P(m2mMeshOGMECHOR02x02x02x02x02x02xO02x02x02x02x02x02xTTLdHOPdLEN,
					routerMacAddress[0],
					routerMacAddress[1],
					routerMacAddress[2],
					routerMacAddress[3],
					routerMacAddress[4],
					routerMacAddress[5],
					data[m2mMeshPacketOriginatorIndex],
					data[m2mMeshPacketOriginatorIndex1],
					data[m2mMeshPacketOriginatorIndex2],
					data[m2mMeshPacketOriginatorIndex3],
					data[m2mMeshPacketOriginatorIndex4],
					data[m2mMeshPacketOriginatorIndex5],
					sequenceNumber,
					data[m2mMeshPacketTTLIndex],
					data[20],
					length);
				}
				#endif
			}
			#ifdef m2mMeshIncludeDebugFeatures
			else if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_OGM_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || routerId == _nodeToLog))
			{
				_debugStream->printf_P(m2mMeshOGMECHOR02x02x02x02x02x02xO02x02x02x02x02x02xTTLdHOPdLENinvalid,
				routerMacAddress[0],
				routerMacAddress[1],
				routerMacAddress[2],
				routerMacAddress[3],
				routerMacAddress[4],
				routerMacAddress[5],
				data[m2mMeshPacketOriginatorIndex],
				data[m2mMeshPacketOriginatorIndex1],
				data[m2mMeshPacketOriginatorIndex2],
				data[m2mMeshPacketOriginatorIndex3],
				data[m2mMeshPacketOriginatorIndex4],
				data[m2mMeshPacketOriginatorIndex5],
				sequenceNumber,
				_lastOGMSequenceNumber);
			}
			#endif
		}
		return;	//An echo, which has been processed
	}
	else
	{
		if(sequenceNumber > _originator[originatorId].lastSequenceNumber || _originator[originatorId].sequenceNumberProtectionWindowActive == false)
		{
			//The sequence number was valid, or old enough to prompt a reset of the protection window, continue processing but enable protection window again
			if(_originator[originatorId].sequenceNumberProtectionWindowActive == false)
			{
				//Re-enable the sequence number protection window
				_originator[originatorId].sequenceNumberProtectionWindowActive = true;
				#ifdef m2mMeshIncludeDebugFeatures
				if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_WARNINGS && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || routerId == _nodeToLog || originatorId == _nodeToLog))
				{
					_debugStream->printf_P(m2mMesh02x02x02x02x02x02xsequencenumberprotectionenabled,
					data[m2mMeshPacketOriginatorIndex],
					data[m2mMeshPacketOriginatorIndex1],
					data[m2mMeshPacketOriginatorIndex2],
					data[m2mMeshPacketOriginatorIndex3],
					data[m2mMeshPacketOriginatorIndex4],
					data[m2mMeshPacketOriginatorIndex5]);
				}
				#endif
			}
			//Update the sequence number
			_originator[originatorId].lastSequenceNumber = sequenceNumber;
		}
		else if(_originator[originatorId].lastSequenceNumber - sequenceNumber > SEQUENCE_NUMBER_MAX_AGE)
		{
			//If it's a very old sequence number disable the protection window to allow the next packet to reset the sequence number
			//this logic is here to handle a reboot, but ignore a single stray weird sequence number
			_originator[originatorId].sequenceNumberProtectionWindowActive = false;
			rollOutTheWelcomeWagon(); //Do some stuff to get a new node updated ASAP
			#ifdef m2mMeshIncludeDebugFeatures
			if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_WARNINGS && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || routerId == _nodeToLog || originatorId == _nodeToLog))
			{
				_debugStream->printf_P(m2mMesh02x02x02x02x02x02xsequencenumberprotectiondisabledpossiblereboot,
					data[m2mMeshPacketOriginatorIndex],
					data[m2mMeshPacketOriginatorIndex1],
					data[m2mMeshPacketOriginatorIndex2],
					data[m2mMeshPacketOriginatorIndex3],
					data[m2mMeshPacketOriginatorIndex4],
					data[m2mMeshPacketOriginatorIndex5]);
			}
			#endif
		}
		else
		{
			return;	//Out of sequence
		}
	}
	uint8_t destinationId;
	if(data[m2mMeshPacketTypeIndex] & SEND_TO_ALL_NODES)
	{
		destinationId = MESH_ORIGINATOR_NOT_FOUND;
	}
	else	//If there's a destination, identify it
	{
		destinationId = _originatorIdFromMac(&data[m2mMeshPacketDestinationIndex]);
		if(destinationId == MESH_ORIGINATOR_NOT_FOUND)
		{
			#ifdef m2mMeshIncludeDebugFeatures
			if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NODE_MANAGEMENT)
			{
				_debugStream->print(F("\r\nDST "));
			}
			#endif
			destinationId = _addOriginator(&data[m2mMeshPacketDestinationIndex],_currentChannel);
			if(destinationId == MESH_ORIGINATOR_NOT_FOUND)
			{
				return;	//Give up on this packet if its destination can't be determined
			}
		}
	}
	#ifdef m2mMeshProcessPacketsInCallback
	m2mMeshPacketBuffer tempBuffer;	//Create a temporary packet buffer to process
	memcpy(&tempBuffer.routerMacAddress, macAddress, 6);
	//tempBuffer.routerMacAddress[0] = macAddress[0];
	//tempBuffer.routerMacAddress[1] = macAddress[1];
	//tempBuffer.routerMacAddress[2] = macAddress[2];
	//tempBuffer.routerMacAddress[3] = macAddress[3];
	//tempBuffer.routerMacAddress[4] = macAddress[4];
	//tempBuffer.routerMacAddress[5] = macAddress[5];
	tempBuffer.routerId = routerId;
	tempBuffer.originatorId = originatorId;
	tempBuffer.destinationId = destinationId;
	tempBuffer.sequenceNumber = sequenceNumber;
	tempBuffer.length = length;
	memcpy(&tempBuffer.data, data, length);
	tempBuffer.timestamp = millis();
	_processPacket(tempBuffer);	//Process the temporary buffer
	#else
	if(_receiveBuffer[_receiveBufferIndex].length == 0)	//The buffer slot is empty
	{
		memcpy(&_receiveBuffer[_receiveBufferIndex].routerMacAddress, macAddress, 6);	//Copy MAC address
		_receiveBuffer[_receiveBufferIndex].routerId = routerId;						//Get the router ID
		_receiveBuffer[_receiveBufferIndex].originatorId = originatorId;				//Copy in the originator ID
		_receiveBuffer[_receiveBufferIndex].destinationId = destinationId;				//Copy in the dstination ID
		_receiveBuffer[_receiveBufferIndex].sequenceNumber = sequenceNumber;				//Copy in the dstination ID
		_receiveBuffer[_receiveBufferIndex].length = length;							//Set the length
		memcpy(&_receiveBuffer[_receiveBufferIndex].data, data, length);				//Copy in the data
		_receiveBuffer[_receiveBufferIndex].timestamp = millis();						//Mark the timestamp
		_rxPackets++;
		#ifdef m2mMeshIncludeDebugFeatures
		if(_debugEnabled == true && (_loggingLevel & MESH_UI_LOG_BUFFER_MANAGEMENT))
		{
			_debugStream->printf_P(m2mMeshfillRCVbufferslotd,
				_receiveBufferIndex,
				length,
				packetTypeDescription[data[m2mMeshPacketTypeIndex] & 0x07],
				macAddress[0],
				macAddress[1],
				macAddress[2],
				macAddress[3],
				macAddress[4],
				macAddress[5]);
		}
		#endif
		_receiveBufferIndex++;												//Advance the buffer index
		_receiveBufferIndex = _receiveBufferIndex%M2MMESHRECEIVEBUFFERSIZE;	//Rollover buffer index
	}
	else
	{
		_droppedRxPackets++;	//Increase the count of dropped packets
		#ifdef m2mMeshIncludeDebugFeatures
		if(_debugEnabled == true && (_loggingLevel & MESH_UI_LOG_BUFFER_MANAGEMENT))
		{
			_debugStream->print(m2mMeshreadRCVbufferfull);
		}
		#endif
	}	
	#endif
	if(_activityLedEnabled)
	{
		_blinkActivityLed();	//Blinky blinky
	}
}

m2mMeshClass& ICACHE_FLASH_ATTR m2mMeshClass::setCallback(M2MMESH_CALLBACK)
{
    this->eventCallback = eventCallback;
    return *this;
}

uint8_t ICACHE_FLASH_ATTR m2mMeshClass::_calculateChecksum(m2mMeshPacketBuffer &packet)
{
	uint8_t checksum = 0;
	for(uint8_t index = m2mMeshPacketDataLengthIndex; index < packet.data[m2mMeshPacketDataLengthIndex]; index++)
	{
		checksum+=packet.data[index] >> 4;
		checksum+=packet.data[index] & 0x0f;
	}
	checksum = checksum ^ _meshId;
	return(checksum);
}

void ICACHE_FLASH_ATTR m2mMeshClass::_addChecksum(m2mMeshPacketBuffer &packet)
{
	packet.data[m2mMeshPacketChecksumIndex] = _calculateChecksum(packet);
}

bool ICACHE_FLASH_ATTR m2mMeshClass::_checksumCorrect(m2mMeshPacketBuffer &packet)
{
	uint8_t checksum = _calculateChecksum(packet);
	if(checksum == packet.data[m2mMeshPacketChecksumIndex])
	{
		return(true);
	}
	#ifdef m2mMeshIncludeDebugFeatures
	else if(_debugEnabled == true && _loggingLevel & m2mMesh.MESH_UI_LOG_ERRORS)
	{
		_debugStream->printf_P(m2mMeshchecksumInvalidreceived2xshouldbe2x, packet.routerMacAddress[0], packet.routerMacAddress[1], packet.routerMacAddress[2], packet.routerMacAddress[3], packet.routerMacAddress[4], packet.routerMacAddress[5], packet.data[m2mMeshPacketChecksumIndex], checksum);
	}
	#endif
	return(false);
}

void ICACHE_FLASH_ATTR m2mMeshClass::housekeeping()
{
	bool _packetSent = false;
	bool _packetReceived = false;
	//Process received packets first, if available
	#ifndef m2mMeshProcessPacketsInCallback
	if(_receiveBufferIndex != _processBufferIndex ||					//Some data in buffer
		_receiveBuffer[_processBufferIndex].length > 0)					//Buffer is full
	{
		#ifdef m2mMeshIncludeDebugFeatures
		if(_debugEnabled == true && (_loggingLevel & MESH_UI_LOG_BUFFER_MANAGEMENT))
		{
			_debugStream->printf_P(m2mMeshreadRCVbufferslotd,
				_processBufferIndex,
				_receiveBuffer[_processBufferIndex].length,
				packetTypeDescription[_receiveBuffer[_processBufferIndex].data[m2mMeshPacketTypeIndex] & 0x07],
				_receiveBuffer[_processBufferIndex].routerMacAddress[0],
				_receiveBuffer[_processBufferIndex].routerMacAddress[1],
				_receiveBuffer[_processBufferIndex].routerMacAddress[2],
				_receiveBuffer[_processBufferIndex].routerMacAddress[3],
				_receiveBuffer[_processBufferIndex].routerMacAddress[4],
				_receiveBuffer[_processBufferIndex].routerMacAddress[5]);
		}
		#endif
		_processPacket(_receiveBuffer[_processBufferIndex]);				//Process the packet
		_processBufferIndex++;												//Advance the buffer index
		_processBufferIndex = _processBufferIndex%M2MMESHRECEIVEBUFFERSIZE;	//Rollover the buffer index
		_packetReceived = true;
	}
	#endif
	//Forward anything in the forwarding buffer before sending anything new
	if((_fowardingBufferWriteIndex != _forwardingBufferReadIndex ||			//Some data in buffer
		_forwardingBuffer[_forwardingBufferReadIndex].length > 0)			//Buffer is full
		&& millis() - _forwardingBuffer[_forwardingBufferReadIndex].timestamp > _forwardingDelay)
	{
		#ifdef m2mMeshIncludeDebugFeatures
		if(_debugEnabled == true && (_loggingLevel & MESH_UI_LOG_BUFFER_MANAGEMENT))
		{
			_debugStream->printf_P(m2mMeshreadFWDbufferslotd,
				_forwardingBufferReadIndex,
				_forwardingBuffer[_forwardingBufferReadIndex].length,
				packetTypeDescription[_forwardingBuffer[_forwardingBufferReadIndex].data[m2mMeshPacketTypeIndex] & 0x07],
				_forwardingBuffer[_forwardingBufferReadIndex].routerMacAddress[0],
				_forwardingBuffer[_forwardingBufferReadIndex].routerMacAddress[1],
				_forwardingBuffer[_forwardingBufferReadIndex].routerMacAddress[2],
				_forwardingBuffer[_forwardingBufferReadIndex].routerMacAddress[3],
				_forwardingBuffer[_forwardingBufferReadIndex].routerMacAddress[4],
				_forwardingBuffer[_forwardingBufferReadIndex].routerMacAddress[5]);
		}
		#endif
		if(_forwardPacket(_forwardingBuffer[_forwardingBufferReadIndex]) == true)	//Process the packet
		{
			_fwdPackets++;
			_packetSent = true;
		}
		else
		{
			_droppedFwdPackets++;
			#ifdef m2mMeshIncludeDebugFeatures
			if(_debugEnabled == true)
			{
				if(_forwardingBuffer[_forwardingBufferReadIndex].data[m2mMeshPacketTypeIndex] & SEND_TO_ALL_NODES)
				{
					Serial.printf("\r\nFWD O:%02u/%02x:%02x:%02x:%02x:%02x:%02x D:%02u/ff:ff:ff:ff:ff:ff R:%02u/%02x:%02x:%02x:%02x:%02x:%02x %s",
					_forwardingBuffer[_forwardingBufferReadIndex].originatorId,
					_forwardingBuffer[_forwardingBufferReadIndex].data[m2mMeshPacketOriginatorIndex],
					_forwardingBuffer[_forwardingBufferReadIndex].data[m2mMeshPacketOriginatorIndex1],
					_forwardingBuffer[_forwardingBufferReadIndex].data[m2mMeshPacketOriginatorIndex2],
					_forwardingBuffer[_forwardingBufferReadIndex].data[m2mMeshPacketOriginatorIndex3],
					_forwardingBuffer[_forwardingBufferReadIndex].data[m2mMeshPacketOriginatorIndex4],
					_forwardingBuffer[_forwardingBufferReadIndex].data[m2mMeshPacketOriginatorIndex5],
					_forwardingBuffer[_forwardingBufferReadIndex].destinationId,
					_forwardingBuffer[_forwardingBufferReadIndex].routerId,
					_forwardingBuffer[_forwardingBufferReadIndex].routerMacAddress[0],
					_forwardingBuffer[_forwardingBufferReadIndex].routerMacAddress[1],
					_forwardingBuffer[_forwardingBufferReadIndex].routerMacAddress[2],
					_forwardingBuffer[_forwardingBufferReadIndex].routerMacAddress[3],
					_forwardingBuffer[_forwardingBufferReadIndex].routerMacAddress[4],
					_forwardingBuffer[_forwardingBufferReadIndex].routerMacAddress[5],
					errorDescriptionTable[_lastError]);
				}
				else
				{
					Serial.printf("\r\nFWD O:%02u/%02x:%02x:%02x:%02x:%02x:%02x D:%02u/%02x:%02x:%02x:%02x:%02x:%02x R:%02u/%02x:%02x:%02x:%02x:%02x:%02x %s",
					_forwardingBuffer[_forwardingBufferReadIndex].originatorId,
					_forwardingBuffer[_forwardingBufferReadIndex].data[m2mMeshPacketOriginatorIndex],
					_forwardingBuffer[_forwardingBufferReadIndex].data[m2mMeshPacketOriginatorIndex1],
					_forwardingBuffer[_forwardingBufferReadIndex].data[m2mMeshPacketOriginatorIndex2],
					_forwardingBuffer[_forwardingBufferReadIndex].data[m2mMeshPacketOriginatorIndex3],
					_forwardingBuffer[_forwardingBufferReadIndex].data[m2mMeshPacketOriginatorIndex4],
					_forwardingBuffer[_forwardingBufferReadIndex].data[m2mMeshPacketOriginatorIndex5],
					_forwardingBuffer[_forwardingBufferReadIndex].destinationId,
					_forwardingBuffer[_forwardingBufferReadIndex].data[m2mMeshPacketDestinationIndex],
					_forwardingBuffer[_forwardingBufferReadIndex].data[m2mMeshPacketDestinationIndex1],
					_forwardingBuffer[_forwardingBufferReadIndex].data[m2mMeshPacketDestinationIndex2],
					_forwardingBuffer[_forwardingBufferReadIndex].data[m2mMeshPacketDestinationIndex3],
					_forwardingBuffer[_forwardingBufferReadIndex].data[m2mMeshPacketDestinationIndex4],
					_forwardingBuffer[_forwardingBufferReadIndex].data[19],
					_forwardingBuffer[_forwardingBufferReadIndex].routerId,
					_forwardingBuffer[_forwardingBufferReadIndex].routerMacAddress[0],
					_forwardingBuffer[_forwardingBufferReadIndex].routerMacAddress[1],
					_forwardingBuffer[_forwardingBufferReadIndex].routerMacAddress[2],
					_forwardingBuffer[_forwardingBufferReadIndex].routerMacAddress[3],
					_forwardingBuffer[_forwardingBufferReadIndex].routerMacAddress[4],
					_forwardingBuffer[_forwardingBufferReadIndex].routerMacAddress[5],
					errorDescriptionTable[_lastError]);
				}
			}
			_lastError = m2mMesh_Success;
			#endif
		}
		_forwardingBuffer[_forwardingBufferReadIndex].length = 0;				//Mark as sent
		_forwardingBufferReadIndex++;											//Advance the buffer index
		_forwardingBufferReadIndex = _forwardingBufferReadIndex%M2MMESHFORWARDINGBUFFERSIZE;	//Rollover the buffer index
	}
	//Now consider sending ELP
	if ((_serviceFlags & PROTOCOL_ELP_SEND) && millis() - (_lastSent[ELP_PACKET_TYPE] - _nextJitter) > _currentInterval[ELP_PACKET_TYPE]) 	//Apply some jitter to the send times
	{
		_nextJitter = random(ANTI_COLLISION_JITTER);
		if(_stable == true && _currentInterval[ELP_PACKET_TYPE] != ELP_DEFAULT_INTERVAL)	//Transition from fast sending
		{
			_currentInterval[ELP_PACKET_TYPE] = ELP_DEFAULT_INTERVAL;
		}
		if(_sendElp(_sendBuffer))
		{
			_lastSent[ELP_PACKET_TYPE] = millis();
			_packetSent = true;
		}
		else
		{
			#ifdef m2mMeshIncludeDebugFeatures
			if(_debugEnabled == true && (_loggingLevel & MESH_UI_LOG_ERRORS))
			{
				_debugStream->printf("\r\nELP send failed \"%s\", retrying", errorDescriptionTable[_lastError]);
			}
			#endif
			_lastSent[ELP_PACKET_TYPE] += random(100,1000);	//Cause a retry in 100-1000ms
		}
	}
	//Conside sending OGM
	else if ((_serviceFlags & PROTOCOL_OGM_SEND) && millis() - (_lastSent[OGM_PACKET_TYPE] - _nextJitter) > _currentInterval[OGM_PACKET_TYPE])
	{
		_nextJitter = random(ANTI_COLLISION_JITTER);
		if(_stable == true && _currentInterval[OGM_PACKET_TYPE] != OGM_DEFAULT_INTERVAL)	//Transition from fast sending
		{
			_currentInterval[OGM_PACKET_TYPE] = OGM_DEFAULT_INTERVAL;
		}
		if(_sendOgm(_sendBuffer))
		{
			_lastSent[OGM_PACKET_TYPE] = millis();
			_packetSent = true;
		}
		else
		{
			#ifdef m2mMeshIncludeDebugFeatures
			if(_debugEnabled == true && (_loggingLevel & MESH_UI_LOG_ERRORS))
			{
				_debugStream->printf("\r\nOGM send failed \"%s\", retrying", errorDescriptionTable[_lastError]);
			}
			#endif
			_lastSent[OGM_PACKET_TYPE] += random(100,1000);	//Cause a retry in 100-1000ms
		}
	}
	//Consider sending NHS
	else if ((_serviceFlags & PROTOCOL_NHS_SEND) && millis() - (_lastSent[NHS_PACKET_TYPE] - _nextJitter) > _currentInterval[NHS_PACKET_TYPE])
	{
		//Check the availability of the sync server
		/*if(not(_actingAsSyncServer) && currentMeshSyncServer != MESH_ORIGINATOR_NOT_FOUND && not dataIsValid(currentMeshSyncServer,NHS_PACKET_TYPE))
		{
			//Choose a new sync server if it's offline
			chooseNewSyncServer();
		}*/
		_nextJitter = random(ANTI_COLLISION_JITTER);
		if(_stable == true && _currentInterval[NHS_PACKET_TYPE] != NHS_DEFAULT_INTERVAL)	//Transition from fast sending
		{
			_currentInterval[NHS_PACKET_TYPE] = NHS_DEFAULT_INTERVAL;
		}
		if(_sendNhs(_sendBuffer))
		{
			_lastSent[NHS_PACKET_TYPE] = millis();
			_packetSent = true;
		}
		else
		{
			#ifdef m2mMeshIncludeDebugFeatures
			if(_debugEnabled == true && (_loggingLevel & MESH_UI_LOG_ERRORS))
			{
				_debugStream->printf("\r\nNHS send failed \"%s\", retrying", errorDescriptionTable[_lastError]);
			}
			#endif
			_lastSent[NHS_PACKET_TYPE] += random(100,1000);	//Cause a retry in 100-1000ms
		}
	}
	//Housekeep the tables after something happens
	if(_packetReceived == true || _packetSent == true || millis() - _lastHousekeeping > _housekeepingInterval)	//Need to refresh the stats and routes
	{
		//Update the broad mesh statistics and fingerprint
		bool joined = false;
		if(_numberOfActiveNeighbours > 0)
		{
			joined = true;
		}
		_numberOfActiveNeighbours = 0;
		_numberOfReachableOriginators = 0;
		//Copy in the local MAC address to the mesh MAC address
		memcpy(&_meshMacAddress, &_localMacAddress, 6);
		for(uint8_t originatorId = 0; originatorId < _numberOfOriginators; originatorId++)
		{
			if(_dataIsValid(originatorId,ELP_PACKET_TYPE))
			{
				_numberOfActiveNeighbours++;
				//Active peer management
				if(_originator[originatorId].isCurrentlyPeer == true)
				{
					if(((_originator[originatorId].hasUsAsPeer == true && _originator[originatorId].peeringExpired == true) || _originator[originatorId].hasUsAsPeer == false) && millis() - _originator[originatorId].peerNeeded > _peerLifetime)
					{
						#ifdef m2mMeshIncludeDebugFeatures
						if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_PEER_MANAGEMENT)
						{
							_debugStream->printf_P(m2mMeshPeering_expired_with,
								originatorId,
								_originator[originatorId].macAddress[0],
								_originator[originatorId].macAddress[1],
								_originator[originatorId].macAddress[2],
								_originator[originatorId].macAddress[3],
								_originator[originatorId].macAddress[4],
								_originator[originatorId].macAddress[5]);
						}
						#endif
						_removePeer(originatorId);	//Both ends have expired or removed, remove peering
					}
				}
			}
			else
			{
				//Passive peer management
				if(_originator[originatorId].isCurrentlyPeer == true && millis() - _originator[originatorId].peerNeeded > _peerLifetime)
				{
					#ifdef m2mMeshIncludeDebugFeatures
					if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NODE_MANAGEMENT)
					{
						_debugStream->printf_P(m2mMeshPeering_expired_with,
							originatorId,
							_originator[originatorId].macAddress[0],
							_originator[originatorId].macAddress[1],
							_originator[originatorId].macAddress[2],
							_originator[originatorId].macAddress[3],
							_originator[originatorId].macAddress[4],
							_originator[originatorId].macAddress[5]);
					}
					#endif
					_removePeer(originatorId);
				}
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
			if(_originator[originatorId].ogmEchoes > 0 && millis() - _originator[originatorId].ogmEchoLastConfirmed > 3*_currentInterval[OGM_PACKET_TYPE]/2)	//We have missed an OGM echo
			{
				#ifdef m2mMeshIncludeDebugFeatures
				if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_OGM_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || originatorId == _nodeToLog))
				{
					_debugStream->printf_P(m2mMeshOGMECHOR02x02x02x02x02x02xO02x02x02x02x02x02xSEQumissed,_originator[originatorId].macAddress[0],_originator[originatorId].macAddress[1],_originator[originatorId].macAddress[2],_originator[originatorId].macAddress[3],_originator[originatorId].macAddress[4],_originator[originatorId].macAddress[5],_localMacAddress[0],_localMacAddress[1],_localMacAddress[2],_localMacAddress[3],_localMacAddress[4],_localMacAddress[5],_lastOGMSequenceNumber);
				}
				#endif
				_originator[originatorId].ogmEchoes = _originator[originatorId].ogmEchoes >> 1;					//Right shift the OGM Echo receipt confirmation bitmask, which will make it worse
				_calculateLtq(originatorId);																	//Recalculate the Local Transmission quality
				_originator[originatorId].ogmEchoLastConfirmed = millis();										//Record the time of the last missed OGM echo
			}
			if(_originator[originatorId].ogmReceived > 0 && millis() - _originator[originatorId].ogmReceiptLastConfirmed > 3*_originator[originatorId].interval[OGM_PACKET_TYPE]/2)	//We have missed an OGM
			{
				#ifdef m2mMeshIncludeDebugFeatures
				if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_OGM_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || originatorId == _nodeToLog))
				{
					_debugStream->printf_P(m2mMeshOGMFROMO02x02x02x02x02x02xmissed,_originator[originatorId].macAddress[0],_originator[originatorId].macAddress[1],_originator[originatorId].macAddress[2],_originator[originatorId].macAddress[3],_originator[originatorId].macAddress[4],_originator[originatorId].macAddress[5]);
				}
				#endif
				_originator[originatorId].ogmReceived = _originator[originatorId].ogmReceived >> 1;				//Right shift the OGM receipt confirmation bitmask, which will make it worse
				_calculateLtq(originatorId);	//Recalculate the Local Transmission quality
				_originator[originatorId].ogmReceiptLastConfirmed = millis();	//Record the time of the last missed OGM
			}
			
		}
		//Process events for the callback function
		if(eventCallback)
		{
			if(joined == false && _numberOfActiveNeighbours > 0)
			{
				eventCallback(meshEvent::joined);				
			}
			else if(joined == true && _numberOfActiveNeighbours == 0)
			{
				eventCallback(meshEvent::left);
			}
		}
		_lastHousekeeping = millis();
	}
	// If the activity LED is lit, check if it's time to turn it off
	if(_activityLedEnabled && _activityLedState == true && millis() - _activityLedTimer > ACTIVITY_LED_BLINK_TIME)
	{
		digitalWrite(_activityLedPin,not _activityLedOnState);
		_activityLedState = false;
	}
}

void ICACHE_FLASH_ATTR m2mMeshClass::_processPacket(m2mMeshPacketBuffer &packet)
{
	#ifdef m2mMeshIncludeDebugFeatures
	if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_ALL_RECEIVED_PACKETS)
	{
		_debugStream->println();
		_debugStream->print(m2mMeshReceivedfrom);
		_debugPacket(packet);
	}
	#endif
	//Start processing the packet
	uint8_t _packetType = packet.data[m2mMeshPacketTypeIndex] & 0x07;
	if(packet.routerId != MESH_ORIGINATOR_NOT_FOUND && (packet.data[m2mMeshPacketTypeIndex] & SEND_TO_ALL_NODES) == 0x00)
	{
		_originator[packet.routerId].peerNeeded = millis();	//This peering is likely needed
	}
	if(_packetType < 3)	//Refresh first/last seen and interval data
	{
		_originator[packet.originatorId].lastSeen[_packetType] = millis();
		uint32_t packetInterval;
		memcpy(&packetInterval, &packet.data[m2mMeshPacketIntervalIndex], sizeof(packetInterval));
		if(packetInterval != _originator[packet.originatorId].interval[_packetType])
		{
			#ifdef m2mMeshIncludeDebugFeatures
			if(_debugEnabled == true && _originator[packet.originatorId].interval[_packetType] > 0 && _loggingLevel & MESH_UI_LOG_INFORMATION && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || packet.routerId == _nodeToLog || packet.originatorId == _nodeToLog))
			{
				_debugStream->printf_P(m2mMeshsoriginator02x02x02x02x02x02xchangedintervalfromdtod,
				packetTypeDescription[packet.data[m2mMeshPacketTypeIndex] & 0x07],
				packet.data[m2mMeshPacketOriginatorIndex],
				packet.data[m2mMeshPacketOriginatorIndex1],
				packet.data[m2mMeshPacketOriginatorIndex2],
				packet.data[m2mMeshPacketOriginatorIndex3],
				packet.data[m2mMeshPacketOriginatorIndex4],
				packet.data[m2mMeshPacketOriginatorIndex5],
				_originator[packet.originatorId].interval[_packetType],
				packetInterval);
			}
			#endif
			_originator[packet.originatorId].interval[_packetType] = packetInterval;
		}
	}
	//At this point we know the router, originator and destination and can check for echoes, which are never processed further
	//Consider the packet for processing so check the destination. Either it is has to be a flood or have this node as the destination
	if((packet.data[m2mMeshPacketTypeIndex] & SEND_TO_ALL_NODES) ||
		(((packet.data[m2mMeshPacketTypeIndex] & SEND_TO_ALL_NODES) == 0x00) && _isLocalMacAddress(&packet.data[m2mMeshPacketDestinationIndex]))
		)
	{
		if(_packetType == ELP_PACKET_TYPE && (_serviceFlags & PROTOCOL_ELP_RECEIVE))			//This is an ELP packet
		{
			_processElp(packet);	//Process the contents of the ELP packet
		}
		else if(_packetType == OGM_PACKET_TYPE && (_serviceFlags & PROTOCOL_OGM_RECEIVE))	//This is an OGM packet
		{
			_processOgm(packet);	//Process the contents of the OGM packet
		}
		else if(_packetType == NHS_PACKET_TYPE && (_serviceFlags & PROTOCOL_NHS_RECEIVE))	//This is a NHS packet
		{
			_processNhs(packet);	//Process the contents of the NHS packet
		}
		else if(_packetType == PRP_PACKET_TYPE)	//This is a trace packet
		{
			_processPrp(packet);	//Process the contents of the PRP packet
		}
		else if(_packetType == TRACE_PACKET_TYPE)	//This is a trace packet
		{
			_processTrace(packet);	//Process the contents of the trace packet
		}
		else if(_packetType == USR_PACKET_TYPE && (_serviceFlags & PROTOCOL_USR_RECEIVE))	//This is a USR packet
		{
			_processUsr(packet);	//Process the contents of the USR packet
		}
		#ifdef m2mMeshIncludeDebugFeatures
		else if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_WARNINGS && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || packet.routerId == _nodeToLog || packet.originatorId == _nodeToLog))
		{
			_debugStream->printf_P(nm2mMeshWARNINGunknowntypedfrom02x02x02x02x02x02x,_packetType,packet.routerMacAddress[0],packet.routerMacAddress[1],packet.routerMacAddress[2],packet.routerMacAddress[3],packet.routerMacAddress[4],packet.routerMacAddress[5]);
		}
		#endif
	}
	#ifdef m2mMeshIncludeDebugFeatures
	/*else if((packet.data[m2mMeshPacketTypeIndex] & PEERING_REQUEST) == 0x00 && _debugEnabled == true && _loggingLevel & MESH_UI_LOG_WARNINGS)
	{
		_debugStream->printf_P(m2mMeshrtrr02x02x02x02x02x02xd02x02x02x02x02x02xunexpectedflood,
		packet.data[m2mMeshPacketOriginatorIndex],
		packet.data[m2mMeshPacketOriginatorIndex1],
		packet.data[m2mMeshPacketOriginatorIndex2],
		packet.data[m2mMeshPacketOriginatorIndex3],
		packet.data[m2mMeshPacketOriginatorIndex4],
		packet.data[m2mMeshPacketOriginatorIndex5],
		packet.data[m2mMeshPacketDestinationIndex],
		packet.data[m2mMeshPacketDestinationIndex1],
		packet.data[m2mMeshPacketDestinationIndex2],
		packet.data[m2mMeshPacketDestinationIndex3],
		packet.data[m2mMeshPacketDestinationIndex4],
		packet.data[19],
		packet.routerMacAddress[0],
		packet.routerMacAddress[1],
		packet.routerMacAddress[2],
		packet.routerMacAddress[3],
		packet.routerMacAddress[4],
		packet.routerMacAddress[5]
		);
	}*/
	#endif
	//Consider a packet for forwarding, it may already have been changed in earlier processing
	//The TTL must be >0 and either a flood or NOT have this node at its source or destination
	if(packet.data[m2mMeshPacketTTLIndex] > 0 &&
		(bool(packet.data[m2mMeshPacketTypeIndex] & SEND_TO_ALL_NODES) ||
		(not _isLocalMacAddress(&packet.data[m2mMeshPacketDestinationIndex])) ||
		(_packetType == TRACE_PACKET_TYPE && _isLocalMacAddress(&packet.data[m2mMeshPacketDestinationIndex]) == true))
		)
	{
		bool doTheForward = false;
		if(_packetType == ELP_PACKET_TYPE && (_serviceFlags & PROTOCOL_ELP_FORWARD))		//This is an ELP packet
		{
			doTheForward = true;
		}
		else if(_packetType == OGM_PACKET_TYPE && (_serviceFlags & PROTOCOL_OGM_FORWARD))	//This is an OGM packet
		{
			doTheForward = true;
		}
		else if(_packetType == NHS_PACKET_TYPE && (_serviceFlags & PROTOCOL_NHS_FORWARD))		//This is a NHS packet
		{
			doTheForward = true;
		}
		else if(_packetType == USR_PACKET_TYPE && (_serviceFlags & PROTOCOL_USR_FORWARD))	//This is a USR packet
		{
			doTheForward = true;
		}
		else if(_packetType == TRACE_PACKET_TYPE && (_serviceFlags & PROTOCOL_USR_FORWARD))	//This is a trace packet
		{
			doTheForward = true;
		}
		if(doTheForward == true)
		{
			packet.data[m2mMeshPacketTTLIndex]--;	//Reduce the TTL
			//packet.data[m2mMeshPacketTypeIndex] = packet.data[m2mMeshPacketTypeIndex] & ~PEERING_REQUEST;	//Remove the peering request flag so it cannot be forwarded
			if(_forwardingBuffer[_fowardingBufferWriteIndex].length == 0)
			{
				_forwardingBuffer[_fowardingBufferWriteIndex].originatorId = packet.originatorId;
				_forwardingBuffer[_fowardingBufferWriteIndex].destinationId = packet.destinationId;
				_forwardingBuffer[_fowardingBufferWriteIndex].routerId = MESH_ORIGINATOR_NOT_FOUND;
				_forwardingBuffer[_fowardingBufferWriteIndex].length = packet.length;
				memcpy(&_forwardingBuffer[_fowardingBufferWriteIndex].data, &packet.data, packet.length);
				_forwardingBuffer[_fowardingBufferWriteIndex].timestamp = packet.timestamp;
				
				#ifdef m2mMeshIncludeDebugFeatures
				if(_debugEnabled == true && (_loggingLevel & MESH_UI_LOG_BUFFER_MANAGEMENT))
				{
					_debugStream->printf_P(m2mMeshfillFWDbufferslotd,
						_applicationBufferWriteIndex,packet.length,
						packetTypeDescription[packet.data[m2mMeshPacketTypeIndex] & 0x07],
						packet.routerMacAddress[0],
						packet.routerMacAddress[1],
						packet.routerMacAddress[2],
						packet.routerMacAddress[3],
						packet.routerMacAddress[4],
						packet.routerMacAddress[5],
						_originator[packet.originatorId].macAddress[0],
						_originator[packet.originatorId].macAddress[1],
						_originator[packet.originatorId].macAddress[2],
						_originator[packet.originatorId].macAddress[3],
						_originator[packet.originatorId].macAddress[4],
						_originator[packet.originatorId].macAddress[5]);
				}
				#endif
				_fowardingBufferWriteIndex++;	//Advance the buffer index
				_fowardingBufferWriteIndex = _fowardingBufferWriteIndex%M2MMESHFORWARDINGBUFFERSIZE;	//Rollover buffer index
			}
			else
			{
				#ifdef m2mMeshIncludeDebugFeatures
				if(_debugEnabled == true && ((_loggingLevel & MESH_UI_LOG_BUFFER_MANAGEMENT) || (_loggingLevel & MESH_UI_LOG_ERRORS)))
				{
					_debugStream->print(m2mMeshFWDbufferfull);
				}
				#endif
				_droppedFwdPackets++;
			}
		}
	}
	packet.length = 0;	//Mark the buffer empty
}

bool ICACHE_FLASH_ATTR m2mMeshClass::_forwardPacket(m2mMeshPacketBuffer &packet)
{
	#ifdef m2mMeshIncludeDebugFeatures
	uint8_t _packetType = packet.data[m2mMeshPacketTypeIndex] & 0x07;
	bool logTheForward = false;
	#endif
	if(_routePacket(packet))
	{
		#ifdef m2mMeshIncludeDebugFeatures
		if(_debugEnabled == true)
		{
			if(_packetType == ELP_PACKET_TYPE)		//This is an ELP packet
			{
				if(_loggingLevel & MESH_UI_LOG_ELP_FORWARDING && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || packet.routerId == _nodeToLog || packet.originatorId == _nodeToLog))
				{
					logTheForward = true;
				}
			}
			else if(_packetType == OGM_PACKET_TYPE)	//This is an OGM packet
			{
				if(_loggingLevel & MESH_UI_LOG_OGM_FORWARDING && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || packet.routerId == _nodeToLog || packet.originatorId == _nodeToLog))
				{
					logTheForward = true;
				}
			}
			else if(_packetType == NHS_PACKET_TYPE)		//This is a NHS packet
			{
				if(_loggingLevel & MESH_UI_LOG_NHS_FORWARDING && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || packet.routerId == _nodeToLog || packet.originatorId == _nodeToLog))
				{
					logTheForward = true;
				}
			}
			else if(_packetType == USR_PACKET_TYPE || _packetType == TRACE_PACKET_TYPE)	//This is a USR packet
			{
				if(_loggingLevel & MESH_UI_LOG_USR_FORWARDING && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || packet.routerId == _nodeToLog || packet.originatorId == _nodeToLog))
				{
					logTheForward = true;
				}
			}
		}
		#endif
		bool sendResult = _sendPacket(packet);	//Send immediately without any complicated routing
		#ifdef m2mMeshIncludeDebugFeatures
		if(sendResult == true && logTheForward == true)
		{
			_debugStream->printf_P(m2mMeshsFWDR02x02x02x02x02x02xO02x02x02x02x02x02xTTLd,
				packetTypeDescription[packet.data[m2mMeshPacketTypeIndex] & 0x07],
				packet.routerMacAddress[0],
				packet.routerMacAddress[1],
				packet.routerMacAddress[2],
				packet.routerMacAddress[3],
				packet.routerMacAddress[4],
				packet.routerMacAddress[5],
				_originator[packet.originatorId].macAddress[0],
				_originator[packet.originatorId].macAddress[1],
				_originator[packet.originatorId].macAddress[2],
				_originator[packet.originatorId].macAddress[3],
				_originator[packet.originatorId].macAddress[4],
				_originator[packet.originatorId].macAddress[5],
				packet.data[m2mMeshPacketTTLIndex]);
		}
		else if(logTheForward == true)
		{
			_debugStream->printf_P(m2mMeshsFWDR02x02x02x02x02x02xO02x02x02x02x02x02xTTLdfailed,
				packetTypeDescription[packet.data[m2mMeshPacketTypeIndex] & 0x07],
				packet.routerMacAddress[0],
				packet.routerMacAddress[1],
				packet.routerMacAddress[2],
				packet.routerMacAddress[3],
				packet.routerMacAddress[4],
				packet.routerMacAddress[5],
				_originator[packet.originatorId].macAddress[0],
				_originator[packet.originatorId].macAddress[1],
				_originator[packet.originatorId].macAddress[2],
				_originator[packet.originatorId].macAddress[3],
				_originator[packet.originatorId].macAddress[4],
				_originator[packet.originatorId].macAddress[5],
				packet.data[m2mMeshPacketTTLIndex]);
		}
		#endif
		return(sendResult);
	}
	else
	{
		#ifdef m2mMeshIncludeDebugFeatures
		if(logTheForward == true)
		{
			_debugStream->printf_P(m2mMeshsFWDR02x02x02x02x02x02xO02x02x02x02x02x02xTTLdfailed,
				packetTypeDescription[packet.data[m2mMeshPacketTypeIndex] & 0x07],
				packet.routerMacAddress[0],
				packet.routerMacAddress[1],
				packet.routerMacAddress[2],
				packet.routerMacAddress[3],
				packet.routerMacAddress[4],
				packet.routerMacAddress[5],
				_originator[packet.originatorId].macAddress[0],
				_originator[packet.originatorId].macAddress[1],
				_originator[packet.originatorId].macAddress[2],
				_originator[packet.originatorId].macAddress[3],
				_originator[packet.originatorId].macAddress[4],
				_originator[packet.originatorId].macAddress[5],
				packet.data[m2mMeshPacketTTLIndex]);
		}
		#endif
		return(false);
	}
}
void ICACHE_FLASH_ATTR m2mMeshClass::rollOutTheWelcomeWagon()
{
	_lastSent[OGM_PACKET_TYPE] = millis() - (_currentInterval[OGM_PACKET_TYPE] - 5000);	//Send OGM in 5s
	if(_actingAsSyncServer)
	{
		_lastSent[NHS_PACKET_TYPE] = millis() - (_currentInterval[NHS_PACKET_TYPE] - 10000);	//Send NHS in 10s
	}
	#ifdef m2mMeshIncludeDebugFeatures
	if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NODE_MANAGEMENT)
	{
		_debugStream->print(m2mMeshshorteningtimerstoinductnewnode);
	}
	#endif
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


bool ICACHE_FLASH_ATTR m2mMeshClass::_sendElp(m2mMeshPacketBuffer &packet)
{
	packet.data[m2mMeshPacketTypeIndex] = ELP_PACKET_TYPE | ELP_DEFAULT_OPTIONS;//Add the ELP packet type
	packet.data[m2mMeshPacketChecksumIndex] = 0;									//Here goes the checksum
	packet.data[m2mMeshPacketTTLIndex] = _currentTtl[ELP_PACKET_TYPE];				//Add the TTL, usually 0 for ELP
	memcpy(&packet.data[m2mMeshPacketSNIndex], &_sequenceNumber, sizeof(_sequenceNumber));	//Add the sequence number
	memcpy(&packet.data[m2mMeshPacketOriginatorIndex], &_localMacAddress[0], 6);					//Add the source address
	memcpy(&packet.data[m2mMeshPacketIntervalIndex], &_currentInterval[ELP_PACKET_TYPE], sizeof(_currentInterval[ELP_PACKET_TYPE]));	//Add the interval
	packet.data[m2mMeshPacketFlagsIndex] = 0;		//Start with zero neighbours
	packet.length = 19;								//This will iterate through filling in the data if necessary
	if(_serviceFlags & PROTOCOL_ELP_INCLUDE_PEERS)	//If adding neighbours, iterate through adding them
	//if(includeNeighbours)							//If adding neighbours, iterate through adding them
	{
		//Only add active peers, but we need to iterate the whole list
		for(uint8_t originator = 0; originator < _numberOfOriginators; originator++)
		{
			if(_dataIsValid(originator, ELP_PACKET_TYPE) && packet.length < ESP_NOW_MAX_PACKET_SIZE - 7) //Don't overflow the packet
			{
				//This originator is an active neighbour so include it
				memcpy(&packet.data[packet.length], &_originator[originator].macAddress[0], 6);
				packet.length+=6;
				if(_originator[originator].isCurrentlyPeer == true)	//Inform neigbours if it's a peer or not
				{
					if(millis() - _originator[originator].peerNeeded > _peerLifetime)
					{
						packet.data[packet.length++]  = 0x03;
					}
					else
					{
						packet.data[packet.length++]  = 0x01;
					}
				}
				else
				{
					packet.data[packet.length++]  = 0x00;
				}
				packet.data[m2mMeshPacketFlagsIndex]++;				//Increment the number of neighbours
			}
		}
	}
	packet.data[m2mMeshPacketDataLengthIndex] = packet.length;
	packet.destinationId = MESH_ORIGINATOR_NOT_FOUND;	//Flood this
	packet.routerId = MESH_ORIGINATOR_NOT_FOUND;	//Flood this
	memcpy(&packet.routerMacAddress[0], &_broadcastMacAddress[0],6);	//Flood this
	bool sendResult = _sendPacket(packet);	//Send immediately without any complicated routing
	#ifdef m2mMeshIncludeDebugFeatures
	if(sendResult == true && _debugEnabled == true)
	{
		if(_loggingLevel & MESH_UI_LOG_ELP_SEND)
		{
			_debugStream->print(F("\r\nELP SEND "));
			//if(includeNeighbours)
			if(_serviceFlags & PROTOCOL_ELP_INCLUDE_PEERS)
			{
				_debugStream->printf_P(TTL02dFLG02xSEQ08xLENdNBRd,packet.data[m2mMeshPacketTTLIndex],packet.data[m2mMeshPacketTypeIndex] & 0x8f,_sequenceNumber,packet.length,_numberOfActiveNeighbours);
			}
			else
			{
				_debugStream->printf_P(m2mMeshTTL02dFLG02xSEQ08xLENd,packet.data[m2mMeshPacketTTLIndex],packet.data[m2mMeshPacketTypeIndex] & 0x8f,_sequenceNumber,packet.length);
			}
		}
		if(_loggingLevel & MESH_UI_LOG_ALL_SENT_PACKETS)
		{
			_debugStream->print(m2mMeshSent);
			_debugPacket(packet);
		}
	}
	else if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_ELP_SEND)
	{
		_debugStream->print(F("\r\nELP SEND fail"));
	}
	#endif
	_sequenceNumber++;
	return(sendResult);
}

void ICACHE_FLASH_ATTR m2mMeshClass::_processElp(m2mMeshPacketBuffer &packet)
{
	#ifdef m2mMeshIncludeDebugFeatures
	if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_ELP_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || packet.routerId == _nodeToLog || packet.originatorId == _nodeToLog))
	{
		if(packet.data[m2mMeshPacketFlagsIndex]>0)
		{
			_debugStream->printf_P(m2mMeshELPRCVR02x02x02x02x02x02xO02x02x02x02x02x02xTTLdNBRS02dLENd,packet.routerMacAddress[0],packet.routerMacAddress[1],packet.routerMacAddress[2],packet.routerMacAddress[3],packet.routerMacAddress[4],packet.routerMacAddress[5],_originator[packet.originatorId].macAddress[0],_originator[packet.originatorId].macAddress[2],_originator[packet.originatorId].macAddress[2],_originator[packet.originatorId].macAddress[3],_originator[packet.originatorId].macAddress[4],_originator[packet.originatorId].macAddress[5],packet.data[m2mMeshPacketTTLIndex],packet.data[m2mMeshPacketFlagsIndex],packet.length);
		}
		else
		{
			_debugStream->printf_P(m2mMeshELPRCVR02x02x02x02x02x02xO02x02x02x02x02x02xTTLdLENd,packet.routerMacAddress[0],packet.routerMacAddress[1],packet.routerMacAddress[2],packet.routerMacAddress[3],packet.routerMacAddress[4],packet.routerMacAddress[5],_originator[packet.originatorId].macAddress[0],_originator[packet.originatorId].macAddress[1],_originator[packet.originatorId].macAddress[2],_originator[packet.originatorId].macAddress[3],_originator[packet.originatorId].macAddress[4],_originator[packet.originatorId].macAddress[5],packet.data[m2mMeshPacketTTLIndex],packet.length);
		}
	}
	#endif
	if(packet.data[m2mMeshPacketFlagsIndex]>0)	//No further processing is necessary unless the ELP contains neighbours
	{
		_originator[packet.originatorId].numberOfPeers = 0;
		_originator[packet.originatorId].numberOfExpiredPeers = 0;
		for(uint8_t neighbour = 0; neighbour < packet.data[m2mMeshPacketFlagsIndex]; neighbour++)
		{
			if(_isLocalMacAddress(&packet.data[m2mMeshNeighbourIndex+neighbour*7]))	//Ignore this node
			{
				#ifdef m2mMeshIncludeDebugFeatures
				if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_ELP_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || packet.routerId == _nodeToLog || packet.originatorId == _nodeToLog)) //Print this neighbour
				{
					_debugStream->printf_P(m2mMeshELPneighbour02x02x02x02x02x02xthisnode,
					packet.data[m2mMeshNeighbourIndex+neighbour*7],
					packet.data[m2mMeshNeighbourIndex1+neighbour*7],
					packet.data[m2mMeshNeighbourIndex2+neighbour*7],
					packet.data[m2mMeshNeighbourIndex3+neighbour*7],
					packet.data[m2mMeshNeighbourIndex4+neighbour*7],
					packet.data[m2mMeshNeighbourIndex5+neighbour*7]);
				}
				#endif
				if(packet.data[m2mMeshNeighbourPeerInfoIndex+neighbour*7] & 0x01)
				{
					if(_originator[packet.originatorId].hasUsAsPeer == false)
					{
						#ifdef m2mMeshIncludeDebugFeatures
						if(_debugEnabled == true && (_loggingLevel & MESH_UI_LOG_PEER_MANAGEMENT))
						{
							_debugStream->printf("\r\nELP node %u added this node as peer",packet.originatorId);
						}
						#endif
						_originator[packet.originatorId].hasUsAsPeer = true;
					}
					_originator[packet.originatorId].numberOfPeers++;
					#ifdef m2mMeshIncludeDebugFeatures
					if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_ELP_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || packet.routerId == _nodeToLog || packet.originatorId == _nodeToLog)) //Print this neighbour
					{
						_debugStream->print(m2mMesh_peer);
					}
					#endif
					if(packet.data[m2mMeshNeighbourPeerInfoIndex+neighbour*7] & 0x02)
					{
						_originator[packet.originatorId].peeringExpired = true;
						_originator[packet.originatorId].numberOfExpiredPeers++;
						#ifdef m2mMeshIncludeDebugFeatures
						if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_ELP_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || packet.routerId == _nodeToLog || packet.originatorId == _nodeToLog))
						{
							_debugStream->print(m2mMesh_expired);
						}
						#endif
					}
				}
				else
				{
					if(_originator[packet.originatorId].hasUsAsPeer == true)
					{
						#ifdef m2mMeshIncludeDebugFeatures
						if(_debugEnabled == true && (_loggingLevel & MESH_UI_LOG_PEER_MANAGEMENT))
						{
							_debugStream->printf("\r\nELP node %u removed the peering with this node",packet.originatorId);
						}
						#endif
						_originator[packet.originatorId].hasUsAsPeer = false;
					}
					_originator[packet.originatorId].peeringExpired = false;
				}
			}
			else
			{
				//It is not this node, so look at adding if necessery
				uint8_t neighbourId = _originatorIdFromMac(&packet.data[19+neighbour*7]);
				if(neighbourId == MESH_ORIGINATOR_NOT_FOUND)
				{
					//This is a new originator
					#ifdef m2mMeshIncludeDebugFeatures
					if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_ELP_RECEIVED) //Print this neighbour and show it's new
					{
						_debugStream->printf_P(m2mMeshELPneighbour02x02x02x02x02x02xnewnode,
						packet.data[m2mMeshNeighbourIndex+neighbour*7],
						packet.data[m2mMeshNeighbourIndex1+neighbour*7],
						packet.data[m2mMeshNeighbourIndex2+neighbour*7],
						packet.data[m2mMeshNeighbourIndex3+neighbour*7],
						packet.data[m2mMeshNeighbourIndex4+neighbour*7],
						packet.data[m2mMeshNeighbourIndex5+neighbour*7]);
					}
					if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NODE_MANAGEMENT)
					{
						_debugStream->print(m2mMeshELP);
					}
					#endif
					neighbourId = _addOriginator(&packet.data[19+neighbour*7],_currentChannel);
				}
				#ifdef m2mMeshIncludeDebugFeatures
				else
				{
					//This is an existing originator
					if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_ELP_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || packet.routerId == _nodeToLog || packet.originatorId == _nodeToLog))
					{
						_debugStream->printf_P(m2mMeshELPneighbour02x02x02x02x02x02x,
						packet.data[m2mMeshNeighbourIndex+neighbour*7],
						packet.data[m2mMeshNeighbourIndex1+neighbour*7],
						packet.data[m2mMeshNeighbourIndex2+neighbour*7],
						packet.data[m2mMeshNeighbourIndex3+neighbour*7],
						packet.data[m2mMeshNeighbourIndex4+neighbour*7],
						packet.data[m2mMeshNeighbourIndex5+neighbour*7]);
					}
				}
				#endif
				if(packet.data[m2mMeshNeighbourPeerInfoIndex+neighbour*7] & 0x01)
				{
					_originator[packet.originatorId].numberOfPeers++;				//Record number of peers
					if(packet.data[m2mMeshNeighbourPeerInfoIndex+neighbour*7] & 0x02)
					{
						_originator[packet.originatorId].numberOfExpiredPeers++;	//Record number of expired peers
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

bool ICACHE_FLASH_ATTR m2mMeshClass::_sendOgm(m2mMeshPacketBuffer &packet)
{
	uint16_t tQ = 65535;
	packet.data[m2mMeshPacketTypeIndex] = OGM_PACKET_TYPE | OGM_DEFAULT_OPTIONS;					//Add the OGM packet type
	packet.data[m2mMeshPacketChecksumIndex] = 0;														//Here goes the checksum
	packet.data[m2mMeshPacketTTLIndex] = _currentTtl[OGM_PACKET_TYPE];							//Add the OGM TTL
	memcpy(&packet.data[m2mMeshPacketSNIndex], &_sequenceNumber, sizeof(_sequenceNumber));
	packet.data[m2mMeshPacketOriginatorIndex] = _localMacAddress[0];
	packet.data[m2mMeshPacketOriginatorIndex1] = _localMacAddress[1];
	packet.data[m2mMeshPacketOriginatorIndex2] = _localMacAddress[2];
	packet.data[m2mMeshPacketOriginatorIndex3] = _localMacAddress[3];
	packet.data[m2mMeshPacketOriginatorIndex4] = _localMacAddress[4];
	packet.data[m2mMeshPacketOriginatorIndex5] = _localMacAddress[5];
	memcpy(&packet.data[m2mMeshPacketIntervalIndex], &_currentInterval[OGM_PACKET_TYPE], sizeof(_currentInterval[OGM_PACKET_TYPE]));
	packet.data[m2mMeshPacketFlagsIndex] = 0;
	memcpy(&packet.data[m2mMeshPacketTqIndex], &tQ, sizeof(tQ));
	packet.length = 20;
	packet.data[m2mMeshPacketDataLengthIndex] = packet.length;
	packet.destinationId = MESH_ORIGINATOR_NOT_FOUND;	//Flood this
	packet.routerId = MESH_ORIGINATOR_NOT_FOUND;	//Flood this
	memcpy(&packet.routerMacAddress[0], &_broadcastMacAddress[0],6);	//Nothing needs doing beyond adding broadcast address
	bool sendResult = _sendPacket(packet);	//Send immediately without any complicated routing
	#ifdef m2mMeshIncludeDebugFeatures
	if(sendResult == true && _debugEnabled == true)
	{
		if(_loggingLevel & MESH_UI_LOG_OGM_SEND)
		{
			_debugStream->printf_P(m2mMeshOGMSENDR02x02x02x02x02x02xO02x02x02x02x02x02xTTL02dFLG02xSEQ08xLENd, packet.routerMacAddress[0], packet.routerMacAddress[1], packet.routerMacAddress[2], packet.routerMacAddress[3], packet.routerMacAddress[4], packet.routerMacAddress[5], _localMacAddress[0], _localMacAddress[1], _localMacAddress[2], _localMacAddress[3], _localMacAddress[4], _localMacAddress[5],packet.data[m2mMeshPacketTTLIndex],packet.data[m2mMeshPacketTypeIndex] & 0x8f,_sequenceNumber,packet.length);
		}
		if(_loggingLevel & MESH_UI_LOG_ALL_SENT_PACKETS)
		{
			_debugStream->println();
			_debugStream->print(m2mMeshSent);
			_debugPacket(packet);
		}
	}
	else if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_OGM_SEND)
	{
		_debugStream->printf_P(m2mMeshOGMSENDfailedTTL02dFlags02xSeq08xLENd,packet.data[m2mMeshPacketTTLIndex],packet.data[m2mMeshPacketTypeIndex] & 0x8f,_sequenceNumber,packet.length);
	}
	#endif
	_lastOGMSequenceNumber = _sequenceNumber;	//Used to check for incoming echoes
	_sequenceNumber++;
	return(sendResult);
}

void ICACHE_FLASH_ATTR m2mMeshClass::_processOgm(m2mMeshPacketBuffer &packet)
{
	#ifdef m2mMeshIncludeDebugFeatures
	if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_OGM_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || packet.routerId == _nodeToLog || packet.originatorId == _nodeToLog))
	{
		_debugStream->printf_P(m2mMeshOGMRECVR02x02x02x02x02x02xO02x02x02x02x02x02xTTLdHOPdLENd,packet.routerMacAddress[0],packet.routerMacAddress[1],packet.routerMacAddress[2],packet.routerMacAddress[3],packet.routerMacAddress[4],packet.routerMacAddress[5],_originator[packet.originatorId].macAddress[0],_originator[packet.originatorId].macAddress[1],_originator[packet.originatorId].macAddress[2],_originator[packet.originatorId].macAddress[3],_originator[packet.originatorId].macAddress[4],_originator[packet.originatorId].macAddress[5],packet.data[m2mMeshPacketTTLIndex],packet.data[20],packet.length);
	}
	#endif
	_originator[packet.originatorId].ogmReceived = _originator[packet.originatorId].ogmReceived >> 1;	//Shift right the OGM count
	_originator[packet.originatorId].ogmReceived = _originator[packet.originatorId].ogmReceived | 0xf000;	//Add a mark on the left of OGM count
	_calculateLtq(packet.originatorId);	//Recalculate the Local Transmission quality
	uint16_t tq;
	memcpy(&tq, &packet.data[m2mMeshPacketTqIndex], sizeof(tq));	//Retrieve the transmission quality in the packet
	if(packet.routerId != packet.originatorId)
	{
		uint16_t currentGtq = _originator[packet.originatorId].gtq;		//Store current TQ for later comparison
		//Compare routerId with originatorID to see if this is direct or forwarded
		if(packet.routerId != _originator[packet.originatorId].selectedRouter)
		{
			if(tq > currentGtq)
			{
				//This is a better indirect route than we already have
				_originator[packet.originatorId].selectedRouter = packet.routerId;
				_originator[packet.originatorId].gtq = tq;
				//_meshHasBecomeUnstable();
				#ifdef m2mMeshIncludeDebugFeatures
				if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_OGM_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || packet.routerId == _nodeToLog || packet.originatorId == _nodeToLog))
				{
					_debugStream->printf_P(m2mMeshOGMR02x02x02x02x02x02xselectedforO02x02x02x02x02x02xTQ04x,
					packet.routerMacAddress[0],
					packet.routerMacAddress[1],
					packet.routerMacAddress[2],
					packet.routerMacAddress[3],
					packet.routerMacAddress[4],
					packet.routerMacAddress[5],
					packet.data[m2mMeshPacketOriginatorIndex],
					packet.data[m2mMeshPacketOriginatorIndex1],
					packet.data[m2mMeshPacketOriginatorIndex2],
					packet.data[m2mMeshPacketOriginatorIndex3],
					packet.data[m2mMeshPacketOriginatorIndex4],
					packet.data[m2mMeshPacketOriginatorIndex5],
					tq);
				}
				#endif
			}
			#ifdef m2mMeshIncludeDebugFeatures
			else
			{
				//This does not change the routing
				if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_OGM_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || packet.routerId == _nodeToLog || packet.originatorId == _nodeToLog))
				{
					_debugStream->printf_P(m2mMeshOGMR02x02x02x02x02x02xforO02x02x02x02x02x02xinferiorTQ04x,
					packet.routerMacAddress[0],
					packet.routerMacAddress[1],
					packet.routerMacAddress[2],
					packet.routerMacAddress[3],
					packet.routerMacAddress[4],
					packet.routerMacAddress[5],
					packet.data[m2mMeshPacketOriginatorIndex],
					packet.data[m2mMeshPacketOriginatorIndex1],
					packet.data[m2mMeshPacketOriginatorIndex2],
					packet.data[m2mMeshPacketOriginatorIndex3],
					packet.data[m2mMeshPacketOriginatorIndex4],
					packet.data[m2mMeshPacketOriginatorIndex5],
					tq);
				}
			}
			#endif
		}
		else if(packet.routerId == _originator[packet.originatorId].selectedRouter)
		{
			if(tq != currentGtq)
			{
				//Update the TQ of the existing route
				_originator[packet.originatorId].gtq = tq;
				//This does not change the routing
				#ifdef m2mMeshIncludeDebugFeatures
				if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_OGM_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || packet.routerId == _nodeToLog || packet.originatorId == _nodeToLog))
				{
					_debugStream->printf_P(m2mMeshOGMR02x02x02x02x02x02xforO02x02x02x02x02x02xupdateTQ04x,
					packet.routerMacAddress[0],
					packet.routerMacAddress[1],
					packet.routerMacAddress[2],
					packet.routerMacAddress[3],
					packet.routerMacAddress[4],
					packet.routerMacAddress[5],
					packet.data[m2mMeshPacketOriginatorIndex],
					packet.data[m2mMeshPacketOriginatorIndex1],
					packet.data[m2mMeshPacketOriginatorIndex2],
					packet.data[m2mMeshPacketOriginatorIndex3],
					packet.data[m2mMeshPacketOriginatorIndex4],
					packet.data[m2mMeshPacketOriginatorIndex5],
					tq);
				}
				#endif
			}
			#ifdef m2mMeshIncludeDebugFeatures
			//This does not change the routing
			else if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_OGM_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || packet.routerId == _nodeToLog || packet.originatorId == _nodeToLog))
			{
				_debugStream->printf_P(m2mMeshOGMR02x02x02x02x02x02xforO02x02x02x02x02x02xTQ04x,packet.data[m2mMeshPacketTTLIndex],packet.data[m2mMeshPacketTypeIndex] & 0x8f,
				packet.routerMacAddress[0],
				packet.routerMacAddress[1],
				packet.routerMacAddress[2],
				packet.routerMacAddress[3],
				packet.routerMacAddress[4],
				packet.routerMacAddress[5],
				packet.data[m2mMeshPacketOriginatorIndex],
				packet.data[m2mMeshPacketOriginatorIndex1],
				packet.data[m2mMeshPacketOriginatorIndex2],
				packet.data[m2mMeshPacketOriginatorIndex3],
				packet.data[m2mMeshPacketOriginatorIndex4],
				packet.data[m2mMeshPacketOriginatorIndex5],
				tq);
			}
			#endif
		}
		//Update the stats on reachable nodes
		if(currentGtq == 0 && _originator[packet.originatorId].gtq > 0)
		{
			_originatorHasBecomeRoutable(packet.originatorId);
		}
		else if(currentGtq > 0 && _originator[packet.originatorId].gtq == 0)
		{
			_originatorHasBecomeUnroutable(packet.originatorId);
		}
	}
	#ifdef m2mMeshIncludeDebugFeatures
	if(_debugEnabled == true &&
		_loggingLevel & MESH_UI_LOG_OGM_RECEIVED &&
		packet.data[m2mMeshPacketFlagsIndex]>0 &&
		(_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || packet.routerId == _nodeToLog || packet.originatorId == _nodeToLog))
	{
		for(uint8_t id = 0; id<packet.data[m2mMeshPacketFlagsIndex]; id++)
		{
			_debugStream->printf_P(m2mMeshOGMforwardingchain02x02x02x02x02x02x,
			packet.data[m2mMeshOGMforwardingChainIndex+id*6],
			packet.data[m2mMeshOGMforwardingChainIndex1+id*6],
			packet.data[m2mMeshOGMforwardingChainIndex2+id*6],
			packet.data[m2mMeshOGMforwardingChainIndex3+id*6],
			packet.data[m2mMeshOGMforwardingChainIndex4+id*6],
			packet.data[m2mMeshOGMforwardingChainIndex5+id*6]);
		}
	}
	#endif
	//Consider the packet for forwarding, whether or not it was selected as router or not
	if(packet.data[m2mMeshPacketTTLIndex] > 0)
	{
		//If our route back to the originator is worse than the GTQ in the packet, reduce GTQ to the LTQ value
		if(_originator[packet.routerId].ltq < tq)
		{
			tq = _originator[packet.routerId].ltq;
		}
		//Impose an arbitrary hop penalty on GTQ and only forward if GTQ is still > 0
		if(tq > OGM_HOP_PENALTY)
		{
			tq = tq - OGM_HOP_PENALTY;
			memcpy(&packet.data[m2mMeshPacketTqIndex], &tq, sizeof(tq));	//Update GTQ in the packet
		}
		else if(packet.routerId != packet.originatorId)
		{
			//Reduce TTL to 0 on low TQ packets so they are NOT forwarded beyond first hop
			//First hop MUST be allowed so TQ can be calculated from zero
			packet.data[m2mMeshPacketTTLIndex] = 0;
		}
		if(packet.data[m2mMeshPacketTTLIndex] > 0)	//Packet will be forwarded despite penalties
		{
			#ifdef m2mMeshIncludeDebugFeatures
			if(_debugEnabled == true &&
				_loggingLevel & MESH_UI_LOG_OGM_FORWARDING &&
				(_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || packet.routerId == _nodeToLog || packet.originatorId == _nodeToLog))
			{
				_debugStream->printf_P(m2mMesh_hoppenaltyappliedTQnow02x, tq);
			}
			#endif
			if(packet.data[m2mMeshPacketDataLengthIndex] < ESP_NOW_MAX_PACKET_SIZE - 6)	//Add in this node's address to the forwarding chain if there's space.
			{
				memcpy(&packet.data[packet.data[m2mMeshPacketDataLengthIndex]], _localMacAddress, 6);			
				packet.data[m2mMeshPacketFlagsIndex]++;
				if(m2mMeshOGMforwardingChainIndex+packet.data[m2mMeshPacketFlagsIndex]*6>packet.length)
				{
					packet.length=m2mMeshOGMforwardingChainIndex+packet.data[m2mMeshPacketFlagsIndex]*6;
				}
				packet.data[m2mMeshPacketDataLengthIndex] = packet.data[m2mMeshPacketDataLengthIndex] + 6; //Increase the 'data length' field
				#ifdef m2mMeshIncludeDebugFeatures
				if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_OGM_FORWARDING && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || packet.routerId == _nodeToLog || packet.originatorId == _nodeToLog))
				{
					_debugStream->printf_P(m2mMesh_addedthisnodetoforwardingchain,
					_localMacAddress[0],
					_localMacAddress[1],
					_localMacAddress[2],
					_localMacAddress[3],
					_localMacAddress[4],
					_localMacAddress[5]);
				}
				#endif
			}
		}
	}
}

void ICACHE_FLASH_ATTR m2mMeshClass::_meshHasBecomeStable()
{
	if(_stable == false)	//The number of nodes and checksum is consistent
	{
		_meshLastChanged = millis();
		//_forwardingDelay = _localMacAddress[5] * 4;	//Delay up to 1020ms to reduce flood collisions and prioritise reliability
		_forwardingDelay = _localMacAddress[5] >> 4;			//Delay up to 255ms to reduce flood collisions and prioritise reliability
		_stable = true;
		if(eventCallback)
		{
			eventCallback(meshEvent::stable);
		}
	}
}
void ICACHE_FLASH_ATTR m2mMeshClass::_meshHasBecomeUnstable()
{
	if(_stable == true)
	{
		_meshLastChanged = millis();
		_forwardingDelay = _localMacAddress[5] >> 6;	//Reduce forwarding delay to stop buffer filling and prioritise fast discover
		_stable = false;
		_currentInterval[ELP_PACKET_TYPE] = ELP_FAST_INTERVAL;
		_currentInterval[OGM_PACKET_TYPE] = OGM_FAST_INTERVAL;
		_currentInterval[NHS_PACKET_TYPE] = NHS_FAST_INTERVAL;
		if(eventCallback)
		{
			eventCallback(meshEvent::changing);
		}
	}
}

void ICACHE_FLASH_ATTR m2mMeshClass::_originatorHasBecomeRoutable(const uint8_t originatorId)
{
	//Originator has become reachable
	_numberOfReachableOriginators++;
	_meshHasBecomeUnstable();
	#ifdef m2mMeshIncludeDebugFeatures
	if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_PACKET_ROUTING && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || originatorId == _nodeToLog))
	{
		_debugStream->printf_P(m2mMeshOGM02x02x02x02x02x02xhasbecomereachable,_originator[originatorId].macAddress[0],_originator[originatorId].macAddress[1],_originator[originatorId].macAddress[2],_originator[originatorId].macAddress[3],_originator[originatorId].macAddress[4],_originator[originatorId].macAddress[5]);
	}
	#endif
}

void ICACHE_FLASH_ATTR m2mMeshClass::_originatorHasBecomeUnroutable(uint8_t originatorId)
{
	//Peer has become unreachable
	if(_numberOfReachableOriginators>0)
	{
		_numberOfReachableOriginators--;
	}
	_originator[originatorId].gtq = 0;
	_originator[originatorId].selectedRouter = MESH_ORIGINATOR_NOT_FOUND;
	_meshHasBecomeUnstable();
	#ifdef m2mMeshIncludeDebugFeatures
	if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_PACKET_ROUTING && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || originatorId == _nodeToLog))
	{
		_debugStream->printf_P(m2mMeshOGM02x02x02x02x02x02xhasbecomeunreachable,_originator[originatorId].macAddress[0],_originator[originatorId].macAddress[1],_originator[originatorId].macAddress[2],_originator[originatorId].macAddress[3],_originator[originatorId].macAddress[4],_originator[originatorId].macAddress[5]);
	}
	#endif
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

bool ICACHE_FLASH_ATTR m2mMeshClass::_sendNhs(m2mMeshPacketBuffer &packet)
{
	packet.data[m2mMeshPacketTypeIndex] = NHS_PACKET_TYPE | NHS_DEFAULT_OPTIONS;				//Add the NHS packet type
	packet.data[m2mMeshPacketChecksumIndex] = 0;												//Here goes the checksum
	packet.data[m2mMeshPacketTTLIndex] = _currentTtl[NHS_PACKET_TYPE];							//Add the NHS TTL
	memcpy(&packet.data[m2mMeshPacketSNIndex], &_sequenceNumber, sizeof(_sequenceNumber));		//Sequence number
	packet.data[m2mMeshPacketOriginatorIndex] =  _localMacAddress[0];							//Origin address
	packet.data[m2mMeshPacketOriginatorIndex1] = _localMacAddress[1];
	packet.data[m2mMeshPacketOriginatorIndex2] = _localMacAddress[2];
	packet.data[m2mMeshPacketOriginatorIndex3] = _localMacAddress[3];
	packet.data[m2mMeshPacketOriginatorIndex4] = _localMacAddress[4];
	packet.data[m2mMeshPacketOriginatorIndex5] = _localMacAddress[5];
	memcpy(&packet.data[m2mMeshPacketIntervalIndex], &_currentInterval[NHS_PACKET_TYPE], sizeof(_currentInterval[NHS_PACKET_TYPE]));
	#if defined(ESP8266)
	packet.data[m2mMeshPacketFlagsIndex] = PROCESSOR_ESP8266;
	#elif defined(ESP32)
	packet.data[m2mMeshPacketFlagsIndex] = PROCESSOR_ESP32;
	#endif
	uint32_t now = millis();
	memcpy(&packet.data[m2mMeshUptimeIndex], &now, sizeof(now));									//Add the current millis() for 'uptime'
	uint32_t freeHeap = uint32_t(ESP.getFreeHeap());												//Add the current free heap
	memcpy(&packet.data[m2mMeshFreeHeapIndex], &freeHeap, sizeof(freeHeap));
	memcpy(&packet.data[m2mMeshInitialFreeHeapIndex], &_initialFreeHeap, sizeof(_initialFreeHeap));	//Add the initial free heap
	#if defined(ESP8266)
	uint32_t maxFreeBlockSize = uint32_t(ESP.getMaxFreeBlockSize());								//Add the max free block size
	#elif defined(ESP32)
	uint32_t maxFreeBlockSize = 0;
	#endif
	memcpy(&packet.data[m2mMeshMaxFreeBlockIndex], &maxFreeBlockSize, sizeof(maxFreeBlockSize));
	#if defined(ESP8266)
	packet.data[m2mMeshHeapFragmentationIndex] = ESP.getHeapFragmentation();							//Add the heap fragmentation
	#elif defined(ESP32)
	packet.data[m2mMeshHeapFragmentationIndex] = 0;
	#endif
	memcpy(&packet.data[m2mMeshRxPacketsIndex], &_rxPackets, sizeof(_rxPackets)); //Add the RX packets
	memcpy(&packet.data[m2mMeshDroppedRxPacketsIndex], &_droppedRxPackets, sizeof(_droppedRxPackets));	//Add the dropped RX packets
	memcpy(&packet.data[m2mMeshTxPacketsIndex], &_txPackets, sizeof(_txPackets));	//Add the TX packets
	memcpy(&packet.data[m2mMeshDroppedTxPacketsIndex], &_droppedTxPackets, sizeof(_droppedTxPackets));//Add the dropped TX packets
	//memcpy(&packet.data[51], &__fwdPackets, sizeof(_fwdPackets));//Add the forwarded packets
	//memcpy(&packet.data[55], &_droppedFwdPackets, sizeof(_droppedFwdPackets));//Add the dropped forwarding packets
	packet.data[m2mMeshActiveNeighboursIndex] = _numberOfActiveNeighbours;		//Add the number of activeNeighbours
	packet.data[m2mMeshNumberOfOriginatorsIndex] = _numberOfOriginators;				//Add the number of originators
	packet.data[m2mMeshMeshFingerprintIndex] = _meshMacAddress[0];				//Add the mesh MAC address
	packet.data[m2mMeshMeshFingerprintIndex1] = _meshMacAddress[1];				//Add the mesh MAC address
	packet.data[m2mMeshMeshFingerprintIndex2] = _meshMacAddress[2];				//Add the mesh MAC address
	packet.data[m2mMeshMeshFingerprintIndex3] = _meshMacAddress[3];				//Add the mesh MAC address
	packet.data[m2mMeshMeshFingerprintIndex4] = _meshMacAddress[4];				//Add the mesh MAC address
	packet.data[m2mMeshMeshFingerprintIndex5] = _meshMacAddress[5];				//Add the mesh MAC address
	packet.data[m2mMeshMeshTxPowerIndex] = uint8_t(_currentTxPower*4);			//Add the TxPower
	packet.length = m2mMeshMeshTxPowerIndex + 1;								//Use this to build the rest of the packet
	if((_serviceFlags & PROTOCOL_NHS_TIME_SERVER) && _actingAsSyncServer && _stable)	//Add the mesh time
	{
		packet.data[m2mMeshPacketFlagsIndex] = packet.data[m2mMeshPacketFlagsIndex] | NHS_FLAGS_SYNCSERVER;
		uint32_t temp = syncedMillis();
		memcpy(&packet.data[packet.length], &temp, sizeof(temp));
		packet.length+=sizeof(temp);
	}
	#if defined (m2mMeshIncludeRTCFeatures)
	if((_serviceFlags & PROTOCOL_NHS_TIME_SERVER) == PROTOCOL_NHS_TIME_SERVER && rtc == true && timezone != nullptr && _meshSyncNegotiated==true && rtcValid())				//Add the RTC info, if valid
	{
		packet.data[m2mMeshPacketFlagsIndex] = packet.data[m2mMeshPacketFlagsIndex] | NHS_FLAGS_RTCSERVER;
		//Timezone
		packet.data[packet.length++] = strlen(timezone);
		memcpy(&packet.data[packet.length], &timezone, strlen(timezone));		//Copy in the timezone
		packet.length += strlen(timezone);										//Advance the index
		//Epoch offset
		time_t epoch;
		time(&epoch);
		uint32_t epochOffset = epoch - ((millis() + _meshSyncOffset)/1000); //Mesh time is in ms so divide by 1000
		memcpy(&packet.data[packet.length], &epochOffset, sizeof(epochOffset));
		packet.length+=sizeof(epochOffset);
	}
	#endif
	if(_nodeName != nullptr)													//Insert the node name, if set
	{
		packet.data[m2mMeshPacketFlagsIndex] = packet.data[m2mMeshPacketFlagsIndex] | NHS_FLAGS_NODE_NAME_SET;				//Mark this in the flags
		packet.data[packet.length++] = strlen(_nodeName);						//Include the length
		memcpy(&packet.data[packet.length], _nodeName, strlen(_nodeName));		//Copy in the node name
		packet.length += strlen(_nodeName);										//Advance the index
	}
	uint8_t originatorCountIndex = packet.length++;									//Store the count of included originators here
	if((_serviceFlags & PROTOCOL_NHS_INCLUDE_ORIGINATORS) && _numberOfOriginators>0)	//This packet will include all the nodes in the mesh
	{
		packet.data[m2mMeshPacketFlagsIndex] = packet.data[m2mMeshPacketFlagsIndex] | NHS_FLAGS_INCLUDES_ORIGINATORS;
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
			memcpy(&packet.data[packet.length], &_originator[originatorToSend].ltq, sizeof(_originator[originatorToSend].ltq));
			packet.length+=sizeof(_originator[originatorToSend].ltq);
			packet.data[originatorCountIndex]++;
			/*if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NHS_SEND)
			{
				_debugStream->printf_P(m2mMeshNHSincluded02x02x02x02x02x02xTQ02x,_originator[originatorToSend].macAddress[0],_originator[originatorToSend].macAddress[1],_originator[originatorToSend].macAddress[2],_originator[originatorToSend].macAddress[3],_originator[originatorToSend].macAddress[4],_originator[originatorToSend].macAddress[5],_originator[originatorToSend].ltq);
			}*/
		}
		/*if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NHS_SEND)
		{
			_debugStream->printf_P(m2mMeshNHSincludeddoriginators,packet.data[originatorCountIndex]);
		}*/
	}
	packet.data[m2mMeshPacketDataLengthIndex] = packet.length;
	memcpy(&packet.routerMacAddress[0], &_broadcastMacAddress[0], 6);	//Set the destination MAC address
	#ifdef m2mMeshIncludeDebugFeatures
	if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NHS_SEND)
	{
		_debugStream->print(m2mMeshNHSSND);
	}
	#endif
	packet.destinationId = MESH_ORIGINATOR_NOT_FOUND;	//Flood this
	packet.routerId = MESH_ORIGINATOR_NOT_FOUND;	//Flood this
	memcpy(&packet.routerMacAddress[0], &_broadcastMacAddress[0],6);	//Nothing needs doing beyond adding broadcast address
	bool sendResult = _sendPacket(packet);
	#ifdef m2mMeshIncludeDebugFeatures
	if(sendResult == true && _debugEnabled == true)
	{
		if(_loggingLevel & MESH_UI_LOG_NHS_SEND)
		{
			_debugStream->printf_P(TTL02dFLG02xSEQ08xLENd,packet.data[m2mMeshPacketTTLIndex],packet.data[m2mMeshPacketTypeIndex] & 0x8f,_sequenceNumber, packet.length);
			if(_actingAsSyncServer)
			{
				char uptime[10];
				_friendlyUptime(syncedMillis(),uptime);
				_debugStream->printf_P(m2mMeshTIMEs,uptime);
			}
			if((_serviceFlags & PROTOCOL_NHS_INCLUDE_ORIGINATORS) && packet.data[originatorCountIndex]>0)
			{
				_debugStream->printf_P(ORGd,packet.data[originatorCountIndex]);
			}
		}
		if(_loggingLevel & MESH_UI_LOG_ALL_SENT_PACKETS)
		{
			_debugStream->print(m2mMeshSent);
			_debugPacket(packet);
		}
	}
	else if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NHS_SEND)
	{
		_debugStream->print(m2mMeshfailed);
	}
	#endif
	_sequenceNumber++;
	return(sendResult);
}

void ICACHE_FLASH_ATTR m2mMeshClass::_processNhs(m2mMeshPacketBuffer &packet)
{
	#ifdef m2mMeshIncludeDebugFeatures
	if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NHS_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || packet.routerId == _nodeToLog || packet.originatorId == _nodeToLog))
	{
		_debugStream->printf_P(m2mMeshNHSR02x02x02x02x02x02xO02x02x02x02x02x02xTTLdLengthd,packet.routerMacAddress[0],packet.routerMacAddress[1],packet.routerMacAddress[2],packet.routerMacAddress[3],packet.routerMacAddress[4],packet.routerMacAddress[5],_originator[packet.originatorId].macAddress[0],_originator[packet.originatorId].macAddress[1],_originator[packet.originatorId].macAddress[2],_originator[packet.originatorId].macAddress[3],_originator[packet.originatorId].macAddress[4],_originator[packet.originatorId].macAddress[5],packet.data[m2mMeshPacketTTLIndex],packet.length);
	}
	#endif
	if(packet.originatorId != MESH_ORIGINATOR_NOT_FOUND)	//Update details about the originator
	{
		_originator[packet.originatorId].flags = packet.data[m2mMeshPacketFlagsIndex];	//Extract the flags
		memcpy(&_originator[packet.originatorId].uptime, &packet.data[m2mMeshUptimeIndex], sizeof(_originator[packet.originatorId].uptime));	//Extract the uptime
		#ifdef m2mMeshIncludeDebugFeatures
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NHS_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || packet.routerId == _nodeToLog || packet.originatorId == _nodeToLog))
		{
			_debugStream->printf_P(m2mMeshNHSUptimedms,_originator[packet.originatorId].uptime);
		}
		#endif
		memcpy(&_originator[packet.originatorId].currentFreeHeap, &packet.data[m2mMeshFreeHeapIndex], sizeof(_originator[packet.originatorId].currentFreeHeap));
		memcpy(&_originator[packet.originatorId].initialFreeHeap, &packet.data[m2mMeshInitialFreeHeapIndex], sizeof(_originator[packet.originatorId].initialFreeHeap));
		#ifdef m2mMeshIncludeDebugFeatures
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NHS_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || packet.routerId == _nodeToLog || packet.originatorId == _nodeToLog))
		{
			_debugStream->printf_P(m2mMeshNHSCurrentFreeHeapdd,_originator[packet.originatorId].currentFreeHeap,_originator[packet.originatorId].initialFreeHeap);
		}
		#endif
		memcpy(&_originator[packet.originatorId].largestFreeBlock, &packet.data[m2mMeshMaxFreeBlockIndex], sizeof(_originator[packet.originatorId].largestFreeBlock));
		_originator[packet.originatorId].heapFragmentation = packet.data[m2mMeshHeapFragmentationIndex];
		memcpy(&_originator[packet.originatorId].rxPackets, &packet.data[m2mMeshRxPacketsIndex], sizeof(_originator[packet.originatorId].rxPackets));
		memcpy(&_originator[packet.originatorId].droppedRxPackets, &packet.data[m2mMeshDroppedRxPacketsIndex], sizeof(_originator[packet.originatorId].droppedRxPackets));
		memcpy(&_originator[packet.originatorId].txPackets, &packet.data[m2mMeshTxPacketsIndex], sizeof(_originator[packet.originatorId].txPackets));
		memcpy(&_originator[packet.originatorId].droppedTxPackets, &packet.data[m2mMeshDroppedTxPacketsIndex], sizeof(_originator[packet.originatorId].droppedTxPackets));
		#ifdef m2mMeshIncludeDebugFeatures
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NHS_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || packet.routerId == _nodeToLog || packet.originatorId == _nodeToLog))
		{
			_debugStream->printf_P(m2mMeshNHSdroppedpacketsddRXddTX,_originator[packet.originatorId].droppedRxPackets,_originator[packet.originatorId].rxPackets,_originator[packet.originatorId].txPackets,_originator[packet.originatorId].droppedTxPackets);
		}
		#endif
		//Extract the number of active neighbours
		_originator[packet.originatorId].numberOfActiveNeighbours = packet.data[m2mMeshActiveNeighboursIndex];
		//Extract the number of originators
		_originator[packet.originatorId].numberOfOriginators = packet.data[m2mMeshNumberOfOriginatorsIndex];
		#ifdef m2mMeshIncludeDebugFeatures
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NHS_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || packet.routerId == _nodeToLog || packet.originatorId == _nodeToLog))
		{
			_debugStream->printf_P(m2mMeshNHSActiveneighboursddmeshMAC02x02x02x02x02x02x,_originator[packet.originatorId].numberOfActiveNeighbours,_originator[packet.originatorId].numberOfOriginators,packet.data[m2mMeshMeshFingerprintIndex],packet.data[m2mMeshMeshFingerprintIndex1],packet.data[m2mMeshMeshFingerprintIndex2],packet.data[m2mMeshMeshFingerprintIndex3],packet.data[m2mMeshMeshFingerprintIndex4],packet.data[m2mMeshMeshFingerprintIndex5]);
		}
		#endif
		//Check the consistency of the network
		if(_originator[packet.originatorId].numberOfOriginators != _numberOfOriginators)
		{
			_meshHasBecomeUnstable();
			#ifdef m2mMeshIncludeDebugFeatures
			if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NHS_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || packet.routerId == _nodeToLog || packet.originatorId == _nodeToLog))
			{
				_debugStream->print(m2mMeshdiffers);
			}
			#endif
		}
		else if(packet.data[m2mMeshMeshFingerprintIndex] != _meshMacAddress[0] | packet.data[m2mMeshMeshFingerprintIndex1] != _meshMacAddress[1] | packet.data[m2mMeshMeshFingerprintIndex2] != _meshMacAddress[2] | packet.data[m2mMeshMeshFingerprintIndex3] != _meshMacAddress[3] | packet.data[m2mMeshMeshFingerprintIndex4] != _meshMacAddress[4] | packet.data[m2mMeshMeshFingerprintIndex5] != _meshMacAddress[5])
		{
			_meshHasBecomeUnstable();
			#ifdef m2mMeshIncludeDebugFeatures
			if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NHS_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || packet.routerId == _nodeToLog || packet.originatorId == _nodeToLog))
			{
				_debugStream->print(m2mMeshdiffers);
			}
			#endif
		}
		else
		{
			_meshHasBecomeStable();
		}
		//Extract the Tx power
		_originator[packet.originatorId].currentTxPower = float(packet.data[m2mMeshMeshTxPowerIndex])/4;
		#ifdef m2mMeshIncludeDebugFeatures
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NHS_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || packet.routerId == _nodeToLog || packet.originatorId == _nodeToLog))
		{
			_debugStream->printf_P(m2mMeshNHSCurrentTXpowerf,4*_originator[packet.originatorId].currentTxPower);
		}
		#endif
		uint8_t receivedPacketIndex = m2mMeshMeshTxPowerIndex + 1;			//Use this to index through the rest of the packet
		if(packet.data[m2mMeshPacketFlagsIndex] & NHS_FLAGS_SYNCSERVER)		//Check for included time
		{
			//The NHS packet is flagged as being from an active sync server, which sends mesh time separately from uptime
			//To help with accuracy only updates with the least number of hops are accepted
			uint32_t receivedSync;
			memcpy(&receivedSync, &packet.data[receivedPacketIndex], sizeof(receivedSync));
			receivedPacketIndex+=sizeof(receivedSync);
			#ifdef m2mMeshIncludeDebugFeatures
			if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NHS_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || packet.routerId == _nodeToLog || packet.originatorId == _nodeToLog))
			{
				_debugStream->printf_P(m2mMeshNHSMeshtimedms,receivedSync);
			}
			#endif
			if(_stable == true)
			{
				_updateMeshSync(receivedSync,packet.originatorId);
			}
		}
		else if(_currentMeshSyncServer == packet.originatorId)	//Check if the sync server has shut down
		{
			_chooseNewSyncServer(); //Choose a new sync server now
		}
		#if defined (m2mMeshIncludeRTCFeatures)
		if(packet.data[m2mMeshPacketFlagsIndex] & NHS_FLAGS_RTCSERVER)		//Check for included TZ and epoch time offset
		{
			if(rtc == true)	//This node is an RTC source
			{
				receivedPacketIndex += (packet.data[receivedPacketIndex] + 4);	//Skip over the RTC timezone and offset
			}
			else
			{
				//Any RTC server can supply the time zone
				uint8_t tzLength = packet.data[receivedPacketIndex++];	//Extract timezone string length
				if(timezone == nullptr)// || strncmp(timezone, char(packet.data[receivedPacketIndex]),tzLength) != 0) //Timezone needs setting
				{
					//if(timezone != nullptr)
					//{
					//	delete[] timezone;	//Free current timezone on heap, this should happen rarely
					//}
					timezone = new char[tzLength + 1];									//Allocate space on heap
					memcpy(&timezone, &packet.data[receivedPacketIndex],tzLength);		//Copy
					timezone[tzLength] = char(0);										//Null terminate
					setenv("TZ",timezone,1);											//Set timezone
					tzset();															//Apply timezone
					#ifdef m2mMeshIncludeDebugFeatures
					if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NHS_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || packet.routerId == _nodeToLog || packet.originatorId == _nodeToLog))
					{
						_debugStream->printf_P(m2mMeshNHSRTCtimezonesset,timezone);
					}
					#endif
				}
				receivedPacketIndex += tzLength;
				//Now get the offset, the node prefers the one with the best uptime to avoid jitter
				uint32_t receivedEpochOffset;
				memcpy(&receivedEpochOffset, &packet.data[receivedPacketIndex], sizeof(receivedEpochOffset));
				receivedPacketIndex+=sizeof(receivedEpochOffset);
				#ifdef m2mMeshIncludeDebugFeatures
				if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NHS_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || packet.routerId == _nodeToLog || packet.originatorId == _nodeToLog))
				{
					char strftime_buf[64];
					//strftime(strftime_buf, sizeof(strftime_buf), "%c", localtime(const_cast<time_t>(&tempUint32.value)));
					const time_t timestamp = receivedEpochOffset + ((millis() + _meshSyncOffset)/1000);
					strftime(strftime_buf, sizeof(strftime_buf), "%c", localtime(&timestamp));
					_debugStream->printf_P(m2mMeshNHSRTCtimeds,receivedEpochOffset,strftime_buf);
				}
				#endif
				if(_currentRTCServer == MESH_ORIGINATOR_NOT_FOUND ||								//No RTC server selected
					packet.originatorId == _currentRTCServer ||										//This is the current RTC server
					_originator[packet.originatorId].uptime > expectedUptime(_currentRTCServer))	//This is preferred over the current RTC server
				{
					if(_currentRTCServer != packet.originatorId)
					{
						_currentRTCServer = packet.originatorId;
					}
					if(receivedEpochOffset != epochOffset)	//Check for epoch time update
					{
						#ifdef m2mMeshIncludeDebugFeatures
						if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NHS_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || packet.routerId == _nodeToLog || packet.originatorId == _nodeToLog) && epochOffset != 0)
						{
							_debugStream->print(m2mMeshdiffers);
							if(receivedEpochOffset > epochOffset)
							{
								_debugStream->print(receivedEpochOffset -  epochOffset);
							}
							else
							{
								_debugStream->print(epochOffset - receivedEpochOffset);
							}
						}
						#endif
						epochOffset = receivedEpochOffset;
						if(_meshSyncNegotiated)
						{
							timeval tv = { epochOffset + ((millis() + _meshSyncOffset) / 1000), 0 };
							settimeofday(&tv, nullptr);
						}
					}
				}
				#ifdef m2mMeshIncludeDebugFeatures
				else if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NHS_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || packet.routerId == _nodeToLog || packet.originatorId == _nodeToLog))
				{
					_debugStream->print(m2mMeshlesspreferred);
				}
				#endif
			}
		}
		#endif
		if(packet.data[m2mMeshPacketFlagsIndex] & NHS_FLAGS_NODE_NAME_SET)	//Look for a node name
		{
			#ifdef m2mMeshIncludeDebugFeatures
			bool nameChanged = false;
			#endif
			uint8_t nodeNameLength = packet.data[receivedPacketIndex++];					//Name length is stored first
			char receivedNodeName[nodeNameLength + 1];										//Temporary string
			memcpy(receivedNodeName, &packet.data[receivedPacketIndex], nodeNameLength);	//Can't use strcpy, there's no null termination
			receivedNodeName[nodeNameLength] = 0;											//Null terminate the supplied name, as it is not null terminated in the packet
			if(_originator[packet.originatorId].nodeName !=nullptr &&
				strcmp(_originator[packet.originatorId].nodeName,receivedNodeName) != 0)			//Node name has changed, free the memory
			{
				#ifdef m2mMeshIncludeDebugFeatures
				if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NHS_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || packet.routerId == _nodeToLog || packet.originatorId == _nodeToLog))
				{
					_debugStream->printf_P(m2mMeshNHSnodenamelendschangedfroms,nodeNameLength,receivedNodeName,_originator[packet.originatorId].nodeName);
					nameChanged = true;
				}
				#endif
				delete[] _originator[packet.originatorId].nodeName;								//New node name, delete the previously allocated memory and reallocate
			}
			if(_originator[packet.originatorId].nodeName == nullptr)	//Name not set, or has been removed
			{
				#ifdef m2mMeshIncludeDebugFeatures
				if(_debugEnabled == true && nameChanged == false && _loggingLevel & MESH_UI_LOG_NHS_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || packet.routerId == _nodeToLog || packet.originatorId == _nodeToLog))
				{
					_debugStream->printf_P(m2mMeshNHSnodenamelendsadded,nodeNameLength,receivedNodeName);
				}
				#endif
				_originator[packet.originatorId].nodeName = new char[nodeNameLength + 1];	//Allocate memory on heap
				if(_originator[packet.originatorId].nodeName)
				{
					strcpy(_originator[packet.originatorId].nodeName,receivedNodeName);	//Copy the node name into the newly allocated memory
				}
				#ifdef m2mMeshIncludeDebugFeatures
				else
				{
					if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NHS_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || packet.routerId == _nodeToLog || packet.originatorId == _nodeToLog))
					{
						_debugStream->print(m2mMeshNHSUnabletostorenodenamenotenoughmemory);
					}
				}
				#endif
			}
			#ifdef m2mMeshIncludeDebugFeatures
			else if(_debugEnabled == true && nameChanged == false && _loggingLevel & MESH_UI_LOG_NHS_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || packet.routerId == _nodeToLog || packet.originatorId == _nodeToLog))
			{
				_debugStream->printf_P(m2mMeshNHSnodenamelendsunchanged,nodeNameLength,receivedNodeName);
			}
			#endif
			receivedPacketIndex += nodeNameLength;	//Advance the packet index past the name
		}
		if(packet.data[m2mMeshPacketFlagsIndex] & NHS_FLAGS_INCLUDES_ORIGINATORS)	//Extract originator information, if included
		{
			uint8_t originatorCount = packet.data[receivedPacketIndex++];	//Retrieve the number of originators in the end of the packet
			#ifdef m2mMeshIncludeDebugFeatures
			if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NHS_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || packet.routerId == _nodeToLog || packet.originatorId == _nodeToLog))
			{
				_debugStream->printf_P(m2mMeshNHScontainsdoriginators,originatorCount);
			}
			#endif
			for(uint8_t i = 0; i<originatorCount && receivedPacketIndex + 8 < ESP_NOW_MAX_PACKET_SIZE; i++) //The node will have filled in as many as it can in the space in the packet
			{
				#ifdef m2mMeshIncludeDebugFeatures
				if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NHS_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || packet.routerId == _nodeToLog || packet.originatorId == _nodeToLog))
				{
					uint16_t packetLtq;
					memcpy(&packetLtq, &packet.data[receivedPacketIndex+6], sizeof(packetLtq));
					_debugStream->printf_P(m2mMeshNHSoriginatordata02x02x02x02x02x02xTQ02x,packet.data[receivedPacketIndex],packet.data[receivedPacketIndex+1],packet.data[receivedPacketIndex+2],packet.data[receivedPacketIndex+3],packet.data[receivedPacketIndex+4],packet.data[receivedPacketIndex+5],packetLtq);
				}
				#endif
				if(not _isLocalMacAddress(&packet.data[receivedPacketIndex]))
				{
					//The included originator MIGHT also be a completely new device, so look for it.
					uint8_t includedOriginatorId = _originatorIdFromMac(&packet.data[receivedPacketIndex]);
					if(includedOriginatorId == MESH_ORIGINATOR_NOT_FOUND)
					{
						#ifdef m2mMeshIncludeDebugFeatures
						if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NHS_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || packet.routerId == _nodeToLog || packet.originatorId == _nodeToLog))
						{
							_debugStream->print(m2mMeshnew);
						}
						//This device has never been seen, create a new originator
						if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NODE_MANAGEMENT)
						{
							_debugStream->print(m2mMeshNHS);
						}
						#endif
						includedOriginatorId = _addOriginator(&packet.data[receivedPacketIndex],_currentChannel);
					}
				}
				#ifdef m2mMeshIncludeDebugFeatures
				else if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NHS_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || packet.routerId == _nodeToLog || packet.originatorId == _nodeToLog))
				{
					_debugStream->print(m2mMeshthisnode);
				}
				#endif
				receivedPacketIndex+=8;
			}
		}
	}
}

bool ICACHE_FLASH_ATTR m2mMeshClass::_sendPrp(m2mMeshPacketBuffer &packet, const uint8_t peer)
{
	packet.data[m2mMeshPacketTypeIndex] = PRP_PACKET_TYPE | PRP_DEFAULT_OPTIONS;	//Add the PRP packet type
	packet.data[m2mMeshPacketChecksumIndex] = 0;									//Here goes the checksum
	packet.data[m2mMeshPacketTTLIndex] = _currentTtl[PRP_PACKET_TYPE];				//Add the TTL, usually 0 for PRP
	memcpy(&packet.data[m2mMeshPacketSNIndex], &_sequenceNumber, sizeof(_sequenceNumber));	//Add the sequence number
	memcpy(&packet.data[m2mMeshPacketOriginatorIndex], _localMacAddress, 6);					//Add the source address
	memcpy(&packet.data[m2mMeshPacketDestinationIndex], _originator[peer].macAddress, 6);	//Add the requested peer MAC address
	packet.length = 20;
	packet.data[m2mMeshPacketDataLengthIndex] = packet.length;
	memcpy(&packet.routerMacAddress[0], &_broadcastMacAddress[0],6);	//Broadcast, even though it has a destination. This gets it past the lack of peering.
	#ifdef m2mMeshIncludeDebugFeatures
	if(_debugEnabled == true && ((_loggingLevel & MESH_UI_LOG_PRP_SEND) || (_loggingLevel & MESH_UI_LOG_ALL_SENT_PACKETS)))
	{
		_debugStream->printf_P(m2mMeshPRPSENDR02x02x02x02x02x02xO02x02x02x02x02x02xTTLdLENd,
		packet.routerMacAddress[0],
		packet.routerMacAddress[1],
		packet.routerMacAddress[2],
		packet.routerMacAddress[3],
		packet.routerMacAddress[4],
		packet.routerMacAddress[5],
		packet.data[m2mMeshPacketDestinationIndex],
		packet.data[m2mMeshPacketDestinationIndex1],
		packet.data[m2mMeshPacketDestinationIndex2],
		packet.data[m2mMeshPacketDestinationIndex3],
		packet.data[m2mMeshPacketDestinationIndex4],
		packet.data[m2mMeshPacketDestinationIndex5],
		packet.data[m2mMeshPacketTTLIndex],
		_sequenceNumber,packet.length);
	}
	#endif
	bool sendResult = _sendPacket(packet);	//Send immediately without any complicated routing
	#ifdef m2mMeshIncludeDebugFeatures
	if(_debugEnabled == true && ((_loggingLevel & MESH_UI_LOG_PRP_SEND) || (_loggingLevel & MESH_UI_LOG_ALL_SENT_PACKETS)))
	{
		if(sendResult == true)
		{
			_debugStream->print(m2mMeshsuccess);
		}
		else if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_PRP_SEND)
		{
			_debugStream->print(m2mMeshfailed);
		}
	}
	#endif
	_sequenceNumber++;
	return(sendResult);
}

bool ICACHE_FLASH_ATTR m2mMeshClass::_sendPrpResponse(m2mMeshPacketBuffer &packet, const uint8_t peer)
{
	packet.data[m2mMeshPacketTypeIndex] = PRP_PACKET_TYPE | PRP_DEFAULT_OPTIONS | RESPONSE;//Add the PRP packet type
	packet.data[m2mMeshPacketChecksumIndex] = 0;									//Here goes the checksum
	packet.data[m2mMeshPacketTTLIndex] = _currentTtl[PRP_PACKET_TYPE];				//Add the TTL, usually 0 for PRP
	memcpy(&packet.data[m2mMeshPacketSNIndex], &_sequenceNumber, sizeof(_sequenceNumber));	//Add the sequence number
	memcpy(&packet.data[m2mMeshPacketOriginatorIndex], _localMacAddress, 6);					//Add the source address
	memcpy(&packet.data[m2mMeshPacketDestinationIndex], _originator[peer].macAddress, 6);	//Add the requested peer MAC address
	packet.length = 20;
	memcpy(packet.routerMacAddress, _originator[peer].macAddress,6);	//Send direct
	#ifdef m2mMeshIncludeDebugFeatures
	if(_debugEnabled == true && ((_loggingLevel & MESH_UI_LOG_PRP_SEND) || (_loggingLevel & MESH_UI_LOG_ALL_SENT_PACKETS)))
	{
		_debugStream->printf_P(m2mMeshPRPSENDresponseR02x02x02x02x02x02xO02x02x02x02x02x02xTTLdLENd,
		packet.routerMacAddress[0],
		packet.routerMacAddress[1],
		packet.routerMacAddress[2],
		packet.routerMacAddress[3],
		packet.routerMacAddress[4],
		packet.routerMacAddress[5],
		packet.data[m2mMeshPacketDestinationIndex],
		packet.data[m2mMeshPacketDestinationIndex1],
		packet.data[m2mMeshPacketDestinationIndex2],
		packet.data[m2mMeshPacketDestinationIndex3],
		packet.data[m2mMeshPacketDestinationIndex4],
		packet.data[m2mMeshPacketDestinationIndex5],
		packet.data[m2mMeshPacketTTLIndex],
		_sequenceNumber,packet.length);
	}
	#endif
	bool sendResult = _sendPacket(packet);	//Send immediately without any complicated routing
	#ifdef m2mMeshIncludeDebugFeatures
	if(_debugEnabled == true && ((_loggingLevel & MESH_UI_LOG_PRP_SEND) || (_loggingLevel & MESH_UI_LOG_ALL_SENT_PACKETS)))
	{
		if(sendResult == true)
		{
			_debugStream->print(m2mMeshsuccess);
		}
		else if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_PRP_SEND)
		{
			_debugStream->print(m2mMeshfailed);
		}
	}
	#endif
	_sequenceNumber++;
	return(sendResult);
}

void ICACHE_FLASH_ATTR m2mMeshClass::_processPrp(m2mMeshPacketBuffer &packet)
{
	if(packet.destinationId == MESH_THIS_ORIGINATOR)
	{
		if(packet.data[m2mMeshPacketTypeIndex] & RESPONSE)
		{
			#ifdef m2mMeshIncludeDebugFeatures
			if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_PRP_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || packet.routerId == _nodeToLog || packet.originatorId == _nodeToLog))
			{
				_debugStream->printf_P(m2mMeshPRPRCVresponseR02x02x02x02x02x02xO02x02x02x02x02x02xTTLdLENd,packet.routerMacAddress[0],packet.routerMacAddress[1],packet.routerMacAddress[2],packet.routerMacAddress[3],packet.routerMacAddress[4],packet.routerMacAddress[5],_originator[packet.originatorId].macAddress[0],_originator[packet.originatorId].macAddress[1],_originator[packet.originatorId].macAddress[2],_originator[packet.originatorId].macAddress[3],_originator[packet.originatorId].macAddress[4],_originator[packet.originatorId].macAddress[5],packet.data[m2mMeshPacketTTLIndex],packet.length);
			}
			#endif
			if(_originator[packet.originatorId].hasUsAsPeer == false)
			{
				_originator[packet.originatorId].hasUsAsPeer = true;	//A PRP response signfies they have this node as a peer
			}
			if(_originator[packet.originatorId].isCurrentlyPeer == false)
			{
				_addPeer(packet.originatorId);	//Add again, just in case
			}
		}
		else
		{
			#ifdef m2mMeshIncludeDebugFeatures
			if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_PRP_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || packet.routerId == _nodeToLog || packet.originatorId == _nodeToLog))
			{
				_debugStream->printf_P(m2mMeshPRPRCVR02x02x02x02x02x02xO02x02x02x02x02x02xTTLdLENd,packet.routerMacAddress[0],packet.routerMacAddress[1],packet.routerMacAddress[2],packet.routerMacAddress[3],packet.routerMacAddress[4],packet.routerMacAddress[5],_originator[packet.originatorId].macAddress[0],_originator[packet.originatorId].macAddress[1],_originator[packet.originatorId].macAddress[2],_originator[packet.originatorId].macAddress[3],_originator[packet.originatorId].macAddress[4],_originator[packet.originatorId].macAddress[5],packet.data[m2mMeshPacketTTLIndex],packet.length);
			}
			#endif
			if(_originator[packet.originatorId].hasUsAsPeer == false)
			{
				_originator[packet.originatorId].hasUsAsPeer = true;	//Nodes only send PRP to peers
			}
			if(_originator[packet.originatorId].isCurrentlyPeer == true)
			{
				_sendPrpResponse(_sendBuffer, packet.originatorId);
			}
			else if(_addPeer(packet.originatorId) == true)
			{
				_sendPrpResponse(_sendBuffer, packet.originatorId);
			}
		}
	}
	#ifdef m2mMeshIncludeDebugFeatures
	else if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_PRP_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || packet.routerId == _nodeToLog || packet.originatorId == _nodeToLog))
	{
		//Not for this node, ignore it
		if(packet.data[m2mMeshPacketTypeIndex] & RESPONSE)
		{
			_debugStream->printf("\r\nPRP ignoring response from node %u",packet.destinationId);
		}
		else
		{
			_debugStream->printf("\r\nPRP ignoring request for node %u",packet.destinationId);
		}
	}
	#endif
}

bool ICACHE_FLASH_ATTR m2mMeshClass::_requestPeering(const uint8_t peer, const bool wait)
{
	#ifdef m2mMeshIncludeDebugFeatures
	if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_PEER_MANAGEMENT)
	{
		_debugStream->printf_P(m2mMeshRequestingpeeringwithnodeuO02x02x02x02x02x02x,
		peer,
		_originator[peer].macAddress[0],
		_originator[peer].macAddress[1],
		_originator[peer].macAddress[2],
		_originator[peer].macAddress[3],
		_originator[peer].macAddress[4],
		_originator[peer].macAddress[5]
		);
	}
	#endif
	_sendPrp(_sendBuffer, peer);	//Request a peering
	#ifndef m2mMeshProcessPacketsInCallback
	if(wait == true)
	{
		uint32_t peeringTimer = millis();
		while(_originator[peer].hasUsAsPeer == false && millis() - peeringTimer < _peeringTimeout)
		{
			if(_receiveBufferIndex != _processBufferIndex ||					//Some data in buffer
				_receiveBuffer[_processBufferIndex].length > 0)					//Buffer is full
			{
				_processPacket(_receiveBuffer[_processBufferIndex]);			//Process the packet
				_processBufferIndex++;												//Advance the buffer index
				_processBufferIndex = _processBufferIndex%M2MMESHRECEIVEBUFFERSIZE;	//Rollover the buffer index
			}
			yield();
		}
		#ifdef m2mMeshIncludeDebugFeatures
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_PEER_MANAGEMENT)
		{
			_debugStream->print(m2mMeshPeeringrequest);
			if(_originator[peer].hasUsAsPeer == true)
			{
				_debugStream->print(m2mMeshsuccess);
			}
			else
			{
				_debugStream->print(m2mMeshfailed);
			}
		}
		#endif
		//delay(1000);
		return(_originator[peer].hasUsAsPeer);
	}
	else
	{
		return(true);
	}
	#else
	return(true);
	#endif
}

void ICACHE_FLASH_ATTR m2mMeshClass::_processUsr(m2mMeshPacketBuffer &packet)
{
	#ifdef m2mMeshIncludeDebugFeatures
	if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_USR_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || packet.routerId == _nodeToLog || packet.originatorId == _nodeToLog))
	{
		_debugStream->printf_P(m2mMeshUSRR02x02x02x02x02x02xO02x02x02x02x02x02xTTLdLengthd,packet.routerMacAddress[0],packet.routerMacAddress[1],packet.routerMacAddress[2],packet.routerMacAddress[3],packet.routerMacAddress[4],packet.routerMacAddress[5],_originator[packet.originatorId].macAddress[0],_originator[packet.originatorId].macAddress[1],_originator[packet.originatorId].macAddress[2],_originator[packet.originatorId].macAddress[3],_originator[packet.originatorId].macAddress[4],_originator[packet.originatorId].macAddress[5],packet.data[m2mMeshPacketTTLIndex],packet.length);
	}
	#endif
	if(_applicationBuffer[_applicationBufferWriteIndex].length == 0) //There's a free slot in the app buffer
	{
		memcpy(&_applicationBuffer[_applicationBufferWriteIndex].routerMacAddress, &packet.routerMacAddress, 6);
		_applicationBuffer[_applicationBufferWriteIndex].length = packet.length;
		memcpy(&_applicationBuffer[_applicationBufferWriteIndex].data, &packet.data, packet.length);
		_applicationBuffer[_applicationBufferWriteIndex].timestamp = packet.timestamp;
		#ifdef m2mMeshIncludeDebugFeatures
		if(_debugEnabled == true && (_loggingLevel & MESH_UI_LOG_BUFFER_MANAGEMENT))
		{
			_debugStream->printf_P(m2mMeshfillAPPbufferslotd,
				_applicationBufferWriteIndex,
				packet.length,
				packetTypeDescription[packet.data[m2mMeshPacketTypeIndex] & 0x07],
				packet.routerMacAddress[0],
				packet.routerMacAddress[1],
				packet.routerMacAddress[2],
				packet.routerMacAddress[3],
				packet.routerMacAddress[4],
				packet.routerMacAddress[5],
				_originator[packet.originatorId].macAddress[0],
				_originator[packet.originatorId].macAddress[1],
				_originator[packet.originatorId].macAddress[2],
				_originator[packet.originatorId].macAddress[3],
				_originator[packet.originatorId].macAddress[4],
				_originator[packet.originatorId].macAddress[5]);
		}
		#endif
		_applicationBufferWriteIndex++;																//Advance the buffer index
		_applicationBufferWriteIndex = _applicationBufferWriteIndex%M2MMESHAPPLICATIONBUFFERSIZE;	//Rollover buffer index
		uint8_t receivedPacketIndex = 18;
		if((packet.data[m2mMeshPacketTypeIndex] & SEND_TO_ALL_NODES) == 0x00)	//If there's a destination, skip past
		{
			receivedPacketIndex+=6;
		}
		_receivedUserPacketIndex = receivedPacketIndex;							//Record where we are in reading the packet for later data retrieval
		_receivedUserPacketIndex++;
		_receivedUserPacketFieldCounter = packet.data[receivedPacketIndex++];	//Retrieve the number of fields in the packet
		#ifdef m2mMeshIncludeDebugFeatures
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_USR_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || packet.routerId == _nodeToLog || packet.originatorId == _nodeToLog))
		{
			_debugStream->printf_P(m2mMeshUSRpacketcontainsdfields,_receivedUserPacketFieldCounter);
			uint8_t field = 0;
			while(receivedPacketIndex < packet.length && field < _receivedUserPacketFieldCounter)
			{
				if(packet.data[receivedPacketIndex] == USR_DATA_BOOL)	//Single bool false
				{
					receivedPacketIndex++;
					_debugStream->printf_P(m2mMeshUSRdatafielddboolfalse,field++);
				}
				else if(packet.data[receivedPacketIndex] == USR_DATA_BOOL_TRUE)	//Single bool true
				{
					receivedPacketIndex++;
					_debugStream->printf_P(m2mMeshUSRdatafielddbooltrue,field++);
				}
				else if((packet.data[receivedPacketIndex] & 0x0f) == USR_DATA_BOOL)	//Array of bool
				{
					uint8_t arrayLength = _numberOfPackedItems(packet.data[receivedPacketIndex], packet.data[receivedPacketIndex+1]);
					receivedPacketIndex+=_packingOverhead(packet.data[receivedPacketIndex]);
					_debugStream->printf_P(m2mMeshUSRdatafielddbooluarray,field++,arrayLength);
					for(uint8_t index = 0; index < arrayLength; index++)
					{
						if(packet.data[receivedPacketIndex++] == USR_DATA_BOOL)
						{
							_debugStream->print(m2mMeshUSRdatadboolfalse);
						}
						else
						{
							_debugStream->print(m2mMeshUSRdatadbooltrue);
						}
					}
				}
				else if(packet.data[receivedPacketIndex] == USR_DATA_UINT8_T)	//Single uint8_t
				{
					receivedPacketIndex++;
					_debugStream->printf_P(m2mMeshUSRdatafieldduint8_td,field++,packet.data[receivedPacketIndex++]);
				}
				else if((packet.data[receivedPacketIndex] & 0x0f) == USR_DATA_UINT8_T)	//Array of uint8_t
				{
					uint8_t arrayLength = _numberOfPackedItems(packet.data[receivedPacketIndex], packet.data[receivedPacketIndex+1]);
					receivedPacketIndex+=_packingOverhead(packet.data[receivedPacketIndex]);
					_debugStream->printf_P(m2mMeshUSRdatafieldduint8_tuarray,field++,arrayLength);
					for(uint8_t index = 0; index < arrayLength; index++)
					{
						_debugStream->printf_P(m2mMeshUSRdataduint8_td,index,packet.data[receivedPacketIndex++]);
					}
				}
				else if(packet.data[receivedPacketIndex] == USR_DATA_UINT16_T)	//Single uint16_t
				{
					receivedPacketIndex++;
					uint16_t temp;
					memcpy(&temp, &packet.data[receivedPacketIndex], sizeof(temp));
					receivedPacketIndex+=sizeof(temp);
					_debugStream->printf_P(m2mMeshUSRdatafieldduint16_td,field++,temp);
				}
				else if((packet.data[receivedPacketIndex] & 0x0f) == USR_DATA_UINT16_T)	//Array of uint16_t
				{
					uint8_t arrayLength = _numberOfPackedItems(packet.data[receivedPacketIndex], packet.data[receivedPacketIndex+1]);
					receivedPacketIndex+=_packingOverhead(packet.data[receivedPacketIndex]);
					_debugStream->printf_P(m2mMeshUSRdatafieldduint16_tuarray,field++,arrayLength);
					uint16_t temp;
					for(uint8_t index = 0; index < arrayLength; index++)
					{
						memcpy(&temp, &packet.data[receivedPacketIndex], sizeof(temp));
						receivedPacketIndex+=sizeof(temp);
						_debugStream->printf_P(m2mMeshUSRdataduint16_td,index,temp);
					}
				}
				else if(packet.data[receivedPacketIndex] == USR_DATA_UINT32_T)	//Single uint32_t
				{
					receivedPacketIndex++;
					uint32_t temp;
					memcpy(&temp, &packet.data[receivedPacketIndex], sizeof(temp));
					receivedPacketIndex+=sizeof(temp);
					_debugStream->printf_P(m2mMeshUSRdatafieldduint32_td,field++,temp);
				}
				else if((packet.data[receivedPacketIndex] & 0x0f) == USR_DATA_UINT32_T)	//Array of uint32_t
				{
					uint8_t arrayLength = _numberOfPackedItems(packet.data[receivedPacketIndex], packet.data[receivedPacketIndex+1]);
					receivedPacketIndex+=_packingOverhead(packet.data[receivedPacketIndex]);
					_debugStream->printf_P(m2mMeshUSRdatafieldduint32_tuarray,field++,arrayLength);
					uint32_t temp;
					for(uint8_t index = 0; index < arrayLength; index++)
					{
						memcpy(&temp, &packet.data[receivedPacketIndex], sizeof(temp));
						receivedPacketIndex+=sizeof(temp);
						_debugStream->printf_P(m2mMeshUSRdataduint32_td,index,temp);
					}
				}
				else if(packet.data[receivedPacketIndex] == USR_DATA_UINT64_T)	//Single uint64_t
				{
					receivedPacketIndex++;
					uint64_t temp;
					memcpy(&temp, &packet.data[receivedPacketIndex], sizeof(temp));
					receivedPacketIndex+=sizeof(temp);
					_debugStream->printf_P(m2mMeshUSRdatafieldduint64_td,field++,temp);
				}
				else if((packet.data[receivedPacketIndex] & 0x0f) == USR_DATA_UINT64_T)	//Array of uint64_t
				{
					uint8_t arrayLength = _numberOfPackedItems(packet.data[receivedPacketIndex], packet.data[receivedPacketIndex+1]);
					receivedPacketIndex+=_packingOverhead(packet.data[receivedPacketIndex]);
					_debugStream->printf_P(m2mMeshUSRdatafieldduint64_tuarray,field++,arrayLength);
					uint64_t temp;
					for(uint8_t index = 0; index < arrayLength; index++)
					{
						memcpy(&temp, &packet.data[receivedPacketIndex], sizeof(temp));
						receivedPacketIndex+=sizeof(temp);
						_debugStream->printf_P(m2mMeshUSRdataduint64_td,index,temp);
					}
				}
				else if(packet.data[receivedPacketIndex] == USR_DATA_INT8_T)	//Single int8_t
				{
					receivedPacketIndex++;
					_debugStream->printf_P(m2mMeshUSRdatafielddint8_td,field++,int8_t(packet.data[receivedPacketIndex++]));
				}
				else if((packet.data[receivedPacketIndex] & 0x0f) == USR_DATA_INT8_T)
				{
					uint8_t arrayLength = _numberOfPackedItems(packet.data[receivedPacketIndex], packet.data[receivedPacketIndex+1]);
					receivedPacketIndex+=_packingOverhead(packet.data[receivedPacketIndex]);
					_debugStream->printf_P(m2mMeshUSRdatafielddint8_tuarray,field++,arrayLength);
					for(uint8_t index = 0; index < arrayLength; index++)
					{
						_debugStream->printf_P(m2mMeshUSRdatadint8_td,index,int8_t(packet.data[receivedPacketIndex++]));
					}
				}
				else if(packet.data[receivedPacketIndex] == USR_DATA_INT16_T)	//Single int16_t
				{
					receivedPacketIndex++;
					int16_t temp;
					memcpy(&temp, &packet.data[receivedPacketIndex], sizeof(temp));
					receivedPacketIndex+=sizeof(temp);
					_debugStream->printf_P(m2mMeshUSRdatafielddint16_td,field++,temp);
				}
				else if((packet.data[receivedPacketIndex] & 0x0f) == USR_DATA_INT16_T)	//Array of int16_t
				{
					uint8_t arrayLength = _numberOfPackedItems(packet.data[receivedPacketIndex], packet.data[receivedPacketIndex+1]);
					receivedPacketIndex+=_packingOverhead(packet.data[receivedPacketIndex]);
					_debugStream->printf_P(m2mMeshUSRdatafielddint16_tuarray,field++,arrayLength);
					int16_t temp;
					for(uint8_t index = 0; index < arrayLength; index++)
					{
						memcpy(&temp, &packet.data[receivedPacketIndex], sizeof(temp));
						receivedPacketIndex+=sizeof(temp);
						_debugStream->printf_P(m2mMeshUSRdatadint16_td,index,temp);
					}
				}
				else if(packet.data[receivedPacketIndex] == USR_DATA_INT32_T)	//Single int32_t
				{
					receivedPacketIndex++;
					int32_t temp;
					memcpy(&temp, &packet.data[receivedPacketIndex], sizeof(temp));
					receivedPacketIndex+=sizeof(temp);
					_debugStream->printf_P(m2mMeshUSRdatafielddint32_td,field++,temp);
				}
				else if((packet.data[receivedPacketIndex] & 0x0f) == USR_DATA_INT32_T)	//Array of int32_t
				{
					uint8_t arrayLength = _numberOfPackedItems(packet.data[receivedPacketIndex], packet.data[receivedPacketIndex+1]);
					receivedPacketIndex+=_packingOverhead(packet.data[receivedPacketIndex]);
					_debugStream->printf_P(m2mMeshUSRdatafielddint32_tuarray,field++,arrayLength);
					int32_t temp;
					for(uint8_t index = 0; index < arrayLength; index++)
					{
						memcpy(&temp, &packet.data[receivedPacketIndex], sizeof(temp));
						receivedPacketIndex+=sizeof(temp);
						_debugStream->printf_P(m2mMeshUSRdatadint32_td,index,temp);
					}
				}
				else if(packet.data[receivedPacketIndex] == USR_DATA_INT64_T)	//Single int64_t
				{
					receivedPacketIndex++;
					int64_t temp;
					memcpy(&temp, &packet.data[receivedPacketIndex], sizeof(temp));
					receivedPacketIndex+=sizeof(temp);
					_debugStream->printf_P(m2mMeshUSRdatafielddint64_td,field++,temp);
				}
				else if((packet.data[receivedPacketIndex] & 0x0f) == USR_DATA_INT64_T) //Array of int64_t
				{
					uint8_t arrayLength = _numberOfPackedItems(packet.data[receivedPacketIndex], packet.data[receivedPacketIndex+1]);
					receivedPacketIndex+=_packingOverhead(packet.data[receivedPacketIndex]);
					_debugStream->printf_P(m2mMeshUSRdatafielddint64_tuarray,field++,arrayLength);
					int64_t temp;
					for(uint8_t index = 0; index < arrayLength; index++)
					{
						memcpy(&temp, &packet.data[receivedPacketIndex], sizeof(temp));
						receivedPacketIndex+=sizeof(temp);
						_debugStream->printf_P(m2mMeshUSRdatadint64_td,index,temp);
					}
				}
				else if(packet.data[receivedPacketIndex] == USR_DATA_FLOAT)	//Single float
				{
					receivedPacketIndex++;
					float temp;
					memcpy(&temp, &packet.data[receivedPacketIndex], sizeof(temp));
					receivedPacketIndex+=sizeof(temp);
					_debugStream->printf_P(m2mMeshUSRdatafielddfloatf,field++,temp);
				}
				else if((packet.data[receivedPacketIndex] & 0x0f) == USR_DATA_FLOAT)	//Array of floats
				{
					uint8_t arrayLength = _numberOfPackedItems(packet.data[receivedPacketIndex], packet.data[receivedPacketIndex+1]);
					receivedPacketIndex+=_packingOverhead(packet.data[receivedPacketIndex]);
					_debugStream->printf_P(m2mMeshUSRdatafielddfloatuarray,field++,arrayLength);
					float temp;
					for(uint8_t index = 0; index < arrayLength; index++)
					{
						memcpy(&temp, &packet.data[receivedPacketIndex], sizeof(temp));
						receivedPacketIndex+=sizeof(temp);
						_debugStream->printf_P(m2mMeshUSRdatadfloatf,index,temp);
					}
				}
				else if(packet.data[receivedPacketIndex] == USR_DATA_DOUBLE)	//Single double
				{
					receivedPacketIndex++;
					double temp;
					memcpy(&temp, &packet.data[receivedPacketIndex], sizeof(temp));
					receivedPacketIndex+=sizeof(temp);
					_debugStream->printf_P(m2mMeshUSRdatafieldddoublef,field++,temp);
				}
				else if((packet.data[receivedPacketIndex] & 0x0f) == USR_DATA_DOUBLE)	//Array of doubles
				{
					uint8_t arrayLength = _numberOfPackedItems(packet.data[receivedPacketIndex], packet.data[receivedPacketIndex+1]);
					receivedPacketIndex+=_packingOverhead(packet.data[receivedPacketIndex]);
					_debugStream->printf_P(m2mMeshUSRdatafieldddoubleuarray,field++,arrayLength);
					double temp;
					for(uint8_t index = 0; index < arrayLength; index++)
					{
						memcpy(&temp, &packet.data[receivedPacketIndex], sizeof(temp));
						receivedPacketIndex+=sizeof(temp);
						_debugStream->printf_P(m2mMeshUSRdataddoublef,index,temp);
					}
				}
				else if(packet.data[receivedPacketIndex] == USR_DATA_CHAR)	//Single char
				{
					receivedPacketIndex++;
					_debugStream->printf_P(m2mMeshUSRdatafielddcharc,field++,char(packet.data[receivedPacketIndex++]));
				}
				else if((packet.data[receivedPacketIndex] & 0x0f) == USR_DATA_CHAR)	//Array of chars
				{
					uint8_t arrayLength = _numberOfPackedItems(packet.data[receivedPacketIndex], packet.data[receivedPacketIndex+1]);
					receivedPacketIndex+=_packingOverhead(packet.data[receivedPacketIndex]);
					_debugStream->printf_P(m2mMeshUSRdatafielddcharuarray,field++,arrayLength);
					for(uint8_t index = 0; index < arrayLength; index++)
					{
						_debugStream->printf_P(m2mMeshUSRdatadcharc,index,char(packet.data[receivedPacketIndex++]));
					}
				}
				else if((packet.data[receivedPacketIndex] & 0xf) == USR_DATA_STRING)
				{
					uint8_t arrayLength = _numberOfPackedItems(packet.data[receivedPacketIndex], packet.data[receivedPacketIndex+1]);
					receivedPacketIndex+=_packingOverhead(packet.data[receivedPacketIndex]);
					char userDataString[arrayLength+1];
					for(uint8_t i = 0; i<arrayLength ; i++)
					{
						userDataString[i] = char(packet.data[receivedPacketIndex++]);
					}
					userDataString[arrayLength] = char(0);
					_debugStream->printf_P(m2mMeshUSRdatafielddStringlends,field++,arrayLength, userDataString);
				}
				else if((packet.data[receivedPacketIndex] & 0x0f) == USR_DATA_STR)	//C string (null terminated char array)
				{
					uint8_t arrayLength = _numberOfPackedItems(packet.data[receivedPacketIndex], packet.data[receivedPacketIndex+1]);
					receivedPacketIndex+=_packingOverhead(packet.data[receivedPacketIndex]);
					char userDataString[arrayLength+1];
					for(uint8_t i = 0; i<arrayLength ; i++)
					{
						userDataString[i] = char(packet.data[receivedPacketIndex++]);
					}
					userDataString[arrayLength] = char(0);
					_debugStream->printf_P(m2mMeshUSRdatafielddcstringlends,field++,arrayLength, userDataString);
				}
				else
				{
					_debugStream->printf_P(m2mMeshUSRdatafielddunknowntypedstoppingdecode,field++,packet.data[receivedPacketIndex]);
					while(receivedPacketIndex < packet.length)
					{
						_debugStream->printf_P(m2mMeshd02x,packet.data[receivedPacketIndex],packet.data[receivedPacketIndex++]);
					}
				}
			}
		}
		#endif
		if(eventCallback)
		{
			eventCallback(meshEvent::message);
		}

	}
	else
	{
		#ifdef m2mMeshIncludeDebugFeatures
		if(_debugEnabled == true && ((_loggingLevel & MESH_UI_LOG_BUFFER_MANAGEMENT) || (_loggingLevel & MESH_UI_LOG_ERRORS)))
		{
			_debugStream->print(m2mMeshreadAPPbufferfull);
		}
		#endif
		_droppedAppPackets++;
	}
}


//Update the mesh time from the sync server. This does NOT allowance for latency across the mesh, so nodes will only be roughly in sync
void ICACHE_FLASH_ATTR m2mMeshClass::_updateMeshSync(const uint32_t newMeshSync, const uint8_t originatorId)
{
	//It's a tie, which is not uncommon when handing over between two servers
	if(syncedMillis() == newMeshSync)
	{
		//If we have better uptime we win a tie, it is unlikely nodes will tie on real uptime unless they share a common PSU
		if(millis() > expectedUptime(originatorId))
		{
			//Become the sync server
			_becomeSyncServer();
		}
		else
		{
			//Concede the sync server and set the mesh time
			_setMeshSync(newMeshSync, originatorId);
		}
	}
	//A higher mesh uptime becomes the reference clock, if the reference clock wanders <1s closer also update it
	else if(newMeshSync != syncedMillis() && (newMeshSync > syncedMillis() || syncedMillis() - newMeshSync < _meshSyncWindow))
	{
		//Set the mesh time
		_setMeshSync(newMeshSync, originatorId);
	}
	else if(newMeshSync < millis())
	{
		//Become the sync server
		_becomeSyncServer();
	}
	else if(_actingAsSyncServer == true && _stable == true && _meshSyncNegotiated == false)
	{
		//Have remained the sync server from the start
		_meshSyncNegotiated = true;
		#if defined (m2mMeshIncludeRTCFeatures)
		if(rtc == false && epochOffset != 0)	//RTC is already synced, set the clock
		{
			timeval tv = { epochOffset + ((millis() + _meshSyncOffset) / 1000), 0 };
			settimeofday(&tv, nullptr);
		}
		#endif
		if(eventCallback)
		{
			eventCallback(meshEvent::synced);			//Trigger the callback
		}
	}
}

bool ICACHE_FLASH_ATTR m2mMeshClass::synced()
{
	return(_meshSyncNegotiated);
}
bool ICACHE_FLASH_ATTR m2mMeshClass::amSyncServer()
{
	return(_actingAsSyncServer);
}
uint8_t ICACHE_FLASH_ATTR m2mMeshClass::syncServer()
{
	return(_currentMeshSyncServer);
}

void ICACHE_FLASH_ATTR m2mMeshClass::_setMeshSync(const uint32_t newMeshSync, const uint8_t originatorId)
{
	//Stop being a sync server if we currently are
	if(_actingAsSyncServer == true)
	{
		_actingAsSyncServer = false;
	}
	//Temporarily record the old time offset
	int32_t oldMeshSyncOffset = _meshSyncOffset;
	_meshSyncOffset = newMeshSync - millis();
	//Track clock drift
	if(abs(oldMeshSyncOffset - _meshSyncOffset)<1000)
	{
		//track cumlative drift
		_meshSyncDrift += oldMeshSyncOffset - _meshSyncOffset;
	}
	else
	{
		//It was a big jump, so clear drift
		_meshSyncDrift = 0;
	}
	#ifdef m2mMeshIncludeDebugFeatures
	if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NHS_RECEIVED && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || originatorId == _nodeToLog))
	{
		if(originatorId != _currentMeshSyncServer)
		{
			_debugStream->printf_P(m2mMeshNHS02x02x02x02x02x02xisnowthetimeserver,_originator[originatorId].macAddress[0],_originator[originatorId].macAddress[1],_originator[originatorId].macAddress[2],_originator[originatorId].macAddress[3],_originator[originatorId].macAddress[4],_originator[originatorId].macAddress[5]);
		}
		if(oldMeshSyncOffset > _meshSyncOffset)
		{
			_debugStream->printf_P(m2mMeshNHStimeoffsetnegdms,oldMeshSyncOffset - _meshSyncOffset);
		}
		else if(oldMeshSyncOffset != _meshSyncOffset && _meshSyncOffset - oldMeshSyncOffset < 5000)
		{
			_debugStream->printf_P(m2mMeshNHStimeoffsetposdms,_meshSyncOffset - oldMeshSyncOffset);
		}
		else if(oldMeshSyncOffset != _meshSyncOffset)
		{
			char uptime[] = "00h00m00s";
			_friendlyUptime(millis() + _meshSyncOffset,uptime);
			_debugStream->printf_P(m2mMeshNHSmeshtimesettos,uptime);
		}
	}
	#endif
	_currentMeshSyncServer = originatorId;
	if(_meshSyncNegotiated == false)
	{
		_meshSyncNegotiated = true;
		#if defined (m2mMeshIncludeRTCFeatures)
		if(rtc == false && epochOffset != 0)	//RTC is already synced, set the clock
		{
			timeval tv = { epochOffset + ((millis() + _meshSyncOffset) / 1000), 0 };
			settimeofday(&tv, nullptr);
		}
		#endif
		if(eventCallback)
		{
			eventCallback(meshEvent::synced);			//Trigger the callback
		}
	}
}


//Choose a new sync server
void ICACHE_FLASH_ATTR m2mMeshClass::_chooseNewSyncServer()
{
  //The current sync server has gone away, so set it as unknown and don't reselect it
  uint8_t previousMeshSyncServer = _currentMeshSyncServer;
  _currentMeshSyncServer = MESH_ORIGINATOR_NOT_FOUND;
  uint32_t longestUp = 0;
  //Iterate through the mesh looking for the likely new sync server
  for(uint8_t originatorId = 0; originatorId < _numberOfOriginators; originatorId++)
  {
    if(originatorId != previousMeshSyncServer && _dataIsValid(originatorId,NHS_PACKET_TYPE) && expectedUptime(originatorId) > millis() && expectedUptime(originatorId) > longestUp)
    {
      //This originator is active and up longer than us, we HOPE it is running as a sync server
	  longestUp = expectedUptime(originatorId);
      _currentMeshSyncServer = originatorId;
    }
  }
  //This node has the highest uptime, so become the sync server
  if(_currentMeshSyncServer == MESH_ORIGINATOR_NOT_FOUND)
  {
    _becomeSyncServer();
	#ifdef m2mMeshIncludeDebugFeatures
    if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_WARNINGS)
    {
      _debugStream->print(m2mMeshSyncserverhasgoneofflinetakingovertimeserverrole);
    }
	#endif
  }
}

//Become the sync server
void ICACHE_FLASH_ATTR m2mMeshClass::_becomeSyncServer()
{
	_actingAsSyncServer = true;
	_currentMeshSyncServer = MESH_ORIGINATOR_NOT_FOUND; 
	if(_meshSyncNegotiated == false)
	{
		_meshSyncNegotiated = true;
		#if defined (m2mMeshIncludeRTCFeatures)
		if(rtc == false && epochOffset != 0)	//RTC is already synced, set the clock
		{
			timeval tv = { epochOffset + ((millis() + _meshSyncOffset) / 1000), 0 };
			settimeofday(&tv, nullptr);
		}
		#endif
		if(eventCallback)
		{
			eventCallback(meshEvent::synced);			//Trigger the callback
		}
	}
}

//Mesh time is just an offset of the node uptime to make it broadly the same as the longest up node
uint32_t ICACHE_FLASH_ATTR m2mMeshClass::syncedMillis()
{
  return(millis() + _meshSyncOffset);
}

void ICACHE_FLASH_ATTR m2mMeshClass::enableStatusLed(uint8_t pin, bool onState)
{
	_statusLedPin = pin;
	pinMode(_statusLedPin,OUTPUT);
	digitalWrite(_statusLedPin, onState);
	_statusLedOnState = onState;
	_statusLedState = true;
	_statusLedEnabled = true;
}

void ICACHE_FLASH_ATTR m2mMeshClass::enableActivityLed(const uint8_t pin, const bool onState)
{
	_activityLedPin = pin;
	pinMode(_activityLedPin,OUTPUT);
	digitalWrite(_activityLedPin,not onState);
	_activityLedOnState = onState;
	_activityLedState = false;
	_activityLedEnabled = true;
}

//Blink the activity LED
void ICACHE_FLASH_ATTR m2mMeshClass::_blinkActivityLed()
{
  if(_activityLedState == false)
  {
    digitalWrite(_activityLedPin,_activityLedOnState);
    _activityLedState = true;
    _activityLedTimer = millis();
  }
}

//Check if a given MAC address is the local one
bool ICACHE_FLASH_ATTR m2mMeshClass::_isLocalMacAddress(const uint8_t *mac)
{
  if(_localMacAddress[0] == mac[0] && _localMacAddress[1] == mac[1] && _localMacAddress[2] == mac[2] && _localMacAddress[3] == mac[3] && _localMacAddress[4] == mac[4] && _localMacAddress[5] == mac[5])
  {
    return (true);
  }
  return (false);
}

//Given a pointer to a MAC address this returns which originator ID it is, MESH_ORIGINATOR_NOT_FOUND is a failure
uint8_t ICACHE_FLASH_ATTR m2mMeshClass::_originatorIdFromMac(const uint8_t *mac)
{
	uint8_t originatorId = MESH_ORIGINATOR_NOT_FOUND;
	for(uint8_t i = 0; i<_numberOfOriginators;i++)
	{
		if(mac[0] == _originator[i].macAddress[0] &&
			mac[1] == _originator[i].macAddress[1] &&
			mac[2] == _originator[i].macAddress[2] &&
			mac[3] == _originator[i].macAddress[3] &&
			mac[4] == _originator[i].macAddress[4] &&
			mac[5] == _originator[i].macAddress[5])
		{
			originatorId = i;
			return(originatorId);
		}
	}
	if(_isLocalMacAddress(mac))
	{
		return(MESH_THIS_ORIGINATOR);
	}
	return(originatorId);
}

uint8_t ICACHE_FLASH_ATTR m2mMeshClass::_originatorIdFromMac(const uint8_t mac0, const uint8_t mac1, const uint8_t mac2, const uint8_t mac3, const uint8_t mac4, const uint8_t mac5)
{
	uint8_t macAddress[6] = {mac0, mac1, mac2, mac3, mac4, mac5};
	return(_originatorIdFromMac(macAddress));
}

//Adds an originator record
uint8_t ICACHE_FLASH_ATTR m2mMeshClass::_addOriginator(const uint8_t* mac, const uint8_t originatorChannel)
{
	if(_numberOfOriginators < _maxNumberOfOriginators)
	{
		//memcpy(&&originator[numberOfOriginators].macAddress[0],mac,6);
		_originator[_numberOfOriginators].macAddress[0] = mac[0];
		_originator[_numberOfOriginators].macAddress[1] = mac[1];
		_originator[_numberOfOriginators].macAddress[2] = mac[2];
		_originator[_numberOfOriginators].macAddress[3] = mac[3];
		_originator[_numberOfOriginators].macAddress[4] = mac[4];
		_originator[_numberOfOriginators].macAddress[5] = mac[5];
		_originator[_numberOfOriginators].channel = originatorChannel;
		_originator[_numberOfOriginators].selectedRouter = MESH_ORIGINATOR_NOT_FOUND;
		#ifdef m2mMeshIncludeDebugFeatures
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NODE_MANAGEMENT)
		{
			_debugStream->printf_P(m2mMesh02x02x02x02x02x02xaddedidd,mac[0],mac[1],mac[2],mac[3],mac[4],mac[5],_numberOfOriginators);
		}
		#endif
		_numberOfOriginators++;
		_meshHasBecomeUnstable();
		rollOutTheWelcomeWagon();			//Do stuff to welcome a new originator
		return(_numberOfOriginators - 1);
	}
	else if(_allowMeshGrowth == true && _numberOfOriginators < 255 - _meshGrowthIncrement)
	{
		#ifdef m2mMeshIncludeDebugFeatures
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NODE_MANAGEMENT)
		{
			_debugStream->printf_P(m2mMeshincreasingmaximummeshsizebyunodestoaccommodate, _meshGrowthIncrement);
		}
		#endif
		if(_increaseMeshSize(_meshGrowthIncrement))
		{
			#ifdef m2mMeshIncludeDebugFeatures
			#endif
			return(_addOriginator(mac, originatorChannel)); //Add the originator
		}
		else
		{
			#ifdef m2mMeshIncludeDebugFeatures
			if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NODE_MANAGEMENT)
			{
				_debugStream->print(m2mMeshfailed);
			}
			#endif
			return(MESH_ORIGINATOR_NOT_FOUND);
		}
	}
	#ifdef m2mMeshIncludeDebugFeatures
	else if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NODE_MANAGEMENT)
	{
		_debugStream->printf_P(m2mMesh02x02x02x02x02x02xcouldnotbeadded,mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	}
	#endif
	_meshHasBecomeUnstable();
	_lastError = m2mMesh_MeshIsFull;
	return(MESH_ORIGINATOR_NOT_FOUND);
}

bool ICACHE_FLASH_ATTR m2mMeshClass::_increaseMeshSize(uint8_t extraNodes)
{
	if(_allowMeshGrowth == true && _numberOfOriginators < 255 - extraNodes)
	{
		m2mMeshOriginatorInfo *_originatorNew = new m2mMeshOriginatorInfo[_maxNumberOfOriginators + _meshGrowthIncrement];
		/*for(uint8_t i = 0; i < _maxNumberOfOriginators; i++)
		{
			_originatorNew[i].nodeName = _originator[i].nodeName;
			_originatorNew[i].macAddress[0] = _originator[i].macAddress[0];
			_originatorNew[i].macAddress[1] = _originator[i].macAddress[1];
			_originatorNew[i].macAddress[2] = _originator[i].macAddress[2];
			_originatorNew[i].macAddress[3] = _originator[i].macAddress[3];
			_originatorNew[i].macAddress[4] = _originator[i].macAddress[4];
			_originatorNew[i].macAddress[5] = _originator[i].macAddress[5];
			_originatorNew[i].channel = _originator[i].channel;
			_originatorNew[i].isCurrentlyPeer = _originator[i].isCurrentlyPeer;
			_originatorNew[i].hasUsAsPeer = _originator[i].hasUsAsPeer;
			_originatorNew[i].peeringExpired = _originator[i].peeringExpired;
			_originatorNew[i].peerNeeded = _originator[i].peerNeeded;
			_originatorNew[i].numberOfPeers = _originator[i].numberOfPeers;
			_originatorNew[i].numberOfExpiredPeers = _originator[i].numberOfExpiredPeers;
			_originatorNew[i].numberOfOriginators = _originator[i].numberOfOriginators;
			_originatorNew[i].numberOfActiveNeighbours = _originator[i].numberOfActiveNeighbours;
			_originatorNew[i].lastSequenceNumber =  _originatorNew[i].lastSequenceNumber;
			_originatorNew[i].sequenceNumberProtectionWindowActive = _originator[i].sequenceNumberProtectionWindowActive;
			_originatorNew[i].lastSeen[0] = _originator[i].lastSeen[0];
			_originatorNew[i].lastSeen[1] = _originator[i].lastSeen[1];
			_originatorNew[i].lastSeen[2] = _originator[i].lastSeen[2];
			_originatorNew[i].interval[0] = _originator[i].interval[0];
			_originatorNew[i].interval[1] = _originator[i].interval[1];
			_originatorNew[i].interval[2] = _originator[i].interval[2];
			_originatorNew[i].ogmReceived = _originator[i].ogmReceived;
			_originatorNew[i].ogmReceiptLastConfirmed = _originator[i].ogmReceiptLastConfirmed;
			_originatorNew[i].ogmEchoes = _originator[i].ogmEchoes;
			_originatorNew[i].ogmEchoLastConfirmed = _originator[i].ogmEchoLastConfirmed;
			_originatorNew[i].ltq = _originator[i].ltq;
			_originatorNew[i].gtq = _originator[i].gtq;
			_originatorNew[i].selectedRouter = _originator[i].selectedRouter;
			_originatorNew[i].flags = _originator[i].flags;
			_originatorNew[i].uptime = _originator[i].uptime;
			_originatorNew[i].currentTxPower = _originator[i].currentTxPower;
			_originatorNew[i].currentFreeHeap = _originator[i].currentFreeHeap;
			_originatorNew[i].initialFreeHeap = _originator[i].initialFreeHeap;
			_originatorNew[i].largestFreeBlock = _originator[i].largestFreeBlock;
			_originatorNew[i].heapFragmentation = _originator[i].heapFragmentation;
			_originatorNew[i].rxPackets = _originator[i].rxPackets;
			_originatorNew[i].txPackets = _originator[i].txPackets;
			_originatorNew[i].droppedRxPackets = _originator[i].droppedRxPackets;
			_originatorNew[i].droppedTxPackets = _originator[i].droppedTxPackets;	
		}*/
		memcpy(_originatorNew, _originator, sizeof(m2mMeshOriginatorInfo[_maxNumberOfOriginators]));
		_maxNumberOfOriginators += extraNodes;
		delete[] _originator;			//Delete the old block on heap (ouch)
		_originator = _originatorNew;	//Point the pointer at the new place on heap
		return(true);
	}
	return(false);
}
void ICACHE_FLASH_ATTR m2mMeshClass::enableDynamicGrowth(uint8_t increment)
{
	_allowMeshGrowth = true;
	if(increment > 0)
	{
		_meshGrowthIncrement = increment;
	}
	else
	{
		_meshGrowthIncrement = 1;
	}
	#ifdef m2mMeshIncludeDebugFeatures
	if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NODE_MANAGEMENT)
	{
		_debugStream->printf_P(m2mMeshenableddynamicmeshgrowthinincrementsofu, _meshGrowthIncrement);
	}
	#endif
}
void ICACHE_FLASH_ATTR m2mMeshClass::disableDynamicGrowth()
{
	#ifdef m2mMeshIncludeDebugFeatures
	if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_NODE_MANAGEMENT)
	{
		_debugStream->print(m2mMeshdisableddynamicmeshgrowth);
	}
	#endif
	_allowMeshGrowth = false;
}

void ICACHE_FLASH_ATTR m2mMeshClass::_calculateLtq(const uint8_t originatorId)
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

bool ICACHE_FLASH_ATTR m2mMeshClass::_dataIsValid(const uint8_t originatorId, const uint8_t dataType)
{
	if(originatorId >= _numberOfOriginators)
	{
		return(false);
	}
	else if(_originator[originatorId].lastSeen[dataType] == 0)
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
uint8_t * ICACHE_FLASH_ATTR m2mMeshClass::getMeshAddress()
{
	return(_localMacAddress);
}

uint8_t * ICACHE_FLASH_ATTR m2mMeshClass::getMeshAddress(const uint8_t id)
{
	return(_originator[id].macAddress);
}

bool ICACHE_FLASH_ATTR m2mMeshClass::nodeNameIsSet()
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
bool ICACHE_FLASH_ATTR m2mMeshClass::nodeNameIsSet(const uint8_t id)
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

//Return the local node name
char * ICACHE_FLASH_ATTR m2mMeshClass::getNodeName()
{
	return(_nodeName);
}
//Return the node name for another node
char * ICACHE_FLASH_ATTR m2mMeshClass::getNodeName(const uint8_t originatorId)
{
	return(_originator[originatorId].nodeName);
}

bool ICACHE_FLASH_ATTR m2mMeshClass::joined()
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

uint8_t ICACHE_FLASH_ATTR m2mMeshClass::lastError()
{
	return(_lastError);
}
void ICACHE_FLASH_ATTR m2mMeshClass::clearLastError()
{
	_lastError = m2mMesh_Success;
}
const char* ICACHE_FLASH_ATTR m2mMeshClass::lastErrorDescription()
{
	return(errorDescriptionTable[_lastError]);
}

bool ICACHE_FLASH_ATTR m2mMeshClass::setNodeName(const char *newName)
{
	if(newName != nullptr)	//Check for garbage in
	{
		//If the name is already set, delete it and free up the memory
		if(nodeNameIsSet())
		{
			delete[] _nodeName;
		}
		//Allocate memory for the name, adding one for the null character
		_nodeName = new char[strlen(newName)+1];
		if(_nodeName != nullptr)
		{
			//Copy the node name into the newly allocated memory
			memcpy(_nodeName, newName, strlen(newName) + 1);
			#ifdef m2mMeshIncludeDebugFeatures
			if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_INFORMATION)
			{
				_debugStream->printf_P("\r\nNode name set to '%s'",&newName);
			}
			#endif
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

bool ICACHE_FLASH_ATTR m2mMeshClass::setNodeName(char *newName)
{
	if(newName != nullptr)	//Check for garbage in
	{
		//If the name is already set, delete it and free up the memory
		if(nodeNameIsSet())
		{
			delete[] _nodeName;
		}
		//Allocate memory for the name, adding one for the null character
		_nodeName = new char[strlen(newName)+1];
		if(_nodeName != nullptr)
		{
			//Copy the node name into the newly allocated memory
			memcpy(_nodeName, newName, strlen(newName) + 1);
			#ifdef m2mMeshIncludeDebugFeatures
			if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_INFORMATION)
			{
				_debugStream->printf_P("\r\nNode name set to '%s'",&newName);
			}
			#endif
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


bool ICACHE_FLASH_ATTR m2mMeshClass::setNodeName(String newName)
{
	uint8_t stringLength = newName.length() + 1;
	char tempArray[stringLength+1];
	newName.toCharArray(tempArray, stringLength);
	return(setNodeName(tempArray));
}

/*
 *	Methods related to tracing nodes
 *
 *
 */

bool ICACHE_FLASH_ATTR m2mMeshClass::trace(uint8_t destId, uint32_t timeout)
{
	if(destId < _numberOfOriginators)
	{
		bool wait = true;
		m2mMeshPacketBuffer packet;
		packet.destinationId = destId;
		packet.data[m2mMeshPacketTypeIndex] = TRACE_PACKET_TYPE | TRACE_DEFAULT_OPTIONS;//Packet type
		if(wait == true)
		{
			packet.data[m2mMeshPacketTypeIndex] = packet.data[m2mMeshPacketTypeIndex] | CONFIRM_SEND;
		}
		packet.data[m2mMeshPacketChecksumIndex] = 0;								//The CRC goes here
		packet.data[m2mMeshPacketTTLIndex] = _currentTtl[TRACE_PACKET_TYPE];	//TTL
		memcpy(&packet.data[m2mMeshPacketSNIndex], &_sequenceNumber, sizeof(_sequenceNumber));	//Add the sequence number
		packet.data[m2mMeshPacketOriginatorIndex] = _localMacAddress[0];			//Origin address
		packet.data[m2mMeshPacketOriginatorIndex1] = _localMacAddress[1];
		packet.data[m2mMeshPacketOriginatorIndex2] = _localMacAddress[2];
		packet.data[m2mMeshPacketOriginatorIndex3] = _localMacAddress[3];
		packet.data[m2mMeshPacketOriginatorIndex4] = _localMacAddress[4];
		packet.data[m2mMeshPacketOriginatorIndex5] = _localMacAddress[5];
		packet.data[m2mMeshPacketDestinationIndex] = _originator[destId].macAddress[0];
		packet.data[m2mMeshPacketDestinationIndex1] = _originator[destId].macAddress[1];
		packet.data[m2mMeshPacketDestinationIndex2] = _originator[destId].macAddress[2];
		packet.data[m2mMeshPacketDestinationIndex3] = _originator[destId].macAddress[3];
		packet.data[m2mMeshPacketDestinationIndex4] = _originator[destId].macAddress[4];
		packet.data[m2mMeshPacketDestinationIndex5] = _originator[destId].macAddress[5];
		uint32_t now = millis();
		memcpy(&packet.data[20], &now, sizeof(now));	//Add the sending time
		packet.data[24] = 0; //Log the number of hops
		packet.length = 25;
		packet.data[m2mMeshPacketDataLengthIndex] = packet.length;
		if(_routePacket(packet))
		{
			_sequenceNumber++;
			if(_sendPacket(packet))
			{
				_txPackets++;
				#ifdef m2mMeshIncludeDebugFeatures
				if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_USR_SEND && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || packet.routerId == _nodeToLog || packet.destinationId == _nodeToLog))
				{
					_debugStream->printf_P(m2mMeshTRACEsentR02x02x02x02x02x02xD02x02x02x02x02x02xTTLdLengthd,
						packet.routerMacAddress[0],
						packet.routerMacAddress[1],
						packet.routerMacAddress[2],
						packet.routerMacAddress[3],
						packet.routerMacAddress[4],
						packet.routerMacAddress[5],
						packet.data[m2mMeshPacketDestinationIndex],
						packet.data[m2mMeshPacketDestinationIndex1],
						packet.data[m2mMeshPacketDestinationIndex2],
						packet.data[m2mMeshPacketDestinationIndex3],
						packet.data[m2mMeshPacketDestinationIndex4],
						packet.data[m2mMeshPacketDestinationIndex5],
						packet.data[m2mMeshPacketTTLIndex],
						packet.length);
				}
				#endif
				_traceResponder = MESH_ORIGINATOR_NOT_FOUND;
				if(wait == true)
				{
					uint32_t traceTimer = millis();
					while(_traceResponder == MESH_ORIGINATOR_NOT_FOUND && millis() - traceTimer < timeout)
					{
						if(_receiveBufferIndex != _processBufferIndex ||					//Some data in buffer
							_receiveBuffer[_processBufferIndex].length > 0)					//Buffer is full
						{
							_processPacket(_receiveBuffer[_processBufferIndex]);			//Process the packet
							_processBufferIndex++;												//Advance the buffer index
							_processBufferIndex = _processBufferIndex%M2MMESHRECEIVEBUFFERSIZE;	//Rollover the buffer index
						}
						yield();
					}
					if(_traceResponder == MESH_ORIGINATOR_NOT_FOUND)
					{
						return(false);	//Timed out
					}
					else
					{
						return(_selectTraceResponse(destId));	//Not timed out, but need to check for actual response packet
					}
				}
				else
				{
					return(true);	//Come straight back
				}
			}
			else
			{
				#ifdef m2mMeshIncludeDebugFeatures
				if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_USR_SEND && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || packet.routerId == _nodeToLog || packet.destinationId == _nodeToLog))
				{
					_debugStream->printf_P(m2mMeshTRACEsentR02x02x02x02x02x02xD02x02x02x02x02x02xTTLdLengthd,
						packet.routerMacAddress[0],
						packet.routerMacAddress[1],
						packet.routerMacAddress[2],
						packet.routerMacAddress[3],
						packet.routerMacAddress[4],
						packet.routerMacAddress[5],
						packet.data[m2mMeshPacketDestinationIndex],
						packet.data[m2mMeshPacketDestinationIndex1],
						packet.data[m2mMeshPacketDestinationIndex2],
						packet.data[m2mMeshPacketDestinationIndex3],
						packet.data[m2mMeshPacketDestinationIndex4],
						packet.data[m2mMeshPacketDestinationIndex5],
						packet.data[m2mMeshPacketTTLIndex],
						packet.length);
				}
				#endif
				_droppedTxPackets++;
			}
		}
	}
	return(false);
}

void ICACHE_FLASH_ATTR m2mMeshClass::_processTrace(m2mMeshPacketBuffer &packet)
{
	#ifdef m2mMeshIncludeDebugFeatures
	if(_debugEnabled == true && (_loggingLevel & MESH_UI_LOG_TRC_RECEIVED) && (_nodeToLog == MESH_ORIGINATOR_NOT_FOUND || packet.routerId == _nodeToLog || packet.originatorId == _nodeToLog))
	{
		if(packet.data[m2mMeshPacketTypeIndex] & RESPONSE)
		{
			_debugStream->printf_P(m2mMeshTRACEresponsereceivedR02x02x02x02x02x02xO02x02x02x02x02x02xTTLdLengthd,
				packet.routerMacAddress[0],
				packet.routerMacAddress[1],
				packet.routerMacAddress[2],
				packet.routerMacAddress[3],
				packet.routerMacAddress[4],
				packet.routerMacAddress[5],
				_originator[packet.originatorId].macAddress[0],
				_originator[packet.originatorId].macAddress[1],
				_originator[packet.originatorId].macAddress[2],
				_originator[packet.originatorId].macAddress[3],
				_originator[packet.originatorId].macAddress[4],
				_originator[packet.originatorId].macAddress[5],
				packet.data[m2mMeshPacketTTLIndex],
				packet.length);
		}
		else
		{
			_debugStream->printf_P(m2mMeshTRACEreceivedR02x02x02x02x02x02xO02x02x02x02x02x02xTTLdLengthd,
				packet.routerMacAddress[0],
				packet.routerMacAddress[1],
				packet.routerMacAddress[2],
				packet.routerMacAddress[3],
				packet.routerMacAddress[4],
				packet.routerMacAddress[5],
				_originator[packet.originatorId].macAddress[0],
				_originator[packet.originatorId].macAddress[1],
				_originator[packet.originatorId].macAddress[2],
				_originator[packet.originatorId].macAddress[3],
				_originator[packet.originatorId].macAddress[4],
				_originator[packet.originatorId].macAddress[5],
				packet.data[m2mMeshPacketTTLIndex],
				packet.length);
		}
	}
	#endif
	if(packet.data[m2mMeshPacketTypeIndex] & RESPONSE)	//This is a trace response for a trace this node sent
	{
		if(_applicationBuffer[_applicationBufferWriteIndex].length == 0) //There's a free slot in the app buffer
		{
			memcpy(&_applicationBuffer[_applicationBufferWriteIndex].routerMacAddress, &packet.routerMacAddress, 6);
			_applicationBuffer[_applicationBufferWriteIndex].length = packet.length;
			memcpy(&_applicationBuffer[_applicationBufferWriteIndex].data, &packet.data, packet.length);
			_applicationBuffer[_applicationBufferWriteIndex].routerId = packet.routerId;
			_applicationBuffer[_applicationBufferWriteIndex].originatorId = packet.originatorId;
			_applicationBuffer[_applicationBufferWriteIndex].destinationId = packet.destinationId;
			_applicationBuffer[_applicationBufferWriteIndex].timestamp = packet.timestamp;
			#ifdef m2mMeshIncludeDebugFeatures
			if(_debugEnabled == true && (_loggingLevel & MESH_UI_LOG_BUFFER_MANAGEMENT))
			{
				_debugStream->printf_P(m2mMeshfillAPPbufferslotd,
					_applicationBufferWriteIndex,
					packet.length,
					packetTypeDescription[packet.data[m2mMeshPacketTypeIndex] & 0x07],
					packet.routerMacAddress[0],
					packet.routerMacAddress[1],
					packet.routerMacAddress[2],
					packet.routerMacAddress[3],
					packet.routerMacAddress[4],
					packet.routerMacAddress[5],
					_originator[packet.originatorId].macAddress[0],
					_originator[packet.originatorId].macAddress[1],
					_originator[packet.originatorId].macAddress[2],
					_originator[packet.originatorId].macAddress[3],
					_originator[packet.originatorId].macAddress[4],
					_originator[packet.originatorId].macAddress[5]);
			}
			#endif
			_applicationBufferWriteIndex++;																//Advance the buffer index
			_applicationBufferWriteIndex = _applicationBufferWriteIndex%M2MMESHAPPLICATIONBUFFERSIZE;	//Rollover buffer index
			if(eventCallback)
			{
				eventCallback(meshEvent::trace);
			}

		}
		else
		{
			#ifdef m2mMeshIncludeDebugFeatures
			if(_debugEnabled == true && ((_loggingLevel & MESH_UI_LOG_BUFFER_MANAGEMENT) || (_loggingLevel & MESH_UI_LOG_ERRORS)))
			{
				_debugStream->print(m2mMeshreadAPPbufferfull);
			}
			#endif
			_droppedAppPackets++;
		}
		_traceResponder = packet.originatorId;
		packet.data[m2mMeshPacketTTLIndex] = 0;	//Smash the TTL to 0 to stop an endless forwarding loop
	}
	else //This node is the target of the trace, turn the packet around and forward it back
	{
		packet.data[m2mMeshPacketTypeIndex] = packet.data[m2mMeshPacketTypeIndex] | RESPONSE;							//Mark it as a response
		memcpy(&packet.data[m2mMeshPacketDestinationIndex], &packet.data[m2mMeshPacketOriginatorIndex], 6);						//Set destination from the source
		packet.destinationId = packet.originatorId;							//Set destination from the source
		packet.originatorId = MESH_THIS_ORIGINATOR;							//Set source as this node
		memcpy(&packet.data[m2mMeshPacketOriginatorIndex], _localMacAddress, 6);						//Set source as this node
		packet.data[m2mMeshPacketTTLIndex] = _currentTtl[TRACE_PACKET_TYPE];						//Reset the TTL
		memcpy(&packet.data[m2mMeshPacketSNIndex], &_sequenceNumber, sizeof(_sequenceNumber));	//Add the sequence number for this node
		_sequenceNumber++;	//Increment this node's sequence number
		if(packet.length < ESP_NOW_MAX_PACKET_SIZE - 6)
		{
			uint8_t hopIndex = 25 + packet.data[24];					//Start overwriting any padding
			memcpy(&packet.data[hopIndex], _localMacAddress, 6);		//Copy in this node's address
			if(packet.length < hopIndex + 6)
			{
				packet.length = hopIndex + 6;							//Increase the length to include extra hop info
			}
			packet.data[24]++;	//Increase the logged hop count
		}
	}
}

bool ICACHE_FLASH_ATTR m2mMeshClass::_selectTraceResponse(uint8_t originatorId)
{
	for(uint8_t i = 0 ; i < M2MMESHAPPLICATIONBUFFERSIZE ; i++)
	{
		if(_applicationBuffer[_applicationBufferReadIndex].length > 0 &&
			((_applicationBuffer[_applicationBufferReadIndex].data[m2mMeshPacketTypeIndex] & 0x07) == TRACE_PACKET_TYPE) &&
			_applicationBuffer[_applicationBufferReadIndex].originatorId == originatorId)
		{
			return(true);	//Found the trace response we're waiting for
		}
		_applicationBufferReadIndex++;									//Advance the buffer index
		_applicationBufferReadIndex = _applicationBufferReadIndex%M2MMESHAPPLICATIONBUFFERSIZE;	//Rollover buffer index
	}
	return(false);
}

uint32_t ICACHE_FLASH_ATTR m2mMeshClass::traceTime()
{
	if((_applicationBuffer[_applicationBufferReadIndex].data[m2mMeshPacketTypeIndex] & 0x07) == TRACE_PACKET_TYPE)
	{
		uint32_t sendTime;
		memcpy(&sendTime, &_applicationBuffer[_applicationBufferReadIndex].data[20], sizeof(sendTime));
		return(millis() - sendTime);
	}
	return(0xFFFFFFFF);	//False return time
}

uint8_t ICACHE_FLASH_ATTR m2mMeshClass::traceHops()
{
	if((_applicationBuffer[_applicationBufferReadIndex].data[m2mMeshPacketTypeIndex] & 0x07) == TRACE_PACKET_TYPE)
	{
		return(_applicationBuffer[_applicationBufferReadIndex].data[24]);
	}
	return(0xff);
}

bool ICACHE_FLASH_ATTR m2mMeshClass::retrieveTraceHop(uint8_t hop, uint8_t* macAddress)
{
	if((_applicationBuffer[_applicationBufferReadIndex].data[m2mMeshPacketTypeIndex] & 0x07) == TRACE_PACKET_TYPE && hop < _applicationBuffer[_applicationBufferReadIndex].data[24])
	{
		memcpy(macAddress,&_applicationBuffer[_applicationBufferReadIndex].data[25+hop*6],6);
		return(true);
	}
	return(false);
}

void ICACHE_FLASH_ATTR m2mMeshClass::markTraceRead()
{
	if(_traceResponder != MESH_ORIGINATOR_NOT_FOUND)
	{
		#ifdef m2mMeshIncludeDebugFeatures
		if(_debugEnabled == true && (_loggingLevel & MESH_UI_LOG_BUFFER_MANAGEMENT))
		{
			_debugStream->printf_P(m2mMeshreadAPPbufferslotd,
				_applicationBufferReadIndex,
				_applicationBuffer[_applicationBufferReadIndex].length,
				packetTypeDescription[_applicationBuffer[_applicationBufferReadIndex].data[m2mMeshPacketTypeIndex] & 0x07],
				_applicationBuffer[_applicationBufferReadIndex].routerMacAddress[0],
				_applicationBuffer[_applicationBufferReadIndex].routerMacAddress[1],
				_applicationBuffer[_applicationBufferReadIndex].routerMacAddress[2],
				_applicationBuffer[_applicationBufferReadIndex].routerMacAddress[3],
				_applicationBuffer[_applicationBufferReadIndex].routerMacAddress[4],
				_applicationBuffer[_applicationBufferReadIndex].routerMacAddress[5]);
		}
		#endif
		if(_selectTraceResponse(_traceResponder))
		{
			_applicationBuffer[_applicationBufferReadIndex].length = 0;								//Mark buffer slot empty
			_applicationBufferReadIndex++;															//Advance the buffer index
			_applicationBufferReadIndex = _applicationBufferReadIndex%M2MMESHAPPLICATIONBUFFERSIZE;	//Rollover buffer index
		}
		_traceResponder = MESH_ORIGINATOR_NOT_FOUND;
	}
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
 
void ICACHE_FLASH_ATTR m2mMeshClass::_buildUserPacketHeader(uint8_t destId)
{
	if(not _buildingUserPacket)
	{
		_userPacketBuffer.data[m2mMeshPacketTypeIndex] = USR_PACKET_TYPE | USR_DEFAULT_OPTIONS;
		_userPacketBuffer.data[m2mMeshPacketTTLIndex] = _currentTtl[USR_PACKET_TYPE];
		//The sequence number is put in at time of sending
		//Origin address
		_userPacketBuffer.data[m2mMeshPacketOriginatorIndex] = _localMacAddress[0];
		_userPacketBuffer.data[m2mMeshPacketOriginatorIndex1] = _localMacAddress[1];
		_userPacketBuffer.data[m2mMeshPacketOriginatorIndex2] = _localMacAddress[2];
		_userPacketBuffer.data[m2mMeshPacketOriginatorIndex3] = _localMacAddress[3];
		_userPacketBuffer.data[m2mMeshPacketOriginatorIndex4] = _localMacAddress[4];
		_userPacketBuffer.data[m2mMeshPacketOriginatorIndex5] = _localMacAddress[5];
		//From here the packet specification is flexible, so use an incrementing index
		_userPacketBuffer.length = 14;
		if(destId != MESH_ORIGINATOR_NOT_FOUND)
		{
			_userPacketBuffer.data[m2mMeshPacketTypeIndex] = _userPacketBuffer.data[m2mMeshPacketTypeIndex] & ~SEND_TO_ALL_NODES;
			_userPacketBuffer.destinationId = destId;
			_userPacketBuffer.data[_userPacketBuffer.length++] = _originator[destId].macAddress[0];
			_userPacketBuffer.data[_userPacketBuffer.length++] = _originator[destId].macAddress[1];
			_userPacketBuffer.data[_userPacketBuffer.length++] = _originator[destId].macAddress[2];
			_userPacketBuffer.data[_userPacketBuffer.length++] = _originator[destId].macAddress[3];
			_userPacketBuffer.data[_userPacketBuffer.length++] = _originator[destId].macAddress[4];
			_userPacketBuffer.data[_userPacketBuffer.length++] = _originator[destId].macAddress[5];
		}
		//Interval
		memcpy(&_userPacketBuffer.data[_userPacketBuffer.length], &_currentInterval[USR_PACKET_TYPE], sizeof(_currentInterval[USR_PACKET_TYPE]));
		_userPacketBuffer.length+=sizeof(_currentInterval[USR_PACKET_TYPE]);
		//Number of fields, record where in the packet it is stored
		_userPacketFieldCounterIndex = _userPacketBuffer.length;
		_userPacketBuffer.data[_userPacketBuffer.length++] = 0;
		_buildingUserPacket = true;
	}
}

void ICACHE_FLASH_ATTR m2mMeshClass::_buildUserPacketHeader(const uint8_t mac0, const uint8_t mac1, const uint8_t mac2, const uint8_t mac3, const uint8_t mac4, const uint8_t mac5)
{
	_buildUserPacketHeader(_originatorIdFromMac(mac0, mac1, mac2, mac3, mac4, mac5));
}

uint8_t ICACHE_FLASH_ATTR m2mMeshClass::payloadLeft()
{
	_buildUserPacketHeader();
	return(ESP_NOW_MAX_PACKET_SIZE - _userPacketBuffer.length);
}

//Functions for setting destination on a packet before sending. These are overloaded to make it easier to set a destination.
//They must be called before adding data to a packet

bool ICACHE_FLASH_ATTR m2mMeshClass::destination(const uint8_t mac0, const uint8_t mac1, const uint8_t mac2, const uint8_t mac3, const uint8_t mac4, const uint8_t mac5)	//Add a destination MAC address
{
	uint8_t destId = _originatorIdFromMac(mac0, mac1, mac2, mac3, mac4, mac5);
	if(destId < _numberOfOriginators)
	{
		_buildUserPacketHeader(destId);
		return(true);
	}
	return(false);
}


bool ICACHE_FLASH_ATTR m2mMeshClass::destination(uint8_t destId)
{
	if(destId < _numberOfOriginators)
	{
		_buildUserPacketHeader(destId);
		return(true);
	}
	return(false);
}

bool ICACHE_FLASH_ATTR m2mMeshClass::destination(char *nodeName)
{
	if(nodeName == nullptr)
	{
		return(false);
	}
	for(uint8_t destId = 0; destId < _numberOfOriginators; destId++)
	{
		if(_originator[destId].nodeName != nullptr && strcmp(nodeName,_originator[destId].nodeName) == 0)
		{
			_buildUserPacketHeader(destId);
			return(true);
		}
	}
	return(false);
}

bool ICACHE_FLASH_ATTR m2mMeshClass::destination(String nodeName)
{
	for(uint8_t destId = 0; destId < _numberOfOriginators; destId++)
	{
		if(nodeName.equals(String(_originator[destId].nodeName)))
		{
			_buildUserPacketHeader(destId);
			return(true);
		}
	}
	return(false);
}

bool ICACHE_FLASH_ATTR m2mMeshClass::add(String dataToAdd)
{
	_buildUserPacketHeader();
	uint16_t numberOfElements = dataToAdd.length();
	if(numberOfElements == 0)
	{
		_userPacketBuffer.data[_userPacketBuffer.length++] = USR_DATA_STRING | 0xf0;
		_userPacketBuffer.data[_userPacketBuffer.length++] = 0;
		_userPacketBuffer.data[_userPacketFieldCounterIndex] = _userPacketBuffer.data[_userPacketFieldCounterIndex] + 1;	//Increment the field counter
		return(true);
	}
	else if(_userPacketBuffer.length + numberOfElements + 1 < ESP_NOW_MAX_PACKET_SIZE)
	{
		if(numberOfElements < 15)
		{
			if(_userPacketBuffer.length + numberOfElements < ESP_NOW_MAX_PACKET_SIZE)
			{
				_userPacketBuffer.data[_userPacketBuffer.length++] = USR_DATA_STRING | (numberOfElements<<4);	//Mark that this field has 15 members or fewer
			}
			else
			{
				return(false);	//Not enough space left in the packet
			}
		}
		else
		{
			if(_userPacketBuffer.length + numberOfElements + 1 < ESP_NOW_MAX_PACKET_SIZE)
			{
				_userPacketBuffer.data[_userPacketBuffer.length++] = USR_DATA_STRING | 0xf0;	//Mark that this field has 15 members or more
				_userPacketBuffer.data[_userPacketBuffer.length++] = numberOfElements;
			}
			else
			{
				return(false);	//Not enough space left in the packet
			}
		}
		for(int i = 0 ; i < numberOfElements ; i++)
		{
			_userPacketBuffer.data[_userPacketBuffer.length++] = dataToAdd[i];
		}
		_userPacketBuffer.data[_userPacketFieldCounterIndex] = _userPacketBuffer.data[_userPacketFieldCounterIndex] + 1;	//Increment the field counter
		return(true);
	}
	else
	{
		return(false);
	}
}

bool ICACHE_FLASH_ATTR m2mMeshClass::add(const char *dataToAdd)
{
	_buildUserPacketHeader();
	if(dataToAdd == nullptr && _userPacketBuffer.length + 2 < ESP_NOW_MAX_PACKET_SIZE)// || strlen(dataToAdd) == 0)	//Handle the edge case of being passed an empty string
	{
		_userPacketBuffer.data[_userPacketBuffer.length++] = USR_DATA_STR;
		_userPacketBuffer.data[_userPacketBuffer.length++] = 0;
		//Increment the field counter
		_userPacketBuffer.data[_userPacketFieldCounterIndex] = _userPacketBuffer.data[_userPacketFieldCounterIndex] + 1;
		return(true);
	}
	else
	{
		uint16_t numberOfElements = strlen(dataToAdd);
		if(numberOfElements < 15)
		{
			if(_userPacketBuffer.length + numberOfElements < ESP_NOW_MAX_PACKET_SIZE)
			{
				_userPacketBuffer.data[_userPacketBuffer.length++] = USR_DATA_STR | (numberOfElements<<4);	//Mark that this field has 15 members or fewer
			}
			else
			{
				return(false);	//Not enough space left in the packet
			}
		}
		else
		{
			if(_userPacketBuffer.length + numberOfElements + 1 < ESP_NOW_MAX_PACKET_SIZE)
			{
				_userPacketBuffer.data[_userPacketBuffer.length++] = USR_DATA_STR | 0xf0;	//Mark that this field has 15 members or more
				_userPacketBuffer.data[_userPacketBuffer.length++] = numberOfElements;
			}
			else
			{
				return(false);	//Not enough space left in the packet
			}
		}
		memcpy(&_userPacketBuffer.data[_userPacketBuffer.length],dataToAdd,numberOfElements);							//Copy in the data
		_userPacketBuffer.length+=numberOfElements;															//Advance the index past the data
		_userPacketBuffer.data[_userPacketFieldCounterIndex] = _userPacketBuffer.data[_userPacketFieldCounterIndex] + 1;	//Increment the field counter
		return(true);
	}
}

bool ICACHE_FLASH_ATTR m2mMeshClass::add(char *dataToAdd)
{
	_buildUserPacketHeader();
	if(dataToAdd == nullptr && _userPacketBuffer.length + 2 < ESP_NOW_MAX_PACKET_SIZE)// || strlen(dataToAdd) == 0)	//Handle the edge case of being passed an empty string
	{
		_userPacketBuffer.data[_userPacketBuffer.length++] = USR_DATA_STR;
		_userPacketBuffer.data[_userPacketBuffer.length++] = 0;
		//Increment the field counter
		_userPacketBuffer.data[_userPacketFieldCounterIndex] = _userPacketBuffer.data[_userPacketFieldCounterIndex] + 1;
		return(true);
	}
	else
	{
		uint16_t numberOfElements = strlen(dataToAdd);
		if(numberOfElements < 15)
		{
			if(_userPacketBuffer.length + numberOfElements < ESP_NOW_MAX_PACKET_SIZE)
			{
				_userPacketBuffer.data[_userPacketBuffer.length++] = USR_DATA_STR | (numberOfElements<<4);	//Mark that this field has 15 members or fewer
			}
			else
			{
				return(false);	//Not enough space left in the packet
			}
		}
		else
		{
			if(_userPacketBuffer.length + numberOfElements + 1 < ESP_NOW_MAX_PACKET_SIZE)
			{
				_userPacketBuffer.data[_userPacketBuffer.length++] = USR_DATA_STR | 0xf0;	//Mark that this field has 15 members or more
				_userPacketBuffer.data[_userPacketBuffer.length++] = numberOfElements;
			}
			else
			{
				return(false);	//Not enough space left in the packet
			}
		}
		memcpy(&_userPacketBuffer.data[_userPacketBuffer.length],dataToAdd,numberOfElements);							//Copy in the data
		_userPacketBuffer.length+=numberOfElements;															//Advance the index past the data
		_userPacketBuffer.data[_userPacketFieldCounterIndex] = _userPacketBuffer.data[_userPacketFieldCounterIndex] + 1;	//Increment the field counter
		return(true);
	}
}


/*	
 *	ESP-Now packets are not ACTUALLY sent until the underlying RTOS gets round to it.
 *
 *	Sending ESP-Now has an optional callback function that acknowledges sending.
 *
 *	By default m2mMeshClass::send(bool wait) waits for this callback to happen, which makes it MUCH more likely the packet is sent succesfully.
 *
 *	It also means the result of the callback can be fed back to the calling method. It is possible for sending to fail.
 *
 *	If you use send(false) it will return faster but the result will almost always be true and
 *	if the script then immediately does slow, blocking activity like accessing an SD card or other peripheral
 *	the packet may not actually get sent.
 */

bool ICACHE_FLASH_ATTR m2mMeshClass::send(bool wait)
{
	if(wait == true)
	{
		_userPacketBuffer.data[m2mMeshPacketTypeIndex] = _userPacketBuffer.data[m2mMeshPacketTypeIndex] | CONFIRM_SEND;
	}
	//Sequence number is added immediately before routing
	memcpy(&_userPacketBuffer.data[m2mMeshPacketSNIndex], &_sequenceNumber, sizeof(_sequenceNumber));
	_sequenceNumber++;
	#ifdef m2mMeshIncludeDebugFeatures
	if (_debugEnabled == true && _loggingLevel & MESH_UI_LOG_ALL_SENT_PACKETS)
	{
		_debugStream->printf_P(m2mMeshSendingpackettype02x,_userPacketBuffer.data[m2mMeshPacketTypeIndex] & 0x07);
	}
	#endif
	_userPacketBuffer.data[m2mMeshPacketDataLengthIndex] = _userPacketBuffer.length;
	if(_routePacket(_userPacketBuffer) == true)	//Choose the best router for this packet
	{
		if(_sendPacket(_userPacketBuffer) == true)
		{
			#ifdef m2mMeshIncludeDebugFeatures
			if (_debugEnabled == true && _loggingLevel & MESH_UI_LOG_USR_SEND)
			{
				_debugStream->printf_P(m2mMeshUSRSNDO02x02x02x02x02x02xTTL02dFlags02x,_userPacketBuffer.data[m2mMeshPacketOriginatorIndex],_userPacketBuffer.data[m2mMeshPacketOriginatorIndex1],_userPacketBuffer.data[m2mMeshPacketOriginatorIndex2],_userPacketBuffer.data[m2mMeshPacketOriginatorIndex3],_userPacketBuffer.data[m2mMeshPacketOriginatorIndex4],_userPacketBuffer.data[m2mMeshPacketOriginatorIndex5],_userPacketBuffer.data[m2mMeshPacketTTLIndex],_userPacketBuffer.data[m2mMeshPacketTypeIndex] & 0xf8);
			}
			#endif
			_userPacketBuffer.length = 0;
			_buildingUserPacket = false;
			return(true);
		}
	}
	else
	{
		if (_debugEnabled == true)
		{
			_debugStream->printf("\r\nRouting failure: code %u \"%s\"", _lastError,errorDescriptionTable[_lastError]);
			
		}
	}
	_userPacketBuffer.length = 0;
	_buildingUserPacket = false;
	_waitingForSend = false;
	_droppedTxPackets++;	//Update the packet stats
	return(false);
}

bool ICACHE_FLASH_ATTR m2mMeshClass::_routePacket(m2mMeshPacketBuffer &packet)
{
	if(packet.data[m2mMeshPacketTypeIndex] & SEND_TO_ALL_NODES)
	{
		memcpy(&packet.routerMacAddress, &_broadcastMacAddress,6);	//Nothing needs doing beyond adding broadcast address
		packet.routerId = MESH_ORIGINATOR_NOT_FOUND;
		return(true);
	}
	else if(packet.destinationId < _numberOfOriginators)
	{
		if(elpIsValid(packet.destinationId) && _originator[packet.destinationId].ltq > _originator[packet.destinationId].gtq)
		{
			packet.routerId = packet.destinationId;	//Send direct, as ELP is valid
		}
		else if(ogmIsValid(packet.destinationId))
		{
			if(_originator[packet.destinationId].selectedRouter == MESH_ORIGINATOR_NOT_FOUND)
			{
				_lastError = m2mMesh_NoIndirectRoute;
				packet.routerId = MESH_ORIGINATOR_NOT_FOUND;	//Fail to route
				return(false);
			}
			else
			{
				packet.routerId = _originator[packet.destinationId].selectedRouter;	//Select the best router
			}
		}
		else
		{
			_lastError = m2mMesh_NoDirectRoute;
			packet.routerId = MESH_ORIGINATOR_NOT_FOUND;	//Fail to route
			return(false);
		}
		#ifdef m2mMeshIncludeDebugFeatures
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_PACKET_ROUTING)
		{
			if(packet.routerId == MESH_ORIGINATOR_NOT_FOUND)
			{
				_debugStream->printf("\r\nRTR for node %u D:%02x:%02x:%02x:%02x:%02x:%02x is flood R:ff:ff:ff:ff:ff:ff",
				packet.destinationId,
				_originator[packet.destinationId].macAddress[0],
				_originator[packet.destinationId].macAddress[1],
				_originator[packet.destinationId].macAddress[2],
				_originator[packet.destinationId].macAddress[3],
				_originator[packet.destinationId].macAddress[4],
				_originator[packet.destinationId].macAddress[5]
				);
			}
			else
			{
				_debugStream->printf("\r\nRTR for node %u D:%02x:%02x:%02x:%02x:%02x:%02x is node %u R:%02x:%02x:%02x:%02x:%02x:%02x",
				packet.destinationId,
				_originator[packet.destinationId].macAddress[0],
				_originator[packet.destinationId].macAddress[1],
				_originator[packet.destinationId].macAddress[2],
				_originator[packet.destinationId].macAddress[3],
				_originator[packet.destinationId].macAddress[4],
				_originator[packet.destinationId].macAddress[5],
				packet.routerId,
				_originator[packet.routerId].macAddress[0],
				_originator[packet.routerId].macAddress[1],
				_originator[packet.routerId].macAddress[2],
				_originator[packet.routerId].macAddress[3],
				_originator[packet.routerId].macAddress[4],
				_originator[packet.routerId].macAddress[5]
				);
			}
		}
		#endif
		if(packet.routerId == MESH_ORIGINATOR_NOT_FOUND)
		{
			memcpy(&packet.routerMacAddress, &_broadcastMacAddress,6);	//Nothing needs doing beyond adding broadcast address
			return(true);
		}
		else
		{
			memcpy(&packet.routerMacAddress, _originator[packet.routerId].macAddress, 6);//Send direct to the next hop router MAC address
			if(elpIsValid(packet.routerId))
			{
				if(_originator[packet.routerId].isCurrentlyPeer == false && _addPeer(packet.routerId) == false)
				{
					_lastError = m2mMesh_UnableToPeerLocally;	//Can't add the local peer
					return(false);
				}
				_originator[packet.routerId].peerNeeded = millis();	//Update the record of when the peer was last needed
				if(_originator[packet.routerId].isCurrentlyPeer == true && _originator[packet.routerId].hasUsAsPeer == false)
				{
					if(_requestPeering(packet.routerId, bool(packet.data[m2mMeshPacketTypeIndex] & CONFIRM_SEND)) == false) //Need to wait for peering, or not, is encoded in the packet
					{
						_lastError = m2mMesh_UnableToConfirmNextHopPeer;
						return(false);
					}
				}
			}
			else
			{
				_lastError = m2mMesh_NoDirectRoute;	//No ELP means no direct route
				return(false);
			}
		}
		return(true);
	}
	else
	{
		_lastError = m2mMesh_UnknownNode;
		return(false);
	}
}
bool IRAM_ATTR m2mMeshClass::_sendPacket(m2mMeshPacketBuffer &packet)
{
	if(packet.length == 0)
	{
		#ifdef m2mMeshIncludeDebugFeatures
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_WARNINGS)
		{
			_debugStream->print(m2mMeshskippingsendingzerolengthpacket);
		}
		#endif
		return(true);
	}
	WiFiMode_t currentWiFiMode = WiFi.getMode();
	if(currentWiFiMode == WIFI_AP || currentWiFiMode == WIFI_AP_STA)
	{
		packet.data[m2mMeshPacketTypeIndex] = packet.data[m2mMeshPacketTypeIndex] | SENDER_IS_SOFTAP;
	}
	while(packet.length < ESP_NOW_MIN_PACKET_SIZE)	//Pad the packet out to the minimum size with garbage
	{
		packet.data[packet.length++] = 0x00;
	}
	_addChecksum(packet);	//Checksum is added immediately before any send or forward
	if(_activityLedEnabled)
	{
		_blinkActivityLed();
	}
	if(packet.data[m2mMeshPacketTypeIndex] & CONFIRM_SEND)	//This is a synchronous send, the default which is safest
	{
		_sendtimer = 0;
		_waitingForSend = true;
		_sendSuccess = false;
	}
	#if defined(ESP8266)
	int8_t sendResult = esp_now_send(packet.routerMacAddress, packet.data, packet.length);
	#elif defined(ESP32)
	esp_err_t sendResult = esp_now_send(packet.routerMacAddress, packet.data, packet.length);
	#endif
	if(sendResult == ESP_OK)
	{
		if(packet.data[m2mMeshPacketTypeIndex] & CONFIRM_SEND)//If this is a synchronous send, wait here for the callback to acknowledge sending
		{
			uint32_t now = millis();
			while(_waitingForSend == true && millis() - now < _sendTimeout)
			{
				delay(1);				//Yield to system for handling callbacks
			}
			if(_sendSuccess == true)
			{
				_txPackets++;			//Update the packet stats
				return(true);			//Feed back the result from the callback function
			}
			else if(millis() - now >= _sendTimeout)
			{
				_droppedTxPackets++;	//Update the packet stats
				_lastError = m2mMesh_PeerAckTimeout;
				return(false);			//Feed back the result from the callback function
			}
			else
			{
				_droppedTxPackets++;	//Update the packet stats
				_lastError = m2mMesh_CannotConfirmSend;
				return(false);			//Feed back the result from the callback function
			}
		}
		else
		{
			_txPackets++;				//Update the packet stats
			return(true);
		}
	}
	else
	{
		_waitingForSend = false;
		_droppedTxPackets++;	//Update the packet stats
		_lastError = m2mMesh_CannotSend;
		#ifdef m2mMeshIncludeDebugFeatures
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_ERRORS)
		{
			_printEspNowErrorDescription(sendResult);
		}
		#endif
		return(false);
	}
}
bool ICACHE_FLASH_ATTR m2mMeshClass::_addPeer(uint8_t originatorId)
{
	if(esp_now_is_peer_exist(_originator[originatorId].macAddress) == true)
	{
		#ifdef m2mMeshIncludeDebugFeatures
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_PEER_MANAGEMENT)
		{
			_debugStream->printf_P(m2mMeshPeer02u02x02x02x02x02x02xalreadyaddednotaddingagain,
				originatorId,
				_originator[originatorId].macAddress[0],
				_originator[originatorId].macAddress[1],
				_originator[originatorId].macAddress[2],
				_originator[originatorId].macAddress[3],
				_originator[originatorId].macAddress[4],
				_originator[originatorId].macAddress[5]);
		}
		#endif
		if(_originator[originatorId].isCurrentlyPeer == false)
		{
			_numberOfPeers++;
		}
		_originator[originatorId].peerNeeded = millis();
		_originator[originatorId].isCurrentlyPeer = true;
		return (true);
	}
	if(_addPeer(_originator[originatorId].macAddress, _currentChannel) == true)
	{
		_originator[originatorId].peerNeeded = millis();
		_originator[originatorId].isCurrentlyPeer = true;
		return(true);
	}
	else
	{
		return(false);
	}
}
bool ICACHE_FLASH_ATTR m2mMeshClass::_addPeer(uint8_t* macAddress, uint8_t peerChannel)
{
	#ifdef m2mMeshIncludeDebugFeatures
	if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_PEER_MANAGEMENT)
	{
		_debugStream->printf_P(m2mMeshaddingpeerxxxxxxchannelu,macAddress[0],macAddress[1],macAddress[2],macAddress[3],macAddress[4],macAddress[5],peerChannel, _numberOfPeers + 1, _maxNumberOfPeers);
	}
	#endif
	if(_numberOfPeers < _maxNumberOfPeers)
	{

		//The API differs between ESP8266 and ESP32, so needs some conditional compilation
		#if defined(ESP8266)
		int result = esp_now_add_peer(macAddress, ESP_NOW_ROLE_COMBO, peerChannel, NULL, 0);
		if(result == ESP_OK)
		#elif defined(ESP32)
		esp_now_peer_info_t newPeer;
		newPeer.peer_addr[0] = (uint8_t) macAddress[0];
		newPeer.peer_addr[1] = (uint8_t) macAddress[1];
		newPeer.peer_addr[2] = (uint8_t) macAddress[2];
		newPeer.peer_addr[3] = (uint8_t) macAddress[3];
		newPeer.peer_addr[4] = (uint8_t) macAddress[4];
		newPeer.peer_addr[5] = (uint8_t) macAddress[5];
		if(WiFi.getMode() == WIFI_STA)
		{
			newPeer.ifidx = WIFI_IF_STA;
		}
		else
		{
			newPeer.ifidx = WIFI_IF_AP;
		}
		newPeer.channel = peerChannel;
		newPeer.encrypt = false;
		int result = esp_now_add_peer(&newPeer);
		if(result == ESP_OK)
		#endif
		{
			#ifdef m2mMeshIncludeDebugFeatures
			if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_PEER_MANAGEMENT)
			{
				_debugStream->print(m2mMeshsuccess);
			}
			#endif
			_numberOfPeers++;
			return(true);
		}
		#ifdef m2mMeshIncludeDebugFeatures
		else if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_PEER_MANAGEMENT)
		{
			_debugStream->print(m2mMeshfailed_code_);
			_printEspNowErrorDescription(result);
		}
		#endif
	}
	#ifdef m2mMeshIncludeDebugFeatures
	if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_PEER_MANAGEMENT)
	{
		_debugStream->print(m2mMeshfailed);
	}
	#endif
	return(false);
}

bool ICACHE_FLASH_ATTR m2mMeshClass::_removePeer(uint8_t originatorId)
{
	#ifdef m2mMeshIncludeDebugFeatures
	if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_PEER_MANAGEMENT)
	{
		_debugStream->printf_P(m2mMeshremovingpeerxxxxxx,
			_originator[originatorId].macAddress[0],
			_originator[originatorId].macAddress[1],
			_originator[originatorId].macAddress[2],
			_originator[originatorId].macAddress[3],
			_originator[originatorId].macAddress[4],
			_originator[originatorId].macAddress[5],
			_numberOfPeers,
			_maxNumberOfPeers);
	}
	#endif
	if(esp_now_is_peer_exist(_originator[originatorId].macAddress) == false)
	{
		#ifdef m2mMeshIncludeDebugFeatures
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_PEER_MANAGEMENT)
		{
			_debugStream->printf(m2mMeshalreadyremoved);
		}
		#endif
		if(_originator[originatorId].isCurrentlyPeer == true)
		{
			_numberOfPeers--;
		}
		_originator[originatorId].isCurrentlyPeer = false;
		return (true);
	}
	int result = esp_now_del_peer(&_originator[originatorId].macAddress[0]);
	if(result == ESP_OK)
	{
		#ifdef m2mMeshIncludeDebugFeatures
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_PEER_MANAGEMENT)
		{
			_debugStream->printf(m2mMeshsuccess);
		}
		#endif
		//Reduce the recorded number of peers
		if(_originator[originatorId].isCurrentlyPeer == true)
		{
			_numberOfPeers--;
		}
		_originator[originatorId].isCurrentlyPeer = false;
		return (true);
	}
	#ifdef m2mMeshIncludeDebugFeatures
	else if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_PEER_MANAGEMENT)
	{
		_debugStream->print(m2mMeshfailed_code_);
		_printEspNowErrorDescription(result);
	}
	#endif
	_originator[originatorId].peerNeeded = millis();	//Avoid instantly retrying this
	return(false);
}


bool ICACHE_FLASH_ATTR m2mMeshClass::clearMessage()	//Clear the message without sending, return true if there was something to clear
{
	_buildingUserPacket = false;
	if(_userPacketBuffer.length > 0)
	{
		_userPacketBuffer.length = 0;
		return(true);
	}
	else
	{
		return(false);
	}
}

//Functions for retrieving data from a message in a user sketch

bool ICACHE_FLASH_ATTR m2mMeshClass::messageWaiting()
{
	for(uint8_t i = 0 ; i < M2MMESHAPPLICATIONBUFFERSIZE ; i++)
	{
		if(_applicationBuffer[_applicationBufferReadIndex].length > 0 &&
			((_applicationBuffer[_applicationBufferReadIndex].data[m2mMeshPacketTypeIndex] & 0x07) == USR_PACKET_TYPE))
		{
			//Non-zero length implies data in the buffer
			return(true);
		}
		//Try the next buffer, ideally 
		_applicationBufferReadIndex++;															//Advance the buffer index
		_applicationBufferReadIndex = _applicationBufferReadIndex%M2MMESHAPPLICATIONBUFFERSIZE;	//Rollover buffer index
	}
	return(false);
}

uint8_t ICACHE_FLASH_ATTR m2mMeshClass::messageSize()
{
	return(_applicationBuffer[_applicationBufferReadIndex].length);
}

void ICACHE_FLASH_ATTR m2mMeshClass::markMessageRead()
{
	#ifdef m2mMeshIncludeDebugFeatures
	if(_debugEnabled == true && (_loggingLevel & MESH_UI_LOG_BUFFER_MANAGEMENT))
	{
		_debugStream->printf_P(m2mMeshreadAPPbufferslotd,
			_applicationBufferReadIndex,
			_applicationBuffer[_applicationBufferReadIndex].length,
			packetTypeDescription[_applicationBuffer[_applicationBufferReadIndex].data[m2mMeshPacketTypeIndex] & 0x07],
			_applicationBuffer[_applicationBufferReadIndex].routerMacAddress[0],
			_applicationBuffer[_applicationBufferReadIndex].routerMacAddress[1],
			_applicationBuffer[_applicationBufferReadIndex].routerMacAddress[2],
			_applicationBuffer[_applicationBufferReadIndex].routerMacAddress[3],
			_applicationBuffer[_applicationBufferReadIndex].routerMacAddress[4],
			_applicationBuffer[_applicationBufferReadIndex].routerMacAddress[5]);
	}
	#endif
	if(_applicationBuffer[_applicationBufferReadIndex].length > 0)
	{
		_applicationBuffer[_applicationBufferReadIndex].length = 0;								//Mark buffer slot empty
		_applicationBufferReadIndex++;															//Advance the buffer index
		_applicationBufferReadIndex = _applicationBufferReadIndex%M2MMESHAPPLICATIONBUFFERSIZE;	//Rollover buffer index
	}
	_receivedUserPacketIndex = ESP_NOW_MAX_PACKET_SIZE;
	_receivedUserPacketFieldCounter = 0;
}

uint8_t ICACHE_FLASH_ATTR m2mMeshClass::sourceId()
{
	return(_originatorIdFromMac(&_applicationBuffer[_applicationBufferReadIndex].data[m2mMeshPacketOriginatorIndex]));
}

bool ICACHE_FLASH_ATTR m2mMeshClass::sourceMacAddress(uint8_t *macAddressArray)
{
	//if(_userPacketReceived)
	if(true)
	{
		macAddressArray[0] = _applicationBuffer[_applicationBufferReadIndex].data[m2mMeshPacketOriginatorIndex];
		macAddressArray[1] = _applicationBuffer[_applicationBufferReadIndex].data[m2mMeshPacketOriginatorIndex1];
		macAddressArray[2] = _applicationBuffer[_applicationBufferReadIndex].data[m2mMeshPacketOriginatorIndex2];
		macAddressArray[3] = _applicationBuffer[_applicationBufferReadIndex].data[m2mMeshPacketOriginatorIndex3];
		macAddressArray[4] = _applicationBuffer[_applicationBufferReadIndex].data[m2mMeshPacketOriginatorIndex4];
		macAddressArray[5] = _applicationBuffer[_applicationBufferReadIndex].data[m2mMeshPacketOriginatorIndex5];
		return(true);
	}
	else
	{
		return(false);
	}
}

uint8_t ICACHE_FLASH_ATTR m2mMeshClass::nextDataType()
{
	if(dataAvailable() > 0)
	{
		uint8_t nextType = _applicationBuffer[_applicationBufferReadIndex].data[_receivedUserPacketIndex] & 0x0f;	//Remove the small array count, if it exists
		uint8_t numberOfPackedItems = _numberOfPackedItems(_applicationBuffer[_applicationBufferReadIndex].data[_receivedUserPacketIndex],_applicationBuffer[_applicationBufferReadIndex].data[_receivedUserPacketIndex+1]);
		if(numberOfPackedItems > 1)
		{
			if(nextType == USR_DATA_STR || nextType == USR_DATA_STRING)	//str and String are packed like arrays but are handled differently
			{
				return(nextType);
			}
			else
			{
				return(nextType | 0xf0);	//Mark this as an array of the underlying type
			}
		}
		else
		{
			if(nextType == USR_DATA_BOOL_TRUE)
			{
				return(USR_DATA_BOOL); //Both USR_DATA_BOOL_TRUE and USR_DATA_BOOL need to return USR_DATA_BOOL
			}
			else
			{
				return(nextType);
			}
		}
	}
	else
	{
		//This is disincentive to read past the end of the message, but won't actually stop you retrieveing dummy values
		return(USR_DATA_UNAVAILABLE);
	}
}

uint8_t ICACHE_FLASH_ATTR m2mMeshClass::dataAvailable()
{
	if(_receivedUserPacketFieldCounter > 0 && _receivedUserPacketIndex < ESP_NOW_MAX_PACKET_SIZE)
	{
		return(_receivedUserPacketFieldCounter);
	}
	return(0);
}

uint8_t ICACHE_FLASH_ATTR m2mMeshClass::retrieveDataLength()
{
	if(dataAvailable() > 0)
	{
		/*if((_applicationBuffer[_applicationBufferReadIndex].data[_receivedUserPacketIndex] & 0xf0) == 0xf0)	//Length is stored in its own byte, following this one
		{
			return(_applicationBuffer[_applicationBufferReadIndex].data[_receivedUserPacketIndex+1]);
		}
		else
		{
			return((_applicationBuffer[_applicationBufferReadIndex].data[_receivedUserPacketIndex] & 0xf0)>>4);	//Length is stored in the top four bits of the type
		}*/
		return(_numberOfPackedItems(_applicationBuffer[_applicationBufferReadIndex].data[_receivedUserPacketIndex],_applicationBuffer[_applicationBufferReadIndex].data[_receivedUserPacketIndex+1]));
	}
	else
	{
		//This is disincentive to read past the end of the message, but won't actually stop you retrieveing dummy values
		return(0);
	}
}

bool ICACHE_FLASH_ATTR m2mMeshClass::retrieveBool()
{
	if(dataAvailable() == 0)
	{
		#ifdef m2mMeshIncludeDebugFeatures
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_WARNINGS)
		{
			_debugStream->print(m2mMesherrorReadBeyondEndOfPacket);
		}
		#endif
		//Return a dummy value if nothing available
		return(false);
	}
	else
	{
		//This does not step past the 'type' as the data is stored in the 'type'
		//Decrement the count of data fields
		_receivedUserPacketFieldCounter--;
		bool temp;
		if(_applicationBuffer[_applicationBufferReadIndex].data[_receivedUserPacketIndex++] == USR_DATA_BOOL_TRUE)
		{
			temp = true;
		}
		else
		{
			temp = false;
		}
		if(dataAvailable() == 0)
		{
			markMessageRead();
		}
		return(temp);
	}
}


uint8_t ICACHE_FLASH_ATTR m2mMeshClass::retrieveUint8_t()
{
	if(dataAvailable() == 0)
	{
		#ifdef m2mMeshIncludeDebugFeatures
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_WARNINGS)
		{
			_debugStream->print(m2mMesherrorReadBeyondEndOfPacket);
		}
		#endif
		//Return a dummy value if nothing available
		return(0x00);
	}
	else
	{
		//Step past the 'type'
		_receivedUserPacketIndex++;
		//Decrement the count of data fields
		_receivedUserPacketFieldCounter--;
		uint8_t temp = uint8_t(_applicationBuffer[_applicationBufferReadIndex].data[_receivedUserPacketIndex++]);
		if(dataAvailable() == 0)
		{
			markMessageRead();
		}
		return(temp);
	}
}

/*bool ICACHE_FLASH_ATTR m2mMeshClass::retrieve(uint8_t &recipient)
{
	if(dataAvailable() == 0)
	{
		#ifdef m2mMeshIncludeDebugFeatures
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_WARNINGS)
		{
			_debugStream->print(m2mMesherrorReadBeyondEndOfPacket);
		}
		#endif
		return(false);
	}
	else
	{
		//Step past the 'type'
		_receivedUserPacketIndex++;
		//Decrement the count of data fields
		_receivedUserPacketFieldCounter--;
		recipient = uint8_t(_applicationBuffer[_applicationBufferReadIndex].data[_receivedUserPacketIndex++]);
		if(dataAvailable() == 0)
		{
			markMessageRead();
		}
		return(true);
	}
}*/

uint16_t ICACHE_FLASH_ATTR m2mMeshClass::retrieveUint16_t()
{
	if(dataAvailable() == 0)
	{
		#ifdef m2mMeshIncludeDebugFeatures
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_WARNINGS)
		{
			_debugStream->print(m2mMesherrorReadBeyondEndOfPacket);
		}
		#endif
		//Return a dummy value if nothing available
		return(0x0000);
	}
	else
	{
		//Step past the 'type'
		_receivedUserPacketIndex++;
		//Decrement the count of data fields
		_receivedUserPacketFieldCounter--;
		uint16_t temp;
		memcpy(&temp, &_applicationBuffer[_applicationBufferReadIndex].data[_receivedUserPacketIndex], sizeof(temp));
		_receivedUserPacketIndex+=sizeof(temp);
		if(dataAvailable() == 0)
		{
			markMessageRead();
		}
		return(temp);
	}
}
uint32_t ICACHE_FLASH_ATTR m2mMeshClass::retrieveUint32_t()
{
	if(dataAvailable() == 0)
	{
		#ifdef m2mMeshIncludeDebugFeatures
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_WARNINGS)
		{
			_debugStream->print(m2mMesherrorReadBeyondEndOfPacket);
		}
		#endif
		//Return a dummy value if nothing available
		return(0x00000000);
	}
	else
	{
		//Step past the 'type'
		_receivedUserPacketIndex++;
		//Decrement the count of data fields
		_receivedUserPacketFieldCounter--;
		uint32_t temp;
		memcpy(&temp, &_applicationBuffer[_applicationBufferReadIndex].data[_receivedUserPacketIndex], sizeof(temp));
		_receivedUserPacketIndex+=sizeof(temp);
		if(dataAvailable() == 0)
		{
			markMessageRead();
		}
		return(temp);
	}
}
/*bool ICACHE_FLASH_ATTR m2mMeshClass::retrieve(uint32_t &recipient)
{
	if(dataAvailable() == 0)
	{
		#ifdef m2mMeshIncludeDebugFeatures
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_WARNINGS)
		{
			_debugStream->print(m2mMesherrorReadBeyondEndOfPacket);
		}
		#endif
		//Return a dummy value if nothing available
		return(false);
	}
	else
	{
		//Step past the 'type'
		_receivedUserPacketIndex++;
		//Decrement the count of data fields
		_receivedUserPacketFieldCounter--;
		unsignedLongToBytes temp;
		temp.b[0] = _applicationBuffer[_applicationBufferReadIndex].data[_receivedUserPacketIndex++];
		temp.b[1] = _applicationBuffer[_applicationBufferReadIndex].data[_receivedUserPacketIndex++];
		temp.b[2] = _applicationBuffer[_applicationBufferReadIndex].data[_receivedUserPacketIndex++];
		temp.b[3] = _applicationBuffer[_applicationBufferReadIndex].data[_receivedUserPacketIndex++];
		if(dataAvailable() == 0)
		{
			markMessageRead();
		}
		recipient = uint32_t(temp.value);
		return(true);
	}
}*/
uint64_t ICACHE_FLASH_ATTR m2mMeshClass::retrieveUint64_t()
{
	if(dataAvailable() == 0)
	{
		#ifdef m2mMeshIncludeDebugFeatures
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_WARNINGS)
		{
			_debugStream->print(m2mMesherrorReadBeyondEndOfPacket);
		}
		#endif
		//Return a dummy value if nothing available
		return(0x00000000);
	}
	else
	{
		//Step past the 'type'
		_receivedUserPacketIndex++;
		//Decrement the count of data fields
		_receivedUserPacketFieldCounter--;
		uint64_t temp;
		memcpy(&temp, &_applicationBuffer[_applicationBufferReadIndex].data[_receivedUserPacketIndex], sizeof(temp));
		_receivedUserPacketIndex+=sizeof(temp);
		if(dataAvailable() == 0)
		{
			markMessageRead();
		}
		return(temp);
	}
}
/*bool ICACHE_FLASH_ATTR m2mMeshClass::retrieve(uint64_t &recipient)
{
	if(dataAvailable() == 0)
	{
		#ifdef m2mMeshIncludeDebugFeatures
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_WARNINGS)
		{
			_debugStream->print(m2mMesherrorReadBeyondEndOfPacket);
		}
		#endif
		//Return a dummy value if nothing available
		return(false);
	}
	else
	{
		//Step past the 'type'
		_receivedUserPacketIndex++;
		//Decrement the count of data fields
		_receivedUserPacketFieldCounter--;
		unsignedLongLongToBytes temp;
		temp.b[0] = _applicationBuffer[_applicationBufferReadIndex].data[_receivedUserPacketIndex++];
		temp.b[1] = _applicationBuffer[_applicationBufferReadIndex].data[_receivedUserPacketIndex++];
		temp.b[2] = _applicationBuffer[_applicationBufferReadIndex].data[_receivedUserPacketIndex++];
		temp.b[3] = _applicationBuffer[_applicationBufferReadIndex].data[_receivedUserPacketIndex++];
		temp.b[4] = _applicationBuffer[_applicationBufferReadIndex].data[_receivedUserPacketIndex++];
		temp.b[5] = _applicationBuffer[_applicationBufferReadIndex].data[_receivedUserPacketIndex++];
		temp.b[6] = _applicationBuffer[_applicationBufferReadIndex].data[_receivedUserPacketIndex++];
		temp.b[7] = _applicationBuffer[_applicationBufferReadIndex].data[_receivedUserPacketIndex++];
		if(dataAvailable() == 0)
		{
			markMessageRead();
		}
		recipient = uint64_t(temp.value);
		return(true);
	}
}*/
int8_t ICACHE_FLASH_ATTR m2mMeshClass::retrieveInt8_t()
{
	if(dataAvailable() == 0)
	{
		#ifdef m2mMeshIncludeDebugFeatures
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_WARNINGS)
		{
			_debugStream->print(m2mMesherrorReadBeyondEndOfPacket);
		}
		#endif
		//Return a dummy value if nothing available
		return(0x00);
	}
	else
	{
		//Step past the 'type'
		_receivedUserPacketIndex++;
		//Decrement the count of data fields
		_receivedUserPacketFieldCounter--;
		int8_t temp = int8_t(_applicationBuffer[_applicationBufferReadIndex].data[_receivedUserPacketIndex++]);
		if(dataAvailable() == 0)
		{
			markMessageRead();
		}
		return(temp);
	}
}
int16_t ICACHE_FLASH_ATTR m2mMeshClass::retrieveInt16_t()
{
	if(dataAvailable() == 0)
	{
		#ifdef m2mMeshIncludeDebugFeatures
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_WARNINGS)
		{
			_debugStream->print(m2mMesherrorReadBeyondEndOfPacket);
		}
		#endif
		//Return a dummy value if nothing available
		return(0x0000);
	}
	else
	{
		//Step past the 'type'
		_receivedUserPacketIndex++;
		//Decrement the count of data fields
		_receivedUserPacketFieldCounter--;
		int16_t temp;
		memcpy(&temp, &_applicationBuffer[_applicationBufferReadIndex].data[_receivedUserPacketIndex], sizeof(temp));
		_receivedUserPacketIndex+=sizeof(temp);
		if(dataAvailable() == 0)
		{
			markMessageRead();
		}
		return(temp);
	}
}
int32_t ICACHE_FLASH_ATTR m2mMeshClass::retrieveInt32_t()
{
	if(dataAvailable() == 0)
	{
		#ifdef m2mMeshIncludeDebugFeatures
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_WARNINGS)
		{
			_debugStream->print(m2mMesherrorReadBeyondEndOfPacket);
		}
		#endif
		//Return a dummy value if nothing available
		return(0x00000000);
	}
	else
	{
		//Step past the 'type'
		_receivedUserPacketIndex++;
		//Decrement the count of data fields
		_receivedUserPacketFieldCounter--;
		int32_t temp;
		memcpy(&temp, &_applicationBuffer[_applicationBufferReadIndex].data[_receivedUserPacketIndex], sizeof(temp));
		_receivedUserPacketIndex+=sizeof(temp);
		if(dataAvailable() == 0)
		{
			markMessageRead();
		}
		return(temp);
	}
}
int64_t ICACHE_FLASH_ATTR m2mMeshClass::retrieveInt64_t()
{
	if(dataAvailable() == 0)
	{
		#ifdef m2mMeshIncludeDebugFeatures
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_WARNINGS)
		{
			_debugStream->print(m2mMesherrorReadBeyondEndOfPacket);
		}
		#endif
		//Return a dummy value if nothing available
		return(0x00000000);
	}
	else
	{
		//Step past the 'type'
		_receivedUserPacketIndex++;
		//Decrement the count of data fields
		_receivedUserPacketFieldCounter--;
		int64_t temp;
		memcpy(&temp, &_applicationBuffer[_applicationBufferReadIndex].data[_receivedUserPacketIndex], sizeof(temp));
		_receivedUserPacketIndex+=sizeof(temp);
		if(dataAvailable() == 0)
		{
			markMessageRead();
		}
		return(temp);
	}
}
float ICACHE_FLASH_ATTR m2mMeshClass::retrieveFloat()
{
	if(dataAvailable() == 0)
	{
		#ifdef m2mMeshIncludeDebugFeatures
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_WARNINGS)
		{
			_debugStream->print(m2mMesherrorReadBeyondEndOfPacket);
		}
		#endif
		//Return a dummy value if nothing available
		return(0.0f);
	}
	else
	{
		//Step past the 'type'
		_receivedUserPacketIndex++;
		//Decrement the count of data fields
		_receivedUserPacketFieldCounter--;
		float temp;
		memcpy(&temp, &_applicationBuffer[_applicationBufferReadIndex].data[_receivedUserPacketIndex], sizeof(temp));
		_receivedUserPacketIndex+=sizeof(temp);
		if(dataAvailable() == 0)
		{
			markMessageRead();
		}
		return(temp);
	}
}
double ICACHE_FLASH_ATTR m2mMeshClass::retrieveDouble()
{
	if(dataAvailable() == 0)
	{
		#ifdef m2mMeshIncludeDebugFeatures
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_WARNINGS)
		{
			_debugStream->print(m2mMesherrorReadBeyondEndOfPacket);
		}
		#endif
		//Return a dummy value if nothing available
		return(0.0f);
	}
	else
	{
		//Step past the 'type'
		_receivedUserPacketIndex++;
		//Decrement the count of data fields
		_receivedUserPacketFieldCounter--;
		double temp;
		memcpy(&temp, &_applicationBuffer[_applicationBufferReadIndex].data[_receivedUserPacketIndex], sizeof(temp));
		_receivedUserPacketIndex+=sizeof(temp);
		if(dataAvailable() == 0)
		{
			markMessageRead();
		}
		return(temp);
	}
}
char ICACHE_FLASH_ATTR m2mMeshClass::retrieveChar()
{
	if(dataAvailable() == 0)
	{
		#ifdef m2mMeshIncludeDebugFeatures
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_WARNINGS)
		{
			_debugStream->print(m2mMesherrorReadBeyondEndOfPacket);
		}
		#endif
		//Return a dummy value if nothing available
		return(' ');
	}
	else
	{
		//Step past the 'type'
		_receivedUserPacketIndex++;
		//Decrement the count of data fields
		_receivedUserPacketFieldCounter--;
		char temp;
		temp = char(_applicationBuffer[_applicationBufferReadIndex].data[_receivedUserPacketIndex++]);
		if(dataAvailable() == 0)
		{
			markMessageRead();
		}
		return(temp);
	}
}
String ICACHE_FLASH_ATTR m2mMeshClass::retrieveString()
{
	if(dataAvailable() == 0)
	{
		#ifdef m2mMeshIncludeDebugFeatures
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_WARNINGS)
		{
			_debugStream->print(m2mMesherrorReadBeyondEndOfPacket);
		}
		#endif
		//Return a dummy value if nothing available
		return("Error: No String");
	}
	else
	{
		_receivedUserPacketFieldCounter--;
		uint8_t dataLength = _numberOfPackedItems(_applicationBuffer[_applicationBufferReadIndex].data[_receivedUserPacketIndex],_applicationBuffer[_applicationBufferReadIndex].data[_receivedUserPacketIndex+1]);
		_receivedUserPacketIndex+=_packingOverhead(_applicationBuffer[_applicationBufferReadIndex].data[_receivedUserPacketIndex]);
		String tempString;
		for(uint8_t i = 0; i<dataLength ; i++)
		{
			tempString += char(_applicationBuffer[_applicationBufferReadIndex].data[_receivedUserPacketIndex++]);
		}
		if(dataAvailable() == 0)
		{
			markMessageRead();
		}
		return(tempString);
	}
}
bool ICACHE_FLASH_ATTR m2mMeshClass::retrieveStr(char *data)
{
	if(dataAvailable() == 0)
	{
		#ifdef m2mMeshIncludeDebugFeatures
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_WARNINGS)
		{
			_debugStream->print(m2mMesherrorReadBeyondEndOfPacket);
		}
		#endif
		data[0] = char(0);	//Just null terminate and return
		return(false);
	}
	else
	{
		_receivedUserPacketFieldCounter--;
		uint8_t dataLength = _numberOfPackedItems(_applicationBuffer[_applicationBufferReadIndex].data[_receivedUserPacketIndex],_applicationBuffer[_applicationBufferReadIndex].data[_receivedUserPacketIndex+1]);
		_receivedUserPacketIndex+=_packingOverhead(_applicationBuffer[_applicationBufferReadIndex].data[_receivedUserPacketIndex]);
		if(dataLength > 0)
		{
			memcpy(data, &_applicationBuffer[_applicationBufferReadIndex].data[_receivedUserPacketIndex],dataLength);
			data[dataLength]=0;
			_receivedUserPacketIndex+=dataLength;
		}
		else
		{
			data[0] = char(0);	//Just null terminate and return
		}
		if(dataAvailable() == 0)
		{
			markMessageRead();
		}
		return(true);
	}
}

void ICACHE_FLASH_ATTR m2mMeshClass::skipRetrieve()
{
	if(dataAvailable() == 0)
	{
		#ifdef m2mMeshIncludeDebugFeatures
		if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_WARNINGS)
		{
			_debugStream->print(m2mMesherrorReadBeyondEndOfPacket);
		}
		#endif
		return;
	}
	else
	{
		uint8_t packingOverhead = _packingOverhead(_applicationBuffer[_applicationBufferReadIndex].data[_receivedUserPacketIndex]);
		uint8_t numberOfPackedItems = _numberOfPackedItems(_applicationBuffer[_applicationBufferReadIndex].data[_receivedUserPacketIndex],_applicationBuffer[_applicationBufferReadIndex].data[_receivedUserPacketIndex+1]);
		switch (_applicationBuffer[_applicationBufferReadIndex].data[_receivedUserPacketIndex] & 0x0f)	//Strip the packed item count
		{
			case USR_DATA_UINT8_T:
				_receivedUserPacketIndex += packingOverhead + sizeof(uint8_t) * numberOfPackedItems;
			break;
			case USR_DATA_UINT16_T:
				_receivedUserPacketIndex += packingOverhead + sizeof(uint16_t) * numberOfPackedItems;
			break;
			case USR_DATA_UINT32_T:
				_receivedUserPacketIndex += packingOverhead + sizeof(uint32_t) * numberOfPackedItems;
			break;
			case USR_DATA_UINT64_T:
				_receivedUserPacketIndex += packingOverhead + sizeof(uint64_t) * numberOfPackedItems;
			break;
			case USR_DATA_INT8_T:
				_receivedUserPacketIndex += packingOverhead + sizeof(int8_t) * numberOfPackedItems;
			break;
			case USR_DATA_INT16_T:
				_receivedUserPacketIndex += packingOverhead + sizeof(int16_t) * numberOfPackedItems;
			break;
			case USR_DATA_INT32_T:
				_receivedUserPacketIndex += packingOverhead + sizeof(int32_t) * numberOfPackedItems;
			break;
			case USR_DATA_INT64_T:
				_receivedUserPacketIndex += packingOverhead + sizeof(int64_t) * numberOfPackedItems;
			break;
			case USR_DATA_FLOAT:
				_receivedUserPacketIndex += packingOverhead + sizeof(float) * numberOfPackedItems;
			break;
			case USR_DATA_CHAR:
				_receivedUserPacketIndex += packingOverhead + sizeof(char) * numberOfPackedItems;
			break;
			case USR_DATA_STRING:
				_receivedUserPacketIndex++;
				_receivedUserPacketIndex += _applicationBuffer[_applicationBufferReadIndex].data[_receivedUserPacketIndex] + 1;
			break;
			case USR_DATA_STR:
				_receivedUserPacketIndex++;
				_receivedUserPacketIndex += _applicationBuffer[_applicationBufferReadIndex].data[_receivedUserPacketIndex] + 1;
			break;
		}
		//Decrement the count of data fields
		_receivedUserPacketFieldCounter--;
		if(dataAvailable() == 0)
		{
			markMessageRead();
		}
		return;
	}
}

uint8_t ICACHE_FLASH_ATTR m2mMeshClass::_packingOverhead(uint8_t fieldType)
{
	if((fieldType & 0xf0) == 0xf0)
	{
		return(2);
	}
	else
	{
		return(1);
	}
}

uint8_t ICACHE_FLASH_ATTR m2mMeshClass::_numberOfPackedItems(uint8_t fieldType, uint8_t nextByte)
{
	if((fieldType & 0xf0) == 0xf0)
	{
		return(nextByte);
	}
	else
	{
		return((fieldType & 0xf0)>>4);	//Number of packed items is encoded with type
	}
}

uint8_t ICACHE_FLASH_ATTR m2mMeshClass::numberOfNodes()
{
	return(_numberOfOriginators);
}
uint8_t ICACHE_FLASH_ATTR m2mMeshClass::numberOfReachableNodes()
{
	return(_numberOfReachableOriginators);
}
bool ICACHE_FLASH_ATTR m2mMeshClass::nodeIsReachableNode(uint8_t originatorId)
{
	if(originatorId < _numberOfOriginators)
	{
		if(_originator[originatorId].ltq > 0 || _originator[originatorId].gtq > 0)
		{
			return(true);
		}
	}
	return(false);
}

uint32_t ICACHE_FLASH_ATTR m2mMeshClass::expectedUptime(uint8_t originatorId) //Returns the current uptime of a node, on the assumption it has continued to run uninterrupted since we last heard from it
{
	if(_dataIsValid(originatorId,NHS_PACKET_TYPE))
	{
		return(_originator[originatorId].uptime + (millis() - _originator[originatorId].lastSeen[NHS_PACKET_TYPE]));
	}
	else
	{
		return(0);
	}
}

uint8_t ICACHE_FLASH_ATTR m2mMeshClass::espnowPeer(uint8_t originatorId)
{
	uint8_t value = 0;
	if(originatorId < _numberOfOriginators)
	{
		
		if(_originator[originatorId].isCurrentlyPeer == true)
		{
			value = value | 0x01;
		}
		if(_originator[originatorId].isCurrentlyPeer == true && millis() - _originator[originatorId].peerNeeded > _peerLifetime)
		{
			value = value | 0x02;
		}
		if(_originator[originatorId].hasUsAsPeer == true)
		{
			value = value | 0x04;
		}
		if(_originator[originatorId].hasUsAsPeer == true && _originator[originatorId].peeringExpired == true)
		{
			value = value | 0x08;
		}
	}
	return(value);
}
uint32_t ICACHE_FLASH_ATTR m2mMeshClass::espnowPeeringLastUsed(uint8_t originatorId)
{
	if(originatorId < _numberOfOriginators)
	{
		return(_originator[originatorId].peerNeeded);
	}
	return(0xFFFFFFFF);
}
uint8_t ICACHE_FLASH_ATTR m2mMeshClass::numberOfPeers(uint8_t originatorId)
{
	if(originatorId < _numberOfOriginators)
	{
		return(_originator[originatorId].numberOfPeers);
	}
	return(0x00);
}
uint8_t ICACHE_FLASH_ATTR m2mMeshClass::numberOfExpiredPeers(uint8_t originatorId)
{
	if(originatorId < _numberOfOriginators)
	{
		return(_originator[originatorId].numberOfExpiredPeers);
	}
	return(0x00);
}

#if defined (m2mMeshIncludeRTCFeatures)
void ICACHE_FLASH_ATTR m2mMeshClass::setNtpServer(const char *server)
{
	configTime(0, 0, server);
	rtc = true;
	_actingAsRTCServer = true;
}
void ICACHE_FLASH_ATTR m2mMeshClass::setTimeZone(const char *tz)
{
	//setenv("TZ","GMTGMT-1,M3.4.0/01,M10.4.0/02",1);
	//tzset();
	timezone = new char[strlen(tz) + 1];
	strlcpy(timezone, tz, strlen(tz) + 1);
	setenv("TZ",timezone,1);
	tzset();
}
bool ICACHE_FLASH_ATTR m2mMeshClass::rtcConfigured()
{
	return(rtc);
}
bool ICACHE_FLASH_ATTR m2mMeshClass::rtcSynced()
{
	if(rtc)
	{
		return(true);
	}
	else if(epochOffset > 0)
	{
		return(true);
	}
	return(false);
}
uint8_t ICACHE_FLASH_ATTR m2mMeshClass::rtcServer()
{
	return(_currentRTCServer);
}
bool ICACHE_FLASH_ATTR m2mMeshClass::rtcValid()
{
    #if defined(ESP32)
    if(getLocalTime(&timeinfo))	//Only works if time is set
    {
		return(true);
	}
	else
	{
		return(false);
	}
    #elif defined(ESP8266)
	time_t now;
	time(&now);
	struct tm * timeinfo;
	timeinfo = localtime(&now);
	if(timeinfo->tm_year > 100)	//If it's post-Y2K the time is probably valid. Better suggestions for checking welcome!
	{
		return(true);
	}
	else
	{
		return(false);
	}
	#endif
}
#endif

/* Debug functions
 *
 * Lots of extra code you may not want that can be eliminated by commenting out the #define
 *
 */
#ifdef m2mMeshIncludeDebugFeatures
//Enable debugging on a stream, usually Serial but let's not assume that. This version accepts the default log level
void m2mMeshClass::enableDebugging(Stream &debugStream)
{
	enableDebugging(debugStream,_loggingLevel);
}
//Enable debugging on a stream, usually Serial but let's not assume that
void m2mMeshClass::enableDebugging(Stream &debugStream, uint32_t level)
{
	_debugStream = &debugStream;	//Set the stream used for debugging
	_debugEnabled = true;			//Flag that debugging is enabled
	_loggingLevel = level;			//Set the logging level
	if(_loggingLevel & MESH_UI_LOG_INFORMATION)
	{
		_debugStream->println();
		_debugStream->print(m2mMeshdebuggingenabled);	//Announce this joyous event
	}
}
//Enable debugging on a stream, after being paused
void m2mMeshClass::enableDebugging(const uint32_t level)
{
	if(_debugStream != nullptr)
	{
		_loggingLevel = level;			//Set the logging level
		_debugEnabled = true;			//Flag that debugging is enabled
		if(_loggingLevel & MESH_UI_LOG_INFORMATION)
		{
			_debugStream->println();
			_debugStream->print(m2mMeshdebuggingenabled);	//Announce this joyous event
		}
	}
}
//Enable debugging on a stream, after being paused
void m2mMeshClass::enableDebugging()
{
	if(_debugStream != nullptr)
	{
		_debugEnabled = true;
		if(_loggingLevel & MESH_UI_LOG_INFORMATION)
		{
			_debugStream->println();
			_debugStream->print(m2mMeshdebuggingenabled);	//Announce this joyous event
		}
	}
}
//Disable debugging
void m2mMeshClass::disableDebugging()
{
	if(_debugEnabled == true && _loggingLevel & MESH_UI_LOG_INFORMATION)
	{
		_debugStream->println();
		_debugStream->print(m2mMeshdebuggingdisabled);	//Warn that debugging is disabled
	}
	_debugEnabled = false;			//Flag that debugging is enabled
}

void m2mMeshClass::nodeToLog(const uint8_t id)	//Sets the node to log
{
	_nodeToLog = id;
}
void m2mMeshClass::logAllNodes()	//Sets logging to all nodes
{
	_nodeToLog = MESH_ORIGINATOR_NOT_FOUND;
}

#if defined(ESP8266)
void m2mMeshClass::_printEspNowErrorDescription(const uint8_t result)
#elif defined(ESP32)
void m2mMeshClass::_printEspNowErrorDescription(const esp_err_t result)
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

void m2mMeshClass::_friendlyUptime(const uint32_t uptime, char * formattedUptime)
{
  uint8_t seconds = (uptime/   1000ul)%60;
  uint8_t minutes = (uptime/  60000ul)%60;
  uint8_t hours =   (uptime/3600000ul);
  sprintf(formattedUptime,"%02dh%02dm%02ds",hours,minutes,seconds);
}

#ifdef ESP8266
void m2mMeshClass::_debugPacket(m2mMeshPacketBuffer &packet)
#elif defined(ESP32)
void m2mMeshClass::_debugPacket(m2mMeshPacketBuffer &packet)
#endif
{
	_debugStream->printf_P(m2mMeshR02x02x02x02x02x02xdbytesm2mMeshType,packet.routerMacAddress[0],packet.routerMacAddress[1],packet.routerMacAddress[2],packet.routerMacAddress[3],packet.routerMacAddress[4],packet.routerMacAddress[5],packet.length);
	if((packet.data[m2mMeshPacketTypeIndex] & 0x07) == ELP_PACKET_TYPE)
	{
		_debugStream->print(F("ELP"));
	}
	else if((packet.data[m2mMeshPacketTypeIndex] & 0x07) == OGM_PACKET_TYPE)
	{
		_debugStream->print(F("OGM"));
	}
	else if((packet.data[m2mMeshPacketTypeIndex] & 0x07) == NHS_PACKET_TYPE)
	{
		_debugStream->print(F("NHS"));
	}
	else if((packet.data[m2mMeshPacketTypeIndex] & 0x07) == USR_PACKET_TYPE)
	{
		_debugStream->print(F("USR"));
	}
	else if((packet.data[m2mMeshPacketTypeIndex] & 0x07) == TRACE_PACKET_TYPE)
	{
		_debugStream->print(F("TRACE"));
	}
	else
	{
		_debugStream->print(F("Unknown"));
	}
	_debugStream->printf_P(m2mMeshChecksumx,packet.data[m2mMeshPacketChecksumIndex]);
	if(_checksumCorrect(packet) == true)
	{
		_debugStream->print(m2mMesh_valid);
	}
	_debugStream->printf_P(m2mMesh_TTLd,packet.data[m2mMeshPacketTTLIndex]);
	_debugStream->printf_P(m2mMesh_Flags2x, packet.data[m2mMeshPacketTypeIndex] & 0xf8);
	_debugStream->printf_P(m2mMesh_Sequencenumberd,packet.sequenceNumber);
	_debugStream->printf_P(m2mMeshSrc02x02x02x02x02x02x,packet.data[m2mMeshPacketOriginatorIndex],packet.data[m2mMeshPacketOriginatorIndex1],packet.data[m2mMeshPacketOriginatorIndex2],packet.data[m2mMeshPacketOriginatorIndex3],packet.data[m2mMeshPacketOriginatorIndex4],packet.data[m2mMeshPacketOriginatorIndex5]);
	uint8_t packetIndex = 14;
	if(packet.data[m2mMeshPacketTypeIndex] & SEND_TO_ALL_NODES)
	{
		_debugStream->print(m2mMeshDstALL);
	}
	else
	{
		_debugStream->printf_P(m2mMeshDst02x02x02x02x02x02x,packet.data[packetIndex++],packet.data[packetIndex++],packet.data[packetIndex++],packet.data[packetIndex++],packet.data[packetIndex++],packet.data[packetIndex++]);
	}
	uint32_t packetInterval;
	memcpy(&packetInterval, &packet.data[packetIndex],sizeof(packetInterval));
	_debugStream->printf_P(m2mMeshIntervalu,packetInterval);
	packetIndex+=sizeof(packetInterval);
	while(packetIndex + 7 < packet.length)
	{
		_debugStream->printf_P(m2mMeshData02x02x02x02x02x02x02x02x,packet.data[packetIndex++],packet.data[packetIndex++],packet.data[packetIndex++],packet.data[packetIndex++],packet.data[packetIndex++],packet.data[packetIndex++],packet.data[packetIndex++],packet.data[packetIndex++]);
	}
	if(packetIndex < packet.length - 6)
	{
		_debugStream->printf_P(m2mMeshData02x02x02x02x02x02x02x,packet.data[packetIndex++],packet.data[packetIndex++],packet.data[packetIndex++],packet.data[packetIndex++],packet.data[packetIndex++],packet.data[packetIndex++],packet.data[packetIndex++]);
	}
	else if(packetIndex < packet.length - 5)
	{
		_debugStream->printf_P(m2mMeshData02x02x02x02x02x02x,packet.data[packetIndex++],packet.data[packetIndex++],packet.data[packetIndex++],packet.data[packetIndex++],packet.data[packetIndex++],packet.data[packetIndex++]);
	}
	else if(packetIndex < packet.length - 4)
	{
		_debugStream->printf_P(m2mMeshData02x02x02x02x02x,packet.data[packetIndex++],packet.data[packetIndex++],packet.data[packetIndex++],packet.data[packetIndex++],packet.data[packetIndex++]);
	}
	else if(packetIndex < packet.length - 3)
	{
		_debugStream->printf_P(m2mMeshData02x02x02x02x,packet.data[packetIndex++],packet.data[packetIndex++],packet.data[packetIndex++],packet.data[packetIndex++]);
	}
	else if(packetIndex < packet.length - 2)
	{
		_debugStream->printf_P(m2mMeshData02x02x02x,packet.data[packetIndex++],packet.data[packetIndex++],packet.data[packetIndex++]);
	}
	else if(packetIndex < packet.length - 1)
	{
		_debugStream->printf_P(m2mMeshData02x02x,packet.data[packetIndex++],packet.data[packetIndex++]);
	}
	else if(packetIndex < packet.length)
	{
		_debugStream->printf_P(m2mMeshData02x,packet.data[packetIndex++]);
	}
}
#endif

/* Public functions used for the 'network analyser' sketch or others that want to peek at nodes
 *
 * If building a LARGE application that does no need them they can
 * be omitted by commenting out the #define in m2mMesh.h
 *
 */
#ifdef m2mMeshIncludeMeshInfoFeatures
uint8_t ICACHE_FLASH_ATTR m2mMeshClass::nodeId(uint8_t *mac)
{
	return(_originatorIdFromMac(mac));
}
bool ICACHE_FLASH_ATTR m2mMeshClass::nodeIsReachable(uint8_t originatorId)
{
	if(_dataIsValid(originatorId,ELP_PACKET_TYPE))
	{
		return(true);
	}
	if(_dataIsValid(originatorId,OGM_PACKET_TYPE))
	{
		return(true);
	}
	return(false);
}
bool ICACHE_FLASH_ATTR m2mMeshClass::macAddress(const uint8_t id,uint8_t *array)
{
	if(id < _numberOfOriginators)
	{
		array[0] = _originator[id].macAddress[0];
		array[1] = _originator[id].macAddress[1];
		array[2] = _originator[id].macAddress[2];
		array[3] = _originator[id].macAddress[3];
		array[4] = _originator[id].macAddress[4];
		array[5] = _originator[id].macAddress[5];
		return(true);
	}
	return(true);
}
bool ICACHE_FLASH_ATTR m2mMeshClass::macAddress(String nodeName,uint8_t *array)
{
	for(uint8_t destId = 0; destId < _numberOfOriginators; destId++)
	{
		if(nodeName.equals(String(_originator[destId].nodeName)))
		{
			array[0] = _originator[destId].macAddress[0];
			array[1] = _originator[destId].macAddress[1];
			array[2] = _originator[destId].macAddress[2];
			array[3] = _originator[destId].macAddress[3];
			array[4] = _originator[destId].macAddress[4];
			array[5] = _originator[destId].macAddress[5];
			return(true);
		}
	}
	array[0] = 0x00;
	array[1] = 0x00;
	array[2] = 0x00;
	array[3] = 0x00;
	array[4] = 0x00;
	array[5] = 0x00;
	return(false);
}
bool ICACHE_FLASH_ATTR m2mMeshClass::macAddress(char* nodeName,uint8_t *array)
{
	if(nodeName != nullptr)
	{
		for(uint8_t destId = 0; destId < _numberOfOriginators; destId++)
		{
			if(_originator[destId].nodeName != nullptr && strcmp(nodeName,_originator[destId].nodeName) == 0)
			{
				array[0] = _originator[destId].macAddress[0];
				array[1] = _originator[destId].macAddress[1];
				array[2] = _originator[destId].macAddress[2];
				array[3] = _originator[destId].macAddress[3];
				array[4] = _originator[destId].macAddress[4];
				array[5] = _originator[destId].macAddress[5];
				return(true);
			}
		}
	}
	array[0] = 0x00;
	array[1] = 0x00;
	array[2] = 0x00;
	array[3] = 0x00;
	array[4] = 0x00;
	array[5] = 0x00;
	return(false);
}
uint8_t ICACHE_FLASH_ATTR m2mMeshClass::maxNumberOfOriginators()
{
	return(_maxNumberOfOriginators);
}
uint8_t ICACHE_FLASH_ATTR m2mMeshClass::numberOfNodes(const uint8_t id)
{
	return(_originator[id].numberOfOriginators);
}
uint8_t ICACHE_FLASH_ATTR m2mMeshClass::numberOfActiveNeighbours()
{
	return(_numberOfActiveNeighbours);
}
uint8_t ICACHE_FLASH_ATTR m2mMeshClass::numberOfActiveNeighbours(const uint8_t id)
{
	return(_originator[id].numberOfActiveNeighbours);
}
bool ICACHE_FLASH_ATTR m2mMeshClass::elpIsValid(const uint8_t id)
{
	return(_dataIsValid(id,ELP_PACKET_TYPE));
}
bool ICACHE_FLASH_ATTR m2mMeshClass::ogmIsValid(const uint8_t id)
{
	return(_dataIsValid(id,OGM_PACKET_TYPE));
}
bool ICACHE_FLASH_ATTR m2mMeshClass::nhsIsValid(const uint8_t id)
{
	return(_dataIsValid(id,NHS_PACKET_TYPE));
}
bool ICACHE_FLASH_ATTR m2mMeshClass::validRoute(const uint8_t id)
{
	return(_originator[id].selectedRouter != MESH_ORIGINATOR_NOT_FOUND);
}
uint8_t ICACHE_FLASH_ATTR m2mMeshClass::selectedRouter(const uint8_t id)
{
	return(_originator[id].selectedRouter);
}
uint32_t ICACHE_FLASH_ATTR m2mMeshClass::ogmReceived(const uint8_t id)
{
	return(_originator[id].ogmReceived);
}
uint32_t ICACHE_FLASH_ATTR m2mMeshClass::ogmEchoes(const uint8_t id)
{
	return(_originator[id].ogmEchoes);
}
uint32_t ICACHE_FLASH_ATTR m2mMeshClass::elpInterval(const uint8_t id)
{
	return(_originator[id].interval[ELP_PACKET_TYPE]);
}
uint32_t ICACHE_FLASH_ATTR m2mMeshClass::ogmInterval(const uint8_t id)
{
	return(_originator[id].interval[OGM_PACKET_TYPE]);
}
uint32_t ICACHE_FLASH_ATTR m2mMeshClass::nhsInterval(const uint8_t id)
{
	return(_originator[id].interval[NHS_PACKET_TYPE]);
}
uint32_t ICACHE_FLASH_ATTR m2mMeshClass::elpLastSeen(const uint8_t id)
{
	return(_originator[id].lastSeen[ELP_PACKET_TYPE]);
}
uint32_t ICACHE_FLASH_ATTR m2mMeshClass::ogmLastSeen(const uint8_t id)
{
	return(_originator[id].lastSeen[OGM_PACKET_TYPE]);
}
uint32_t ICACHE_FLASH_ATTR m2mMeshClass::nhsLastSeen(const uint8_t id)
{
	return(_originator[id].lastSeen[NHS_PACKET_TYPE]);
}
bool ICACHE_FLASH_ATTR m2mMeshClass::actingAsSyncServer()
{
	return(_actingAsSyncServer);
}
bool ICACHE_FLASH_ATTR m2mMeshClass::actingAsSyncServer(const uint8_t id)
{
	return(_originator[id].flags & NHS_FLAGS_SYNCSERVER);
}
#if defined (m2mMeshIncludeRTCFeatures)
bool ICACHE_FLASH_ATTR m2mMeshClass::actingAsRTCServer()
{
	return(_actingAsRTCServer);
}
bool ICACHE_FLASH_ATTR m2mMeshClass::actingAsRTCServer(const uint8_t id)
{
	return(_originator[id].flags & NHS_FLAGS_RTCSERVER);
}
#endif
uint32_t ICACHE_FLASH_ATTR m2mMeshClass::initialFreeHeap(const uint8_t id)
{
	return(_originator[id].initialFreeHeap);
}
uint32_t ICACHE_FLASH_ATTR m2mMeshClass::currentFreeHeap(const uint8_t id)
{
	return(_originator[id].currentFreeHeap);
}
uint32_t ICACHE_FLASH_ATTR m2mMeshClass::largestFreeBlock(const uint8_t id)
{
	return(_originator[id].largestFreeBlock);
}
uint8_t ICACHE_FLASH_ATTR m2mMeshClass::heapFragmentation(const uint8_t id)
{
	return(_originator[id].heapFragmentation);
}
uint32_t ICACHE_FLASH_ATTR m2mMeshClass::sequenceNumber()
{
	return(_sequenceNumber);
}
uint32_t ICACHE_FLASH_ATTR m2mMeshClass::lastSequenceNumber(const uint8_t id)
{
	return(_originator[id].lastSequenceNumber);
}
/*float ICACHE_FLASH_ATTR m2mMeshClass::supplyVoltage(const uint8_t id)
{
	return(_originator[id].supplyVoltage);
}*/
uint8_t ICACHE_FLASH_ATTR m2mMeshClass::flags(const uint8_t id)
{
	return(_originator[id].flags);
}
uint32_t ICACHE_FLASH_ATTR m2mMeshClass::localTransmissionQuality(const uint8_t id)
{
	return(_originator[id].ltq);
}
uint32_t ICACHE_FLASH_ATTR m2mMeshClass::globalTransmissionQuality(const uint8_t id)
{
	return(_originator[id].gtq);
}
uint16_t ICACHE_FLASH_ATTR m2mMeshClass::serviceFlags()
{
	return(_serviceFlags);
}
void ICACHE_FLASH_ATTR m2mMeshClass::setServiceFlags(uint16_t flags)
{
	_serviceFlags = flags;
}
float  ICACHE_FLASH_ATTR m2mMeshClass::currentTxPower()
{
	return(_currentTxPower);
}
float  ICACHE_FLASH_ATTR m2mMeshClass::currentTxPower(const uint8_t id)
{
	return(_originator[id].currentTxPower);
}
float ICACHE_FLASH_ATTR m2mMeshClass::txPowerFloor()
{
	return(_txPowerFloor);
}
uint8_t ICACHE_FLASH_ATTR m2mMeshClass::currentMeshSyncServer()
{
	return(_currentMeshSyncServer);
}
int32_t ICACHE_FLASH_ATTR m2mMeshClass::meshSyncDrift()
{
	return(_meshSyncDrift);
}
bool ICACHE_FLASH_ATTR m2mMeshClass::stable()
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
uint32_t ICACHE_FLASH_ATTR m2mMeshClass::loggingLevel()
{
	return(_loggingLevel);
}
void ICACHE_FLASH_ATTR m2mMeshClass::setLoggingLevel(const uint32_t level)
{
	_loggingLevel = level;
}
uint32_t ICACHE_FLASH_ATTR m2mMeshClass::rxPackets()
{
	return(_rxPackets);
}
uint32_t ICACHE_FLASH_ATTR m2mMeshClass::rxPackets(const uint8_t id)
{
	return(_originator[id].rxPackets);
}
uint32_t ICACHE_FLASH_ATTR m2mMeshClass::txPackets()
{
	return(_txPackets);
}
uint32_t ICACHE_FLASH_ATTR m2mMeshClass::txPackets(const uint8_t id)
{
	return(_originator[id].txPackets);
}
uint32_t ICACHE_FLASH_ATTR m2mMeshClass::droppedRxPackets()
{
	return(_droppedRxPackets);
}
uint32_t ICACHE_FLASH_ATTR m2mMeshClass::droppedRxPackets(const uint8_t id)
{
	return(_originator[id].droppedRxPackets);
}
uint32_t ICACHE_FLASH_ATTR m2mMeshClass::droppedTxPackets()
{
	return(_droppedTxPackets);
}
uint32_t ICACHE_FLASH_ATTR m2mMeshClass::droppedTxPackets(const uint8_t id)
{
	return(_originator[id].droppedTxPackets);
}
#endif
m2mMeshClass m2mMesh;	//Create an instance of the class, as only one is practically usable at a time
#endif