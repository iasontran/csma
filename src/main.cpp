#include "simDefinitions.h"
#include "scenario.h"
#include <ctime>

int SIM_TIME_slots = (SIM_TIME / (SLOT * pow(10, -6)));
int DIFS_slots = ceil((double)DIFS / SLOT);
int SIFS_slots = ceil((double)SIFS / SLOT);
int FRAME_slots = ((FRAME * 8.0) / (TRANSMISSION_RATE * pow(10, 6)) / (SLOT * pow(10, -6)));
int ACK_slots = (ACK * 8 / (TRANSMISSION_RATE * pow(10, 6)) / (SLOT * pow(10, -6)));
int RTS_slots = (RTS * 8 / (TRANSMISSION_RATE * pow(10, 6)) / (SLOT * pow(10, -6)));
int CTS_slots = (CTS * 8 / (TRANSMISSION_RATE * pow(10, 6)) / (SLOT * pow(10, -6)));
int CW_MAX = 1024;
int CW_0 = 4;

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
	int a_collisions = 0, c_collisions = 0, collisions = 0, collision_temp = 0;	// Number of collisions
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
    int diff = 0;

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
                
                if ( ( c_curr + DIFS_slots + c_back ) == ( tot_slots + DIFS_slots + a_back ) ) {
                    // A and C's backoff end at same slot
                    tot_slots += DIFS_slots + a_back + FRAME_slots + SIFS_slots + ACK_slots;    // unsure if exactly right, but should be where ACK is supposed to send
                    collisions++;
                    collision_temp++;
                    a_back = generate_backoff(collision_temp, CW_0);
                    c_back = generate_backoff(collision_temp, CW_0);
                    continue;
                } else if ( ( c_curr + DIFS_slots + c_back ) < ( tot_slots + DIFS_slots + a_back ) && ( c_curr + DIFS_slots + c_back ) > ( tot_slots + DIFS_slots ) ) {
                    // C transmits before A (current low arrival) and is within A's backoff range
                    a_back = (tot_slots + DIFS_slots + a_back) - (c_curr + DIFS_slots + c_back);    // calculate new backoff for A
                    c_back = generate_backoff(0, CW_0);
                    tot_slots = c_curr + DIFS_slots + c_back + FRAME_slots + SIFS_slots + ACK_slots;    // move current total slots to C's ACK slot
                    c_success++;    // increment for successful C transmission
                    k++;
                    collision_temp = 0;
                } else if ( ( c_curr + DIFS_slots + c_back ) < ( tot_slots + DIFS_slots + a_back ) && ( c_curr + DIFS_slots + c_back ) < ( tot_slots + DIFS_slots ) ) {
                    // C's arrival is less than A's current time slot backoff but C is also outside the range of A's backoff (no backoff freeze)
                    tot_slots += DIFS_slots + a_back + FRAME_slots + SIFS_slots + ACK_slots;    // add DIFS, backoff, frame, SIFS, and ack to total slots
                    a_back = generate_backoff(0, CW_0);
                    a_success++;    // increment successful A transmission
                    collision_temp = 0; // reset collision count (not sure if necessary) **
                    j++;    // increment to next A arrival slot
                } else if ( ( c_curr + DIFS_slots + c_back ) > ( tot_slots + DIFS_slots + a_back ) && ( c_curr + DIFS_slots ) < ( tot_slots + DIFS_slots + a_back ) ) {
                    // A transmits before C's backoff and is within C's backoff range
                    c_back = (c_curr + DIFS_slots + c_back) - (tot_slots + DIFS_slots + a_back);    // calculate new backoff for C
                    a_back = generate_backoff(0, CW_0);
                    tot_slots += DIFS_slots + a_back + FRAME_slots + SIFS_slots + ACK_slots;    // move current total slots to ACK slot
                    a_success++;    // increment for successful A transmission
                    j++;
                    collision_temp = 0;
                } else if ( ( c_curr + DIFS_slots + c_back ) > ( tot_slots + DIFS_slots + a_back ) && ( c_curr + DIFS_slots ) > ( tot_slots + DIFS_slots + a_back ) ) {
                    // A transmits before's C's backoff but transmits before C's backoff begins (no backoff freeze occurs)
                    tot_slots += DIFS_slots + a_back + FRAME_slots + SIFS_slots + ACK_slots;    // add DIFS, backoff, frame, SIFS, and ack to total slots
                    a_back = generate_backoff(0, CW_0);
                    a_success++;    // increment successful A transmission
                    collision_temp = 0; // reset collision count (not sure if necessary) **
                    j++;    // increment to next A arrival slot
                }

                /*
				// check when C's arrival occurs before A's backoff end
				if ((tot_slots + DIFS_slots + a_back) > c_curr) {
                    // calculate backoff difference
                    diff = (tot_slots + DIFS_slots + a_back) - (c_curr + DIFS_slots + c_back);
					// collision case when backoffs end at same time
					if (diff == 0) {
						tot_slots += DIFS_slots + a_back + FRAME_slots + SIFS_slots + ACK_slots;    // unsure if exactly right, but should be where ACK is supposed to send
						collisions++;
                        collision_temp++;
						a_back = generate_backoff(collision_temp, CW_0);
						c_back = generate_backoff(collision_temp, CW_0);
						continue;
					}
					else {    // no collision, but check earlier packet
						// if A transmits earlier than C
						if (diff < 0) {   // should be negative if A transmits earlier than C
							c_back = (c_curr + DIFS_slots + c_back) - (tot_slots + DIFS_slots + a_back);    // calculate new backoff for C
                            a_back = generate_backoff(0, CW_0);
							tot_slots += DIFS_slots + a_back + FRAME_slots + SIFS_slots + ACK_slots;    // move current total slots to ACK slot
							a_success++;    // increment for successful A transmission
                            j++;
                            collision_temp = 0;
						}
						else {    // C transmits earlier than A
							a_back = (tot_slots + DIFS_slots + a_back) - (c_curr + DIFS_slots + c_back);    // calculate new backoff for A
                            c_back = generate_backoff(0, CW_0);
							tot_slots = c_curr + DIFS_slots + c_back + FRAME_slots + SIFS_slots + ACK_slots;    // move current total slots to C's ACK slot
							c_success++;    // increment for successful C transmission
                            k++;
                            collision_temp = 0;
						}
					}
				}
				else {    // when A successfully transmits
                 tot_slots += DIFS_slots + a_back + FRAME_slots + SIFS_slots + ACK_slots;    // add DIFS, backoff, frame, SIFS, and ack to total slots
                 a_back = generate_backoff(0, CW_0);
                 a_success++;    // increment successful A transmission
                 collision_temp = 0; // reset collision count (not sure if necessary) **
                 j++;    // increment to next A arrival slot

				}
                 */
			}
			else if (c_curr < a_curr) { // current C arrival is earlier than A arrival
				if (tot_slots < c_curr) { // if total slots is less than current C arrival time
					tot_slots = c_curr;
				}
                
                if ( ( a_curr + DIFS_slots + a_back ) == ( tot_slots + DIFS_slots + c_back ) ) {
                    // A and C's backoff end at same slot
                    tot_slots += DIFS_slots + c_back + FRAME_slots + SIFS_slots + ACK_slots;    // unsure if exactly right, but should be where ACK is supposed to send
                    collision_temp++;
                    collisions++;
                    a_back = generate_backoff(collision_temp, CW_0);
                    c_back = generate_backoff(collision_temp, CW_0);
                    continue;
                } else if ( ( a_curr + DIFS_slots + a_back ) < ( tot_slots + DIFS_slots + c_back ) && ( a_curr + DIFS_slots + a_back ) > ( tot_slots + DIFS_slots ) ) {
                    // A transmits before C (current low arrival) and is within C's backoff range
                    c_back = (tot_slots + DIFS_slots + c_back) - (a_curr + DIFS_slots + a_back);    // calculate new backoff for C
                    tot_slots = a_curr + DIFS_slots + a_back + FRAME_slots + SIFS_slots + ACK_slots;    // move current total slots to A's ACK slot
                    a_back = generate_backoff(0, CW_0);
                    a_success++;    // increment for successful C transmission
                    j++;
                    collision_temp = 0;
                } else if ( ( a_curr + DIFS_slots + a_back ) < ( tot_slots + DIFS_slots + c_back ) && ( a_curr + DIFS_slots + a_back ) < ( tot_slots + DIFS_slots ) ) {
                    // A's arrival is less than C's current time slot backoff but A is also outside the range of C's backoff (no backoff freeze)
                    tot_slots += DIFS_slots + c_back + FRAME_slots + SIFS_slots + ACK_slots;    // add DIFS, backoff, frame, SIFS, and ack to total slots
                    c_back = generate_backoff(0, CW_0);
                    c_success++;    // increment successful A transmission
                    k++;    // increment to next A arrival slot
                    collision_temp = 0;
                } else if ( ( a_curr + DIFS_slots + a_back ) > ( tot_slots + DIFS_slots + c_back ) && ( tot_slots + DIFS_slots + c_back ) > ( a_curr + DIFS_slots ) ) {
                    // C transmits before A's backoff and is within A's backoff range
                    a_back = (a_curr + DIFS_slots + a_back) - (tot_slots + DIFS_slots + c_back);    // calculate new backoff for C
                    tot_slots += DIFS_slots + c_back + FRAME_slots + SIFS_slots + ACK_slots;    // move current total slots to ACK slot
                    c_back = generate_backoff(0, CW_0);
                    c_success++;    // increment for successful A transmission
                    k++;
                    collision_temp = 0;
                } else if ( ( a_curr + DIFS_slots + a_back ) > ( tot_slots + DIFS_slots + c_back ) && ( tot_slots + DIFS + c_back ) < ( a_curr + DIFS_slots ) ) {
                    // C transmits before's A backoff but transmits before A's backoff begins (no backoff freeze occurs)
                    tot_slots += DIFS_slots + c_back + FRAME_slots + SIFS_slots + ACK_slots;    // add DIFS, backoff, frame, SIFS, and ack to total slots
                    c_back = generate_backoff(0, CW_0);
                    c_success++;    // increment successful A transmission
                    k++;    // increment to next A arrival slot
                    collision_temp = 0;
                }

                /*
				// check if A arrival will transmit during C's backoff
				if ((tot_slots + DIFS_slots + c_back) > a_curr) {
                    diff = (tot_slots + DIFS_slots + c_back) - (a_curr + DIFS_slots + a_back);
					// collision case when backoffs end at same time
					if (diff == 0) {
						tot_slots += DIFS_slots + c_back + FRAME_slots + SIFS_slots + ACK_slots;    // unsure if exactly right, but should be where ACK is supposed to send
                        collision_temp++;
                        collisions++;
						a_back = generate_backoff(collision_temp, CW_0);
						c_back = generate_backoff(collision_temp, CW_0);
						continue;
					}
					else {    // no collision, but check earlier packet
						// if C transmits earlier than A
						if (diff < 0) {   // should be negative if C transmits earlier than A
							a_back = (a_curr + DIFS_slots + a_back) - (tot_slots + DIFS_slots + c_back);    // calculate new backoff for C
							tot_slots += DIFS_slots + c_back + FRAME_slots + SIFS_slots + ACK_slots;    // move current total slots to ACK slot
							c_success++;    // increment for successful A transmission
                            k++;
                            collision_temp = 0;
						}
						else {    // A transmits earlier than C
							c_back = (tot_slots + DIFS_slots + c_back) - (a_curr + DIFS_slots + a_back);    // calculate new backoff for C
							tot_slots = a_curr + DIFS_slots + a_back + FRAME_slots + SIFS_slots + ACK_slots;    // move current total slots to A's ACK slot
							a_success++;    // increment for successful C transmission
                            j++;
                            collision_temp = 0;
						}
					}
				}
				else {    // when C successfully transmits
					tot_slots += DIFS_slots + c_back + FRAME_slots + SIFS_slots + ACK_slots;    // add DIFS, backoff, frame, SIFS, and ack to total slots
					c_success++;    // increment successful A transmission
					collisions = 0; // reset collision count (not sure if necessary) **
					k++;    // increment to next A arrival slot
                    collision_temp = 0;
				}
                */
			}
			else {    // A and C arrivals are tied
				// assign either current arrival as current slot
				tot_slots = a_curr;
				if ((a_curr + DIFS_slots + a_back) == (c_curr + DIFS_slots + c_back)) {   // when backoffs end at same time, collision will occur
					tot_slots += DIFS_slots + a_back + FRAME_slots + SIFS_slots + ACK_slots;    // use either backoff to move current slot to ACK slot
					collisions++;
                    collision_temp++;
					a_back = generate_backoff(collision_temp, CW_0);
					c_back = generate_backoff(collision_temp, CW_0);
					continue;
				}
				else {    // determine which set has earlier backoff time
                    diff = a_back - c_back;
					// if A transmits earlier than C
					if (diff < 0) {   // should be negative if A transmits earlier than C
						c_back = (c_curr + DIFS_slots + c_back) - (a_curr + DIFS_slots + a_back);    // calculate new backoff for C
						tot_slots += DIFS_slots + a_back + FRAME_slots + SIFS_slots + ACK_slots;    // move current total slots to ACK slot
						a_success++;    // increment for successful A transmission
                        collision_temp = 0;
					}
					else if (diff > 0) {    // C transmits earlier than A
						a_back = (a_curr + DIFS_slots + a_back) - (c_curr + DIFS_slots + c_back);    // calculate new backoff for A
						tot_slots = c_curr + DIFS_slots + c_back + FRAME_slots + SIFS_slots + ACK_slots;    // move current total slots to C's ACK slot
						c_success++;    // increment for successful C transmission
                        collision_temp = 0;
					}
                    else {
                        std::cout << "Something went wrong (A and C backoff equal when determining which backoff is earlier in equal arrival time." << std::endl;
                    }
				}
			}
		}

		break;
	case A_VCS:	// Scenario A, CSMA 2
		lambda_A = 50;
		lambda_C = 50;
		for (int itr = 0; itr < 20; itr++) {
			a_poisson_set = generate_poisson(lambda_A);
			c_poisson_set = generate_poisson(lambda_C);
			a_arrival = generate_arrival(a_poisson_set);
			c_arrival = generate_arrival(c_poisson_set);
			runScenarioA2(a_arrival, c_arrival);
		}
		break;
	case B_CA:	// Scenario B, CSMA 1
		runScenarioB1(a_arrival, c_arrival);
		system("pause");
		break;
	case B_VCS:	// Scenario B, CSMA 2
		runScenarioB2(a_arrival, c_arrival);
		system("pause");
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
