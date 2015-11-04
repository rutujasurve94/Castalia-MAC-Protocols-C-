//****************************************************************************
//*  Copyright: National ICT Australia,  2007 - 2010                         *
//*  Developed at the ATP lab, Networked Systems research theme              *
//*  Author(s): Yuriy Tselishchev                                            *
//*  This file is distributed under the terms in the attached LICENSE file.  *
//*  If you do not find this file, copies can be found by writing to:        *
//*                                                                          *
//*      NICTA, Locked Bag 9013, Alexandria, NSW 1435, Australia             *
//*      Attention:  License Inquiry.                                        *
//*                                                                          *  
//****************************************************************************/

package node.communication.mac.swiftMac;

simple SwiftMac like node.communication.mac.iMac {
 parameters:
	bool collectTraceInfo = default (true);
	int macMaxPacketSize = default (0);		//BypassMac does not limit packet size
	int macPacketOverhead = default (8);	//only 2 fields are added in simple MAC frame, 8 bytes in total
	int macBufferSize = default (0);		//BypassMac does not use buffers

	int dutyCycleLength = default(1000);	// 'T'
	int epsilon = default(10);				// small pause for waking up the neighbors			
	int del = default(990);
	// Parameters specific to SwiftMac

	int nodeType = default(0);
	int range = default(40);
	int rxRate = default(2048);

	int max_retries = default(3);
	int max_backoff = default(3);



 gates:
	output toNetworkModule;
	output toRadioModule;
	input fromNetworkModule;
	input fromRadioModule;
	input fromCommModuleResourceMgr;
}

