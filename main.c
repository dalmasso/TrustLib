#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "TrustLib.h"

// Network Configuration
#define MULTICAST_IP				"224.168.0.1"
#define MULTICAST_PORT 				10000
#define MULTICAST_TTL				1
#define MULTICAST_TIMEOUT			2


#define SENSOR_TRANSACTION_TYPE			35		// Also the Size of Sensor Transaction: Publisher (33) + Sensor Data (2)
#define TRIG_REQUEST_TRANSACTION_TYPE	68		// Also the Size of Sensor Transaction: Subscriber (33) + SmartContract (35)
#define REQUEST_TRANSACTION_TYPE		166		// Also the Size of Request Transaction
#define CONSENSUS_TRANSACTION_TYPE		201		// Also the Size of Consensus Transaction


// Set Socket Configuration Type
typedef struct sockaddr_in SOCKADDR_IN;


typedef struct Transaction Transaction;
struct Transaction
{
	uint8_t Subscriber[33];
	uint8_t Publisher[33];
	uint8_t SmartContract[35];
	uint8_t State[32];
	uint16_t DCoin;
	uint16_t Nonce;
	uint8_t Signature[64];
};



// Serialize Transaction
// MyTX: 			Transaction to serialize
// TXType:			Type of Transaction to transmit (Request or Consensus Transaction)
// SerializedTX:	Serialized Transaction
void SerializedTransaction(Transaction MyTX, int TXType, uint8_t* SerializedTX)
{
	uint8_t Offset = 0;
	uint8_t temp;

	// Subscriber (Consensus Transaction only)
	if (TXType == CONSENSUS_TRANSACTION_TYPE)
	{
		memcpy(SerializedTX, MyTX.Subscriber, sizeof(MyTX.Subscriber));
		Offset += sizeof(MyTX.Subscriber);
	}

	// Publisher
	memcpy(SerializedTX+Offset, MyTX.Publisher, sizeof(MyTX.Publisher));
	Offset += sizeof(MyTX.Publisher);

	// SmartContract
	memcpy(SerializedTX+Offset, MyTX.SmartContract, sizeof(MyTX.SmartContract));
	Offset += sizeof(MyTX.SmartContract);

	// State
	memcpy(SerializedTX+Offset, MyTX.State, sizeof(MyTX.State));
	Offset += sizeof(MyTX.State);

	// DCoin (Consensus Transaction only)
	if (TXType == CONSENSUS_TRANSACTION_TYPE)
	{
		// For DCoin MSB then LSB bytes
		temp = MyTX.DCoin>>8;
		memcpy(SerializedTX+Offset, &temp, sizeof(temp));
		Offset += sizeof(temp);
		
		// For DCoin MSB then LSB bytes
		temp = MyTX.DCoin & 0x00FF;
		memcpy(SerializedTX+Offset, &temp, sizeof(temp));
		Offset += sizeof(temp);
	}

	// For Nonce MSB then LSB bytes
	temp = MyTX.Nonce>>8;
	memcpy(SerializedTX+Offset, &temp, sizeof(temp));
	Offset += sizeof(temp);
	
	// For Nonce MSB then LSB bytes
	temp = MyTX.Nonce & 0x00FF;
	memcpy(SerializedTX+Offset, &temp, sizeof(temp));
	Offset += sizeof(temp);
	
	// Signature
	memcpy(SerializedTX+Offset, MyTX.Signature, sizeof(MyTX.Signature));
}


