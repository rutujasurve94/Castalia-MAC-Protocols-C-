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

#include "SwiftMac.h"

Define_Module(SwiftMac);

/* Handle packet received from upper (network) layer.
 * We need to create a MAC packet, (here it can just be the generic MacPacket)
 * and encapsulate the received network packet before forwarding it to RadioLayer
 */


void SwiftMac::startup()
{
	countTime = 0;
	switch((int)par("nodeType"))
	{
		case 0: myType = STATIC; break;
		case 1: myType = MOBILE; break;
	}
	range = par("range");
	rxRate = par("rxRate");
	time_frame_length = par("dutyCycleLength");  // read in the value of duty cycle. 
	epsilon = par("epsilon");

	myMobilityManager = check_and_cast\
						<VirtualMobilityManager *>\
						(getParentModule()->getParentModule()->\
							getSubmodule("MobilityManager"));

	//TRACE_ID("Initializing...");
	myLocation = myMobilityManager->getLocation();
	//TRACE_ID("@ [" << myLocation.x << "," << myLocation.y << "]");


	max_retries = par("max_retries");
	max_backoff = par("max_backoff");

	if(myType == MOBILE) max_retries = 10;
	frameToBeSent = NULL;
	currentPacket = NULL;

	stillToSendPacket = false;
	switch(myType)
	{
		case MOBILE:
		{
			prevVelocity.speed = INVALID_VAL;
			prevVelocity.theta = INVALID_VAL;

			myState = S_ACTIVE;
			break;
		}
		case STATIC:
		{
			
			myState = S_FREE;
			break;
		}
	}

	engagedNode = INVALID_VAL;
	prevVelocity.speed = 0;
	prevVelocity.theta = 0;

	currentVelocity.speed = 0;
	currentVelocity.theta = 0;
	INVALIDATE_LEASE(currentLease);
	ERRORLOG("71");


	declareOutput("PacketCounts");
	declareOutput("Latency");
	declareHistogram("Lat", 0, 5, 10);
	declareOutput("Time");
	collectOutput("PacketCounts", SELF_MAC_ADDRESS, "ID", SELF_MAC_ADDRESS+1);

}




void SwiftMac::fromNetworkLayer(cPacket * pkt, int destination)
{
	TRACE_ID("\n");
	collectOutput("PacketCounts", SELF_MAC_ADDRESS, "From N/W", 1);

	if(myType != MOBILE || myState != S_ACTIVE || myState == S_MOBILE_TX_PAUSED )
	{ 
		collectOutput("PacketCounts", SELF_MAC_ADDRESS, "DISCARDED");
		TRACE_ID("Dropping, stateI " << myState);
		return;
	}
	
	currentVelocity = myMobilityManager->getVelocity();
	/*TRACE_ID("My Location: x: " << myMobilityManager->getLocation().x << \
		" y: " << myMobilityManager->getLocation().y);*/
	inTime = simTime();
	
	//TRACE_ID("> "<< SIMTIME_DBL(simTime()));

	if(ABS(DIFF(currentVelocity.speed, prevVelocity.speed)) > \
		TOLERANCE_VARIATION_SPEED \
		&&\
		ABS(DIFF(currentVelocity.theta, prevVelocity.theta)) > \
		TOLERANCE_VARIATION_THETA)
	{
		TRACE_ID("invalidating current lease");
		INVALIDATE_LEASE(currentLease);

		prevVelocity =  currentVelocity;
		INVALIDATE_TIMER(T_LEASE_EXPIRY);
	}

	

	// Check if current lease is valid; if not, initiate new lease acquiry

	if(VALID(currentLease.nodeId) && currentLease.data > PKTSIZE(pkt))
	{
		
		TRACE_ID("Valid Lease with " << currentLease.nodeId << \
			" for another " << currentLease.data);

		myState = S_TXING;

		// Step 1. Frame the packet;
		framePacket(pkt, M_LEASE_FOLLOW_UP, currentLease.nodeId);
		
		// Step 2. Send th frameToBeSent;
		sendFrame();
		// Step 3. UpdateLease
		currentLease.data -= PKTSIZE(frameToBeSent);
		
		collectOutput("PacketCounts", SELF_MAC_ADDRESS, "CLEANTX", 1);

	}
	else
	{
		TRACE_ID("Shall Acquire a new lease");
		// Lease not valid / expired
		currentPacket =  pkt->dup();
		
		//delete pkt;

		// issue a lease request
		framePacket(pkt, M_BIND_REQUEST, BROADCAST_MAC_ADDRESS);
		//framePacket(pkt, M_BIND_REQUEST, 1);

		// Initiate timer to re send the bind request.


		setTimer(T_MOBILE_SEARCH_TIMEOUT, TIME_MOBILE_SEARCH_THRESHOLD);

		// send the bind request to all
		sendFrame();
		
		myState = S_SEARCHING;
	}




}


