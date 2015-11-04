#include "Lantern.h"
#include "MacPacket_m.h"
#include <vector>
#include <random>
#include <chrono>
#include <ctime>
 using namespace std;

Define_Module(Lantern);


struct Lease
{
	int nodeId;
	int data;
	double timeCap;
	int stopafter;
	int stopfor;
};

struct Lease lease;
struct ack_params
{
	Lease offeredLease;
	double priority;
};

	NodeType myType;	
	MacState myState;

struct ack_params ack;
struct nodeLocRec {
 	double locX;
 	double locY;
};


vector <nodeLocRec> nodesLocVec;
static int k = 0;
int j = 0;
static int l = 0;
int p = 0;
int p1 = 0;
int n;
int validLease = 1;
int total = 100;
vector <int> neighbours;
int bneighbours[10];
int firstwakeup[10];

void Lantern::fromRadioLayer(cPacket * pkt, double rssi, double lqi)
{
	LanternMacPacket *incoming = dynamic_cast <LanternMacPacket*>(pkt);
	

	int destination = incoming->getDestination();
	int source = incoming->getSource();
	
	//	trace()<<"This is node " <<SELF_MAC_ADDRESS<<" !!";
	

	if( 0 != SELF_MAC_ADDRESS){
		myType = MOBILE;
		myState = S_FREE;
		if(incoming->getType() == 0)	{
			trace()<< "Received request packet at "<<SELF_MAC_ADDRESS;
			trace()<<"Destination of incoming was "<<incoming->getDestination();
			setTimer(SENDRESP, (rand() / double(RAND_MAX)));
			//commented code, put in SENDRESP
			
		}
	}
	else{
		
		trace()<<"Received response packet at"<<simTime()<<"!";
		cModule *theSNModule;
			theSNModule = getParentModule()->getParentModule()->getParentModule();
			cModule *tmpNode;
			tmpNode = theSNModule->getSubmodule("node",SELF_MAC_ADDRESS);
			double x = tmpNode->par("xCoor"); 
			double y = tmpNode->par("yCoor");
			int s = incoming->getSource();
			trace()<<"Posn of mobile is "<<x<<" "<<y<<"For s= "<<s;
		if(count==0){
			nodetobeBound = incoming->getSource();
			d = (x-nodesLocVec.at(s).locX)*(x-nodesLocVec.at(s).locX) + (y-nodesLocVec.at(s).locY)*(y-nodesLocVec.at(s).locY);
			count++;
			trace()<<"For the "<<count<<"st packet"<< "Node to be  bound at first is "<<nodetobeBound;;
		}
		else {
			trace()<<"Co-or are x = "<<nodesLocVec.at(s-1).locX<<"and y = "<<nodesLocVec.at(s-1).locY;
			double dist = ( x-nodesLocVec.at(s-1).locX)*(x-nodesLocVec.at(s-1).locX) + (y-nodesLocVec.at(s-1).locY)*(y-nodesLocVec.at(s-1).locY);
			trace()<<"Dist at "<<count<<"packet is "<<dist;
			if(dist > d)	{
			
				d = dist;
				nodetobeBound = s;
			}
			count++;
			trace()<<"For the "<<count<<"th packet";
			trace()<<"Node  to be  bound is "<<nodetobeBound;
		}
		if(count == 5){
			//set the mac state to bound, instead of searching. count==9 because all nodes are covered.
		
		lease.data = 40;
		lease.nodeId=nodetobeBound;
		lease.timeCap=5; 
		lease.stopafter=3;
		lease.stopfor =2;
		trace()<<"Establishing lease with "<<lease.nodeId<<"!";
		//Validate on 3T cycle basis
		myState = S_TXING;
		setTimer(SONE,0);

		/*
		int isfirst;
		isfirst=1;
		sendData(lease.nodeId, &lease.data,isfirst, validLease);
		total = total -10;
		isfirst=0;
		int noactivity;
		while(total){
		lease.data =10;
		sendData(lease.nodeId, &lease.data, 0, validLease);
		total = total -10;
		if ((total==60)||(total==30)||(total==10)){
			trace()<<"Going to check for activity at total "<<total;
			noactivity = checkactivity(lease.nodeId);
			if(noactivity){
				setneighbourstosleep(lease.nodeId);
			}
		}

		} */
	}
}
//2 is the data-msg

	if(incoming->getType() == 2)	{
			if(incoming->getDestination()==SELF_MAC_ADDRESS){
			myState = S_RXING;
			trace()<< "Received data packet at "<<SELF_MAC_ADDRESS;
			trace()<<"Destination of incoming was "<<incoming->getDestination();
			}	
		}




	if(incoming->getType() == 4)	{
			trace()<< "received sleep packet at "<<SELF_MAC_ADDRESS;
			toRadioLayer(createRadioCommand(SET_STATE, SLEEP));
			trace()<<"Sleeping due to no activity";
	}


if(incoming->getType() == 3)	{

	for(int i=0;bneighbours[i]!=0;i++){
			if(bneighbours[i]==SELF_MAC_ADDRESS){
			firstwakeup[i]++;
			trace()<< "Received wakeup beacon packet at "<<SELF_MAC_ADDRESS;
			toRadioLayer(createRadioCommand(SET_STATE, RX));
			if(firstwakeup[i]==1){
			trace()<<"Setting 3T timer";
			setTimer(S_SLEEPING,30);
		}
		else {

			trace()<<"Cancelling previous timer and setting 3T timer at "<<getClock();
			cancelTimer(S_SLEEPING);
			setTimer(S_SLEEPING,30);
		}
		}
		}
			//toRadioLayer(createRadioCommand(SET_STATE, RX));
			
	}


}



