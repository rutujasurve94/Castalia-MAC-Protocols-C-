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

#include "ComplexLineMobility.h"

Define_Module(ComplexLineMobility);

void ComplexLineMobility::initialize()
{
	VirtualMobilityManager::initialize();

	updateInterval = par("updateInterval");
	updateInterval = updateInterval / 1000;

	loc1_x = nodeLocation.x;
	loc1_y = nodeLocation.y;
	loc1_z = 0 ;

	stop_moving = 0 ;
	changedirection_counter = 0 ;
	test_timeout = 0 ;
	testmovetime = 1 ;
	test_start_delay = 0 ;

	min_x = par("min_x");	
	max_x = par("max_x");
	min_y = par("min_y");
	max_y = par("max_y");

	start_delay = par("start_delay");
	speed = par("speed");

// this part is for define where the nodes need to go at the end of their timeout
	define_last_destination_for_movement = par ("define_last_destination_for_movement");
	destination_min_x = par("destination_min_x");	
	destination_max_x = par("destination_max_x");
	destination_min_y = par("destination_min_y");
	destination_max_y = par("destination_max_y");

	
// this part is for settding the timeout timer for the mobility
	set_timeout_for_mobility = par ("set_timeout_for_mobility") ;
	timeout = par("timeout");


// seed the random variable:
	srand(std::time(0));

// this part is for checking the speed condition, fixed or varies?
	define_varies_speed = par("define_varies_speed");
	
	if ((define_varies_speed)){
		min_speed = par("min_speed");
		max_speed = par("max_speed");	
		speed = (rand()%(max_speed-min_speed))+min_speed;
	}

//end

	movetime = par("movetime");
	pausetime = par("pausetime");

	change_direction_end_of_current_movetime = par("change_direction_end_of_current_movetime") ;
	change_direction_end_of_current_route = par("change_direction_end_of_current_route") ;

	trace() << "-->check  change_direction_end_of_current_route : " << change_direction_end_of_current_route ;
	trace() << "-->check  change_direction_end_of_current_movetime : " << change_direction_end_of_current_movetime  ;
	trace() << "-->check  define_varies_speed : " << define_varies_speed  ;

		if ((movetime + pausetime) >= testmovetime) {
			loc2_x = (rand()%(max_x-min_x))+min_x;
			loc2_y = (rand()%(max_y-min_y))+min_y;
			}

	distance = sqrt(pow(loc1_x - loc2_x, 2) + pow(loc1_y - loc2_y, 2));
	direction = 1;
	if (speed > 0 && distance > 0) {
		double tmp = (distance / speed) / updateInterval;
		
			
			trace() << "--> check the distance : " << distance ;

			this->velocity.speed = speed;
			this->velocity.theta = atan2((loc2_y - loc1_y),(loc2_x - loc1_x));

			trace() << "[MIR] loc2_y : " << loc2_y << ", loc2_x : " << loc2_x;
			trace() << "[MIR] loc1_y : " << loc1_y << ", loc1_x : " << loc1_x;

			trace() << "[MIR] Init Velociy(speed):" << this->velocity.speed;
			trace() << "[MIR] Init velocity(theta): " << this->velocity.theta;

			incr_x = (loc2_x - loc1_x) / tmp;
			incr_y = (loc2_y - loc1_y) / tmp;

			setLocation(loc1_x, loc1_y, loc1_z);
			scheduleAt(simTime() + updateInterval,
			new MobilityManagerMessage("Periodic location update message", MOBILITY_PERIODIC));
		
	}
}