void SwiftMac::fromRadioLayer(cPacket * pkt, double rssi, double lqi)
{
	// this is where we take care of packets arriving from radio layer.

	SwiftPacket *incoming = dynamic_cast <SwiftPacket *>(pkt);

	if(incoming == NULL) return;


	int destination = incoming->getDestination();
	int source = incoming->getSource();
	//TRACE_ID("A Packet Arrives at the radio layer from " << source << " for " << destination);
	switch(myType)
	{
		case STATIC:
		{
			frameHandlerStaticNode(incoming, source, destination);
			break;
		}

		case MOBILE:
		{
			frameHandlerMobileNode(incoming, source, destination);
			break;
		}
	}

}




void SwiftMac::frameHandlerMobileNode(SwiftPacket * incoming, int source,\
	int destination)
{

	// The only case where mobile node is expected to receive any data from 
	// radio layer is when it is waiting for binding.. 

	// Step 1. Check if we are the inteded recepeint

	if(destination != SELF_MAC_ADDRESS) return;

	// Step 2. Check if we are expecting a BIND_ACK, i.e., we are in searching
	// state

	TRACE_ID("myState: " << myState);
	if(myState != S_SEARCHING) return;

	switch(incoming->getType())
	{
		case M_BIND_ACK:
		{

			TRACE_ID("Bind Ack received");
			currentLease = incoming->getAck_param().offeredLease;
			currentLease.nodeId = incoming->getSource();
			// resume sending the currentOutBound Packet
			myState = S_TXING;
			TRACE_ID("currentLease with: " << currentLease.nodeId \
				<< " for " << currentLease.data);
			framePacket(currentPacket, M_LEASE_FOLLOW_UP, currentLease.nodeId);
			sendFrame();
			
			currentLease.data -= PKTSIZE(frameToBeSent);
			cancelTimer(T_MOBILE_SEARCH_TIMEOUT);

			setTimer(T_LEASE_EXPIRY, currentLease.timeCap);
			collectOutput("PacketCounts", SELF_MAC_ADDRESS, "BACKRX", 1);

			break;
		}

		default: // unexpected...
					break;

	}
}