//1st T cycle.
void Lantern::sendData(int i, int *j, int validlease){
		trace()<<"In sendData";
		if(validlease){
		trace()<<"Lease valid";
		if(*j > 0){
			trace()<<"Sending data: ";
			*j -= 1;
			trace()<<"no. permitted is "<< *j;
			/*
			if(!hopflag) {
				trace()<<"1st T cycle";

		}

			if(hopflag){
				trace()<<"Not first";

			}


*/

			myState=S_TXING;
			LanternMacPacket *b = new LanternMacPacket("Data", MAC_LAYER_PACKET);
			LanternMacPacket *in = new LanternMacPacket("Data", MAC_LAYER_PACKET);
			b->setType(MDATA);
			b->setSource(0);
			b->setDestination(i);
			b->setByteLength(1);
			encapsulatePacket(in, b);
			in->setSource(0);
			in->setDestination(i);
			in->setType(MDATA);
			toRadioLayer(in);
			toRadioLayer(createRadioCommand(SET_STATE, TX));
			
			
			trace()<<"Data: Sent";

			if(*j==0)
				validLease=0;

		}

		else {
			trace()<<"New lease needs to be established";
		}

	}
	else { 

		trace()<<"Invalid lease";
		
	}
	}

void Lantern::tinterval(int i){

trace()<<"In tinterval";
//to wakeup neigh of nodetobebound

int node = getParentModule()->getParentModule()->getParentModule()->par("numNodes");

	cModule *theSNModule;
	theSNModule = getParentModule()->getParentModule()->getParentModule();
	cModule *tmpNode;
	nodeLocRec tmpRec;
	cModule *myNode;
	trace()<<"Total nodes "<<node;
	int x , y;
	myNode= theSNModule->getSubmodule("node",i);
			x = myNode->par("xCoor");
			y = myNode->par("yCoor");
int dist;
int ar[node];
for (int g=0;g<node;g++){
	ar[g]=0;
}
int k =0;
	for(int m=1; m < node; m++){
			tmpNode = theSNModule->getSubmodule("node",m);
			tmpRec.locX = tmpNode->par("xCoor");
			tmpRec.locY = tmpNode->par("yCoor");
			//trace() << "X coor and Y coor of "<<m<<" are: " << tmpRec.locX << "  " << tmpRec.locY;
			nodesLocVec.push_back(tmpRec);
			dist = (x-tmpRec.locX)*(x-tmpRec.locX) + (y-tmpRec.locY)*(y-tmpRec.locY);
			nodesLocVec.push_back(tmpRec);
		//	trace()<<"Its dist  from" <<i<<" is "<<dist;

			if(dist<2){
				if(m!=nodetobeBound){
				ar[k]= m;
			//	trace()<<"Neighbour found "<<m;
				k++;
			}
			}
	}
	
	trace()<<"number of neighbours are: "<<k<<"and are ";
	for ( int g =0;ar[g]!=0;g++){
		trace()<<ar[g];
		
	}

	
	

		trace()<<"Waking up neighbours of "<<i;




		LanternMacPacket *wPacket[k] ;
		for(int m=0; ar[m]!=0; m++){

		wPacket[m] = new LanternMacPacket("Wakeup packet", MAC_LAYER_PACKET);
		wPacket[m]->setType(MWAKEUP);
		
		wPacket[m]->setSource(i);

		wPacket[m]->setDestination(ar[m]);
		
		wPacket[m]->setByteLength(1);
		trace()<<"Sending wakeup packet to neighbour " << ar[m];
		
		toRadioLayer(wPacket[m]);
		
		toRadioLayer(createRadioCommand(SET_STATE, TX));
	
		wPacket[m] = NULL;

	}
	






}

