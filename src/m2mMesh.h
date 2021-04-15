#ifndef m2mMesh_h
#define m2mMesh_h
//Comment out the following line to remove some unnecessary functions for interrogating the mesh in user applications
//#define m2mMeshIncludeDebugFeatures
#include <Arduino.h>

//Different base libraries are needed for ESP8266/ESP8285 and ESP32
#if defined(ESP8266)
	//Include the ESP8266 WiFi and ESP-Now libraries
	//Also define some missing values for return values to match the ESP32 equivalents
	#define ESP_OK 0
	#define ESP_ERR_ESPNOW_NOT_INIT 248
	#define ESP_ERR_ESPNOW_ARG 249
	#define ESP_ERR_ESPNOW_INTERNAL 250
	#define ESP_ERR_ESPNOW_NO_MEM 251
	#define ESP_ERR_ESPNOW_NOT_FOUND 252
	#define ESP_ERR_ESPNOW_IF 253
	#define ESP_FAIL 255
	#include <ESP8266WiFi.h>
	extern "C" {
	#include <espnow.h>
	#include "user_interface.h"
	}
#elif defined(ESP32)
	//Include the ESP32 WiFi and ESP-Now libraries
	#include <WiFi.h>
	extern "C" {
	#include <esp_now.h>
	}
#endif