void ComplexLineMobility::handleMessage(cMessage * msg)
{

	int msgKind = msg->getKind();
	switch (msgKind) {

	case MOBILITY_PERIODIC:{


			// when move time finish, reprogram new route for moving nodes 
		if ( (testmovetime == 0 && change_direction_end_of_current_movetime ) || (changedirection == 1 &&  change_direction_end_of_current_route )){

			loc1_x = nodeLocation.x ;  // nodes already are here
			loc1_y = nodeLocation.y ;  

			loc2_x = (rand()%(max_x-min_x))+min_x;  // create new random location
			loc2_y = (rand()%(max_y-min_y))+min_y;

			trace() << "--> creat new line from: (" << loc1_x << "," <<loc1_y <<") to: (" << loc2_x << "," << loc2_y << ")" ;
		
			distance = sqrt(pow(loc1_x - loc2_x, 2) + pow(loc1_y - loc2_y, 2));
			direction = 1;

			if ((define_varies_speed)){
			speed = (rand()%(max_speed-min_speed))+min_speed;
			trace() << "--> test speed: current speed is: " << speed ;
			}

			this->velocity.speed = speed;
			this->velocity.theta = atan2((loc2_y - loc1_y),(loc2_x - loc1_x));

			trace() << "[MIR] loc2_y : " << loc2_y << ", loc2_x : " << loc2_x;
			trace() << "[MIR] loc1_y : " << loc1_y << ", loc1_x : " << loc1_x;

			trace() << "[MIR] updated velociy(speed):" << this->velocity.speed;
			trace() << "[MIR] updated velocity(theta): " << this->velocity.theta;


			if (speed > 0 && distance > 0) {
				double tmp = (distance / speed) / updateInterval;
					
			trace() << "--> check the distance :" << distance ;

				incr_x = (loc2_x - loc1_x) / tmp;
				incr_y = (loc2_y - loc1_y) / tmp;

				changedirection = 0 ;
				testmovetime = 0 ;
				}
				else trace() << "--> there is some error happened in this part " ;
		}
		// end of reprograming 


		if ((set_timeout_for_mobility) && (test_timeout >= timeout)) {

			change_direction_end_of_current_movetime = 0 ; // we inactive the defining new route
			change_direction_end_of_current_route = 0 ;

			if ((define_last_destination_for_movement)){
				loc1_x = nodeLocation.x ;  // nodes already are here
				loc1_y = nodeLocation.y ;  

				loc2_x = (rand()%(destination_max_x-destination_min_x))+destination_min_x;  // create new random location
				loc2_y = (rand()%(destination_max_y-destination_min_y))+destination_min_y;

				trace() << " --> the last destination fixed as :  (" << loc2_x <<"," << loc2_y << ")" ;



				distance = sqrt(pow(loc1_x - loc2_x, 2) + pow(loc1_y - loc2_y, 2));
				direction = 1;

				if ((define_varies_speed)){
				speed = (rand()%(max_speed-min_speed))+min_speed;
				trace() << " --> test speed: current speed is: " << speed ;
				}

				this->velocity.speed = speed;
				this->velocity.theta = atan2((loc2_y - loc1_y),(loc2_x - loc1_x));

				trace() << "[MIR] loc2_y : " << loc2_y << ", loc2_x : " << loc2_x;
				trace() << "[MIR] loc1_y : " << loc1_y << ", loc1_x : " << loc1_x;

				trace() << "[MIR] updated velociy(speed):" << this->velocity.speed;
				trace() << "[MIR] updated velocity(theta): " << this->velocity.theta;


				if (speed > 0 && distance > 0) {
					double tmp = (distance / speed) / updateInterval;
					
					trace() << "--> check the distance : " << distance ;

					incr_x = (loc2_x - loc1_x) / tmp;
					incr_y = (loc2_y - loc1_y) / tmp;

					changedirection = 0 ;
					testmovetime = 0 ;
					}
				else trace() << " --> there is some error happened in this part " ;
			}


			set_timeout_for_mobility = 0 ;
			stop_moving = 1 ;
			last_counter = changedirection_counter ;

		} //end if checking set timeout

		test_start_delay += updateInterval; // counter for messaring start time

		if ( (stop_moving) && ( changedirection_counter > last_counter)) { // test timeout
			break; // if you wana see the more trace you can remove this break command
			trace() << " --> node reached it's end position which is fixed as (" << loc2_x <<"," << loc2_y << ")" ; 
		}
		else {

		if (test_start_delay >= start_delay) {
				testmovetime += updateInterval; // counter for messaring movetime
				test_timeout += updateInterval; // counter for messaring timeout

			if (direction) {

				if (movetime >= testmovetime){  // if node in move period increase node location
				nodeLocation.x += incr_x;
				nodeLocation.y += incr_y;
				}
				else if ((movetime + pausetime) >= testmovetime) // if node in stop time don't change node location
			 		{
					// null

					} else {
					testmovetime = 0 ;
					}
	
				if (   (incr_x > 0 && nodeLocation.x > loc2_x)
				    || (incr_x < 0 && nodeLocation.x < loc2_x)
				    || (incr_y > 0 && nodeLocation.y > loc2_y)
				    || (incr_y < 0 && nodeLocation.y < loc2_y)) {
					direction = 0;
					nodeLocation.x -= (nodeLocation.x - loc2_x) * 2;
					nodeLocation.y -= (nodeLocation.y - loc2_y) * 2;

					changedirection = 1 ;
					changedirection_counter += 1 ;
				}
			} else {

				if (movetime >= testmovetime){  // if node in move period increase node location
				nodeLocation.x -= incr_x;
				nodeLocation.y -= incr_y;
				}
				else if ((movetime + pausetime) >= testmovetime) // if node in stop time don't change node location
			 		{
					// null
			
					} else {
						testmovetime = 0 ;
						}
								
				if (   (incr_x > 0 && nodeLocation.x < loc1_x)
				    || (incr_x < 0 && nodeLocation.x > loc1_x)
				    || (incr_y > 0 && nodeLocation.y < loc1_y)
				    || (incr_y < 0 && nodeLocation.y > loc1_y)) {
					direction = 1;
					changedirection_counter += 1 ;

					nodeLocation.x -= (nodeLocation.x - loc1_x) * 2;
					nodeLocation.y -= (nodeLocation.y - loc1_y) * 2;

					changedirection = 1 ;

				}
			} //end of if direction
		} // end of if start_delay
		} // end of the else for test timeout

			notifyWirelessChannel();
			scheduleAt(simTime() + updateInterval,
				new MobilityManagerMessage("Periodic location update message", MOBILITY_PERIODIC));

			trace() << "changed location(x:y) to " << nodeLocation.x << " : " << nodeLocation.y ;
			break;
		}

		default:{
			trace() << "WARNING: Unexpected message " << msgKind;
		}
	}

	delete msg;
	msg = NULL;
}

Velocity ComplexLineMobility::getVelocity()
{
	return velocity;
}