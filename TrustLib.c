#include "TrustLib.h"


/*** TrustLib Variables ***/

// TrustLib - TrustList
TrustList MyTrustList[TRUSTLIST_SIZE];

// TrustLib - Latency Start & End
struct timeval LatencyStart, LatencyEnd;



/*** TrustLib Functions ***/

// Init TrustList
void InitTrustList(void)
{
	for (int i=0; i < TRUSTLIST_SIZE; i++)
	{
		memset(MyTrustList[i].NodeID, 0, NODEID_SIZE);
		MyTrustList[i].TrustLevel = 0.0;
		MyTrustList[i].LimitTime = 0;
	}
}


// Latency Chronometer
// Start_End: Start (0) / End (1)
void LatencyChronometer(uint8_t Start_End)
{
	if (Start_End == 0)
		gettimeofday(&LatencyStart, NULL);
	else
		gettimeofday(&LatencyEnd, NULL);
}


// Compute Normalization
// Type of normaliation: Latency, Signal, Battery, Fiability
float Normalization(uint8_t Type, float Value, double MIN_VALUE, double MAX_VALUE)
{
	switch(Type)
	{
		case LATENCY_NORMALIZATION:
			return (float) ((((LatencyEnd.tv_sec - LatencyStart.tv_sec) * 1000000 + LatencyEnd.tv_usec) - LatencyStart.tv_usec) - (double) MIN_VALUE)/((double) MAX_VALUE - (double) MIN_VALUE);

		default:
			return (float)((Value - (int) MIN_VALUE)/((int) MAX_VALUE - (int) MIN_VALUE));
	}
}


/*
float NormalizedSignalQuality2(int NetworkSocket)
{
	int SignalLevel;
	struct iwreq req;
	struct iw_statistics stats;

	// Set Network Interface Name
	strcpy(req.ifr_name, NETWORK_INTERFACE);
	req.u.data.length = sizeof(struct iw_statistics);
	req.u.data.pointer = &stats;

	// Recover the Signal Strength
	if(ioctl(NetworkSocket, SIOCGIWSTATS, &req) == -1)
		SignalLevel = LOW_SIGNAL;

	else
		SignalLevel = ((struct iw_statistics *)req.u.data.pointer)->qual.level - 256;

	// Normalization
	return (float)((SignalLevel - LOW_SIGNAL)/(HIGH_SIGNAL-LOW_SIGNAL));
}
*/

// Compute Sensor Aggregation
float SensorAggregation(int TransactionStatus, float NLatency, float NSignalQuality, float NBatteryLevel)
{
	if (TransactionStatus == SUCCESS)
		return (float) ( (1/ ((1 + NLatency) + (2 - NSignalQuality))) * (1/ NBatteryLevel) );
	else
		return - (float) ( ((1 + NLatency) + (2 - NSignalQuality)) * (1/ NBatteryLevel) );
}


// Compute Actuator Aggregation
float ActuatorAggregation(int ActionStatus, float NLatency, float NSignalQuality)
{
	if (ActionStatus == SUCCESS)
		return (float) (1/ ((1 + NLatency) + (2 - NSignalQuality)));
	else
		return - (float) ((1 + NLatency) + (2 - NSignalQuality));
}


// Compute Gateway Aggregation
float GatewayAggregation(int Utility, float NFiabilityScore, float NSignalQuality)
{
	if (Utility == SUCCESS)
		return (float) (1/ ((2 - NFiabilityScore) + (2 - NSignalQuality)));
	else
		return - (float) ((2 - NFiabilityScore) + (2 - NSignalQuality));
}


// Compute TrustLib Algorithm
void TrustLibAlgorithm(uint8_t* NodeID, float AggregationValue)
{
	int i = 0;
	int Found = 0;
	int FreePos = -1;

	// Recover Current Date
	uint32_t CurrentDate = time(NULL);

	// Parse the TrustList
	for (i = 0; i < TRUSTLIST_SIZE; i++)
	{
		// Detect if this index is free
		if (MyTrustList[i].TrustLevel > 0.0)
		{
			// Check if it is the NodeID
			if (strcmp(NodeID, MyTrustList[i].NodeID) == 0)
			{
				// Check Current Date to apply Penality
				if (CurrentDate > MyTrustList[i].LimitTime)
					MyTrustList[i].TrustLevel += (float) (AggregationValue - PENALITY);

				else
					MyTrustList[i].TrustLevel += AggregationValue;

				// Update Limit Time
				MyTrustList[i].LimitTime = CurrentDate + NEXT_LIMIT_TIME_S;

				// NodeID Found Flag
				Found = 1;
			}

			// Other Nodes
			else
			{
				// Check Current Date to apply Penality
				if (CurrentDate > MyTrustList[i].LimitTime)
				{
					MyTrustList[i].TrustLevel -= (float) PENALITY;

					// Update Limit Time
					MyTrustList[i].LimitTime = CurrentDate + NEXT_LIMIT_TIME_S;
				}
			}
		}

		// Save free index
		else
			FreePos = i;
	}

	// NodeID not found in TrustList
	// Aggregation is positive
	// TrustList is not full
	if ( (Found == 0) && (AggregationValue > 0) && (FreePos != -1) )
	{
		memcpy(MyTrustList[FreePos].NodeID, NodeID, NODEID_SIZE);
		MyTrustList[FreePos].TrustLevel = AggregationValue;
		MyTrustList[FreePos].LimitTime = CurrentDate + NEXT_LIMIT_TIME_S;
	}
}


// Get TrustLevel of NodeID
float GetNodeTrustLevel(uint8_t* NodeID)
{
	for(int i=0; i<TRUSTLIST_SIZE; i++)
	{
		if (strcmp(NodeID, MyTrustList[i].NodeID) == 0)
			return MyTrustList[i].TrustLevel;
	}
	return -1.0;
}


// Get LimitTime of NodeID
float GetNodeLimitTime(uint8_t* NodeID)
{
	for(int i=0; i<TRUSTLIST_SIZE; i++)
	{
		if (strcmp(NodeID, MyTrustList[i].NodeID) == 0)
			return MyTrustList[i].LimitTime;
	}
	return -1.0;
}


// Get Index of NodeID with best TrustLevel
int GetBestNode(void)
{
	uint8_t BestIndex = 0;

	for(int i=0; i<TRUSTLIST_SIZE; i++)
	{
		if (MyTrustList[i].TrustLevel > MyTrustList[BestIndex].TrustLevel)
			BestIndex = i;
	}
	return BestIndex;
}


// Display TrustList
void DisplayTrustList(void)
{
	printf("---- TRUSTLIST ----\n");
	for(int i=0;i<TRUSTLIST_SIZE; i++)
	{
		printf("Index %d:\n", i);
		printf("Node ID:\t");

		// Display Node ID
		for (int j = 0; j < NODEID_SIZE; j++)
			printf("%.2X", MyTrustList[i].NodeID[j]);

		// Display TrustLevel
		printf("\nTrustLevel:\t%f\n", MyTrustList[i].TrustLevel);

		// Display LimitTime
		printf("Limit Time:\t%ld\n", MyTrustList[i].LimitTime);
		printf("**********\n");
	}
}