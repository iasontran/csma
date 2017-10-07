#define _CRT_SECURE_NO_WARNINGS 
/*
*
*/

#include <iostream>
#include <string>
#include <cmath>
#include <ctime>
#include <cstdio>
#include <vector>
#include <cstdlib>

// Simulation Parameters
#define SIM_TIME 10 // in seconds
#define SLOT 20 // in microseconds
#define DIFS 5 // in microseconds 40 originally
#define SIFS 10 // in microseconds
#define CW_MAX 1024 // in slots
#define CW_0 4 // in slots
#define FRAME 1500 // in bytes
#define ACK 30 // in bytes
#define RTS 30 // in bytes
#define CTS 30 // in bytes
#define TRANSMISSION_RATE 6 // in Mbps

class Station {
private:
	std::string id;
	std::string data;
	std::string toStation;
public:

};

int backoffgen(int collide, double CW) {
    int bound = ((2 ^ collide) * CW) - 1;
    int output = rand() % bound;
    return output;
}

int main(int argc, char *argv[]) {

	// Set the time seed
	srand((int)time(0));

	// Simulation Variables in Slots
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

	// Variables for A and C nodes
	std::vector<double> a_arrival, c_arrival;	//std::vector someVal, someVal.push_back(val) << adds to end of list
	std::vector <double> a_poisson_set, c_poisson_set;	// The set of randomized values in slots
	
	// Variables for scenarios??
	std::string scenario;
	std::string metric;
	int a_back;
	int c_back;
	double a_curr;
	double c_curr;

	// Variables for the process
	bool active_node = false;
	bool next_active = false;
	bool a_done = false;
	bool c_done = false;
	bool entry = true;
	double a_success = 0;
	double c_success = 0;
	double collisions = 0;
	double CW0 = 4; // in slots

	// Converting all units into SLOT time, so that we can increment by doing tot_slot = tot_slot + sifs_slot + difs_slot etc
	
    SIM_TIME_slots = lround(SIM_TIME / (SLOT * pow(10,-6)));
	DIFS_slots = ceil((double)DIFS / SLOT);
	SIFS_slots = ceil((double)SIFS / SLOT);
	FRAME_slots = lround((FRAME * 8.0) / (TRANSMISSION_RATE * pow(10, 6)) / (SLOT * pow(10, -6)));
	ACK_slots = lround(ACK * 8 / (TRANSMISSION_RATE * pow(10, 6)) / (SLOT * pow(10, -6)));
	RTS_slots = lround(RTS * 8 / (TRANSMISSION_RATE * pow(10, 6)) / (SLOT * pow(10, -6)));
	CTS_slots = lround(CTS * 8 / (TRANSMISSION_RATE * pow(10, 6)) / (SLOT * pow(10, -6)));
	
    
    /*
	// Selection of scenario
	while (entry) {
		printf("Select ""Concurrent"" Communications or ""Hidden"" Terminals: ");
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
		printf("Choose CSMA type (CA or CA/VC): ");
		std::cin >> metric;
		if (metric.compare("CA") == 0) {
			entry = false;
		}
		else if (metric.compare("CA/VC") == 0) {
			entry = false;
		}
		else {
			printf("Please pick either \"CA\" or \"CA/VC\".");
			continue;
		}
	}
	*/
    
    
	// Poisson output done here, the / 2 part is just to make the set smaller. Without it, it seems the largest time is like 2x more than what is needed within sim time
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

	// Convert Poisson set into an arrival set
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
    
    // sample sets
    a_arrival.push_back(100);
    // 5 + backoff_a + 100 + 1500 + 10 + 30 = 1650
    c_arrival.push_back(130);
    // 5 + backoff_a  + 1500 + 10 + 30 = 3198
	
	size = a_arrival.size() + c_arrival.size();
	a_back = backoffgen(0, CW_0);
	c_back = backoffgen(0, CW_0);
    a_curr = a_arrival.at(0);
	c_curr = c_arrival.at(0);
	int j = 0;
	int k = 0;

	while (!(a_done && c_done)) {   // while a and c arrival times still exist
		for (int i = 0; i < size; i++) {    // iterate through size of both lists combined
			if (i == 0) {     // condition for beginning of iterations
				if (a_curr < c_curr) {    // if a arrival is smaller than c, begin transmitting a
					tot_slots = tot_slots + a_curr + DIFS + a_back;     // add a arrival, DIFS, and a backoff to total slots
                    if (j + 1 == a_arrival.size()) {
                        a_done = true;
                    }
                    else {
                        j++;
                        a_curr = a_arrival.at(j);       // update current a arrival early for next iteration
                    }
				}
				else if (a_curr > c_curr) {       // if c arrival is smaller than a, begin transmitting c
					active_node = true;     // flag to check which node is active for next if statements
					tot_slots = tot_slots + c_curr + DIFS + c_back;     // add c arrival, DIFS, and c backoff to total slots
                    if (k + 1 == c_arrival.size()) {
                        c_done = true;
                    }
                    else {
                        k++;
                        c_curr = c_arrival.at(k);   // update current c arrival early for next iteration
                    }
				}
				else {  // collision?

				}

				if ((active_node == false) && (c_curr < tot_slots)) {     // if a is active and c's arrival is less than current slots with a arrival, DIFS, and backoff added
					j--;
					a_curr = a_arrival.at(j);
					collisions++;
					a_back = backoffgen(collisions, CW0);
					c_back = backoffgen(collisions, CW0);
					i--;
					continue;
					// collision?
				}
				else if ((active_node == true) && (a_curr < tot_slots)) {     // if c is active and a's arrival is less than current slots with c arrival, DIFS, and backoff added
					k--;
					c_curr = c_arrival.at(k);
					collisions++;
					a_back = backoffgen(collisions, CW0);
					c_back = backoffgen(collisions, CW0);
					i--;
					continue;
					// collision?
				}
				else {      // if no possible collision, add frame data, sifs, and ack time to total slots
					tot_slots = tot_slots + FRAME + SIFS + ACK;
					if (active_node == false) {   // if a successfully sent frame, increment a success counter
						a_success++;
					}
					else {  // if c successfully sent frame, increment c success counter
						c_success++;
					}
				}
			}
			else {  // for iterations after first
                if (a_done) {
                    active_node = true;
                    tot_slots = tot_slots + DIFS + c_back;
                    if (k + 1 == c_arrival.size()) {  // if not at end of arrival list, set next arrival
                        c_done = true;
                    }
                    else {
                        k++;
                        c_curr = c_arrival.at(k);
                    }
                }
                else if (c_done) {
                    active_node = false;
                    tot_slots = tot_slots + DIFS + a_back;
                    if (j + 1 == a_arrival.size()) {  // if not at end of a arrival list, set next arrival
                        a_done = true;
                    }
                    else {
                        j++;
                        a_curr = a_arrival.at(j);
                    }
                }
				else if (a_curr < tot_slots) {    // if a arrival is less than current slot total, begin adding to total slots
                    active_node = false;
                    tot_slots = tot_slots + DIFS + a_back;
					if (j + 1 == a_arrival.size()) {  // if not at end of a arrival list, set next arrival
                        a_done = true;
					}
                    else {
                        j++;
                        a_curr = a_arrival.at(j);
                    }
				}
				else if (c_curr < tot_slots) {    // if c arrival is less than current slot total, begin adding to total slots
					active_node = true;
					tot_slots = tot_slots + DIFS + a_back;
					if (k + 1 == c_arrival.size()) {  // if not at end of arrival list, set next arrival
                        c_done = true;
					}
                    else {
                        k++;
                        c_curr = c_arrival.at(k);
                    }
				}
				else {
					// collision?
				}

				if ((active_node == false) && (c_curr < tot_slots) && !a_done) {     // if a is active and c's arrival is less than current slots with a arrival, DIFS, and backoff added
					j--;
					a_curr = a_arrival.at(j);
					collisions++;
					a_back = backoffgen(collisions, CW0);
					c_back = backoffgen(collisions, CW0);
					i--;
					continue;
				}
				else if ((active_node == true) && (a_curr < tot_slots) && !c_done) {     // if c is active and a's arrival is less than current slots with c arrival, DIFS, and backoff added
					k--;
					c_curr = c_arrival.at(k);
					collisions++;
					a_back = backoffgen(collisions, CW0);
					c_back = backoffgen(collisions, CW0);
					i--;
					continue;
				}
				else {      // if no possible collision, add frame data, sifs, and ack time to total slots
					tot_slots = tot_slots + FRAME + SIFS + ACK;
					if (active_node == false) {   // if a successfully sent frame, increment a success counter
						a_success++;
					}
					else {  // if c successfully sent frame, increment c success counter
						c_success++;
					}
				}
			}
		}
	}
}
