#define _CRT_SECURE_NO_WARNINGS 
/*
*
*/

#include <iostream>
#include <string>
#include <cmath>
#include <cstdio>
#include <vector>
#include <cstdlib>

// Simulation Parameters
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

class Station {
private:
	std::string id;
	std::string data;
	std::string toStation;
public:

};

int main(int argc, char *argv[]) {
	std::string scenario;
	std::string metric;
	std::vector<double> a_arrival;
	std::vector<int> a_back;
	std::vector<double> c_arrival;
	std::vector<int> c_back;
	Station a_curr;
	Station c_curr;

	// Simulation Variables in Slots
	double tot_slots = 0.0;
	double SIM_TIME_slots = 0.0;
	double DIFS_slots = 0.0;
	double SIFS_slots = 0.0;
	double FRAME_slots = 0.0;
	double ACK_slots = 0.0;
	double RTS_slots = 0.0;
	double CTS_slots = 0.0;

	// Variables for the process
	bool active_node = false;
	bool next_active = false;
	bool a_done = false;
	bool b_done = false;
	bool entry = true;
	double a_success;
	double c_success;
	double collisions;
	double CW0 = 4; // in slots

	// Converting all units into SLOT time, so that we can increment by doing tot_slot = tot_slot + sifs_slot + difs_slot etc
	SIM_TIME_slots = lround(SIM_TIME / (SLOT * pow(10,-6)));
	DIFS_slots = ceil((double)DIFS / SLOT);
	SIFS_slots = ceil((double)SIFS / SLOT);
	FRAME_slots = lround(FRAME * 8 / (TRANSMISSION_RATE * (10 ^ 6)) / (SLOT * pow(10, -6)));
	ACK_slots = lround(ACK * 8 / (TRANSMISSION_RATE * pow(10, 6)) / (SLOT * pow(10, -6)));
	RTS_slots = lround(RTS * 8 / (TRANSMISSION_RATE * pow(10, 6)) / (SLOT * pow(10, -6)));
	CTS_slots = lround(CTS * 8 / (TRANSMISSION_RATE * pow(10, 6)) / (SLOT * pow(10, -6)));

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



	// Poisson output done here


	/*
	size = a_arrival.size() + c_arrival.size();
	a_back = backoffgen(0, CW_0);
	c_back = backoffgen(0, CW_0);
	a_curr = a_curr.at(0);
	c_curr = c_curr.at(0);
	int j = 0;
	int k = 0;

	while (!(a_done && b_done)) {   // while a and c arrival times still exist
		for (i = 0; i < size - 1; i++) {    // iterate through size of both lists combined
			if (i == 0) {     // condition for beginning of iterations
				if (a_curr < c_curr) {    // if a arrival is smaller than c, begin transmitting a
					tot_slots = tot_slots + a_curr + DIFS + a_back;     // add a arrival, DIFS, and a backoff to total slots
					j++;
					a_curr = a_arrival.at(j);       // update current a arrival early for next iteration
				}
				else if (a_curr > c_curr) {       // if c arrival is smaller than a, begin transmitting c
					active_node = true;     // flag to check which node is active for next if statements
					tot_slots = tot_slots + c_curr + DIFS + c_back;     // add c arrival, DIFS, and c backoff to total slots
					k++;
					c_curr = c_arrival.at(k);   // update current c arrival early for next iteration
				}
				else {  // collision?

				}

				if ((active_node == false) && (c_curr < tot_slots)) {     // if a is active and c's arrival is less than current slots with a arrival, DIFS, and backoff added
					j--;
					a_curr = a_arrival.at(j);
					collision++;
					a_back = backoffgen(collision, CW0);
					c_back = backoffgen(collision, CW0);
					i--;
					continue;
					// collision?
				}
				else if ((active_node == true) && (a_curr < tot_slots)) {     // if c is active and a's arrival is less than current slots with c arrival, DIFS, and backoff added
					k--;
					c_curr = c_arrival.at(k);
					collision++;
					a_back = backoffgen(collision, CW0);
					c_back = backoffgen(collision, CW0);
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
				if (a_curr < c_curr) {    // if a arrival is less than c arrival, begin adding to total slots
					tot_slots = tot_slots + a_curr + DIFS + a_back;
					if (j + 1 != a_arrival.size()) {  // if not at end of a arrival list, set next arrival
						j++;
						a_curr = a_arrival.at(j);
					}
				}
				else if (a_curr > c_curr) {    // if c arrival is less than a arrival, begin adding to total slots
					active_node = true;
					tot_slots = tot_slots + a_curr + DIFS + a_back;
					if (k + 1 != c_arrival.size()) {  // if not at end of arrival list, set next arrival
						k++;
						c_curr = c_arrival.at(k);
					}
				}
				else {
					// collision?
				}

				if ((active_node == false) && (c_curr < tot_slots)) {     // if a is active and c's arrival is less than current slots with a arrival, DIFS, and backoff added
					j--;
					a_curr = a_arrival.at(j);
					collision++;
					a_back = backoffgen(collision, CW0);
					c_back = backoffgen(collision, CW0);
					i--;
					continue;
				}
				else if ((active_node == true) && (a_curr < tot_slots)) {     // if c is active and a's arrival is less than current slots with c arrival, DIFS, and backoff added
					k--;
					c_curr = c_arrival.at(k);
					collision++;
					a_back = backoffgen(collision, CW0);
					c_back = backoffgen(collision, CW0);
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
	*/
}

int backoffgen(int collide, double CW) {
	int bound = ((2 ^ collide) * CW) - 1;
	int output = rand() % bound;
	return output;
}
