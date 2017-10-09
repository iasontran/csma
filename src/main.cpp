#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>

/*
Simulation Parameters
*/
#define SIM_TIME 10 // in seconds
#define SLOT 20 // in microseconds
#define DIFS 40 // in microseconds
#define SIFS 10 // in microseconds
#define CW_MAX 1024 // in slots
#define CW_0 4 // in slots
#define FRAME 1500 // in bytes
#define ACK 30 // in bytes
#define RTS 30 // in bytes
#define CTS 30 // in bytes
#define TRANSMISSION_RATE 6 // in Mbps

/*
Define four states, using arbitrary negatives to not conflict with anything else
*/
#define A_CA -10
#define A_VCS -11
#define B_CA -12
#define B_VCS -13

/*
Probably not using????
*/
class Station {
private:
	std::string id;
	std::string data;
	std::string toStation;
public:

};

/*
Function to generate the backoff value
*/
int backoffgen(int collide, double CW) {
	int bound = (pow(2, collide) * CW) - 1;
	int output;
	bool done = false;

	while (!done) {	// Keep generating new backoff but must be less than CW_MAX
		output = abs(rand() % bound);
		if (output < CW_MAX) {
			done = true;
		}
	}

    return output;
}

/*
Print scenario options
*/
void printOptions() {
	std::cout << "Select a scenario:\n";
	std::cout << "A1: Concurrent Communication with CSMA Collision Avoidance\n";
	std::cout << "A2: Concurrent Communication with CSMA Virtual Carrier Sensing\n";
	std::cout << "B1: Hidden Terminal Communication with CSMA Collision Avoidance\n";
	std::cout << "B2: Hidden Terminal Communication with CSMA Virtual Carrier Sensing\n";
}

