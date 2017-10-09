#include "simDefinitions.h"
#include "scenario.h"

void runScenarioA2(std::vector<int> a_arrival, std::vector<int> c_arrival) {
	int tot_slots = 0;
	int a_curr = a_arrival.at(0);
	int c_curr = c_arrival.at(0);
	int a_back = generate_backoff(0, CW_0);
	int c_back = generate_backoff(0, CW_0);
	int a_index = 0, c_index = 0;
	int a_success = 0, c_success = 0;
	int collisions = 0, total_iterations = 0;

	while (tot_slots < SIM_TIME_slots) {
		// Every new slot transmission will have at least DIFS, calling it running because current is being used.
		int running_slot = DIFS_slots;
		int RTS_NAV = RTS_slots + SIFS_slots + CTS_slots + SIFS_slots + FRAME_slots + SIFS_slots + ACK_slots;
		int temp_collision = 0;
		/*
		If total slot is currently idle, and no queue, jump ahead.
		Ex1: tot_slots = 0, a_curr = 130 and c_curr = 180, this will jump to 130.
		Ex2: tot_slots = 512, a_curr = 921 and c_curr = 802, this will jump to 802.
		*/
		if (tot_slots < a_curr && tot_slots < c_curr) {
			(a_curr <= c_curr) ? (tot_slots = a_curr) : (tot_slots = c_curr);
		}

		if (a_curr == c_curr) {
			// If backoffs are the same, increment collision. Generate new backoffs until no longer same
			if (a_back == c_back) {
				running_slot += a_back + DIFS_slots;
				a_back = generate_backoff(++temp_collision, CW_0);
				c_back = generate_backoff(temp_collision, CW_0);
				std::cout << "!!!!COLLISION!!!!" << std::endl;
			}
			else {
				// Decrement backoff until one is 0, adding time to tempSlot.
				(a_back < c_back) ? (a_index++) : (c_index++);
				while (a_back != 0 && c_back != 0) {
					a_back--;
					c_back--;
					running_slot++;
				}
				// Check which one hit 0 first, update and generate new backoff
				if (a_back == 0) {
					running_slot += RTS_NAV;
					a_back = generate_backoff(0, CW_0);
					a_success++;
				}
				else {
					running_slot += RTS_NAV;
					c_back = generate_backoff(0, CW_0);
					c_success++;
				}
			}
		}
		else if (a_curr < c_curr) {
			// Move a_index to next arrival since we're processing this one
			a_index++;

			// If backoffs are the same, increment collision
			if ((a_curr > tot_slots ? a_curr : tot_slots) + a_back + DIFS_slots >= c_curr + DIFS_slots) {
				running_slot += a_back + DIFS_slots;
				a_back = generate_backoff(++temp_collision, CW_0);
				c_back = generate_backoff(temp_collision, CW_0);
				std::cout << "!!!!COLLISION!!!!" << std::endl;
			}
			else {
				// Decrement backoff until one is 0, adding time to tempSlot.
				while (a_back != 0) {
					a_back--;
					running_slot++;
				}
				// Check which one hit 0 first, update and generate new backoff
				running_slot += RTS_NAV;
				a_back = generate_backoff(0, CW_0);
				a_success++;
			}
		}
		else {
			// Move a_index to next arrival since we're processing this one
			c_index++;

			// If backoffs are the same, increment collision
			if ((c_curr > tot_slots ? c_curr : tot_slots) + c_back + DIFS_slots >= a_curr + DIFS_slots) {
				running_slot += c_back + DIFS_slots;
				a_back = generate_backoff(++temp_collision, CW_0);
				c_back = generate_backoff(temp_collision, CW_0);
				std::cout << "!!!!COLLISION!!!!" << std::endl;
			}
			else {
				// Decrement backoff until one is 0, adding time to tempSlot.
				while (c_back != 0) {
					c_back--;
					running_slot++;
				}
				// Check which one hit 0 first, update and generate new backoff
				running_slot += RTS_NAV;
				c_back = generate_backoff(0, CW_0);
				c_success++;
			}
		}

		// Update tot_slot, a_curr, c_curr
		tot_slots += running_slot;	// Add the running slot to the total slow
		collisions += temp_collision;
		a_curr = a_arrival.at(a_index);
		c_curr = c_arrival.at(c_index);
		total_iterations++;
		std::cout << total_iterations << ") Current time: " << tot_slots << std::endl;
		std::cout << "Current collisions: " << collisions << std::endl << std::endl;
	}

	std::cout << "\n\nTotal iterations to complete simulation time: " << total_iterations << std::endl;
	std::cout << "Total number of collisions: " << collisions << "\tSuccessful A nodes: " << a_success << "\tSuccessful C nodes: " << c_success << std::endl;
}