//Error messages if debug enabled
#ifdef m2mMeshIncludeDebugFeatures
const char errorReadBeyondEndOfPacket[] PROGMEM = "\r\nm2mMesh tried to read beyond end of packet";
const char nm2mMeshstartedwithcapacityfordnodes[] PROGMEM = "\r\nm2mMesh started with capacity for %d nodes";
const char nm2mMeshOGMECHOR02x02x02x02x02x02xO02x02x02x02x02x02xTTLdHOPdLEN[] PROGMEM = "\r\nm2mMesh OGM ECHO R:%02x:%02x:%02x:%02x:%02x:%02x O:%02x:%02x:%02x:%02x:%02x:%02x TTL:%d HOP:%d LEN:%d";
const char m2mMesh02x02x02x02x02x02xsequencenumberprotectionenabled[] PROGMEM = "\r\nm2mMesh %02x:%02x:%02x:%02x:%02x:%02x sequence number protection enabled";
const char m2mMeshsoriginator02x02x02x02x02x02xchangedintervalfromdtod[] PROGMEM = "\r\nm2mMesh %s originator %02x:%02x:%02x:%02x:%02x:%02x changed interval from %d to %d";
const char nm2mMeshWARNINGunknowntypedfrom02x02x02x02x02x02x[] PROGMEM = "\r\nm2mMesh WARNING: unknown type %d from %02x:%02x:%02x:%02x:%02x:%02x";
const char m2mMeshsFWDR02x02x02x02x02x02xO02x02x02x02x02x02xTTLd[] PROGMEM = "\r\nm2mMesh %s FWD R:%02x:%02x:%02x:%02x:%02x:%02x O:%02x:%02x:%02x:%02x:%02x:%02x TTL:%d";
const char m2mMeshsFWDR02x02x02x02x02x02xO02x02x02x02x02x02xTTLdfailed[] PROGMEM = "\r\nm2mMesh %s FWD R:%02x:%02x:%02x:%02x:%02x:%02x O:%02x:%02x:%02x:%02x:%02x:%02x TTL:%d failed";
const char m2mMesh02x02x02x02x02x02xsequencenumberprotectiondisabledpossiblereboot[] PROGMEM = "\r\nm2mMesh %02x:%02x:%02x:%02x:%02x:%02x sequence number protection disabled, possible reboot";
const char m2mMeshWARNINGincorrectprotocolversiondfrom02x02x02x02x02x02x[] PROGMEM = "\r\nm2mMesh WARNING: incorrect protocol version %d from %02x:%02x:%02x:%02x:%02x:%02x";
const char TTL02dFLG02xSEQ08xLENdNBRd[] PROGMEM = "TTL:%02d FLG:%02x SEQ:%08x LEN:%d NBR:%d";
const char m2mMeshSent[] PROGMEM = "\r\nm2mMesh Sent ";
const char m2mMeshELPRCVR02x02x02x02x02x02xO02x02x02x02x02x02xTTLdNBRS02dLENd[] PROGMEM = "\r\nm2mMesh ELP RCV R:%02x:%02x:%02x:%02x:%02x:%02x O:%02x:%02x:%02x:%02x:%02x:%02x TTL:%d NBRS:%02d LEN:%d";
const char m2mMeshELPRCVR02x02x02x02x02x02xO02x02x02x02x02x02xTTLdLENd[] PROGMEM = "\r\nm2mMesh ELP RCV R:%02x:%02x:%02x:%02x:%02x:%02x O:%02x:%02x:%02x:%02x:%02x:%02x TTL:%d LEN:%d";
const char m2mMeshELPneighbour02x02x02x02x02x02xthisnode[] PROGMEM = "\r\nm2mMesh ELP neighbour %02x:%02x:%02x:%02x:%02x:%02x - this node";
const char m2mMeshELPneighbour02x02x02x02x02x02xnewnode[] PROGMEM =  "\r\nm2mMesh ELP neighbour %02x:%02x:%02x:%02x:%02x:%02x - new node";
const char m2mMeshELPneighbour02x02x02x02x02x02x[] PROGMEM =  "\r\nm2mMesh ELP neighbour %02x:%02x:%02x:%02x:%02x:%02x";
const char m2mMeshELP[] PROGMEM =  "\r\nm2mMesh ELP ";
const char m2mMeshOGMSNDTTL02dFLG02xSEQ08xLENd[] PROGMEM =  "\r\nm2mMesh OGM SND TTL:%02d FLG:%02x SEQ:%08x LEN:%d";
const char m2mMeshOGMSNDfailedTTL02dFlags02xSeq08xLENd[] PROGMEM =  "\r\nm2mMesh OGM SND failed TTL:%02d Flags:%02x Seq:%08x LEN:%d";
const char m2mMeshOGMRCVR02x02x02x02x02x02xO02x02x02x02x02x02xTTLdHOPdLENd[] PROGMEM =  "\r\nm2mMesh OGM RCV R:%02x:%02x:%02x:%02x:%02x:%02x O:%02x:%02x:%02x:%02x:%02x:%02x TTL:%d HOP:%d LEN:%d";
const char m2mMeshOGMR02x02x02x02x02x02xselectedforO02x02x02x02x02x02xTQ04x[] PROGMEM =  "\r\nm2mMesh OGM R:%02x%02x%02x%02x%02x%02x selected for O:%02x%02x%02x%02x%02x%02x TQ:%04x";
const char m2mMeshOGMR02x02x02x02x02x02xforO02x02x02x02x02x02xinferiorTQ04x[] PROGMEM =  "\r\nm2mMesh OGM R:%02x%02x%02x%02x%02x%02x for O:%02x%02x%02x%02x%02x%02x inferior TQ:%04x";
const char m2mMeshOGMR02x02x02x02x02x02xforO02x02x02x02x02x02xupdateTQ04x[] PROGMEM = "\r\nm2mMesh OGM R:%02x%02x%02x%02x%02x%02x for O:%02x%02x%02x%02x%02x%02x update TQ:%04x";
const char m2mMeshOGMR02x02x02x02x02x02xforO02x02x02x02x02x02xTQ04x[] PROGMEM = "\r\nm2mMesh OGM R:%02x%02x%02x%02x%02x%02x for O:%02x%02x%02x%02x%02x%02x TQ:%04x";
const char m2mMeshOGMforwardingchain02x02x02x02x02x02x[] PROGMEM = "\r\nm2mMesh OGM forwarding chain %02x:%02x:%02x:%02x:%02x:%02x";
const char m2mMeshOGMhoppenaltyappliedTQnow02x[] PROGMEM = "\r\nm2mMesh OGM hop penalty applied, TQ now %02x";
const char m2mMeshOGM02x02x02x02x02x02xhasbecomereachable[] PROGMEM = "\r\nm2mMesh OGM %02x%02x%02x%02x%02x%02x has become reachable";
const char m2mMeshOGM02x02x02x02x02x02xhasbecomeunreachable[] PROGMEM = "\r\nm2mMesh OGM %02x%02x%02x%02x%02x%02x has become unreachable";
const char m2mMeshNHSSND[] PROGMEM = "\r\nm2mMesh NHS SND";
const char TTL02dFLG02xSEQ08xLENd[] PROGMEM = " TTL:%02d FLG:%02x SEQ:%08x LEN:%d";
const char ORGd[] PROGMEM = " ORG:%d";
const char m2mMeshsuccess[] PROGMEM = "- success";
const char failed[] PROGMEM = " failed";
const char m2mMeshNHSR02x02x02x02x02x02xO02x02x02x02x02x02xTTLdLengthd[] PROGMEM = "\r\nm2mMesh NHS R:%02x:%02x:%02x:%02x:%02x:%02x O:%02x:%02x:%02x:%02x:%02x:%02x TTL:%d Length:%d";
const char m2mMeshNHSUptimedms[] PROGMEM = "\r\nm2mMesh NHS Uptime %dms";
const char m2mMeshNHSCurrentFreeHeapdd[] PROGMEM = "\r\nm2mMesh NHS Current Free Heap %d/%d";
const char m2mMeshNHSdroppedpacketsddRXddTX[] PROGMEM = "\r\nm2mMesh NHS dropped packets %d/%dRX %d/%dTX";
const char m2mMeshNHSActiveneighboursddMAC02x02x02x02x02x02x[] PROGMEM = "\r\nm2mMesh NHS Active neighbours %d/%d MAC:%02x:%02x:%02x:%02x:%02x:%02x";
const char differs[] PROGMEM = " - differs";
const char m2mMeshNHSCurrentTXpowerf[] PROGMEM = "\r\nm2mMesh NHS Current TX power %f";
const char m2mMeshNHSSupplyvoltagefV[] PROGMEM = "\r\nm2mMesh NHS Supply voltage %fV";
const char m2mMeshNHSMeshtimedms[] PROGMEM = "\r\nm2mMesh NHS Mesh time %dms";
const char m2mMeshNHSnodenamelendschangedfroms[] PROGMEM = "\r\nm2mMesh NHS node name len=%d '%s' changed from '%s'!";
const char m2mMeshNHSnodenamelends[] PROGMEM = "\r\nm2mMesh NHS node name len=%d '%s'";
const char m2mMeshNHScontainsdoriginators[] PROGMEM = "\r\nm2mMesh NHS contains %d originators";
const char m2mMeshNHSoriginatordata02x02x02x02x02x02xTQ02x[] PROGMEM = "\r\nm2mMesh NHS originator data %02x:%02x:%02x:%02x:%02x:%02x TQ:%02x";
const char m2mMeshUSRR02x02x02x02x02x02xO02x02x02x02x02x02xTTLdLengthd[] PROGMEM = "\r\nm2mMesh USR R:%02x:%02x:%02x:%02x:%02x:%02x O:%02x:%02x:%02x:%02x:%02x:%02x TTL:%d Length:%d";
const char m2mMeshUSRpacketcontainsdfields[] PROGMEM =		"\r\nm2mMesh USR packet contains %d fields";
const char m2mMeshUSRdatafieldduint8_td[] PROGMEM =			"\r\nm2mMesh USR data field %d uint8_t %d";
const char m2mMeshUSRdatafieldduint16_td[] PROGMEM =		"\r\nm2mMesh USR data field %d uint16_t %d";
const char m2mMeshUSRdatafieldduint32_td[] PROGMEM =		"\r\nm2mMesh USR data field %d uint32_t %d";
const char m2mMeshUSRdatafielddint8_td[] PROGMEM =			"\r\nm2mMesh USR data field %d int8_t %d";
const char m2mMeshUSRdatafielddint16_td[] PROGMEM =			"\r\nm2mMesh USR data field %d int16_t %d";
const char m2mMeshUSRdatafielddint32_td[] PROGMEM =			"\r\nm2mMesh USR data field %d int32_t %d";
const char m2mMeshUSRdatafielddfloatf[] PROGMEM =			"\r\nm2mMesh USR data field %d float %f";
const char m2mMeshUSRdatafielddcharc[] PROGMEM =			"\r\nm2mMesh USR data field %d char '%c'";
const char m2mMeshUSRdatafielddStringlends[] PROGMEM =		" \r\nm2mMesh USR data field %d String len=%d '%s'";
const char m2mMeshUSRdatafielddchararraylends[] PROGMEM =	" \r\nm2mMesh USR data field %d char array len=%d '%s'";
const char m2mMeshUSRdatafielddunknowntypedstoppingdecode[] PROGMEM = " \r\nm2mMesh USR data field %d unknown type %d, stopping decode ";
const char m2mMeshNHS02x02x02x02x02x02xisnowthetimeserver[] PROGMEM = "\r\nm2mMesh NHS %02x:%02x:%02x:%02x:%02x:%02x is now the time server";
const char m2mMeshNHStimeoffsetnegdms[] PROGMEM = "\r\nm2mMesh NHS time offset -%dms";
const char m2mMeshNHStimeoffsetposdms[] PROGMEM = "\r\nm2mMesh NHS time offset +%dms";
const char m2mMeshNHSmeshtimesettos[] PROGMEM = "\r\nm2mMesh NHS mesh time set to %s";
const char m2mMesh02x02x02x02x02x02xdbytesm2mMeshType[] PROGMEM = "%02x:%02x:%02x:%02x:%02x:%02x %d bytes\r\nm2mMesh Type:";
const char m2mMeshVersiond[] PROGMEM = " Version:%d";
const char m2mMeshTTLd[] PROGMEM = " TTL:%d";
const char m2mMeshFlagsd[] PROGMEM = " Flags:%d";
const char m2mMeshSequencenumberd[] PROGMEM = " Sequence number:%d\r\n";
const char m2mMeshSrc02x02x02x02x02x02x[] PROGMEM = "m2mMesh Src:%02x:%02x:%02x:%02x:%02x:%02x";
const char m2mMeshDst02x02x02x02x02x02x[] PROGMEM = " Dst:%02x:%02x:%02x:%02x:%02x:%02x";
const char m2mMeshData02x02x02x02x02x02x02x02x[] PROGMEM =	"\r\nm2mMesh Data:%02x %02x %02x %02x %02x %02x %02x %02x";
const char m2mMeshData02x02x02x02x02x02x02x[] PROGMEM =		"\r\nm2mMesh Data:%02x %02x %02x %02x %02x %02x %02x --";
const char m2mMeshData02x02x02x02x02x02x[] PROGMEM =		"\r\nm2mMesh Data:%02x %02x %02x %02x %02x %02x --";
const char m2mMeshData02x02x02x02x02x[] PROGMEM =			"\r\nm2mMesh Data:%02x %02x %02x %02x %02x --";
const char m2mMeshData02x02x02x02x[] PROGMEM =				"\r\nm2mMesh Data:%02x %02x %02x %02x --";
const char m2mMeshData02x02x02x[] PROGMEM =					"\r\nm2mMesh Data:%02x %02x %02x --";
const char m2mMeshData02x02x[] PROGMEM =					"\r\nm2mMesh Data:%02x %02x --";
const char m2mMeshData02x[] PROGMEM =						"\r\nm2mMesh Data:%02x --";
const char m2mMesh02x02x02x02x02x02xaddedidd[] PROGMEM = 		"%02x:%02x:%02x:%02x:%02x:%02x added id=%d";
const char m2mMesh02x02x02x02x02x02xcouldnotbeadded[] PROGMEM = "%02x:%02x:%02x:%02x:%02x:%02x could not be added";
const char m2mMeshNodenamesettos[] PROGMEM = "\r\nNode name set to '%s'";
const char m2mMeshSendingpackettype02xversion02x[] PROGMEM = "\r\nm2mMesh Sending packet type %02x version %02x";
const char m2mMeshUSRSNDO02x02x02x02x02x02xTTL02dFlags02x[] PROGMEM = "\r\nm2mMesh USR SND O:%02x%02x%02x%02x%02x%02x TTL:%02d Flags:%02x";
const char m2mMeshstartedwithautomaticallocationofmemoryStabilitymaybeimpacted[] PROGMEM = "\r\nm2mMesh started with automatic allocation of memory. Stability may be impacted.";
const char m2mMeshTTL02dFLG02xSEQ08xLENd[] PROGMEM = "TTL:%02d FLG:%02x SEQ:%08x LEN:%d";
const char m2mMeshTIMEs[] PROGMEM = " TIME:%s";
const char m2mMeshd02x[] PROGMEM = "%d/%02x ";
const char m2mMeshDstALL[] PROGMEM = " Dst:ALL";
const char m2mMeshInterval[] PROGMEM = " Interval:";
const char m2mMeshdebuggingenabled[] PROGMEM = "m2mMesh debugging enabled";
const char m2mMeshdebuggingdisabled[] PROGMEM = "m2mMesh debugging disabled";
const char m2mMeshinitialisingESPNOW[] PROGMEM = "m2mMesh initialising ESP-NOW ";
const char m2mMeshfailedrestartingin3s[] PROGMEM = "- failed, restarting in 3s";
const char m2mMeshaddingbroadcastMACaddressasapeertoenablemeshdiscovery[] PROGMEM = "m2mMesh adding broadcast MAC address as a 'peer' to enable mesh discovery ";
const char m2mMeshReceivedfrom[] PROGMEM = "m2mMesh Received from ";
const char m2mMeshRTR[] PROGMEM = "\r\nm2mMesh RTR ";
const char m2mMeshSoftAPison[] PROGMEM = " SoftAP is on";
const char m2mMeshNHSUnabletostorenodenamenotenoughmemory[] PROGMEM = "\r\nm2mMesh NHS Unable to store node name, not enough memory";
const char m2mMeshnew[] PROGMEM = " - new";
const char m2mMeshthisnode[] PROGMEM = " - this node";
const char m2mMeshPreviousUSRmessagenotreadpacketdropped[] PROGMEM = "\r\nm2mMesh Previous USR message not read, packet dropped";
const char m2mMeshTimeserverhasgoneofflinetakingovertimeserverrole[] PROGMEM = "Time server has gone offline, taking over time server role";
//const char m2mMeshPacketsentto02x02x02x02x02x02x[] PROGMEM = "Packet sent to %02x:%02x:%02x:%02x:%02x:%02x";
//const char m2mMeshNHSincluded02x02x02x02x02x02xTQ02x[] PROGMEM = "\r\nm2mMesh NHS included %02x:%02x:%02x:%02x:%02x:%02x TQ:%02x";
//const char m2mMeshNHSincludeddoriginators[] PROGMEM = "\r\nm2mMesh NHS included %d originators";
#endif