int main(int argc, char *argv[]) {

	/*
	Set the time seed
	*/
	srand((int)time(0));

	/*
	Simulation Variables in Slots
	*/
	double tot_slots = 0.0;
	double SIM_TIME_slots = 0.0;
	double DIFS_slots = 0.0;
	double SIFS_slots = 0.0;
	double FRAME_slots = 0.0;
	double ACK_slots = 0.0;
	double RTS_slots = 0.0;
	double CTS_slots = 0.0;
	double lambda_A = 200.0;	// set of 50, 100, 200, 300 frame/sec
	double lambda_C = 200.0;	// set of 50, 100, 200, 300 frame/sec
    double A_SLOTS = 0.0;
    double C_SLOTS = 0.0;

	/*
	Variables for selecting scenario
	*/
	std::string scenario;
	int choice;
	bool entry = true;

	/*
	Variables for nodes A and C
	*/
	std::vector<double> a_poisson_set, c_poisson_set;	// The set of randomized values in slots
	std::vector<double> a_arrival, c_arrival;			// List of converted arrival times (summed up poisson set)
	double a_curr, c_curr;								// Current arrival time from a_arrival and c_arrival
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
	Converting all units into SLOT time. We can increment by doing:
	tot_slot += DIFS_slots + (a_back or c_back) + FRAME_slots + SIFS_slots + ACK_slots; 
	*/
    SIM_TIME_slots = lround(SIM_TIME / (SLOT * pow(10,-6)));
	DIFS_slots = ceil((double)DIFS / SLOT);
	SIFS_slots = ceil((double)SIFS / SLOT);
	FRAME_slots = lround((FRAME * 8.0) / (TRANSMISSION_RATE * pow(10, 6)) / (SLOT * pow(10, -6)));
	ACK_slots = lround(ACK * 8 / (TRANSMISSION_RATE * pow(10, 6)) / (SLOT * pow(10, -6)));
	RTS_slots = lround(RTS * 8 / (TRANSMISSION_RATE * pow(10, 6)) / (SLOT * pow(10, -6)));
	CTS_slots = lround(CTS * 8 / (TRANSMISSION_RATE * pow(10, 6)) / (SLOT * pow(10, -6)));
    
    /*
	Selection of scenario
	*/
	while (entry) {
		printOptions();
		std::cout << "Your choice: ";
		std::cin >> scenario;
		if (scenario.compare("A1") == 0 || scenario.compare("a1") == 0) {
			entry = false;
			choice = A_CA;
		}
		else if (scenario.compare("A2") == 0 || scenario.compare("a2") == 0) {
			entry = false;
			choice = A_VCS;
		}
		else if (scenario.compare("B1") == 0 || scenario.compare("b1") == 0) {
			entry = false;
			choice = B_CA;
		}
		else if (scenario.compare("B2") == 0 || scenario.compare("b2") == 0) {
			entry = false;
			choice = B_VCS;
		}
		else {
			std::cout << "\nInvalid option, try again.\n\n";
		}
	}

    /*
	Poisson output done here, the / 2 part is just to make 
	the set smaller. Without it, it seems the largest time 
	is roughly 2x more than what is needed within sim time
	*/
	for (int i = 0; i < (SIM_TIME_slots / FRAME_slots / 2); i++) {
		double uniform_rand_A = 0.0, uniform_rand_C = 0.0;
		// Mod to get a value between 0-99, / to get a value between 0 to 1
		uniform_rand_A = (rand() % 100)/100.0;	
		uniform_rand_C = (rand() % 100)/100.0;	
		// Calculate the Poisson value, units are sec/frame
		uniform_rand_A = -(1 / lambda_A)*log(1 - uniform_rand_A);
		uniform_rand_C = -(1 / lambda_C)*log(1 - uniform_rand_C);
		// Convert to slots, sec/frame / slot duration = slot/frame, long integer round
		uniform_rand_A = lround(uniform_rand_A / (SLOT * pow(10, -6)));
		uniform_rand_C = lround(uniform_rand_C / (SLOT * pow(10, -6)));
		// Add to vectors
		a_poisson_set.push_back(uniform_rand_A);
		c_poisson_set.push_back(uniform_rand_C);
	}

	/*
	Convert Poisson set into an arrival set
	*/
	a_arrival.push_back(a_poisson_set[0]);
	c_arrival.push_back(c_poisson_set[0]);
	// Using just a_poisson_set.size() because both a and c should be the same size from the randomizer
	for (int i = 1; i < a_poisson_set.size(); i++) {
		int tempSum = 0;
		tempSum = a_arrival[i - 1] + a_poisson_set[i];
		a_arrival.push_back(tempSum);
		tempSum = c_arrival[i - 1] + c_poisson_set[i];
		c_arrival.push_back(tempSum);
	}
     

    double size;
	size = a_arrival.size() + c_arrival.size();
	a_back = backoffgen(0, CW_0);
	c_back = backoffgen(0, CW_0);
    a_curr = a_arrival.at(0);
	c_curr = c_arrival.at(0);
	int j = 0;
	int k = 0;

	int testVarOnlyWillRemoveWhenDone = 0;
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
						a_back = backoffgen(collisions, CW_0);
						c_back = backoffgen(collisions, CW_0);
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
						a_back = backoffgen(collisions, CW_0);
						c_back = backoffgen(collisions, CW_0);
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
					a_back = backoffgen(collisions, CW_0);
					c_back = backoffgen(collisions, CW_0);
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
		SIM_TIME_slots = 10000;
		std::cout << "Testing with sim_time_slot of: " << SIM_TIME_slots << std::endl;
		while (tot_slots < SIM_TIME_slots) {
			// Every new slot transmission will have at least DIFS, calling it running because current is being used.
			int running_slot = DIFS_slots;	
			int RTS_NAV = RTS_slots + SIFS_slots + CTS_slots + SIFS_slots + FRAME_slots + SIFS_slots + ACK_slots;

			/*
			If total slot is currently idle, and no queue, jump ahead.
			Ex1: tot_slots = 0, a_curr = 130 and c_curr = 180, this will jump to 130.
			Ex2: tot_slots = 512, a_curr = 921 and c_curr = 802, this will jump to 802.
			*/
			if (tot_slots < a_curr && tot_slots < c_curr) {
				(a_curr <= c_curr) ? (tot_slots = a_curr) : (tot_slots = c_curr);
			}

			
			if (a_curr == c_curr) {
				// If backoffs are the same, increment collision
				if (a_back == c_back) {
					collisions++;
				}
				// Generate new backoffs until no longer same
				while (a_back == c_back) {
					a_back = backoffgen(collisions, CW_0);
					c_back = backoffgen(collisions, CW_0);
				}
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
					a_back = backoffgen(collisions, CW_0);
				}
				else {
					running_slot += RTS_NAV;
					c_back = backoffgen(collisions, CW_0);
				}
			}
			else if (a_curr < c_curr) {
				// Move a_index to next arrival since we're processing this one
				a_index++;

				// If backoffs are the same, increment collision
				if (  (a_curr > tot_slots ? a_curr : tot_slots) + a_back + DIFS_slots >= c_curr + DIFS_slots ) {
					collisions++; 
					a_back = backoffgen(collisions, CW_0);
					c_back = backoffgen(collisions, CW_0);
				}

				// Decrement backoff until one is 0, adding time to tempSlot.
				while (a_back != 0) {
					a_back--;
					running_slot++;
				}
				// Check which one hit 0 first, update and generate new backoff
				running_slot += RTS_NAV;
				a_back = backoffgen(collisions, CW_0);
			}
			else {
				// Move a_index to next arrival since we're processing this one
				c_index++;

				// If backoffs are the same, increment collision
				if ((c_curr > tot_slots ? c_curr : tot_slots) + c_back + DIFS_slots >= a_curr + DIFS_slots) {
					collisions++;
					a_back = backoffgen(collisions, CW_0);
					c_back = backoffgen(collisions, CW_0);
				}

				// Decrement backoff until one is 0, adding time to tempSlot.
				while (c_back != 0) {
					c_back--;
					running_slot++;
				}
				// Check which one hit 0 first, update and generate new backoff
				running_slot += RTS_NAV;
				c_back = backoffgen(collisions, CW_0);
			}

			// Update tot_slot, a_curr, c_curr
			tot_slots += running_slot;	// Add the running slot to the total slow
			a_curr = a_arrival.at(a_index);
			c_curr = c_arrival.at(c_index);
			std::cout << testVarOnlyWillRemoveWhenDone++ << ") Current total slots:" << tot_slots << "\na_back val: " << a_back << "\tc_back val: " << c_back << "\tcurrent collision: " << collisions << std::endl;
		}
		break;
	case B_CA:	// Scenario B, CSMA 1
    
		break;
	case B_VCS:	// Scenario B, CSMA 2
    
		break;
	}
}

/*
SMA/CA with virtual carrier sensing enabled:
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