void SwiftMac::frameHandlerStaticNode(SwiftPacket * incoming, int source, \
	int destination)
{
	// So, we are a static node, in all, we are expecting only three kinds of packets

	// 1. M_LEASE_FOLLOW_UP, 2. M_BIND_REQUEST, 3. Noise...

	if(destination != SELF_MAC_ADDRESS && destination != BROADCAST_MAC_ADDRESS)
	{	
		if(incoming->getType() == M_BIND_ACK)
		{
			INVALIDATE_TIMER(T_WAIT_TO_ACK_TIMEOUT);
			frameToBeSent = NULL;
		//	TRACE_ID("wont ack..");
		}
		//TRACE_ID("Dropping..");
		return;	
	} 

	switch(incoming->getType())
	{
		case M_LEASE_FOLLOW_UP:
		{
			if (myState == S_STATIC_RX_PAUSED_SEND_WAKE_UP_NEIGHBORS){
				trace()<<"Reception is paused, ignoring packet"<<endl;
				break;
			}

			TRACE_ID("Lease follow up received");
			toNetworkLayer(decapsulatePacket(incoming));
			myState = S_IDLE;


			TRACE_ID(" timeStamp" << incoming->getTimeStamp());

			simtime_t d = simTime() - incoming->getTimeStamp();

			collectOutput("Latency", SELF_MAC_ADDRESS+1, "DATARX", SIMTIME_DBL(d));
			TRACE_ID("@@ " << SIMTIME_DBL(d));
			collectOutput("Time", countTime++, "", SIMTIME_DBL(d));
			collectHistogram("Lat", SIMTIME_DBL(d));

			TRACE_ID("< " << SIMTIME_DBL(simTime()));

			collectOutput("PacketCounts", SELF_MAC_ADDRESS, "DATARX", 1);
			break;
		}

		case M_BIND_REQUEST:
		{
			TRACE_ID("bind request received");
			TRACE_ID("Source: " << source << " Destination: " << destination);
			// Calculate the LEASE values, a1nd send back an akcnoledgement
			ack_params ack = computeLease(incoming);
			if(ack.offeredLease.data <= 100)
			{
				TRACE_ID("Lease not good, not sending..");
				return;
			}
			//assert(ack.offeredLease.data <= 0);
			frameToBeSent = NULL;
			framePacket(NULL, M_BIND_ACK, source, ack);
			TRACE_ID("offering lease of " << ack.offeredLease.data);

			myState = S_WAITING_TO_BINDACK;

			TRACE_ID("frameToBeSent to: " << frameToBeSent->getDestination());
			setTimer(T_WAIT_TO_ACK_TIMEOUT, ack.priority);
			collectOutput("PacketCounts", SELF_MAC_ADDRESS, "BREQRX", 1);
			break;
		}


		case M_WAKE_UP :{
			if(!first_wake_up){
			INVALIDATE_TIMER(T_STATIC_3T_EXPIRY);
			setTimer(T_STATIC_3T_EXPIRY, time_frame_length);
			
		}

			else {

					setTimer(T_STATIC_3T_EXPIRY, time_frame_length);
					first_wake_up=0;

			}





		}
		default: // not expected to handle.. 
					break;
	}

}




/*

		Function to compute the LEASE struct's attributes

*/

ack_params SwiftMac::computeLease(SwiftPacket * incoming)
{
	// the real stuf begins here..

	// Step 1. Isolate the location and velocity
	LEASE retLease;
	ack_params  retAck;

	NodeLocation_type itsLocation = \
						incoming->getLocation();
	Velocity itsVelocity = incoming->getVelocity();

	// calculate the delta
	retLease.data = calculateDelta(itsLocation, itsVelocity);
	//assert(retLease.data >= 0);
	retLease.nodeId = incoming->getSource();
	retLease.timeCap = SIMTIME_DBL(simTime())+retLease.data/(double)rxRate; 
	// calculate the priority
	retAck.priority = calculatePriority(retLease.data);

	retAck.offeredLease = retLease;

	return retAck;

}



/*

		A function to send the frame pointed by toBeSentFrame

*/

void SwiftMac::sendFrame()
{
	TRACE_ID("Sending frame");
	
	if(frameToBeSent != NULL)
	{
		TRACE_ID("Destination: " << frameToBeSent->getDestination());
		if(radioModule->isChannelClear())
		{
			// channel clear, clear to send
			toRadioLayer(frameToBeSent);
			_TRANSMIT;
			myState = S_ACTIVE;
			switch(frameToBeSent->getType())
			{
				case M_BIND_REQUEST:
				{
					collectOutput("PacketCounts", SELF_MAC_ADDRESS, "BREQTX", 1);
					break;
				}

				case M_LEASE_FOLLOW_UP:
				{
					collectOutput("PacketCounts", SELF_MAC_ADDRESS, "DATATX", 1);
					break;
				}

				case M_BIND_ACK:
				{
					assert(frameToBeSent->getAck_param().offeredLease.data >= 0);
					collectOutput("PacketCounts", SELF_MAC_ADDRESS, "BACKTX", 1);
					break;
				}
			}
		}
		else
		{
			TRACE_ID("Channel Noise: " << radioModule->isChannelClear());
			double backoffTIme = random();
			backoffTIme *= TIME_MAX_BACKOFF_LENGTH;
			backoffTIme *= numBackoffs;
			//Channel not clear; wait for sometime..
			setTimer(T_RETRY_TRIGGER, backoffTIme);
		}	
	}


}









/*

		Funtion to frame the pkt, and update frameToBeSent

*/


