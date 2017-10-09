#include "simDefinitions.h"
#include "scenario.h"

void runScenarioB2(std::vector<int> a_arrival, std::vector<int> c_arrival) {
	int tot_slots = 0;
	int a_curr = a_arrival.at(0);
	int c_curr = c_arrival.at(0);
	int a_back = generate_backoff(0, CW_0);
	int c_back = generate_backoff(0, CW_0);
	int a_index = 0, c_index = 0;
	int a_success = 0, c_success = 0;
	int a_temp = 0, c_temp = 0;
	int collisions = 0, total_iterations = 0;
	int temp_collision = 0;

	while (tot_slots < SIM_TIME_slots) {
		// Every new slot transmission will have at least DIFS, calling it running because current is being used.
		int running_slot = DIFS_slots;
		int RTS_NAV = RTS_slots + SIFS_slots + CTS_slots + SIFS_slots + FRAME_slots + SIFS_slots + ACK_slots;
		int CTS_NAV = CTS_slots + SIFS_slots + FRAME_slots + SIFS_slots + ACK_slots;
		
		/*
		If total slot is currently idle, and no queue, jump ahead.
		Ex1: tot_slots = 0, a_curr = 130 and c_curr = 180, this will jump to 130.
		Ex2: tot_slots = 512, a_curr = 921 and c_curr = 802, this will jump to 802.
		*/
		if (tot_slots < a_curr && tot_slots < c_curr) {
			(a_curr <= c_curr) ? (tot_slots = a_curr) : (tot_slots = c_curr);
		}

		// Start the process, since hidden, will track both transmitters
		// (a_curr > tot_slots ? a_curr : tot_slots)
		a_temp = (a_curr > tot_slots ? a_curr : tot_slots) + RTS_slots + SIFS_slots;	// Total time 'a' will take before backoff is added
		c_temp = (c_curr > tot_slots ? c_curr : tot_slots) + RTS_slots + SIFS_slots;	// Total time 'c' will take before backoff is added
		
		if (a_curr == c_curr) {
			if (a_back == c_back) {	
				/*
				In this event, both will transmit since both will not know the other.
				Easiest one to track, will increment collision and just send a NO_ACK
				Which is the same as an ACK but doesn't increment success.
				*/
				temp_collision++;
				running_slot += RTS_NAV + a_back;
				a_back = generate_backoff(temp_collision, CW_0);
				c_back = generate_backoff(temp_collision, CW_0);
			}
			else {
				if (a_temp + a_back < c_curr + c_back + DIFS_slots) {
					/*
					Event where 'a' finishes sending an RTS before 'c' finishes its backoff.
					I believe in this case, 'c' will freeze as it hasn't gotten to send the RTS yet.
					This means 'a' will successfully transmit.
					*/
					while (a_back != 0 && c_back != 0) {
						a_back--;
						c_back--;
						running_slot++;	// Will add a_back, while decrementing c_back until freeze
					}

					running_slot += RTS_NAV;
					a_back = generate_backoff(0, CW_0);
					a_index++;
					a_success++;
					collisions += temp_collision;
					temp_collision = 0;
				}
				else if (c_temp + c_back < a_curr + a_back + DIFS_slots){
					/*
					Event where 'c' finishes sending an RTS before 'a' finishes its backoff.
					I believe in this case, 'a' will freeze as it hasn't gotten to send the RTS yet.
					This means 'c' will successfully transmit.
					*/
					while (a_back != 0 && c_back != 0) {
						a_back--;
						c_back--;
						running_slot++; // Will add a_back, while decrementing c_back until freeze
					}
					// Add rest of CTS time
					running_slot += RTS_NAV;
					c_back = generate_backoff(0, CW_0);
					c_index++;
					c_success++;
					collisions += temp_collision;
					temp_collision = 0;
				}
				else {
					/*
					Event where 'a' and 'c' backoff times are too close together that one will
					transmit before the other finishes sending the RTS. Thus, NO ACK.
					*/
					temp_collision++;
					running_slot += RTS_NAV + (a_back > c_back ? a_back : c_back);
					a_back = generate_backoff(temp_collision, CW_0);
					c_back = generate_backoff(temp_collision, CW_0);
				}
			}
		}
		else if (a_curr < c_curr) {
			if (a_temp + a_back < c_curr + c_back + DIFS_slots) {
				/*
				Event where 'a' sends RTS to receiver before 'c' arrives
				This means 'a' will successfully transmit as 'c' will have seen the CTS_NAV and freeze
				*/
				running_slot += RTS_NAV + a_back;
				a_index++;
				a_back = generate_backoff(0, CW_0);
				a_success++;
				collisions += temp_collision;
				temp_collision = 0;
			}
			else {
				/*
				Event where 'a' sends RTS to receiver but 'c' sends before receiver can send a CTS.
				This will result in a collision.
				*/
				temp_collision++;
				running_slot += RTS_NAV + (a_back > c_back ? a_back : c_back);
				a_back = generate_backoff(temp_collision, CW_0);
				c_back = generate_backoff(temp_collision, CW_0);
			}
		}
		else {
			if (c_temp + c_back < a_curr + a_back + DIFS_slots) {
				/*
				Event where 'c' sends RTS to receiver before 'a' arrives
				This means 'c' will successfully transmit as 'a' will have seen the CTS_NAV and freeze
				*/
				running_slot += RTS_NAV + c_back;
				c_index++;
				c_back = generate_backoff(0, CW_0);
				c_success++;
				collisions += temp_collision;
				temp_collision = 0;
			}
			else {
				/*
				Event where 'c' sends RTS to receiver but 'a' sends before receiver can send a CTS.
				This will result in a collision.
				*/
				temp_collision++;
				running_slot += RTS_NAV + (a_back > c_back ? a_back : c_back);
				a_back = generate_backoff(temp_collision, CW_0);
				c_back = generate_backoff(temp_collision, CW_0);
			}
		}

		// Update tot_slot, a_curr, c_curr
		tot_slots += running_slot;	// Add the running slot to the total slow
		a_curr = a_arrival.at(a_index);
		c_curr = c_arrival.at(c_index);
		total_iterations++;
		std::cout << total_iterations << ") Current time: " << tot_slots << std::endl;
		std::cout << "Current collisions: " << collisions << std::endl << std::endl;
	}

	std::cout << "\n\nTotal iterations to complete simulation time: " << total_iterations << std::endl;
	std::cout << "Total number of collisions: " << collisions << "\tSuccessful A nodes: " << a_success << "\tSuccessful C nodes: " << c_success << std::endl;
}