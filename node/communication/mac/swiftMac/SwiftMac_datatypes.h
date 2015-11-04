#ifndef _SWIFTDATATYPES_H_
#define _SWIFTDATATYPES_H_


enum NODE_STATE 
{
	
	S_ACTIVE,
	S_SLEEPING,
	S_BOUND,

	S_IDLE,
	S_SEARCHING,

	S_RXING,
	S_TXING,
	 
	S_FREE,
	S_LEASED,
	S_BACKEDOFF,
	S_WAITING_TO_BINDACK,
	
	// POSSIBLE STATES - 


	// states related to phase 1
	S_MOBILE_TX_PAUSED,
	S_MOBILE_TX_ACTIVE,
	S_STATIC_RX_PAUSED_SEND_WAKE_UP_NEIGHBORS,
	S_STATIC_RX_ACTIVE,
	
};


enum NODE_TYPE
{
	STATIC = 0,
	MOBILE
};


enum SWIFTMAC_TIMER_EVENTS
{
	T_MOBILE_SEARCH_TIMEOUT = 0,
	T_RETRY_TRIGGER,
	T_STATIC_WAIT_TIMEOUT,
	T_WAIT_TO_ACK_TIMEOUT,
	T_LEASE_EXPIRY,
	T_STARTUP_TIMER,
	T_MOBILE_TX_PAUSE,			// for following the schedule
	T_MOBILE_TX_RESUME,
	T_STATIC_RX_PAUSE,
	T_STATIC_RX_RESUME,
	T_STATIC_3T_EXPIRY,
	T_STATIC_T_EXPIRY
};



enum SWIFTMAC_PACKET_TYPE
{
	M_BIND_REQUEST,
	M_BIND_ACK,
	M_DATA,
	M_LEASE_FOLLOW_UP,
	M_WAKE_UP
};



struct LEASE
{
	int nodeId;
	int data;
	double timeCap;
};

struct ack_params
{
	LEASE offeredLease;
	int schedule[10]; 		// schedule for transmission
	double priority;
};


struct backoff_pair
{
	int address;
	int backoff;
};

#endif