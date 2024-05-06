#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>


// TrustLib Parameters
#define TRUSTLIST_SIZE				50
#define NODEID_SIZE					33
#define NEXT_LIMIT_TIME_S			2
#define PENALITY					1

#define LATENCY_NORMALIZATION		0
#define SIGNAL_NORMALIZATION		1
#define BATTERY_NORMALIZATION		2
#define FIABILITY_NORMALIZATION		3

#define LATENCY_MAX_US				30000000
#define LATENCY_MIN_US				0

#define LOW_SIGNAL					-100
#define HIGH_SIGNAL					-37

#define BATTERY_MIN					0
#define BATTERY_MAX					100

#define FIBILITY_MIN				0
#define FIBILITY_MAX				1

#define SUCCESS						1
#define FAIL						-1



// TrustList
typedef struct TrustList TrustList;
struct TrustList
{
	uint8_t NodeID[NODEID_SIZE];
	float TrustLevel;
	uint32_t LimitTime;
};

/*** TrustLib Functions ***/

// Init TrustList
void InitTrustList(void);


// Latency Chronometer
// Start_End: Start (0) / End (1)
void LatencyChronometer(uint8_t Start_End);


// Compute Normalization
// Type of normaliation: Latency, Signal, Battery, Fiability
float Normalization(uint8_t Type, float Value, double MIN_VALUE, double MAX_VALUE);
//#include <sys/ioctl.h>
//#include <linux/wireless.h>
//float NormalizedSignalQuality2(int NetworkSocket);


// Compute Sensor Aggregation
float SensorAggregation(int TransactionStatus, float NLatency, float NSignalQuality, float NBatteryLevel);


// Compute Actuator Aggregation
float ActuatorAggregation(int ActionStatus, float NLatency, float NSignalQuality);


// Compute Gateway Aggregation
float GatewayAggregation(int Utility, float NFiabilityScore, float NSignalQuality);


// Compute TrustLib Algorithm
void TrustLibAlgorithm(uint8_t* NodeID, float AggregationValue);


// Get TrustLevel of NodeID
float GetNodeTrustLevel(uint8_t* NodeID);


// Get LimitTime of NodeID
float GetNodeLimitTime(uint8_t* NodeID);


// Get Index of NodeID with best TrustLevel
int GetBestNode(void);


// Display TrustList
void DisplayTrustList(void);