//Unions used to pack data into packets

union unsignedIntToBytes
{
  uint8_t b[2];
  uint16_t value;
};

union unsignedLongToBytes
{
  uint8_t b[4];
  uint32_t value;
};

union intToBytes
{
  uint8_t b[2];
  int16_t value;
};

union longToBytes
{
  uint8_t b[4];
  int32_t value;
};

union floatToBytes
{
  uint8_t b[4];
  float value;
};

struct originatorInfo									//A structure for storing information about originators (nodes)
{
	char *nodeName = nullptr;							//Node name, which may or may not be set. Memory is allocated dynamically at runtime
	uint8_t macAddress[6] = {0, 0, 0, 0, 0, 0};			//Defaults to nothing
	uint8_t channel = 0;								//Defaults to whatever the mesh channel is
	uint8_t flags = 0;									//Defaults to no flags
	//Mesh info
	uint8_t numberOfOriginators = 0;					//Number of originators this node asserts is in the mesh
	uint8_t numberOfActiveNeighbours = 0;				//Number of active neighbours this node sees traffic from
	//Routing info
	uint32_t lastSequenceNumber = 0;					//The last sequence number from this originator
	bool sequenceNumberProtectionWindowActive = true;	//Used to handle rollover and out of sequence packets
	uint32_t lastSeen[5] = {0, 0, 0, 0, 0};				//The last time each protocol ELP/OGM/NHS/USR/FSP was seen
	uint32_t interval[5] = {0, 0, 0, 0, 0};				//The expected interval for each protocol ELP/OGM/NHS/USR/FSP
	uint16_t ogmReceived = 0;							//The number of OGMs received
	uint32_t ogmReceiptLastConfirmed = 0;				//Last time an incoming OGM was checked for
	uint16_t ogmEchoes = 0;								//The number of OGM ecoes received
	uint32_t ogmEchoLastConfirmed = 0;					//Time of the last OGM echo
	uint16_t ltq = 0;									//Local Transmission Quality
	uint16_t gtq = 0;									//Global Transmission Quality
	uint8_t selectedRouter = 0;							//Best global router for this node
	//Health info
	uint32_t uptime = 0;								//The time the device has been up
	float supplyVoltage = 0;							//Supply voltage as measured by the ESP, whether it is the Vcc or battery voltage up to you
	float currentTxPower = 0;							//Radio transmission power, if managing transmit power
	uint32_t currentFreeHeap = 0;
	uint32_t initialFreeHeap = 0;
	uint32_t rxPackets = 0;								//Monitor buffer receive problems
	uint32_t txPackets = 0;								//Monitor buffer send problems
	uint32_t droppedRxPackets = 0;						//Monitor buffer receive problems
	uint32_t droppedTxPackets = 0;						//Monitor buffer send problems
};