void SwiftMac::framePacket(cPacket *pkt, SWIFTMAC_PACKET_TYPE type, int destination)
{
	ERRORLOG("framePacket");
	SwiftPacket *out = new SwiftPacket("new packet", MAC_LAYER_PACKET);
	encapsulatePacket(out, pkt);
	out->setSource(SELF_MAC_ADDRESS);
	out->setDestination(destination);

	out->setType(type);

	ERRORLOG("packetFramed");
	switch(type)
	{
		case M_DATA:
		{
			// dummy place holder

			break;
		}

		case M_BIND_REQUEST:
		{
			// step 1. Copy the current packet, and create a request frame
			
			currentPacket = pkt->dup();
			myLocation = myMobilityManager->getLocation();
			
			//encapsulatePacket(out, NULL);
			
			out->setVelocity(currentVelocity);
			out->setLocation(myLocation);

			break;
		}

		case M_BIND_ACK:
		{
			break;
		}

		case M_LEASE_FOLLOW_UP:
		{

			out->setDestination(destination);
			TRACE_ID("@ " <<SIMTIME_DBL(inTime));
			out->setTimeStamp(inTime);
			//encapsulatePacket(out, pkt);
			break;
		}
	}

	//frameToBeSent = out->dup();
	frameToBeSent = out;
	ERRORLOG("410");
	//delete out;
	//delete pkt;
}



/*


			Overloaded funtion to frame a BIND_ACK frame.


*/


void SwiftMac::framePacket(cPacket *pkt, SWIFTMAC_PACKET_TYPE type, \
	int destination, ack_params ack_param)
{
	TRACE_ID("B_ACK, destination: " << destination);
	if(type == M_BIND_ACK)
	{
		

		SwiftPacket *bindAck = new SwiftPacket("ack packet", MAC_LAYER_PACKET);
		encapsulatePacket(bindAck, NULL);
		
		bindAck->setDestination(destination);
	/*	
		TRACE_ID("B_ACK, destination: " << destination);
		TRACE_ID("bindAck to " << bindAck->getDestination());
	*/	
		bindAck->setSource(SELF_MAC_ADDRESS);
		bindAck->setType(type);
		bindAck->setAck_param(ack_param);

		
		//frameToBeSent = bindAck->dup();
		
		frameToBeSent = bindAck;
	/*	TRACE_ID("This bindAck: " << bindAck);
		TRACE_ID("bindAck to " << bindAck->getDestination());
		TRACE_ID("frameToBeSent to " << frameToBeSent->getDestination());
	*/	
	}
}



