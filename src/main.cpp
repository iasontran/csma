#include "simDefinitions.h"
#include "scenario.h"
#include <ctime>

int main(int argc, char *argv[]) {
	/*
	Set the time seed
	*/
	srand((int)time(0));

	/*
	Simulation Variables in Slots
	*/
	int tot_slots = 0;
	int lambda_A = 200;	// set of 50, 100, 200, 300 frame/sec
	int lambda_C = 200;	// set of 50, 100, 200, 300 frame/sec
	int A_SLOTS = 0.0;
	int C_SLOTS = 0.0;

	/*
	Variables for selecting scenario
	*/
	int choice = 0;

	/*
	Variables for nodes A and C
	*/
	std::vector<int> a_poisson_set, c_poisson_set;	// The set of randomized values in slots
	std::vector<int> a_arrival, c_arrival;			// List of converted arrival times (summed up poisson set)
	int a_curr, c_curr;								// Current arrival time from a_arrival and c_arrival
	int a_back, c_back;									// Backoff value (randomly generated)
	int a_success = 0, c_success = 0;					// Number of successful transmissions
	int a_collisions = 0, c_collisions = 0, collisions = 0;	// Number of collisions
	int a_index = 0, c_index = 0;		// Tracking index of a_arrival and c_arrival

	// Variables for the other processes??????
	bool active_node = false;
	bool next_active = false;
	bool a_done = false;
	bool c_done = false;
    
    /*
	Selection of scenario
	*/
	printOptions();
	choice = getScenario();

	/*
	Generates a random arrival time
	*/
	a_poisson_set = generate_poisson(lambda_A);
	c_poisson_set = generate_poisson(lambda_C);

	/*
	Convert Poisson set into an arrival set
	*/
	a_arrival = generate_arrival(a_poisson_set);
	c_arrival = generate_arrival(c_poisson_set);

    double size;
	size = a_arrival.size() + c_arrival.size();
	a_back = generate_backoff(0, CW_0);
	c_back = generate_backoff(0, CW_0);
    a_curr = a_arrival.at(0);
	c_curr = c_arrival.at(0);
	int j = 0;
	int k = 0;

	// Increment every loop to see how many cycles it needed to get to SIM_TIME_slots
	int total_iterations = 0;	
	switch (choice) {
	case A_CA:	// Scenario A, CSMA
		while (tot_slots < SIM_TIME_slots) {   // while total time is less than simulated time
			// fetch arrival time
			a_curr = a_arrival.at(j);
			c_curr = c_arrival.at(k);

			// navigate through a's arrival set if available

			// check if current A arrival is earlier than C arrival
			if (a_curr < c_curr) {
				if (tot_slots < a_curr) { // if current total slot is less than current A arrival time
					tot_slots = a_curr; // set current total slots to A arrival time
				}

				// check if C arrival will transmit during A's backoff
				if ((tot_slots + DIFS_slots + a_back) > c_curr) {
					// collision case when backoffs end at same time
					if ((c_curr + DIFS_slots + c_back) == (tot_slots + DIFS_slots + a_back)) {
						tot_slots += DIFS_slots + a_back + FRAME_slots + SIFS_slots + ACK_slots;    // unsure if exactly right, but should be where ACK is supposed to send
						collisions++;
						a_back = generate_backoff(collisions, CW_0);
						c_back = generate_backoff(collisions, CW_0);
						continue;
					}
					else {    // no collision, but check earlier packet
						// if A transmits earlier than C
						if (((tot_slots + DIFS_slots + a_back) - (c_curr + DIFS_slots + c_back)) < 0) {   // should be negative if A transmits earlier than C
							c_back = (c_curr + DIFS_slots + c_back) - (tot_slots + DIFS_slots + a_back);    // calculate new backoff for C
							tot_slots += DIFS_slots + a_back + FRAME_slots + SIFS_slots + ACK_slots;    // move current total slots to ACK slot
							a_success++;    // increment for successful A transmission
						}
						else {    // C transmits earlier than A
							a_back = (tot_slots + DIFS_slots + a_back) - (c_curr + DIFS_slots + c_back);    // calculate new backoff for A
							tot_slots = c_curr + DIFS_slots + c_back + FRAME_slots + SIFS_slots + ACK_slots;    // move current total slots to C's ACK slot
							c_success++;    // increment for successful C transmission
						}
					}
				}
				else {    // when A successfully transmits
					tot_slots += DIFS_slots + a_back + FRAME_slots + SIFS_slots + ACK_slots;    // add DIFS, backoff, frame, SIFS, and ack to total slots
					a_success++;    // increment successful A transmission
					collisions = 0; // reset collision count (not sure if necessary) **
					j++;    // increment to next A arrival slot
				}
			}
			else if (c_curr < a_curr) { // current C arrival is earlier than A arrival
				if (tot_slots < c_curr) { // if total slots is less than current C arrival time
					tot_slots = c_curr;
				}

				// check if A arrival will transmit during C's backoff
				if ((tot_slots + DIFS_slots + c_back) > a_curr) {
					// collision case when backoffs end at same time
					if ((a_curr + DIFS_slots + a_back) == (tot_slots + DIFS_slots + c_back)) {
						tot_slots += DIFS_slots + c_back + FRAME_slots + SIFS_slots + ACK_slots;    // unsure if exactly right, but should be where ACK is supposed to send
						collisions++;
						a_back = generate_backoff(collisions, CW_0);
						c_back = generate_backoff(collisions, CW_0);
						continue;
					}
					else {    // no collision, but check earlier packet
						// if C transmits earlier than A
						if (((tot_slots + DIFS_slots + c_back) - (a_curr + DIFS_slots + a_back)) < 0) {   // should be negative if C transmits earlier than A
							a_back = (a_curr + DIFS_slots + a_back) - (tot_slots + DIFS_slots + c_back);    // calculate new backoff for C
							tot_slots += DIFS_slots + c_back + FRAME_slots + SIFS_slots + ACK_slots;    // move current total slots to ACK slot
							c_success++;    // increment for successful A transmission
						}
						else {    // A transmits earlier than C
							c_back = (c_curr + DIFS_slots + c_back) - (tot_slots + DIFS_slots + a_back);    // calculate new backoff for C
							tot_slots = a_curr + DIFS_slots + a_back + FRAME_slots + SIFS_slots + ACK_slots;    // move current total slots to A's ACK slot
							a_success++;    // increment for successful C transmission
						}
					}
				}
				else {    // when C successfully transmits
					tot_slots += DIFS_slots + c_back + FRAME_slots + SIFS_slots + ACK_slots;    // add DIFS, backoff, frame, SIFS, and ack to total slots
					c_success++;    // increment successful A transmission
					collisions = 0; // reset collision count (not sure if necessary) **
					k++;    // increment to next A arrival slot
				}
			}
			else {    // A and C arrivals are tied
				// assign either current arrival as current slot
				tot_slots = a_curr;
				if ((a_curr + DIFS_slots + a_back) == (c_curr + DIFS_slots + c_back)) {   // when backoffs end at same time, collision will occur
					tot_slots += DIFS_slots + a_back + FRAME_slots + SIFS_slots + ACK_slots;    // use either backoff to move current slot to ACK slot
					collisions++;
					a_back = generate_backoff(collisions, CW_0);
					c_back = generate_backoff(collisions, CW_0);
					continue;
				}
				else {    // determine which set has earlier backoff time
					// if A transmits earlier than C
					if (((tot_slots + DIFS_slots + a_back) - (c_curr + DIFS_slots + c_back)) < 0) {   // should be negative if A transmits earlier than C
						c_back = (c_curr + DIFS_slots + c_back) - (tot_slots + DIFS_slots + a_back);    // calculate new backoff for C
						tot_slots += DIFS_slots + a_back + FRAME_slots + SIFS_slots + ACK_slots;    // move current total slots to ACK slot
						a_success++;    // increment for successful A transmission
					}
					else {    // C transmits earlier than A
						a_back = (tot_slots + DIFS_slots + a_back) - (c_curr + DIFS_slots + c_back);    // calculate new backoff for A
						tot_slots = c_curr + DIFS_slots + c_back + FRAME_slots + SIFS_slots + ACK_slots;    // move current total slots to C's ACK slot
						c_success++;    // increment for successful C transmission
					}
				}
			}
		}

		break;
	case A_VCS:	// Scenario A, CSMA 2
		runScenarioA2(a_arrival, c_arrival);
		system("pause");
		break;
	case B_CA:	// Scenario B, CSMA 1
    
		break;
	case B_VCS:	// Scenario B, CSMA 2
    
		break;
	}
}

/*
CSMA/CA with virtual carrier sensing enabled:
RTS and CTS frames are exchanged before the transmission  of  a  frame.   
If  RTS  transmissions  collide,  stations  invoke  the  exponential  backoff
mechanism outlined in 1(c).  Otherwise,  stations that overhear an RTS/CTS message 
defer from transmission for the time indicated in the NAV vector
*/

// OLD SCENARIO SELECTION
/*
	while (entry) {
		printf("Select \"Concurrent\" Communications or \"Hidden\" Terminals: ");
		std::cin >> scenario;
		if (scenario.compare("Concurrent") == 0) {
			entry = false;
		}
		else if (scenario.compare("Hidden") == 0) {
			entry = false;
		}
		else {
			printf("Please pick either \"Concurrent\" or \"Hidden\".\n");
			continue;
		}
	}
	entry = true;
	while (entry) {
		printf("Choose CSMA type (CA or VCS): ");
		std::cin >> metric;
		if (metric.compare("CA") == 0) {
			entry = false;
		}
		else if (metric.compare("VCS") == 0) {
			entry = false;
		}
		else {
			printf("Please pick either \"CA\" or \"VCS\".");
			continue;
		}
	}
*/