int Lantern::checkactivity(int nodeid){

trace()<<"In check activity";


return 1;

}

void Lantern::sendwakeuptoneighbours(int i){

trace()<<"Sending wakeup beacons to neighbours of "<< i<<"to  check activity";
int node = getParentModule()->getParentModule()->getParentModule()->par("numNodes");

	cModule *theSNModule;
	theSNModule = getParentModule()->getParentModule()->getParentModule();
	cModule *tmpNode;
	nodeLocRec tmpRec;
	cModule *myNode;
	//trace()<<"Total nodes "<<node;
	int x , y;
	myNode= theSNModule->getSubmodule("node",i);
			x = myNode->par("xCoor");
			y = myNode->par("yCoor");
int dist;
int ar[node];
for (int g=0;g<node;g++){
	ar[g]=0;
}
int k =0;
	for(int m=1; m < node; m++){
			tmpNode = theSNModule->getSubmodule("node",m);
			tmpRec.locX = tmpNode->par("xCoor");
			tmpRec.locY = tmpNode->par("yCoor");
		//	trace() << "X coor and Y coor of "<<i<<" are: " << tmpRec.locX << "  " << tmpRec.locY;
		//	nodesLocVec.push_back(tmpRec);
			dist = (x-tmpRec.locX)*(x-tmpRec.locX) + (y-tmpRec.locY)*(y-tmpRec.locY);
			nodesLocVec.push_back(tmpRec);
		//	trace()<<"Its dist is"<<dist;
			if(dist<2){
				if(m!=nodetobeBound){
				ar[k]= m;
				bneighbours[k]=m;
				k++;
			}
			}
	}
	
	trace()<<"In sendwakeuptoneighbours, number of neighbours are: "<<k<<" and are ";
	for ( int g =0;bneighbours[g]!=0;g++){
		trace()<<bneighbours[g];
		
	}

	
	

		
		LanternMacPacket *sPacket;
		LanternMacPacket *ins ;

		/*
	for ( int g =0;ar[g]!=0;g++){
		 sPacket[g] = new LanternMacPacket("Set neigh to sleep", MAC_LAYER_PACKET);
		 ins[g] = new LanternMacPacket("Set neigh to sleep", MAC_LAYER_PACKET);
		sPacket[g]->setType(MSLEEP);
		trace()<<"type set to MSLEEP";
		sPacket[g]->setSource(SELF_MAC_ADDRESS);
		sPacket[g]->setDestination(ar[g]);
		trace()<<"setting to sleep "<<ar[g];
		sPacket[g]->setByteLength(1);
		//bPacket[i]->setFrameType(BEACON_FRAME);
		encapsulatePacket(ins[g], sPacket[g]);
		ins[g]->setSource(SELF_MAC_ADDRESS);

		ins[g]->setDestination(ar[g]);
		ins[g]->setType(MSLEEP);
		//trace()<<"Sending packet to "<<in[i]->getDestination();
		toRadioLayer(ins[g]);
		toRadioLayer(createRadioCommand(SET_STATE, TX));
		trace()<<"sleep Packet  sent to "<<ar[g];
	}
 */
	 sPacket = new LanternMacPacket("Set neigh to sleep", MAC_LAYER_PACKET);
		 ins = new LanternMacPacket("Set neigh to sleep", MAC_LAYER_PACKET);
		sPacket->setType(MWAKEUP);
		trace()<<"type set to MWAKEUP";
		sPacket->setSource(i);
		sPacket->setDestination(BROADCAST_MAC_ADDRESS);
		sPacket->setByteLength(1);
	
		encapsulatePacket(ins, sPacket);
		ins->setSource(SELF_MAC_ADDRESS);

		ins->setDestination(BROADCAST_MAC_ADDRESS);
		ins->setType(MWAKEUP);
		//trace()<<"Sending packet to "<<in[i]->getDestination();
		toRadioLayer(ins);
		toRadioLayer(createRadioCommand(SET_STATE, TX));
		trace()<<"BROADCAST WAKEUP DONE";







}
void Lantern::fromNetworkLayer(cPacket * pkt, int destination)
{
	if(pkt == NULL)	return;
	if(SELF_MAC_ADDRESS!= 0)	return;
	LanternMacPacket *macFrame = new LanternMacPacket("Lantern packet", MAC_LAYER_PACKET);
	encapsulatePacket(macFrame, pkt);
	macFrame->setSource(SELF_MAC_ADDRESS);
	macFrame->setDestination(BROADCAST_MAC_ADDRESS);
	toRadioLayer(macFrame);
	toRadioLayer(createRadioCommand(SET_STATE, TX));
	//trace()<<"Network layer";
}



