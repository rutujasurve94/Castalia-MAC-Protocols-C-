#ifndef _LANTERN_H_
#define _LANTERN_H_

#include <omnetpp.h>
#include "VirtualMac.h"
#include "LanternMacPacket_m.h"

using namespace std;

enum NodeType {
	STATIC = 0,
	MOBILE 
};
enum MacState{
	
	S_ACTIVE,
	S_SLEEPING,
	S_BOUND,
	S_WAKENEIGHBOURS,
	S_IDLE,
	S_SEARCHING,

	S_RXING,
	S_TXING,
	 
	S_FREE,
	S_LEASED,
	S_BACKEDOFF,
	S_WAITING_TO_BINDACK,
	SENDRESP,
	SONE,
	STWO,
	STHREE,
	TERMINATE
	

};

enum LANTERN_TIMER_EVENTS
{
	T_MOBILE_SEARCH_TIMEOUT = 0,
	T_RETRY_TRIGGER,
	T_STATIC_WAIT_TIMEOUT,
	T_WAIT_TO_ACK_TIMEOUT,
	T_LEASE_EXPIRY,
	T_STARTUP_TIMER
};


enum LANTERN_PACKET_TYPE
{
	MREQ,
	MRESP,
	MDATA,
	MWAKEUP,
	MSLEEP
};





struct backoff_pair
{
	int address;
	int backoff;
};





class Lantern: public VirtualMac
{	
	int count = 0;
	int nodetobeBound;
	double d;
	int hopflag =0;
	int hopcount = 0;
	protected:
		void startup();
		void fromRadioLayer(cPacket *, double, double);
		void fromNetworkLayer(cPacket *, int);
		void timerFiredCallback(int);
		void sendMreqPacket();
		//void cycle(vector<int>& neighbours, int);
		//void checkactivity(vector<int>& neighbours, int);
		//void wakeupneighbours(vector<int>& neighbours);
		void toNetworkLayer(cMessage * macMsg);
		void toRadioLayer(cMessage * macMsg);
		void sendData(int j, int *i, int validLease);
		void tinterval(int i);
		int checkactivity(int nodeId);
			
		void sendwakeuptoneighbours(int nodeId);
			
		//void sendMrespPacket();
	    //void setMacState(int newState);

};

#endif