// DeSerialize Transaction
// SerializedTX:	Serialized Transaction to Deserialize
// TXType:			Type of received Transaction (Sensor Data, Request Transaction Trigger, Request or Consensus Transaction)
// MyTX: 			Deserialized Transaction
void DeSerializeTransaction(uint8_t* SerializedTX, int TXType, Transaction* MyTX)
{
	uint8_t Offset = 0;
	uint16_t temp;

	// Sensor Data: Publisher (33) + Data (2)
	if (TXType == SENSOR_TRANSACTION_TYPE)
	{
		memcpy(MyTX->Publisher, SerializedTX+Offset, 33);
		Offset += 33;

		// Warning, Sensor Data (in DCoin part) SerializedTX in MSB - LSB bytes
		temp = (uint8_t)*(SerializedTX+Offset)*256 + (uint8_t)*(SerializedTX+Offset+1);
		(*MyTX).DCoin = temp;
	}

	// Request Transaction Trigger from Grafana Interface
	else if (TXType == TRIG_REQUEST_TRANSACTION_TYPE)
	{
		// Subscriber
		memcpy(MyTX->Subscriber, SerializedTX+Offset, 33);
		Offset += 33;

		// SmartContract
		memcpy(MyTX->SmartContract, SerializedTX+Offset, 35);
	}

	// Resquest/Consensus Transaction
	else
	{
		// Subscriber
		memcpy(MyTX->Subscriber,SerializedTX+Offset, 33);
		Offset += 33;

		// Request Transaction (Subscriber == Publisher)
		if (TXType == REQUEST_TRANSACTION_TYPE)
			memcpy(MyTX->Publisher,MyTX->Subscriber, 33);

		else
		{
			memcpy(MyTX->Publisher,SerializedTX+Offset, 33);
			Offset += 33;
		}

		// SmartContract
		memcpy(MyTX->SmartContract, SerializedTX+Offset, 35);
		Offset +=35;

		// State
		memcpy(MyTX->State, SerializedTX+Offset, 32);
		Offset += 32;

		// DCoin (Consensus Transaction only)
		if (TXType == CONSENSUS_TRANSACTION_TYPE)
		{
			// Warning, DCoin SerializedTX in MSB - LSB bytes
			temp = (uint8_t)*(SerializedTX+Offset)*256 + (uint8_t)*(SerializedTX+Offset+1);
			(*MyTX).DCoin = temp;
			Offset += 2;
		}
		else
			(*MyTX).DCoin = 0x0000;	// Default Value

		// Nonce
		// Warning, Nonce SerializedTX in MSB - LSB bytes
		temp = (uint8_t)*(SerializedTX+Offset)*256 + (uint8_t)*(SerializedTX+Offset+1);
		(*MyTX).Nonce = temp;
		Offset += 2;			

		// Signature
		memcpy(MyTX->Signature, SerializedTX+Offset, 64);
	}
}


// Initialization of RX Socket Network
// TopicIPAddr: IP Address of the Topic to listen
// Return:		ERROR (-1) / Reception Socket File Descriptor
int NetworkInitRXSocket(void)
{
	// RX Socket Configuration
	int RXSocket;
	SOCKADDR_IN RXConfig;

	// MulticastGroup (IP Multicast Address of the Group, Local IP Address of Interface)
	struct ip_mreq MulticastGroup = {inet_addr(MULTICAST_IP), htonl(INADDR_ANY)};

	// Multicast Timeout (Timeout in Second, Timeout in uSecond)
	struct timeval MulticastTimeout = {MULTICAST_TIMEOUT, 0};

	// Create Reception Socket
	RXSocket = socket(AF_INET, SOCK_DGRAM, 0);

	if (RXSocket < 0)
		return -1;

	// Configuration of RXSocket
	memset(&RXConfig, 0, sizeof(RXConfig));
	RXConfig.sin_family = AF_INET;
	RXConfig.sin_port = htons(MULTICAST_PORT);
	RXConfig.sin_addr.s_addr = htonl(INADDR_ANY);

	// Set Multicast Receive Timeout
	if (setsockopt(RXSocket, SOL_SOCKET, SO_RCVTIMEO, (char*) &MulticastTimeout, sizeof(MulticastTimeout)) < 0)
		return -1;

	// Join the MulticastGroup
	if (setsockopt(RXSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*) &MulticastGroup, sizeof(MulticastGroup)) < 0)
		return -1;

	// Bind RXSocket
	if (bind(RXSocket, (struct sockaddr*) &RXConfig, sizeof(RXConfig)) < 0)
		return -1;

	return RXSocket;   
}