void SwiftMac::timerFiredCallback(int event)
{
	switch(event)
	{

		case T_RETRY_TRIGGER:
		{
			TRACE_ID("Retrying tx");
			// check if we can still retry sending...
			if(numReTries++ < max_retries)
			{
				sendFrame();
			}
			else
			{
			// No we cannot.. drop this packet...
				TRACE_ID("Dropping...");
				frameToBeSent = NULL;
				myState = S_ACTIVE;
				numReTries = 0;
				numBackoffs = 1;
				collectOutput("PacketCounts", SELF_MAC_ADDRESS, "Dropped");
			}


			break;
		}

		case T_MOBILE_SEARCH_TIMEOUT:
		{
			// This happens when no bind_ack_received..

			cancelTimer(T_MOBILE_SEARCH_TIMEOUT);
			cancelTimer(T_RETRY_TRIGGER);

			myState = S_ACTIVE;
			TRACE_ID("TimeOut");

			break;
		}


		case T_WAIT_TO_ACK_TIMEOUT:
		{
			TRACE_ID("Sending out bindAck");
			sendFrame();
			break;
		}

		case T_LEASE_EXPIRY:
		{
			TRACE_ID("T_LEASE_EXPIRY");
			INVALIDATE_LEASE(currentLease);
			INVALIDATE_TIMER(T_LEASE_EXPIRY);
			break;
		}

		// phase 2 time fired events...

		case T_MOBILE_TX_PAUSE:
		{
			// change the state to paused. 
			myState = S_MOBILE_TX_PAUSED;
			// set timer to expire at epsilon
			setTimer(T_MOBILE_TX_RESUME, epsilon);
			TRACE_ID("Dropping...");
			break;
		}

		case T_MOBILE_TX_RESUME:
		{
			// change the state to active..
			myState = S_MOBILE_TX_ACTIVE;
			// set the timer to expire at delta_i+1. event: T_MOBILE_TX_PAUSE
			if(mob_iterator<10){
			setTimer(T_MOBILE_TX_PAUSE, schedule[mob_iterator++]);
		}
		else{

			setTimer(T_MOBILE_TX_PAUSE, del);
		}
		//send data packets
		SwiftPacket *sPacket;
		SwiftPacket *ins ;
		for(int i =0;i<5;i++){
	 	sPacket = new SwiftPacket("Data", MAC_LAYER_PACKET);
		 ins = new SwiftPacket("DATA", MAC_LAYER_PACKET);
		 //should be M_LEASE_FOLLOW_UP or M_DATA
		sPacket->setType(M_LEASE_FOLLOW_UP);
		trace()<<"Sending Data packets";
		sPacket->setSource(SELF_MAC_ADDRESS); //or 0
		sPacket->setDestination(boundnode);
		sPacket->setByteLength(1);
	
		encapsulatePacket(ins, sPacket);
		ins->setSource(SELF_MAC_ADDRESS);

		ins->setDestination(boundnode);
		ins->setType(M_LEASE_FOLLOW_UP);
		//trace()<<"Sending packet to "<<in[i]->getDestination();
		toRadioLayer(ins);
		toRadioLayer(createRadioCommand(SET_STATE, TX));
		trace()<<"DATA PACKET SENT"<<endl;

		}




			break;
		}

		case T_STATIC_RX_PAUSE:
		{
			// change the state.. 
		myState = S_STATIC_RX_PAUSED_SEND_WAKE_UP_NEIGHBORS;
			// set the timer for epsilon
		setTimer(T_STATIC_RX_RESUME, epsilon);
			//send wakwup to neighbors
		SwiftPacket *sPacket;
		SwiftPacket *ins ;

	 	sPacket = new SwiftPacket("Set neigh to sleep", MAC_LAYER_PACKET);
		 ins = new SwiftPacket("Set neigh to sleep", MAC_LAYER_PACKET);
		sPacket->setType(M_WAKE_UP);
		trace()<<"type set to MWAKEUP";
		sPacket->setSource(SELF_MAC_ADDRESS);
		sPacket->setDestination(BROADCAST_MAC_ADDRESS);
		sPacket->setByteLength(1);
	
		encapsulatePacket(ins, sPacket);
		ins->setSource(SELF_MAC_ADDRESS);

		ins->setDestination(BROADCAST_MAC_ADDRESS);
		ins->setType(M_WAKE_UP);
		//trace()<<"Sending packet to "<<in[i]->getDestination();
		toRadioLayer(ins);
		toRadioLayer(createRadioCommand(SET_STATE, TX));
		trace()<<"BROADCAST WAKEUP DONE";

			break;
		}

		case T_STATIC_RX_RESUME:
		{

			myState = S_STATIC_RX_ACTIVE;
			if(static_iterator<10){
			setTimer(T_STATIC_RX_PAUSE, schedule[static_iterator++]);
			}
			else {
				setTimer(T_STATIC_RX_PAUSE, del);
			}
			// change the state

			// for first T cycle over when static_iterator = 10..
			// set timer for delta_i+1, event: T_STATIC_TX_PAUSE


			break;
		}

		case T_STATIC_3T_EXPIRY :{


			toRadioLayer(createRadioCommand(SET_STATE, SLEEP));
		}



		default:
		{
			break;
		}
	}
}




double SwiftMac::random()
{

	std::default_random_engine generator;
	std::uniform_real_distribution<double> dist(0.001, 0.05);
	return dist(generator);
	double randomNumber;
	if(randomGenFlipper){
		randomNumber =  rand()%RANDOMNESS_GRANULARITY;
		randomNumber /= RANDOMNESS_GRANULARITY;
	}
	else
	{
		rand();
		randomNumber = rand()%RANDOMNESS_GRANULARITY;
		randomNumber /= RANDOMNESS_GRANULARITY;
	}

	randomGenFlipper = !randomGenFlipper;
	return randomNumber;
}