void Lantern::toNetworkLayer(cMessage * macMsg)
{
	//trace() << "Delivering [" << macMsg->getName() << "] to Network layer";
	send(macMsg, "toNetworkModule");
}

void Lantern::toRadioLayer(cMessage * macMsg)
{
		//trace() << "Delivering [" << macMsg->getName() << "] to Radio layer";

	send(macMsg, "toRadioModule");
}

void Lantern::startup()



			//setTimer(S_SLEEPING,3);


{
		for(int i=0;i<10;i++){
bneighbours[i]=0;
firstwakeup[i]=0;

}

	//trace() <<getClock();
	int node = getParentModule()->getParentModule()->getParentModule()->par("numNodes");

	cModule *theSNModule;
	theSNModule = getParentModule()->getParentModule()->getParentModule();
	cModule *tmpNode;
	nodeLocRec tmpRec;
	

	for(int i=1; i < node; i++){
			tmpNode = theSNModule->getSubmodule("node",i);
			tmpRec.locX = tmpNode->par("xCoor");
			tmpRec.locY = tmpNode->par("yCoor");
		//	trace() << "X coor and Y coor of "<<i<<" are: " << tmpRec.locX << "  " << tmpRec.locY;
			nodesLocVec.push_back(tmpRec);
	}

			tmpNode = theSNModule->getSubmodule("node",SELF_MAC_ADDRESS);
			tmpRec.locX = tmpNode->par("xCoor");
			tmpRec.locY = tmpNode->par("yCoor");

	//trace() << "X coor and Y coor of "<<SELF_MAC_ADDRESS<<" are: " << tmpRec.locX << "  " << tmpRec.locY;

	if(SELF_MAC_ADDRESS == 0){
	//	trace()<<"Im the mobile node";
		myState = S_SEARCHING;
	//	trace()<<myState;



//broadcast request

		LanternMacPacket *bPacket;
		LanternMacPacket *in ;
		
		 bPacket = new LanternMacPacket("Binding Request", MAC_LAYER_PACKET);
		 in = new LanternMacPacket("Binding Request", MAC_LAYER_PACKET);
		bPacket->setType(MREQ);
		bPacket->setSource(SELF_MAC_ADDRESS);
		bPacket->setDestination(BROADCAST_MAC_ADDRESS);
		bPacket->setByteLength(1);
		encapsulatePacket(in, bPacket);
		in->setSource(SELF_MAC_ADDRESS);
		in->setDestination(BROADCAST_MAC_ADDRESS);
		trace()<<"Sending packet to "<<in->getDestination();
		in->setType(MREQ);
		toRadioLayer(in);
		toRadioLayer(createRadioCommand(SET_STATE, TX));
	}	
	else{
		


		
		tmpNode = theSNModule->getSubmodule("node",SELF_MAC_ADDRESS);
		double x = tmpNode->par("xCoor"); 
		double y = tmpNode->par("yCoor");
		//trace() << x << " " << y;
		double dist;
		for(;j<nodesLocVec.size(); j++){
			//trace() << x-nodesLocVec.at(j).locX << " " << y-nodesLocVec.at(j).locY;
			dist = (x-nodesLocVec.at(j).locX)*(x-nodesLocVec.at(j).locX) + (y-nodesLocVec.at(j).locY)*(y-nodesLocVec.at(j).locY);
			//trace() << dist;
			if(dist == 0)	continue;
			if(dist < 2)	neighbours.push_back(j+1 - 9*p);			
		}
		p++;

		for(; l<neighbours.size(); l++){
			//trace() << "Neighbours of "<< x << " and " << y << " " << neighbours.at(l);
		}


		


	}
}