// Initialization of TX Socket Network 
// Return:  ERROR (-1) / Transmission Socket File Descriptor
int NetworkInitTXSocket(void)
{
	// TX Socket Configuration
	int TXSocket;

	// Multicast TTL Configuration
	u_char TTL = MULTICAST_TTL;

	// Multicast Timeout (Timeout in Second, Timeout in USecond)
	struct timeval MulticastTimeout = {MULTICAST_TIMEOUT, 0};
	
	// Create Transmission Socket
	TXSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (TXSocket < 0)
		return -1;

	// Configure Multicast TTL
	if (setsockopt(TXSocket, IPPROTO_IP, IP_MULTICAST_TTL, &TTL, sizeof(TTL)) < 0)
		return -1;

	// Set Multicast Transmit Timeout
	if (setsockopt(TXSocket, SOL_SOCKET, SO_SNDTIMEO, (char*) &MulticastTimeout, sizeof(MulticastTimeout)) < 0)
		return -1;
	return TXSocket;
}

// Send Transaction (Request or Consensus)  to the Network
// TXSocket:	Transmission Socket File Descriptor
// TXType:		Type of Transaction to transmit (Request or Consensus)
// MyTX:		Transaction to transmit
// Return:		Number of Transmitted bytes
ssize_t Send_Transaction(int TXSocket, int TXType, Transaction MyTX)
{
	uint8_t SerializedTX[201];

	// TXSocket Configuration
	SOCKADDR_IN TXConfig;
	memset(&TXConfig, 0, sizeof(TXConfig));
	TXConfig.sin_family = AF_INET;
	TXConfig.sin_port = htons(MULTICAST_PORT);
	TXConfig.sin_addr.s_addr = inet_addr(MULTICAST_IP);

	// Serialize Transaction
	SerializedTransaction(MyTX, TXType, SerializedTX);

	// Send Transaction to the Network (TXType == TXSize)
	return sendto(TXSocket, SerializedTX, TXType, 0, (struct sockaddr*) &TXConfig, sizeof(TXConfig));
}


// Receive Transaction from Network
// RXSocket:	Reception Socket File Descriptor
// MyTX:		Received Transaction
// Return:		No Data Available (-1) / Sensor Data or Request Transaction or Consensus Transaction Available (>0)
int Receive_Transaction(int RXSocket, Transaction* MyTX)
{
	uint8_t SerializedTX[201];
	ssize_t Received; 

	SOCKADDR_IN ClientConfig;
	socklen_t Length = sizeof(ClientConfig);

	// Read Socket
	Received = recvfrom(RXSocket, &SerializedTX, sizeof(SerializedTX), 0, (struct sockaddr*) &ClientConfig, &Length);

	// Sensor Data
	// Request Transaction (REQUEST_TRANSACTION_TYPE == Size of Request Transaction)
	// Consensus Transaction (CONSENSUS_TRANSACTION_TYPE == Size of Consensus Transaction)
	if (Received > 0)
	{
		DeSerializeTransaction(SerializedTX, Received, MyTX);
		return Received;
	}

	// No received
	else
		return -1;
}


// Close Network
// RXSocket:	Reception Socket File Descriptor
// TXSocket:	Transmission Socket File Descriptor
void DeInit_Network(int* RXSocket, int* TXSocket)
{
	close(*RXSocket);
	close(*TXSocket);
}