struct packetBuffer										//A structure for storing ESP-Now packets
{
	uint8_t macAddress[6] = {0, 0, 0, 0, 0, 0};			//Source or destination MAC address
	uint8_t length;										//Amount of data in the packet
	uint8_t data[250];									//Size the data for the largest possible packet
	uint32_t timestamp = 0;								//Time when the packet went into the buffer
};

/*
 * Circular buffer class, taken from Embedded Artistry but with the thread safe locking removed for the ESP8266
 *
 * https://embeddedartistry.com/blog/2017/05/17/creating-a-circular-buffer-in-c-and-c/
 * https://github.com/embeddedartistry/embedded-resources/blob/master/examples/cpp/circular_buffer.cpp
 *
 * Used for receive packet buffers
 *
 */
/*template <class T>
class circular_buffer {
public:
	explicit circular_buffer(size_t size) :
		buf_(std::unique_ptr<T[]>(new T[size])),
		max_size_(size)
	{

	}

	void put(T item)
	{
		#if not defined(ESP8266)
		std::lock_guard<std::mutex> lock(mutex_);
		#endif
		buf_[head_] = item;

		if(full_)
		{
			tail_ = (tail_ + 1) % max_size_;
		}

		head_ = (head_ + 1) % max_size_;

		full_ = head_ == tail_;
	}

	T get()
	{
		#if not defined(ESP8266)
		std::lock_guard<std::mutex> lock(mutex_);
		#endif

		if(empty())
		{
			return T();
		}

		//Read data and advance the tail (we now have a free space)
		auto val = buf_[tail_];
		full_ = false;
		tail_ = (tail_ + 1) % max_size_;

		return val;
	}

	void reset()
	{
		#if not defined(ESP8266)
		std::lock_guard<std::mutex> lock(mutex_);
		#endif
		head_ = tail_;
		full_ = false;
	}

	bool empty() const
	{
		//if head and tail are equal, we are empty
		return (!full_ && (head_ == tail_));
	}

	bool full() const
	{
		//If tail is ahead the head by 1, we are full
		return full_;
	}

	size_t capacity() const
	{
		return max_size_;
	}

	size_t size() const
	{
		size_t size = max_size_;

		if(!full_)
		{
			if(head_ >= tail_)
			{
				size = head_ - tail_;
			}
			else
			{
				size = max_size_ + head_ - tail_;
			}
		}

		return size;
	}

private:
	#if not defined(ESP8266)
	std::mutex mutex_;
	#endif
	std::unique_ptr<T[]> buf_;
	size_t head_ = 0;
	size_t tail_ = 0;
	const size_t max_size_;
	bool full_ = 0;
};*/

class m2mMesh
{

	public:

		//Constructor functions
		m2mMesh();
		//Destructor function
		~m2mMesh();

		//Setup functions
		void begin();										//Start the mesh, with the default maximum number of nodes (16)
		void begin(uint8_t);								//Start the mesh with a specific maximum number of nodes. Set to 0 for unlimited dynamic allocation of memory, which may cause instability.
		void begin(uint8_t, uint8_t);						//Start the mesh with a specific maximum number of nodes and channel.
		void end();											//Stop the mesh and free most memory. Configured items such as the node name or maximum number of nodes ARE retained but no originator/routing information.
		
		//Maintenance functions
		void housekeeping();								//Must be run at least once a second to make sure mesh packets are processed

		//Configuration functions
		bool nodeNameIsSet();								//Returns true if the node name is set, false otherwise
		bool setNodeName(const char *);							//Set the node name
		bool setNodeName(String);							//Set the node name
		char * getNodeName();								//Get a pointer to the node name
		char * getNodeName(uint8_t);						//Get a pointer to the node name for another node
		uint8_t * getMeshAddress();							//Get a pointer to the mesh MAC address
		
		//Status functions
		bool joined();										//Has this node joined the mesh
		bool meshIsStable();								//Has the mesh membership changed recently
		float supplyVoltage();								//Returns the supply voltage once the resistor ladder value is set
		uint32_t time();									//Returns 'mesh time' which should be broadly synced across all the nodes, useful for syncing events
		uint8_t numberOfOriginators();						//Returns the total number of originators in the mesh
		uint8_t numberOfReachableOriginators();				//Returns the number of originators reachable from this node
		uint32_t expectedUptime(uint8_t);					//Uptime of a node, assuming it has continued running
		bool nodeNameIsSet(uint8_t);						//Is a node's name set

		//Sending data
		//bool destination(uint8_t);			//Add a destination ID to a message. Without a destination it is flooded to the whole mesh.
		//bool destination(char *);			//Add a destination to a message. This can be a node name or MAC address, the function will attempt to distinguish between them.
		//bool destination(String);			//Add a destination to a message. This can be a node name or MAC address.
		void destination(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);	//Add a destination MAC address
		uint8_t payloadLeft();				//Returns the number of bytes left in the packet, helps with checking before adding
		bool add(uint8_t);					//Add some uint8_t data to a message
		bool add(uint16_t);					//Add some uint16_t data to a message
		bool add(uint32_t);					//Add some uint16_t data to a message
		bool add(int8_t);					//Add some int8_t data to a message
		bool add(int16_t);					//Add some int16_t data to a message
		bool add(int32_t);					//Add some int32_t data to a message
		bool add(char);						//Add some char data to a message
		bool add(float);					//Add some float data to a message
		bool add(String);					//Add some String data to a message
		bool add(char *);					//Add some char array data to a message
		bool add(uint8_t *, uint8_t);		//Add an array of uint8_t, mostly used to send chunks of binary data
		bool send();						//Send the message now
		void clear();						//Clear the message without sending

		//Receiving data
		bool messageWaiting();				//Is there a message waiting
		uint8_t messageSize();				//Size of the message, perhaps not useful
		void markMessageRead();				//Marks the message as read, discarding any data left
		uint8_t sourceId();					//Which ID is the message from
		bool sourceMacAddress(uint8_t *);	//Which MAC address is the message from
		bool dataAvailable();				//Is there data to read
		uint8_t nextDataType();				//Which type the next piece of data is
		
