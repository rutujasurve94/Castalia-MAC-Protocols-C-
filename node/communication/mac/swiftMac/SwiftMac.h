/****************************************************************************
 *  Copyright: National ICT Australia,  2007 - 2011                         *
 *  Developed at the ATP lab, Networked Systems research theme              *
 *  Author(s): Yuriy Tselishchev, Athanassios Boulis                        *
 *  This file is distributed under the terms in the attached LICENSE file.  *
 *  If you do not find this file, copies can be found by writing to:        *
 *                                                                          *
 *      NICTA, Locked Bag 9013, Alexandria, NSW 1435, Australia             *
 *      Attention:  License Inquiry.                                        *
 *                                                                          *
 ****************************************************************************/

#ifndef _SWIFTMAC_H_
#define _SWIFTSMAC_H_

#include <omnetpp.h>
#include "VirtualMac.h"
#include "VirtualMobilityManager.h"
#include "RoutingPacket_m.h"
#include "SwiftMac_datatypes.h"
#include "SwiftMac_m.h"


#include <random>
#include <cstdlib>
#include <cstdio>
#include <math.h>
 #include <cassert>
#include <vector>

#define TIME_MAX_BACKOFF_LENGTH	1
#define INVALID_VAL -2

#define TOLERANCE_VARIATION_SPEED	0.0001
#define TOLERANCE_VARIATION_THETA	0.0001

#define TOLERACNE_LENGTH 0.0001

#define DIFF(x,y) ((x) - (y))

#define ABS(x) (((x)<0)? -1*(x) : (x))


#define RANDOMNESS_GRANULARITY 1000
#define DELTA_MAX 20000000


#define TIME_MOBILE_SEARCH_THRESHOLD 0.150

#define TRACE(x) (trace() << x);
#define TRACE_ID(x) if(myType == STATIC){\
						TRACE("[S:" << SELF_MAC_ADDRESS\
						 <<"]	" << x );}\
						else{\
							TRACE("[M:" << SELF_MAC_ADDRESS\
								<<"]	" << x);}



#define INVALIDATE_LEASE(y) {y.nodeId = INVALID_VAL;\
							 y.data = INVALID_VAL;}





#define INVALIDATE_TIMER(x) { if(getTimer(x) != 0)cancelTimer(x);}
#define EXTEND_TIMER(ev,t) {INVALIDATE_TIMER((ev)); setTimer((ev), (t));}							 
#define VALID(x)	((x) == INVALID_VAL ? false : true)

#define PKTSIZE(x) (x->getByteLength())



#define _TRANSMIT {toRadioLayer(createRadioCommand(\
 						SET_STATE, TX));}



// Macro to calculate distance between two points

#define DISTANCE(a,b) (sqrt(pow((a.x - b.x),2) + pow((a.y - b.y), 2)))

#define MAGNITUDE(c) (sqrt(pow(c.x,2) + pow(c.y,2)))
#define DOT_PRODUCT(a, b) (a.x * b.x + a.y*b.y)

FILE *errorlogFile;
#define ERRORLOG(x)	{errorlogFile = fopen("errorlog.txt", "a");\
						fprintf(errorlogFile, "[%d] %s\n", SELF_MAC_ADDRESS, x);;\
					fclose(errorlogFile);}



using namespace std;

class SwiftMac: public VirtualMac
{
	/* In order to create a MAC based on VirtualMacModule, we need to define only two
	 * functions: one to handle a packet received from the layer above (routing),
	 * and one to handle a packet from the layer below (radio)
	 */

	NODE_TYPE myType;		// myType
	NODE_STATE myState;		// myState;

	int range;
	int max_retries;
	int max_backoff;

	int numReTries;
	int numBackoffs;

	int engagedNode;

	int rxRate;

	int mob_iterator = 0 ;
	int static_iterator = 0;
	VirtualMobilityManager * myMobilityManager;
	SwiftPacket * frameToBeSent;
	cPacket * currentPacket;

	bool stillToSendPacket;

	double leaseDuration;
	double random();
 	bool randomGenFlipper;

 	simtime_t inTime;


 	int countTime;

 	Velocity prevVelocity;
 	Velocity currentVelocity;
 	NodeLocation_type myLocation;
 	LEASE currentLease;



 	//del + epsilon = dutycyclelength
 	int boundnode =4; //change later
 	int del;
 	int epsilon;			// pause length.. 
 	int time_frame_length;  // length of duty cycle. 
 	int schedule[10]; 		// txing schedule
 	int first_wake_up =1 ;

 	void frameHandlerStaticNode(SwiftPacket *, int, int);
 	void frameHandlerMobileNode(SwiftPacket *, int, int);
 	ack_params computeLease(SwiftPacket *);

 	int calculateDelta(NodeLocation_type, Velocity);
 	double calculatePriority(int);

protected:
	void fromRadioLayer(cPacket *, double, double);
	void fromNetworkLayer(cPacket *, int);


	void startup();


	void timerFiredCallback(int);
	void framePacket(cPacket *, SWIFTMAC_PACKET_TYPE, int);
	void framePacket(cPacket *, SWIFTMAC_PACKET_TYPE, int, ack_params);
	void sendFrame();
};

#endif
