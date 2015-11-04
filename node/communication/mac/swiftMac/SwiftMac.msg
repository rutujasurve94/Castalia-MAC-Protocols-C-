cplusplus {{
	#include "MacPacket_m.h"
	#include "VirtualMobilityManager.h"
	#include "SwiftMac_datatypes.h"
}}


class MacPacket;
struct Velocity;
struct NodeLocation_type;
struct ack_params;

enum SWIFTMAC_PACKET_TYPE;

packet SwiftPacket extends MacPacket
{
	int type enum (SWIFTMAC_PACKET_TYPE);
	simtime_t timeStamp;

	int payload_size = 0;

	Velocity velocity;
	NodeLocation_type location;

	ack_params ack_param;
}