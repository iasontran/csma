#include "simDefinitions.h"
#include "scenario.h"

void runScenarioA2(std::vector<int> a_arrival, std::vector<int> c_arrival) {
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
	int RTS_NAV = 0;
	RTS_NAV = SIFS_slots + CTS_slots + SIFS_slots + FRAME_slots + SIFS_slots + ACK_slots; // the 0 is the backoff, just a place holder
	std::ofstream myFile;
	myFile.open("a2out.txt", std::ios::app);
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
		if ((a_curr + a_back == c_curr + c_back)) {
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
		else if (abs((a_curr + a_back) - (c_curr + c_back)) < RTS_slots) {
			if ((a_curr + a_back) > (c_curr + c_back)) {
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
		else if (abs((a_curr + a_back) - (c_curr + c_back)) >= RTS_slots) {
			if ((a_curr + a_back) > (c_curr + c_back)) {
				temp_collisions = 0;
				total_slots = (total_slots < c_curr) ? c_curr : total_slots;
				total_slots += DIFS_slots + c_back + RTS_slots + RTS_NAV;
				a_back = abs((a_curr + a_back) - (c_curr + c_back));
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
	myFile << total_iterations << "\t" << total_collisions << "\t" << a_success << "\t" << c_success << std::endl;
	myFile.close();
	std::cout << "\n\nTotal iterations to complete simulation time: " << total_iterations << std::endl;
	std::cout << "Total number of total_collisions: " << total_collisions << "\tSuccessful A nodes: " << a_success << "\tSuccessful C nodes: " << c_success << std::endl;
	
}

/*
		If total slot is currently idle, and no queue, jump ahead.
		Ex1: total_slots = 0, a_curr = 130 and c_curr = 180, this will jump to 130.
		Ex2: total_slots = 512, a_curr = 921 and c_curr = 802, this will jump to 802.
		Ex3: total_slots = 1024, a_curr = 921, c_curr = 1400, no change to tot_slot
		
		if (total_slots < a_arrival.at(a_index) && total_slots < c_curr) {
			(a_curr <= c_curr) ? (total_slots = a_curr) : (total_slots = c_curr);
		}
*/

// while (total_slots < SIM_TIME_slots) {
// 	/*
// 	If total slot is currently idle, and no queue, jump ahead.
// 	Ex1: total_slots = 0, a_curr = 130 and c_curr = 180, this will jump to 130.
// 	Ex2: total_slots = 512, a_curr = 921 and c_curr = 802, this will jump to 802.
// 	*/
// 	if (total_slots < a_curr && total_slots < c_curr) {
// 		(a_curr <= c_curr) ? (total_slots = a_curr) : (total_slots = c_curr);
// 	}
// 
// 	// Sets the current arrivals to be either the index if tot_slot is less. 
// 	a_curr = (a_arrival.at(a_index) > total_slots) ? a_arrival.at(a_index) : total_slots;
// 	c_curr = (c_arrival.at(c_index) > total_slots) ? c_arrival.at(c_index) : total_slots;
// 
// 	// Event when both arrivals are the same
// 	if (a_curr == c_curr) {
// 
// 		// Check whether a_curr is before or after tot_slots. If it's after, tot_slots needs to forward time to it.
// 		total_slots = ((a_curr > total_slots) ? (a_curr) : (total_slots));
// 
// 		/*
// 		When both backoffs are the same, the RTS collides, so collision incremented, generate new backoff
// 		d = DIFS = 2, a = A_back, c = C_back, r = RTS = 2
// 
// 		A|ddaaarr ~~ Both end at same time
// 		C|ddcccrr ~~ Thus, collision
// 		*/
// 		if (a_back == c_back) {
// 			temp_collisions++;
// 			total_collisions++;
// 			total_slots += DIFS_slots + a_back + RTS_slots + RTS_NAV;
// 			a_back = generate_backoff(temp_collisions, CW_0);
// 			c_back = generate_backoff(temp_collisions, CW_0);
// 			//std::cout << "!!!!COLLISION!!!!" << std::endl;
// 		}
// 		else {
// 			// Assigning the lower/higher backoffs so I don't have to do both a_back > c_back and c_back > a_back
// 			if (a_back < c_back) {
// 				low_back = a_back;
// 				high_back = c_back;
// 			}
// 			else {
// 				low_back = c_back;
// 				high_back = a_back;
// 			}
// 			/*
// 			First station starts transmitting, but second station starts transmitting before first finishes.
// 			d = DIFS = 2, a = A_back, c = C_back, r = RTS = 2
// 
// 			L|ddaaarr	~~ Since LOW did not finish and HIGH started
// 			H|ddccccrr	~~ RTS will overlap, thus collision
// 			*/
// 			if ((DIFS_slots + high_back) < (DIFS_slots + low_back + RTS_slots)) {
// 				temp_collisions++;
// 				total_collisions++;
// 				total_slots += DIFS_slots + high_back + RTS_slots + RTS_NAV;
// 				a_back = generate_backoff(temp_collisions, CW_0);
// 				c_back = generate_backoff(temp_collisions, CW_0);
// 				//std::cout << "!!!!COLLISION!!!!" << std::endl;
// 			}
// 			/*
// 			First station starts and finishes transmitting before the second station starts, so second station will wait.
// 			d = DIFS = 2, a = A_back, c = C_back, r = RTS = 2
// 
// 			L|ddaaarr	~~ Since LOW finishes before HIGH started
// 			H|ddcccccrr	~~ RTS does not overlap, thus success
// 			*/
// 			else {
// 				// Check which one hit 0 first, update and generate new backoff
// 				if (a_back < c_back) {
// 					c_back -= (a_back + RTS_slots);
// 					a_back = generate_backoff(0, CW_0);
// 					a_index++;
// 					a_success++;
// 				}
// 				else {
// 					a_back -= (c_back + RTS_slots);
// 					c_back = generate_backoff(0, CW_0);
// 					c_index++;
// 					c_success++;
// 				}
// 				temp_collisions = 0;
// 				total_slots += DIFS_slots + low_back + RTS_slots + RTS_NAV;
// 			}
// 		}
// 	}
// 	else if (a_curr < c_curr) {
// 		// Check whether a_curr is before or after tot_slots. If it's after, tot_slots needs to forward time to it.
// 		total_slots = ((a_curr > total_slots) ? (a_curr) : (total_slots));
// 
// 		/*
// 		A starts transmitting, C starts transmitting before A finishes
// 		d = DIFS = 2, a = A_back, c = C_back, r = RTS = 2
// 
// 		A|ddaaaaarr	~~ Since C starts before A finishes
// 		CC|ddcccrr	~~ RTS does overlap, thus collision
// 		*/
// 		if ((c_curr + DIFS_slots + c_back + RTS_slots <= a_curr + DIFS_slots + a_back + RTS_slots)
// 			&& (c_curr + DIFS_slots + c_back > a_curr + DIFS_slots + a_back)) {
// 			temp_collisions++;
// 			total_collisions++;
// 			total_slots += DIFS_slots + c_back + RTS_slots + RTS_NAV;
// 			a_back = generate_backoff(temp_collisions, CW_0);
// 			c_back = generate_backoff(temp_collisions, CW_0);
// 			////std::cout << "!!!!COLLISION!!!!" << std::endl;
// 		}
// 		/*
// 		A starts transmitting, C starts transmitting before a finishes
// 		d = DIFS = 2, a = A_back, c = C_back, r = RTS = 2
// 
// 		A|ddaaaaaarr	~~ Since C starts before A starts
// 		CC|ddcccrr		~~ RTS does not overlap, thus C success
// 		*/
// 		else if ((c_curr + DIFS_slots + c_back + RTS_slots <= a_curr + DIFS_slots + a_back)) {
// 			total_slots = c_curr + DIFS_slots + c_back + RTS_slots + RTS_NAV;
// 			c_back = generate_backoff(0, CW_0);
// 			c_index++;
// 			c_success++;
// 			temp_collisions = 0;
// 		}
// 		/*
// 		A starts transmitting and finishes before C starts
// 		d = DIFS = 2, a = A_back, c = C_back, r = RTS = 2
// 
// 		A|ddaaaarr		~~ Since C starts after A finishes
// 		CC|ddcccccrr	~~ RTS does not overlap, thus A success
// 		*/
// 		else {
// 			total_slots += DIFS_slots + a_back + RTS_slots + RTS_NAV;
// 			a_back = generate_backoff(0, CW_0);
// 			a_index++;
// 			a_success++;
// 			temp_collisions = 0;
// 		}
// 	}
// 	else {
// 		// Check whether a_curr is before or after tot_slots. If it's after, tot_slots needs to forward time to it.
// 		total_slots = ((c_curr > total_slots) ? (c_curr) : (total_slots));
// 
// 		/*
// 		A starts transmitting, C starts transmitting before a finishes
// 		d = DIFS = 2, a = A_back, c = C_back, r = RTS = 2
// 
// 		AA|ddaaarr	~~ Since C starts before A finishes
// 		C|ddcccccrr	~~ RTS does overlap, thus collision
// 		*/
// 		if ((a_curr + DIFS_slots + a_back + RTS_slots <= c_curr + DIFS_slots + c_back + RTS_slots)
// 			&& (a_curr + DIFS_slots + a_back > c_curr + DIFS_slots + c_back)) {
// 			temp_collisions++;
// 			total_collisions++;
// 			total_slots += DIFS_slots + a_back + RTS_slots + RTS_NAV;
// 			a_back = generate_backoff(temp_collisions, CW_0);
// 			c_back = generate_backoff(temp_collisions, CW_0);
// 			////std::cout << "!!!!COLLISION!!!!" << std::endl;
// 		}
// 		/*
// 		A starts transmitting, C starts transmitting before a finishes
// 		d = DIFS = 2, a = A_back, c = C_back, r = RTS = 2
// 
// 		AA|ddaaarr	~~ Since C starts before A starts
// 		C|ddcccccrr		~~ RTS does not overlap, thus C success
// 		*/
// 		else if ((a_curr + DIFS_slots + a_back + RTS_slots <= c_curr + DIFS_slots + c_back)) {
// 			total_slots = a_curr + DIFS_slots + a_back + RTS_slots + RTS_NAV;
// 			a_back = generate_backoff(0, CW_0);
// 			a_index++;
// 			a_success++;
// 			temp_collisions = 0;
// 
// 		}
// 		/*
// 		C starts transmitting and finishes before A starts
// 		d = DIFS = 2, a = A_back, c = C_back, r = RTS = 2
// 
// 		AA|ddaaaaarr		~~ Since A starts after C finishes
// 		C|ddccccrr	~~ RTS does not overlap, thus C success
// 		*/
// 		else {
// 			total_slots += DIFS_slots + c_back + RTS_slots + RTS_NAV;
// 			c_back = generate_backoff(0, CW_0);
// 			c_index++;
// 			c_success++;
// 			temp_collisions = 0;
// 		}
// 	}
// 	total_iterations++;
// 	//std::cout << total_iterations << ") Current time: " << total_slots << std::endl;
// 	//std::cout << "Current total_collisions: " << total_collisions << std::endl << std::endl;
// }