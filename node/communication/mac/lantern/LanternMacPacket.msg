cplusplus {{
#include "MacPacket_m.h"
#include "VirtualMobilityManager.h"
#include "Lantern.h"
}}


class MacPacket;
//struct Velocity;
//struct NodeLocation_type;
struct ack_params;

enum LANTERN_PACKET_TYPE;

packet LanternMacPacket extends MacPacket 
{
	int type enum (LANTERN_PACKET_TYPE);
	simtime_t timeStamp;
	int payload_size = 0;
	//Velocity velocity;
	//NodeLocation_type location;
	//ack_params ack_param;
	int testField = 0;
};