int SwiftMac::calculateDelta(NodeLocation_type m, Velocity v)
{

	    int ret = 0;

	    if(v.speed == 0) return DELTA_MAX;

	    NodeLocation_type s = myMobilityManager->getLocation();
	    NodeLocation_type V = {v.speed*cos(v.theta), v.speed*sin(v.theta)};
	    

	    double d = sqrt(pow(s.x - m.x, 2) + pow(s.y - m.y,2));

	    //TRACE_ID("D: " << d);
	    if(d > range) 
	    {
	    		TRACE_ID("This happened: range: " << range << " d :" <<d);
	    		return 0;
		}
	    //printf("\t-> v in vector notation: %f, %f", V.x, V.y); 

	    double v_dot_d = V.x*(s.x - m.x) + V.y*(s.y - m.y);

	    //printf("\t--> v.d : %f",v_dot_d);


	    double d_cos_alpha = v_dot_d/v.speed;
	    double d_sin_alpha = d*sqrt(1 - pow(d_cos_alpha/d, 2));

	    double _d = d_cos_alpha + sqrt(range*range - d_sin_alpha*d_sin_alpha);
	    
	   // printf("\n %f is the _d", _d);
	  //  TRACE_ID("_d is : " << _d);
	    if(_d <= 0) return 0;

	    _d /= v.speed;
	    _d *= rxRate;

	    _d = _d > DELTA_MAX ? DELTA_MAX:_d;

	   // TRACE_ID("delta: " << _d);

	    return (int)_d;
	//    return ret;
	





	/*int ret =  0;

	if(v.speed == 0)
	{	TRACE_ID("ghatak..");
		return DELTA_MAX;
	}

	TRACE_ID("Velocity: speed " << v.speed << " theta: " << v.theta);



	double v_sin_theta = v.speed * sin(v.theta);
	double v_cos_theta = v.speed * cos(v.theta);
	
	TRACE_ID("v_cos_theta " << v_cos_theta);
	TRACE_ID("v_sin_theta " << v_sin_theta);
	

	NodeLocation_type vector_v;
	vector_v.x = v_cos_theta;
	vector_v.y = v_sin_theta;

	TRACE_ID("v: " << vector_v.x << "," << vector_v.y);
	
	TRACE_ID("mobLoc: " << l.x << ", " << l.y);
	TRACE_ID("myLocation " << myLocation.x << "," << myLocation.y);

	l.x -= myLocation.x;
	l.y -= myLocation.y;


	if( ABS(l.x) < TOLERACNE_LENGTH && ABS(l.y) < TOLERACNE_LENGTH)
	{
		TRACE_ID("katai bhaunt tareeke se ghatak");
		return DELTA_MAX;	
	} 


	 TRACE_ID("l " << l.x << "," << l.y);

	double dot_product = DOT_PRODUCT(vector_v, l);

	 TRACE_ID("v.l " << dot_product);

	double cos_alpha = dot_product/(v.speed*MAGNITUDE(l));

	double d_cos_alpha = cos_alpha*MAGNITUDE(l);
	double d_sin_alpha = sqrt(pow(MAGNITUDE(l),2) - pow(d_cos_alpha,2));

	 TRACE_ID("d_cos_alpha " << d_cos_alpha << ", d_sin_alpha " << d_sin_alpha);


	double span = d_cos_alpha + sqrt(pow(range,2) + pow(d_sin_alpha,2));

	double delta = span/v.speed;

	ret = (delta*rxRate > DELTA_MAX) ? DELTA_MAX : delta*rxRate;


	/*
		span is the lenght o
	*/
	//assert(ret <= 0);

/*		TRACE_ID("data:	" << ret);
	return ret;*/
}



double SwiftMac::calculatePriority(int delta)
{
	double ret;

	/*

			We can add more complex priority functions later on, right now, the priority is directly propotional to the Delta it can accept.


			Also, priority decides how long a node shall hold back before replying..


	*/


	ret = 1 - delta/(1.0*DELTA_MAX);
	ret *= ret;
	ret /= 100;

	TRACE_ID("Replying after: " << ret);



	return ret;
}