		bool retrieve(uint8_t&);

		uint8_t retrieveUint8_t();
		uint16_t retrieveUint16_t();
		uint32_t retrieveUint32_t();
		int8_t retrieveInt8_t();
		int16_t retrieveInt16_t();
		int32_t retrieveInt32_t();
		float retrieveFloat();
		char retrieveChar();
		String retrieveString();
		uint8_t retrieveDataLength();
		void retrieveCharArray(char *);
		void retrieveUint8_tArray(uint8_t *);


		/* Public functions used for the 'network analyser' sketch
		 *
		 * If building a LARGE application that does no need them they can
		 * be omitted by commenting out the #define in m2mMesh.h
		 *
		 */
		#ifdef m2mMeshIncludeDebugFeatures
		//Used for the 'network analyser' sketch
		uint8_t maxNumberOfOriginators();
		uint8_t numberOfOriginators(uint8_t);				//Returns the total number of originators in the mesh for this node
		uint8_t numberOfActiveNeighbours();					//Number of active neighbours for this node
		uint8_t numberOfActiveNeighbours(uint8_t);			//Number of active neighbours for another node
		void macAddress(uint8_t,uint8_t *);					//Supplies the MAC address of an originator when passed an array
		uint8_t flags(uint8_t id);							//Flags set by the originator
		bool elpIsValid(uint8_t);							//Is ELP up?
		bool ogmIsValid(uint8_t);							//Is OGM up?
		bool nhsIsValid(uint8_t);							//Is NHS up?
		bool validRoute(uint8_t);							//Is there a valid route to a destination
		uint32_t ogmReceived(uint8_t);						//Number of OGMs received
		uint32_t ogmEchoes(uint8_t);						//Number of OGMs echoes received
		uint32_t elpInterval(uint8_t);						//Expected ELP interval
		uint32_t ogmInterval(uint8_t);						//Expected OGM interval
		uint32_t nhsInterval(uint8_t);						//Expected NHS interval
		uint32_t elpLastSeen(uint8_t);						//Last time ELP was seen
		uint32_t ogmLastSeen(uint8_t);						//Last time OGM was seen
		uint32_t nhsLastSeen(uint8_t);						//Last time NHS was seen
		bool actingAsTimeServer();							//Is this node acting as a time server
		bool actingAsTimeServer(uint8_t);					//Is a node acting as a time server
		uint32_t initialFreeHeap(uint8_t);					//Initial free heap of a node
		uint32_t currentFreeHeap(uint8_t);					//Current free heap of a node
		uint32_t sequenceNumber();							//Last sequence number seen from a node
		uint32_t lastSequenceNumber(uint8_t);				//Last sequence number seen from a node
		float supplyVoltage(uint8_t);						//Supply or battery voltage for another node
		bool softApState();									//State of the SoftAP on this node
		bool softApState(uint8_t);							//State of the SoftAP on a node
		uint32_t localTransmissionQuality(uint8_t);			//Local Transmission quality to a node
		uint32_t globalTransmissionQuality(uint8_t);		//Global Transmission quality to a node
		uint8_t selectedRouter(uint8_t);					//Selected router for a destination
		uint16_t serviceFlags();							//Returns current service flags
		void setServiceFlags(uint16_t );					//Sets the service flags
		float currentTxPower();								//Returns current TxPower for this node
		float currentTxPower(uint8_t);						//Returns current TxPower for another node
		float txPowerFloor();								//Returns the current tx power floor
		uint8_t currentMeshTimeServer();					//The node this one is using as a time server
		int32_t meshTimeDrift();							//Returns 'mesh time' drift
		uint32_t rxPackets();								//Stats on received packets
		uint32_t rxPackets(uint8_t);						//Stats on received packets for another node
		uint32_t txPackets();								//Stats on transmitted packets
		uint32_t txPackets(uint8_t);						//Stats on transmitted packets for another node
		uint32_t droppedRxPackets();						//Stats on dropped receive packets
		uint32_t droppedRxPackets(uint8_t);					//Stats on dropped receive packets for another node
		uint32_t droppedTxPackets();						//Stats on dropped transmit packets
		uint32_t droppedTxPackets(uint8_t);					//Stats on dropped transmit packets for another node
		#endif


		//Built in peripheral support
		void enableActivityLed(uint8_t,bool);				//Enables an activity LED, argument are the pin and the 'on' state. This blinks on send/receive.
		void enableStatusLed(uint8_t,bool);					//Enables a status LED, arguments are the pin and the 'on' state. This counts out the number of neighbours.

		
		#ifdef ESP8266
		void espNowReceiveCallback(uint8_t*, uint8_t*, uint8_t);	//Callbacks for the espnow library have to be public due to the wrapper function, but we do not expect somebody to call them
		void espNowSendCallback(uint8_t*, uint8_t);
		#elif defined(ESP32)
		void espNowReceiveCallback(const uint8_t* , const uint8_t*, int32_t);
		void espNowSendCallback(const uint8_t*, esp_now_send_status_t);
		#endif

		//Debugging functions
		void enableDebugging(Stream &);						//Start debugging on a Stream, probably Serial but could be elsewhere
		void enableDebugging(Stream &, uint32_t);			//Start debugging on a Stream, probably Serial but could be elsewhere, and change the default logging level
		void enableDebugging();								//Start debugging if previously stopped
		void enableDebugging(uint32_t);						//Start debugging if previously stopped and set a level
		void disableDebugging();							//Stop debugging
		uint32_t loggingLevel();							//Returns current log level
		void setLoggingLevel(uint32_t );					//Sets current log level
		void nodeToLog(uint8_t);							//Sets the node to log
		void logAllNodes();									//Sets the node to log
		const uint32_t MESH_UI_LOG_ALL_SENT_PACKETS = 1048576ul;
		const uint32_t MESH_UI_LOG_ALL_RECEIVED_PACKETS = 524288ul;
		const uint32_t MESH_UI_LOG_WIFI_POWER_MANAGEMENT = 262144ul;
		const uint32_t MESH_UI_LOG_AP_MANAGEMENT = 131072ul;
		const uint32_t MESH_UI_LOG_SCANNING = 65536ul;
		const uint32_t MESH_UI_LOG_PEER_MANAGEMENT = 32768ul;
		const uint32_t MESH_UI_LOG_USR_SEND = 16384ul;
		const uint32_t MESH_UI_LOG_USR_RECEIVED = 8192ul;
		const uint32_t MESH_UI_LOG_USR_FORWARDING = 4096ul;
		const uint32_t MESH_UI_LOG_NHS_SEND = 2048ul;
		const uint32_t MESH_UI_LOG_NHS_RECEIVED = 1024ul;
		const uint32_t MESH_UI_LOG_NHS_FORWARDING = 512ul;
		const uint32_t MESH_UI_LOG_OGM_SEND = 256ul;
		const uint32_t MESH_UI_LOG_OGM_RECEIVED = 128ul;
		const uint32_t MESH_UI_LOG_OGM_FORWARDING = 64ul;
		const uint32_t MESH_UI_LOG_ELP_SEND = 32ul;
		const uint32_t MESH_UI_LOG_ELP_RECEIVED = 16ul;
		const uint32_t MESH_UI_LOG_ELP_FORWARDING = 8ul;
		const uint32_t MESH_UI_LOG_INFORMATION = 4ul;
		const uint32_t MESH_UI_LOG_WARNINGS = 2ul;
		const uint32_t MESH_UI_LOG_ERRORS = 1ul;