int main (void)
{
	int RXSocket;
	int TXSocket;
	int Result;
	Transaction MyTX;

	struct timeval Start, End;
	float MyTest;

	TrustList MyTEST[TRUSTLIST_SIZE];

	for (int i=0; i < TRUSTLIST_SIZE; i++)
	{
		memset(MyTEST[i].NodeID, rand()%256, NODEID_SIZE);
		MyTEST[i].TrustLevel = 1.0;
		MyTEST[i].LimitTime = time(NULL);
	}


	printf("Hello World\n");

	// Init Network
	RXSocket = NetworkInitRXSocket();
	TXSocket = NetworkInitTXSocket();

	if ( (RXSocket == -1) || (TXSocket == -1) )
		return -1;

	// Create New Transactions
	memset(MyTX.Subscriber, 1, sizeof(MyTX.Subscriber));
	memset(MyTX.Publisher, 2, sizeof(MyTX.Publisher));
	memset(MyTX.SmartContract, 3, sizeof(MyTX.SmartContract));
	memset(MyTX.State, 4, sizeof(MyTX.State));
	memset(MyTX.Signature, 5, sizeof(MyTX.Signature));

	// Init Random Number Generator
	time_t t;
    srand((unsigned) time(&t));

/*
	printf("\n\n*** Evaluation of Signal normalization 2 ***\n");
	for(int i=0; i<1000;i++)
	{
		gettimeofday(&Start, NULL);
		MyTest = NormalizedSignalQuality2(RXSocket);
		gettimeofday(&End, NULL);
		printf("TIME: %ld\n", ((End.tv_sec - Start.tv_sec) * 1000000 + End.tv_usec) - Start.tv_usec);
	}
*/

	printf("*** Evaluation of Latency normalization ***\n");
	for(int i=0; i<1000;i++)
	{
		LatencyChronometer(0);

		Result = Send_Transaction(TXSocket, REQUEST_TRANSACTION_TYPE, MyTX);

		if (Result == REQUEST_TRANSACTION_TYPE)
		{
			Result = Receive_Transaction(RXSocket, &MyTX);
			LatencyChronometer(1);
		}
		else
			printf("ERROR SEND: %d\n", Result);

		// Compute Normalized Latency
		gettimeofday(&Start, NULL);
		MyTest = Normalization(LATENCY_NORMALIZATION, 0, LATENCY_MIN_US, LATENCY_MAX_US);
		gettimeofday(&End, NULL);
		printf("TIME: %ld\n", ((End.tv_sec - Start.tv_sec) * 1000000 + End.tv_usec) - Start.tv_usec);
	}


	printf("\n\n*** Evaluation of Signal normalization ***\n");
	for(int i=0; i<1000;i++)
	{
		float r = -rand()%101;
		gettimeofday(&Start, NULL);
		MyTest = Normalization(SIGNAL_NORMALIZATION, r, LOW_SIGNAL, HIGH_SIGNAL);
		gettimeofday(&End, NULL);
		printf("TIME: %ld\n", ((End.tv_sec - Start.tv_sec) * 1000000 + End.tv_usec) - Start.tv_usec);
	}


	printf("\n\n*** Evaluation of Battery normalization ***\n");
	for(int i=0; i<1000;i++)
	{
		float r = -rand()%101;
		gettimeofday(&Start, NULL);
		MyTest = Normalization(BATTERY_NORMALIZATION, r, BATTERY_MIN, BATTERY_MAX);
		gettimeofday(&End, NULL);
		printf("TIME: %ld\n", ((End.tv_sec - Start.tv_sec) * 1000000 + End.tv_usec) - Start.tv_usec);
	}


	printf("\n\n*** Evaluation of Fiability normalization ***\n");
	for(int i=0; i<1000;i++)
	{
		float r = -rand()%101;
		gettimeofday(&Start, NULL);
		MyTest = Normalization(FIABILITY_NORMALIZATION, r, FIBILITY_MIN, FIBILITY_MAX);
		gettimeofday(&End, NULL);
		printf("TIME: %ld\n", ((End.tv_sec - Start.tv_sec) * 1000000 + End.tv_usec) - Start.tv_usec);
	}

	printf("\n\n*** Evaluation of Sensor Aggregation ***\n");
	for(int i=0; i<1000;i++)
	{
		int r = rand()%2;
		gettimeofday(&Start, NULL);
		MyTest = SensorAggregation(r, 0.0, 1.0, 1.0);
		gettimeofday(&End, NULL);
		printf("TIME: %ld \t%d\n", ((End.tv_sec - Start.tv_sec) * 1000000 + End.tv_usec) - Start.tv_usec,r);
	}


	printf("\n\n*** Evaluation of Actuator Aggregation ***\n");
	for(int i=0; i<1000;i++)
	{
		int r = rand()%2;
		gettimeofday(&Start, NULL);
		MyTest = ActuatorAggregation(r, 0.0, 1.0);
		gettimeofday(&End, NULL);
		printf("TIME: %ld \t%d\n", ((End.tv_sec - Start.tv_sec) * 1000000 + End.tv_usec) - Start.tv_usec, r);
	}


	printf("\n\n*** Evaluation of Gateway Aggregation ***\n");
	for(int i=0; i<1000;i++)
	{
		int r = rand()%2;
		gettimeofday(&Start, NULL);
		MyTest = GatewayAggregation(r, 1.0, 1.0);
		gettimeofday(&End, NULL);
		printf("TIME: %ld \t%d\n", ((End.tv_sec - Start.tv_sec) * 1000000 + End.tv_usec) - Start.tv_usec,r);
	}



	printf("\n\n*** Evaluation of TrustList Initialization  ***\n");
	for(int i=0; i<1000;i++)
	{
		gettimeofday(&Start, NULL);
		InitTrustList();
		gettimeofday(&End, NULL);
		printf("TIME: %ld\n", ((End.tv_sec - Start.tv_sec) * 1000000 + End.tv_usec) - Start.tv_usec);
	}

	
	printf("\n\n*** Evaluation of TrustList Management - Add x %d  ***\n", TRUSTLIST_SIZE);
	for(int i=0; i<TRUSTLIST_SIZE;i++)
	{
		gettimeofday(&Start, NULL);
		TrustLibAlgorithm(MyTEST[i].NodeID, 5.0);
		gettimeofday(&End, NULL);
		printf("TIME: %ld\n", ((End.tv_sec - Start.tv_sec) * 1000000 + End.tv_usec) - Start.tv_usec);
	}

	DisplayTrustList();


	printf("\n\n*** Evaluation of TrustList Recover TrustLevel ***\n");
	for(int i=0; i<1000;i++)
	{
		// Select random node
		int idx = rand() % TRUSTLIST_SIZE+1;

		gettimeofday(&Start, NULL);
		GetNodeTrustLevel(MyTEST[idx].NodeID);
		gettimeofday(&End, NULL);

		printf("TIME: %ld\n", ((End.tv_sec - Start.tv_sec) * 1000000 + End.tv_usec) - Start.tv_usec);
	}


	printf("\n\n*** Evaluation of TrustList Recover LimitTime ***\n");
	for(int i=0; i<1000;i++)
	{
		// Select random node
		int idx = rand() % TRUSTLIST_SIZE+1;

		gettimeofday(&Start, NULL);
		GetNodeLimitTime(MyTEST[idx].NodeID);
		gettimeofday(&End, NULL);

		printf("TIME: %ld\n", ((End.tv_sec - Start.tv_sec) * 1000000 + End.tv_usec) - Start.tv_usec);
	}

	// To pass out LimitTime
	sleep(5);

	printf("\n\n*** Evaluation of TrustList Management ***\n");
	for(int i=0; i<1000;i++)
	{
		// Select random node
		int idx = rand() % TRUSTLIST_SIZE+1;

		gettimeofday(&Start, NULL);
		TrustLibAlgorithm(MyTEST[idx].NodeID, 2.0);
		gettimeofday(&End, NULL);

		printf("TIME: %ld\n", ((End.tv_sec - Start.tv_sec) * 1000000 + End.tv_usec) - Start.tv_usec);
	}


	printf("\n\n*** Evaluation of TrustList Get Best Node  ***\n");
	for(int i=0; i<1000;i++)
	{
		int get = 0;
		gettimeofday(&Start, NULL);
		get = GetBestNode();
		gettimeofday(&End, NULL);
		printf("TIME: %ld  %d\n", ((End.tv_sec - Start.tv_sec) * 1000000 + End.tv_usec) - Start.tv_usec, get);
	}

	DisplayTrustList();

	DeInit_Network(&RXSocket, &TXSocket);
	printf("\nEnd of Node\n");
}