void Lantern::timerFiredCallback(int timer)
{
	switch (timer) {


				case SENDRESP: {

			LanternMacPacket *b = new LanternMacPacket("Binding Response", MAC_LAYER_PACKET);
			LanternMacPacket *in = new LanternMacPacket("Binding Response", MAC_LAYER_PACKET);
			b->setType(MRESP);
			b->setSource(SELF_MAC_ADDRESS);
			b->setDestination(0);
			b->setByteLength(1);
			encapsulatePacket(in, b);
			in->setSource(SELF_MAC_ADDRESS);
			in->setDestination(0);
			in->setType(MRESP);
			toRadioLayer(in);
			toRadioLayer(createRadioCommand(SET_STATE, TX));
			trace() << "Sending response packet";
				break;
				}
				case S_SLEEPING:{

			trace() << "Initiating sleep procedure";

			 toRadioLayer(createRadioCommand(SET_STATE,SLEEP));

			 trace() << "Initiated sleep procedure";

			break;
		}

		case S_ACTIVE:{
			/* Random offset selected for creating a new schedule has expired. 
			 * If at this stage still no schedule was received, MAC creates 
			 * its own schedule and tries to broadcast it
			*/
			 trace() << "Initiating wakeup procedure";

			 toRadioLayer(createRadioCommand(SET_STATE, RX));

			trace() << "Initiated wakeup procedure";
			break;
		}

		case S_BOUND:{

			trace() << "Initiated bound procedure";

			break;
		}

		case S_WAKENEIGHBOURS: {

			trace() << "Initiated neigh procedure";

			break;
		}
		
		case S_IDLE: {
					trace() << "Initiated idle procedure";


			break;


		}

		case S_SEARCHING : {
							trace() << "Initiated searching procedure";


			break;
		}
	

		case SONE : {
			//Here, node 0 sends data packets via the senddata() method;
			for(int i=0;i<3;i++){
				sendData(nodetobeBound,&lease.data,1);
			}
			trace()<<"In state S1";
			setTimer(STWO,3);

			break;
		}


		case STWO : {
			//Here, nodetobe bound sends wakeupbeacon broadcast to its neighbours
			//In radiolayer of neighbours, cancel current timer and set another timer for 3T

			trace()<<"In state S2";
			trace()<<"Invoking wake beacon method";
			sendwakeuptoneighbours(nodetobeBound);




			
			if(hopcount==2){
				trace()<<"Done with 1st Tcycle";
				hopflag =1;
			}
			if(!hopflag){
				hopcount++;
			setTimer(SONE,2);
		}
			else {
				hopcount++;
				if(hopcount<7)
				setTimer(STHREE,2);
			}

			if(hopcount==7){
				trace()<<"Done with packets";
				setTimer(TERMINATE,0);
			}

			break;
		}

		case STHREE :{
			trace()<<"In state S3";
			for(int i=0;i<8;i++){
				sendData(nodetobeBound,&lease.data,1);
			}
			setTimer(STWO,8);

			break;
		}
		case TERMINATE :{
			trace()<<"TERMINATING THE PHASE";
			trace()<<"Mobile node "<<SELF_MAC_ADDRESS;
			myState = S_SEARCHING;//SEacrhing
			break;
		}



		default : {
		}
		}
	}
	



