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

//****************************************************************************
//* Modified at: Universiti Sains Malaysia (USM) 2011		             *
//* Author(s): Mahdi Zareei (zarei.1982@gmail.com)                           *
//***************************************************************************/

package node.mobilityManager.complexLineMobility;

simple ComplexLineMobility like node.mobilityManager.iMobilityManager {
 parameters:
	bool collectTraceInfo = default (false);
	double updateInterval = default (1000);
	double xCoorDestination = default (0);
	double yCoorDestination = default (0);


	double min_x = default (0);
	double max_x = default (10);
	double min_y = default (0);
	double max_y = default (10);

	double min_speed = default (1);
	double max_speed = default (2);


	double destination_min_x = default (0);
	double destination_max_x = default (10);
	double destination_min_y = default (0);
	double destination_max_y = default (10);

	bool define_varies_speed = default (false);

	bool set_timeout_for_mobility = default (false);
	bool define_last_destination_for_movement = default (false);

	double speed = default (1);
	double movetime = default (10); // how many second node need to move, you can change it in omnepp.ini
	double pausetime = default (5);  // how many second node should stop, you can change it in omnepp.ini

	double start_delay = default (0);
	double timeout = default (60);

	bool change_direction_end_of_current_movetime = default (false) ;
	bool change_direction_end_of_current_route = default (false) ;
}