		const uint16_t PROTOCOL_USR_FORWARD = 32768;		//Node will forward user data - node will forward user application, essential for a relay but perhaps undesirable on end nodes
		const uint16_t PROTOCOL_USR_RECEIVE = 16384;		//Node will process user data - node will process user application data, not necessary on some devices
		const uint16_t PROTOCOL_USR_SEND = 8192;			//Node will send user data - node sends user application data, not necessary on some devices
		const uint16_t PROTOCOL_NHS_SUPPLY_VOLTAGE = 4096;	//Node will share its power information - useful to check the battery on devices, not essential
		const uint16_t PROTOCOL_NHS_TIME_SERVER = 2048;		//Node will share its time information	- this maintains a central timestamp across the nodes to help synchronise events
		const uint16_t PROTOCOL_NHS_INCLUDE_ORIGINATORS		//Node will share originator information - it will forward a list of ALL nodes, speeding global mesh discovery
			= 1024;
		const uint16_t PROTOCOL_NHS_FORWARD = 512;			//Node will forward NHS data - it will forward information about other nodes, useful for remote troubleshooting
		const uint16_t PROTOCOL_NHS_RECEIVE = 256;			//Node will process NHS data - it will process information about the health of other nodes, not typically essential
		const uint16_t PROTOCOL_NHS_SEND = 128;				//Node will send NHS data - it will shares 'health' information about itself
		const uint16_t PROTOCOL_OGM_FORWARD = 64;			//Node will forward OGM data - it will forward information about how to reach non-local nodes, essential for a functioning mesh
		const uint16_t PROTOCOL_OGM_RECEIVE = 32;			//Node will process OGM data - it will read information about how to reach non-local nodes
		const uint16_t PROTOCOL_OGM_SEND = 16;				//Node will send OGM data - it will share routing information about itself with non-local nodes
		const uint16_t PROTOCOL_ELP_INCLUDE_PEERS = 8;		//Node will share neighbour information - it will share information about its nearby neighbours, speeding local mesh discovery
		const uint16_t PROTOCOL_ELP_FORWARD = 4;			//Node will forward forward ELP packets - it will forward ELP packets from neighbours, typically this is disabled
		const uint16_t PROTOCOL_ELP_RECEIVE = 2;			//Node will process ELP packets - it listens for nearby neighbours
		const uint16_t PROTOCOL_ELP_SEND = 1;				//Node will send ELP packets - it will announce itself to nearby neighbours

		const uint8_t USR_DATA_UINT8_T = 0x01;				//Used to denote an uint8_t in user data
		const uint8_t USR_DATA_UINT16_T = 0x2;				//Used to denote an uint16_t in user data
		const uint8_t USR_DATA_UINT32_T = 0x03;				//Used to denote an uint32_t in user data
		const uint8_t USR_DATA_INT8_T = 0x04;				//Used to denote an int8_t in user data
		const uint8_t USR_DATA_INT16_T = 0x5;				//Used to denote an int16_t in user data
		const uint8_t USR_DATA_INT32_T = 0x06;				//Used to denote an int32_t in user data
		const uint8_t USR_DATA_FLOAT = 0x07;				//Used to denote a float in user data
		const uint8_t USR_DATA_CHAR = 0x08;					//Used to denote a char in user data
		const uint8_t USR_DATA_STRING = 0x09;				//Used to denote a String in user data
		const uint8_t USR_DATA_CHAR_ARRAY = 0x0a;			//Used to denote a character array in user data
		const uint8_t USR_DATA_UINT8_T_ARRAY = 0x0b;		//Used to denote a character array in user data
		const uint8_t USR_DATA_UNAVAILABLE = 0xff;			//Used to denote an uint8_t in user data

