/****************************************************************************
 *  Copyright: National ICT Australia,  2007 - 2010                         *
 *  Developed at the ATP lab, Networked Systems research theme              *
 *  Author(s): Yuriy Tselishchev                                            *
 *  This file is distributed under the terms in the attached LICENSE file.  *
 *  If you do not find this file, copies can be found by writing to:        *
 *                                                                          *
 *      NICTA, Locked Bag 9013, Alexandria, NSW 1435, Australia             *
 *      Attention:  License Inquiry.                                        *
 *                                                                          *  
 ****************************************************************************/

/****************************************************************************
 * Modified at: Universiti Sains Malaysia (USM)	2011		            *
 * Author(s): Mahdi Zareei (zarei.1982@gmail.com)                           *
 ***************************************************************************/

#ifndef _MOBILITYMODULE_H_
#define _MOBILITYMODULE_H_

#include "MobilityManagerMessage_m.h"
#include "VirtualMobilityManager.h"
#include <math.h>

using namespace std;

class ComplexLineMobility: public VirtualMobilityManager {
 private:
	/*--- The .ned file's parameters ---*/
	double updateInterval;
	double loc1_x;
	double loc1_y;
	double loc1_z;

	double loc2_x;
	double loc2_y;


	int min_x;
	int max_x;
	int min_y;
	int max_y;

	int min_speed;
	int max_speed;

	int destination_min_x;
	int destination_max_x;
	int destination_min_y;
	int destination_max_y;

	double speed;
	Velocity velocity;


	double movetime;
	double pausetime;

	double start_delay ;
	double timeout ;	

	bool set_timeout_for_mobility ;
	bool define_last_destination_for_movement ;
	bool change_direction_end_of_current_movetime ;
	bool change_direction_end_of_current_route ;

	bool define_varies_speed ;

	/*--- Custom class parameters ---*/
	double incr_x;
	double incr_y;

	double distance;
	int direction;

	double testmovetime;

	int stop_moving;

	double test_start_delay;

	double test_timeout;

	int changedirection;
	int changedirection_counter;
	int last_counter;

 protected:
	void initialize();
	void handleMessage(cMessage * msg);

 public:
 	Velocity getVelocity();
};

#endif
