#include "simDefinitions.h"
#include "scenario.h"

void runScenarioB2(std::vector<int> a_arrival, std::vector<int> c_arrival) {
	int total_slots = 0;
	int a_curr = 0, c_curr = 0;
	int a_index = 0, c_index = 0;
	int a_success = 0, c_success = 0;
	int a_back = generate_backoff(0, CW_0);
	int c_back = generate_backoff(0, CW_0);
	int low_back = 0, high_back = 0;
	int total_collisions = 0, total_iterations = 0, temp_collisions = 0;
	// Modifying variables based on lecture slides 3.3 Wireless MA Slide 12
	// He has RTS NAV begin right after RTS finishes transmitting.
	int RTS_NAV = SIFS_slots + CTS_slots + SIFS_slots + FRAME_slots + SIFS_slots + ACK_slots;
	
	// Sets the initial arrivals
	a_curr = a_arrival.at(a_index);
	c_curr = c_arrival.at(c_index);
	while (total_slots < SIM_TIME_slots) {
		// Increment iterations
		total_iterations++;

		/*
		Collision: A and C arrive at receiver at the same time
		In this event, can ignore DIFS/SIFS/RTS/CTS/FRAME because they going to finish sending the RTS at the same time

		a_curr > c_curr		a_curr == c_curr	a_curr < c_curr
		a_back < c_back		a_back == c_back	a_back > c_back
		AAAA|__|NAV			AAA|___|NAV			AA|____|NAV
		CC|____|NAV			CCC|___|NAV			CCCC|__|NAV
		*/
		if ( (a_curr + a_back == c_curr + c_back) ) {
			total_slots = (total_slots < a_curr) ? a_curr : total_slots;
			temp_collisions++;
			total_collisions++;
			total_slots += DIFS_slots + a_back + RTS_slots + RTS_NAV;
			a_back = generate_backoff(temp_collisions, CW_0);
			c_back = generate_backoff(temp_collisions, CW_0);
		}

		/*
		Collision: One station starts sending RTS while other station already did
		
		1|dd___rr++++
		B|______Ss
		2|dd__rr+++
		*/
		else if ( abs( (a_curr + a_back) - (c_curr + c_back) ) < RTS_slots + SIFS_slots + CTS_slots ) {
			if ( (a_curr + a_back) > (c_curr + c_back) ) {
				total_slots = (total_slots < a_curr) ? a_curr : total_slots;
			}
			else {
				total_slots = (total_slots < c_curr) ? c_curr : total_slots;
			}
			temp_collisions++;
			total_collisions++;
			total_slots += DIFS_slots + a_back + RTS_slots + RTS_NAV;
			a_back = generate_backoff(temp_collisions, CW_0);
			c_back = generate_backoff(temp_collisions, CW_0);
		}
		/*
		Success: One station starts sending RTS while other station already did

		Diff == 2				Diff > 2
		Frozen w/ backoff = 0	Frozen w/ backoff leftover
		1|dd____\rr++++			1|dd________\___rr
		B|______Ss				B|__________SsccSs
		2|dd__rr+++				2|dd__rrSscc
		*/
		else if ( abs( (a_curr + a_back) - (c_curr + c_back) ) >= RTS_slots + SIFS_slots + CTS_slots ) {
			if ( (a_curr + a_back) > (c_curr + c_back) ) {
				temp_collisions = 0;
				total_slots = (total_slots < c_curr) ? c_curr : total_slots;
				total_slots += DIFS_slots + c_back + RTS_slots + RTS_NAV;
				a_back = abs( (a_curr + a_back) - (c_curr + c_back) );
				c_back = generate_backoff(temp_collisions, CW_0);
				c_success++;
				c_index++;
			}
			else {
				temp_collisions = 0;
				total_slots = (total_slots < a_curr) ? a_curr : total_slots;
				total_slots += DIFS_slots + a_back + RTS_slots + RTS_NAV;
				c_back = abs((a_curr + a_back) - (c_curr + c_back));
				a_back = generate_backoff(temp_collisions, CW_0);
				a_success++;
				a_index++;
			}
		}

		// Get the arrival time at whatever index we're on
		a_curr = a_arrival.at(a_index);
		c_curr = c_arrival.at(c_index);
		// If arrival time is behind total_time, bring it up to total time
		a_curr = (a_curr > total_slots) ? a_curr : total_slots;
		c_curr = (c_curr > total_slots) ? c_curr : total_slots;
	}

	std::cout << "\n\nTotal iterations to complete simulation time: " << total_iterations << std::endl;
	std::cout << "Total number of total_collisions: " << total_collisions << "\tSuccessful A nodes: " << a_success << "\tSuccessful C nodes: " << c_success << std::endl;
}