	private:
		//High level variables
		uint8_t _currentChannel = 1;						//Mesh channel is set at begin() and should never change
		char *_nodeName = nullptr;							//Node name, set with setNodeName()
		uint8_t _localMacAddress[6];						//Local MAC address
		uint8_t _broadcastMacAddress[6] = 					//Broadcast MAC address
		{0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
		bool _joinedMesh = false;							//Is this node part of an active mesh
		uint32_t _sequenceNumber = 1;						//Sequence number added to every packet
		uint16_t _serviceFlags = 0xffff ^					//Controls which protocols and services this node offers
			(PROTOCOL_NHS_SUPPLY_VOLTAGE |
			PROTOCOL_ELP_FORWARD);
		uint32_t _currentInterval[5];						//Packet sending intervals, per packet type
		uint32_t _lastSent[5];								//Internal timers, per packet type
		uint32_t _currentTtl[5];							//TTLs, per packet type
		bool _softAPstate = false;							//SoftAp enabled?
		bool _softAPstateChanged = false;					//Does the SoftAp need changing
		uint32_t _meshLastChanged = 0;
		bool _activityOcurred = false;
		uint32_t _lastHousekeeping = 0;
		const uint32_t _housekeepingInterval = 1000ul;

		//Packet buffers
		//circular_buffer<packetBuffer> _receiveBuffer(4);	//Circular packet buffer for incoming packets
		packetBuffer _receiveBuffer;						//Single receive buffer is available for mesh traffic
		packetBuffer _sendBuffer;							//Single send buffer is available

		
		//Debugging variables
		Stream *_debugStream = nullptr;						//Pointer to debugging stream
		bool _debugEnabled = false;							//Whether stream debugging is enabled or not

		//Originator related variables
		originatorInfo *_originator;						//Pointer to the originator table, which is allocated in the constructor function
		uint8_t _maxNumberOfOriginators = 16;				//The default number of maximum originators. This can be changed at begin(). Set to 0 for automatic allocation of resource, which may cause heap fragmentation and affect stability.
		uint8_t _numberOfOriginators = 0;					//The current number of originators
		uint8_t _numberOfReachableOriginators = 0;			//The current number of reachable originators
		uint8_t _numberOfActiveNeighbours = 0;				//The current number of nearby active neighbours
		uint8_t _meshMacAddress[6] = {0,0,0,0,0,0};			//A synthetic MAC address XORed from all the members used to indicate stability

		//Global packet flags & values
		const uint8_t ESP_NOW_MIN_PACKET_SIZE = 64;			//Minimum packet size
		const uint8_t ESP_NOW_MAX_PACKET_SIZE = 250;		//Maximum packet size
		const uint8_t MESH_PROTOCOL_VERSION = 0x01;			//Mesh version
		const uint8_t NO_FLAGS = 0x00;						//No flags on packet
		const uint8_t SEND_TO_ALL_NODES = 0x80;				//If in the packet it flags this packet be sent to all ESP-Now nodes and does not include a destination address
		const uint32_t ANTI_COLLISION_JITTER = 250ul;		//Jitter in milliseconds to try and avoid in-air collisions
		const uint8_t MESH_ORIGINATOR_NOT_FOUND = 255;		//Signifies that a node is unset or unknown
		const uint8_t MESH_NO_MORE_ORIGINATORS_LEFT = 254;	//Signifies the mesh is 'full' and no more members can join


		//ELP - Echo Location Protocol packet flags & values
		const uint8_t ELP_PACKET_SIZE = 19;
		const uint8_t ELP_PACKET_TYPE = 0x00;				//The first octet of every ESP-Now packet identifies what it is
		const uint8_t ELP_DEFAULT_TTL = 0;					//A TTL of >0 means it may be forwarded
		const uint8_t ELP_DEFAULT_FLAGS = SEND_TO_ALL_NODES;//Default ELP flags
		const uint8_t ELP_FLAGS_INCLUDES_PEERS = 0x01;		//Flag set when ELP includes peer information
		const uint8_t ELP_FLAGS_PEER_REMOVE_REQUEST = 0x02;	//Flag set indicating sending originator would like to be removed as peer
		const uint8_t ELP_FLAGS_PEER_REMOVED = 0x04;		//Flag set to indicate destination originator has been removed as peer

		const uint32_t ELP_DEFAULT_INTERVAL = 10000ul;		//How often to send ELP (default every 10s)
		const uint16_t LTQ_MAX_VALUE = 65535;				//Maximum value for Local Transmit Quality
		const uint16_t LTQ_STARTING_VALUE = 32768;			//Starting value for Local Transmit Quality given to a new peer

		//OGM - Originator Message packet flags & values
		const uint8_t OGM_PACKET_SIZE = 32;					//OGM packets are a fixed size
		const uint8_t OGM_PACKET_TYPE = 0x01;				//The first octet of every ESP-Now packet identifies what it is
		const uint8_t OGM_DEFAULT_TTL = 50;            		//A TTL of >0 means it may be forwared. OGM MUST be forwarded for a multi-hop mesh to form
		const uint8_t OGM_DEFAULT_FLAGS = SEND_TO_ALL_NODES;//Default OGM flags
		const uint32_t OGM_DEFAULT_INTERVAL = 60000ul; 		//How often to send OGM (default every 60s)
		const uint8_t OGM_HOP_PENALTY = 0x00ff;				//Drop the GTQ in OGM packets before forwarding
		const uint8_t SEQUENCE_NUMBER_MAX_AGE = 8;			//Used for detection of sequence number resets/rollover

		//NHS - Node Health/Status packet flags & values
		const uint8_t NHS_PACKET_TYPE = 0x02;          		//The first octet of every ESP-Now packet identifies what it is
		const uint8_t NHS_DEFAULT_TTL = 50;            		//A TTL of >0 means it may be forwarded.
		const uint8_t NHS_DEFAULT_FLAGS = SEND_TO_ALL_NODES;//Default NHS flags
		const uint8_t NHS_FLAGS_TIMESERVER = 0x1;        	//Flag set when acting as a time server
		const uint8_t NHS_FLAGS_SOFTAP_ON = 0x2;        	//Flag set when acting as an AP
		const uint8_t NHS_FLAGS_NODE_NAME_SET = 0x4;		//Flag set when the NHS packet includes a friendly name
		const uint8_t NHS_FLAGS_INCLUDES_ORIGINATORS = 0x08;//Flag set when the NHS packet includes peer information
		const uint8_t NHS_FLAGS_INCLUDES_VCC = 0x10;		//Flag set when the NHS packet includes supply voltage
		const uint32_t NHS_DEFAULT_INTERVAL = 300000ul; 	//How often to send NHS (default every 5m)

		bool _actingAsTimeServer = true;					//Everything starts out as a potential time server, but defers to longer running nodes
		uint8_t _currentMeshTimeServer 						//The current time server
			= MESH_ORIGINATOR_NOT_FOUND;
		int32_t _meshTimeOffset = 0;						//NHS keeps a semi-synced clock to the device with the longest uptime. This is not uber-accurate and may fluctuate slightly!
		int32_t _meshTimeDrift = 0;							//Track clock drift over time, just for info
		uint32_t _initialFreeHeap = ESP.getFreeHeap();		//Used to get a percentage value for free heap
		uint32_t _currentFreeHeap = _initialFreeHeap;		//Monitor free heap
		uint32_t _rxPackets = 0;							//Monitor received packets
		uint32_t _txPackets = 0;							//Monitor sent packets
		uint32_t _droppedRxPackets = 0;						//Monitor buffer receive problems
		uint32_t _droppedTxPackets = 0;						//Monitor buffer send problems

		//USR - User data packet flags and values
		const uint8_t USR_PACKET_SIZE = 15;					//USR packets are variable length but have a 32 byte header
		const uint8_t USR_PACKET_TYPE = 0x03;				//The first octet of every ESP-Now packet identifies what it is
		const uint8_t USR_DEFAULT_TTL = 50;					//A TTL of >0 means it may be forwared. OGM MUST be forwarded for a multi-hop mesh to form
		const uint8_t USR_DEFAULT_FLAGS = SEND_TO_ALL_NODES;//Not yet implemented
		const uint8_t USR_MAX_PACKET_SIZE = 250;			//Maximum packet size for user data
		const uint32_t USR_DEFAULT_INTERVAL = 1000;			//How often to send low priority user data

		//FSP - File Sync Protocol, for SDFat filesystem
		const uint8_t FSP_PACKET_TYPE = 0x04;				//The first octet of every ESP-Now packet identifies what it is
		const uint8_t FSP_DEFAULT_TTL = 50;					//A TTL of >0 means it may be forwared. OGM MUST be forwarded for a multi-hop mesh to form
		const uint8_t FSP_DEFAULT_FLAGS = SEND_TO_ALL_NODES;//Not yet implemented
		const uint32_t FSP_DEFAULT_INTERVAL = 1000;			//How often to send low priority file sync data

		//Power management & sleep related variables
		const uint32_t POWER_MANAGEMENT_INTERVAL = 10000;	//How often to adjust the Wi-Fi Tx power
		const uint32_t FLOOR_MANAGEMENT_INTERVAL = 300000;	//How often we review the power floor (5m)
		const float MIN_TX_POWER = 0.0f;					//Minimum Tx power. THis is multiplied by four internally by the Wi-Fi library so 0-20.5 is actually 0-82dBm
		const float MAX_TX_POWER = 20.5f;					//Maximum Tx power
		const float VCC_HIGH_THRESHOLD = 3.6f;				//High voltage that triggers an out of bounds warning
		const float VCC_LOW_THRESHOLD = 3.0f;				//Low voltage that triggers an out of bounds warning

		bool _powerManagementEnabled = true;

		float _currentTxPower = MAX_TX_POWER;				//Always start at maximum power to help with joining the mesh
		float _txPowerFloor = MIN_TX_POWER;					//We find a 'floor' on the TX power to stop nodes oscillating in and out of packet loss
		uint32_t _txPowerFloorTimer = 0;					//How often this node reconsiders the power floor
		uint32_t _powerManagementTimer = 0;					//Don't do power management until there's been enough time to pick up and settle some peers

		//Time syncing functions
		void _updateMeshTime(uint32_t, uint8_t);			//Updates the time from an advertising time server
		void _setMeshTime(uint32_t, uint8_t);
		void _chooseNewTimeServer();						//If the current time server disappears, try to pick a new one
		void _becomeTimeServer();
		
		//ESP-NOW related functions in many of these there are lots of preprocessor directives to handle API differences between ESP8266/8285 and ESP32 at compile time
		void _initESPNow();									//Initialises ESP-NOW
		#if defined(ESP8266)
		uint8_t _sendPacket(packetBuffer &);					//Sends ESP-NOW from a packet buffer
		#elif defined(ESP32)
		esp_err_t _sendPacket(packetBuffer &);
		#endif

		//Originator management functions
		uint8_t _originatorIdFromMac(uint8_t *);			//Finds the ID of an originator from the MAC address or MESH_ORIGINATOR_NOT_FOUND if it isn't found
		uint8_t _originatorIdFromMac(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
		uint8_t _addOriginator(uint8_t *,uint8_t);			//Adds an originator given the MAC address and channel
		void _calculateLtq(uint8_t);						//Calculates the LTQ for a node
		bool _isLocalMacAddress(uint8_t *);					//Checks if a MAC address is the local one

		//Routing protocol functions
		bool _sendElp(packetBuffer &);						//Send ELP with default settings
		bool _sendElp(bool, packetBuffer &);				//Send ELP with or without peers
		bool _sendElp(uint8_t, packetBuffer &);				//Send ELP with specific TTL
		bool _sendElp(bool,uint8_t, packetBuffer &);		//Send ELP with or without peers and specific TTL
		void _processElp(uint8_t, uint8_t, packetBuffer &);	//Process incoming ELP payload


		bool _sendOgm(packetBuffer &);						//Send OGM with default settings
		void _processOgm(uint8_t, uint8_t, packetBuffer &);	//Process incoming OGM payload

		bool _sendNhs(packetBuffer &);						//Send NHS with default settings, using the supplied buffer to build the packet
		void _processNhs(uint8_t, uint8_t, packetBuffer &);	//Process incoming NHS payload
		
		void _processUsr(uint8_t, uint8_t, packetBuffer &);	//Process incoming USR payload

		void _processPacket(packetBuffer &);				//Process packet headers and handle forwarding
		bool _dataIsValid(uint8_t, uint8_t);				//Is a particular protocol up?
		void _originatorHasBecomeRoutable(uint8_t);
		void _originatorHasBecomeUnroutable(uint8_t);

		//Status LED
		const uint32_t STATUS_LED_BLINK_TIME = 25ul;		//How long the status LED is on
		const uint32_t STATUS_LED_PAUSE_TIME = 300ul;		//How long between flashes
		const uint32_t STATUS_LED_FREQUENCY = 10000ul;		//How frequently the status is signalled
		bool _statusLedEnabled = false;						//Is the status LED enabled?
		bool _statusLedState = false;						//Is the status LED lit?
		uint8_t _statusLedPin = 0;							//The status LED pin
		bool _statusLedOnState = HIGH;						//The pin state when the LED is lit
		uint32_t _statusLedTimer = 0;						//Used to synchronise the status LEDs
		uint8_t _statusLedCount = 0;						//Flash counter

		//Activity LED
		const uint32_t ACTIVITY_LED_BLINK_TIME = 10ul;		//How long the activity LED is on
		bool _activityLedEnabled = false;					//Is the activity LED enabled?
		bool _activityLedState = false;						//Is the activity LED lit?
		uint8_t _activityLedPin = 0;						//The activity LED pin
		bool _activityLedOnState = HIGH;					//The pin state when the LED is lit
		uint32_t _activityLedTimer = 0;						//Used to hold the LED on for a fixed time
		void _blinkActivityLed();							//Function switches on activity LED

		//User data related variables
		uint8_t _userPacketIndex = 0;								//Current position in the user packet
		uint8_t _userPacketFieldCounterIndex = 0;					//The index where the field count is
		//uint8_t _userPacket[USR_MAX_PACKET_SIZE];					//The user packet
		uint8_t _userPacket[250];									//The user packet

		bool _userPacketReceived = false;							//Is there a packet waiting to be read
		uint8_t _receivedUserPacketIndex = 0;						//Index during decode
		uint8_t _receivedUserPacketLength = 0;						//Length of packet to decode
		uint8_t _receivedUserPacketFieldCounter = 0;						//Fields left in packet
		//uint8_t _receivedUserPacket[USR_MAX_PACKET_SIZE];
		
		uint8_t _receivedUserPacket[250];
		
		bool _buildingUserPacket = false;													//Are we building a user data packet? Mustn't do it twice
		void _buildUserPacketHeader();														//Create the user packet header for a broadcast packet
		void _buildUserPacketHeader(uint8_t);												//Create the user packet header based off a station ID
		void _buildUserPacketHeader(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);	//Create the user packet header based off a MAC address
		
		//Debugging related functions
		uint32_t _loggingLevel = MESH_UI_LOG_INFORMATION | MESH_UI_LOG_WARNINGS | MESH_UI_LOG_ERRORS | MESH_UI_LOG_PEER_MANAGEMENT;
		uint8_t _nodeToLog = MESH_ORIGINATOR_NOT_FOUND;
		#if defined(ESP8266)
		void _errorDescription(uint8_t);					//Prints an error description
		#elif defined(ESP32)
		void _errorDescription(esp_err_t);
		#endif
		#ifdef ESP8266
		//void _debugPacket(uint8_t *, uint8_t *, uint8_t); //Decode incoming packet for debugging purpose
		void _debugPacket(packetBuffer &); //Decode incoming packet for debugging purpose
		#elif defined(ESP32)
		//void _debugPacket(const uint8_t *, const uint8_t *, int32_t);
		void _debugPacket(packetBuffer &);
		#endif
		void _friendlyUptime(uint32_t, char *);				//Formats uptime as a duration string
		void _packetTypeDescription(char*,uint8_t);			//Formats a packet type into a char array

		
};

//m2mMesh mesh;						//Create the sole possible instance of the m2mMesh class straight away for the user to access

#endif