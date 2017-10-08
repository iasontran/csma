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

int backoffgen(int collide, double CW) {
    int bound = (pow(2, collide) * CW) - 1;
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
    double A_SLOTS = 0.0;
    double C_SLOTS = 0.0;

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
    double a_collisions = 0;
    double c_collisions = 0;
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
	
	size = a_arrival.size() + c_arrival.size();
	a_back = backoffgen(0, CW_0);
	c_back = backoffgen(0, CW_0);
    a_curr = a_arrival.at(0);
	c_curr = c_arrival.at(0);
	int j = 0;
	int k = 0;

    // Scenario A, CSMA 1
	while ( tot_slots < SIM_TIME_slots ) {   // while total time is less than simulated time
        // fetch arrival time
        a_curr = a_arrival.at(j);
        c_curr = c_arrival.at(k);
        
        // navigate through a's arrival set if available

        // check if current A arrival is earlier than C arrival
        if ( a_curr < c_curr ) {
            if ( tot_slots < a_curr ) { // if current total slot is less than current A arrival time
                tot_slots = a_curr; // set current total slots to A arrival time
            }
            
            // check if C arrival will transmit during A's backoff
            if ( ( tot_slots + DIFS_slots + a_back ) > c_curr ) {
                // collision case when backoffs end at same time
                if ( ( c_curr + DIFS_slots + c_back ) == ( tot_slots + DIFS_slots + a_back ) ) {
                    tot_slots += DIFS_slots + a_back + FRAME_slots + SIFS_slots + ACK_slots;    // unsure if exactly right, but should be where ACK is supposed to send
                    collisions++;
                    a_back = backoffgen(collisions, CW0);
                    c_back = backoffgen(collisions, CW0);
                    continue;
                } else {    // no collision, but check earlier packet
                    // if A transmits earlier than C
                    if ( (( tot_slots + DIFS_slots + a_back ) - ( c_curr + DIFS_slots + c_back )) < 0 ) {   // should be negative if A transmits earlier than C
                        c_back = (c_curr + DIFS_slots + c_back) - (tot_slots + DIFS_slots + a_back);    // calculate new backoff for C
                        tot_slots += DIFS_slots + a_back + FRAME_slots + SIFS_slots + ACK_slots;    // move current total slots to ACK slot
                        a_success++;    // increment for successful A transmission
                    } else {    // C transmits earlier than A
                        a_back = (tot_slots + DIFS_slots + a_back) - (c_curr + DIFS_slots + c_back);    // calculate new backoff for A
                        tot_slots = c_curr + DIFS_slots + c_back + FRAME_slots + SIFS_slots + ACK_slots;    // move current total slots to C's ACK slot
                        c_success++;    // increment for successful C transmission
                    }
                }
            } else {    // when A successfully transmits
                tot_slots += DIFS_slots + a_back + FRAME_slots + SIFS_slots + ACK_slots;    // add DIFS, backoff, frame, SIFS, and ack to total slots
                a_success++;    // increment successful A transmission
                collisions = 0; // reset collision count (not sure if necessary) **
                j++;    // increment to next A arrival slot
            }
        } else if ( c_curr < a_curr ) { // current C arrival is earlier than A arrival
            if ( tot_slots < c_curr ) { // if total slots is less than current C arrival time
                tot_slots = c_curr;
            }
            
            // check if A arrival will transmit during C's backoff
            if ( ( tot_slots + DIFS_slots + c_back ) > a_curr ) {
                // collision case when backoffs end at same time
                if ( ( a_curr + DIFS_slots + a_back ) == ( tot_slots + DIFS_slots + c_back ) ) {
                    tot_slots += DIFS_slots + c_back + FRAME_slots + SIFS_slots + ACK_slots;    // unsure if exactly right, but should be where ACK is supposed to send
                    collisions++;
                    a_back = backoffgen(collisions, CW0);
                    c_back = backoffgen(collisions, CW0);
                    continue;
                } else {    // no collision, but check earlier packet
                    // if C transmits earlier than A
                    if ( (( tot_slots + DIFS_slots + c_back ) - ( a_curr + DIFS_slots + a_back )) < 0 ) {   // should be negative if C transmits earlier than A
                        a_back = (a_curr + DIFS_slots + a_back) - (tot_slots + DIFS_slots + c_back);    // calculate new backoff for C
                        tot_slots += DIFS_slots + c_back + FRAME_slots + SIFS_slots + ACK_slots;    // move current total slots to ACK slot
                        c_success++;    // increment for successful A transmission
                    } else {    // A transmits earlier than C
                        c_back = (c_curr + DIFS_slots + c_back) - (tot_slots + DIFS_slots + a_back);    // calculate new backoff for C
                        tot_slots = a_curr + DIFS_slots + a_back + FRAME_slots + SIFS_slots + ACK_slots;    // move current total slots to A's ACK slot
                        a_success++;    // increment for successful C transmission
                    }
                }
            } else {    // when C successfully transmits
                tot_slots += DIFS_slots + c_back + FRAME_slots + SIFS_slots + ACK_slots;    // add DIFS, backoff, frame, SIFS, and ack to total slots
                c_success++;    // increment successful A transmission
                collisions = 0; // reset collision count (not sure if necessary) **
                k++;    // increment to next A arrival slot
            }
        } else {    // A and C arrivals are tied
            // assign either current arrival as current slot
            tot_slots = a_curr;
            if ( ( a_curr + DIFS_slots + a_back ) == ( c_curr + DIFS_slots + c_back ) ) {   // when backoffs end at same time, collision will occur
                tot_slots += DIFS_slots + a_back + FRAME_slots + SIFS_slots + ACK_slots;    // use either backoff to move current slot to ACK slot
                collisions++;
                a_back = backoffgen(collisions, CW0);
                c_back = backoffgen(collisions, CW0);
                continue;
            } else {    // determine which set has earlier backoff time
                // if A transmits earlier than C
                if ( (( tot_slots + DIFS_slots + a_back ) - ( c_curr + DIFS_slots + c_back )) < 0 ) {   // should be negative if A transmits earlier than C
                    c_back = (c_curr + DIFS_slots + c_back) - (tot_slots + DIFS_slots + a_back);    // calculate new backoff for C
                    tot_slots += DIFS_slots + a_back + FRAME_slots + SIFS_slots + ACK_slots;    // move current total slots to ACK slot
                    a_success++;    // increment for successful A transmission
                } else {    // C transmits earlier than A
                    a_back = (tot_slots + DIFS_slots + a_back) - (c_curr + DIFS_slots + c_back);    // calculate new backoff for A
                    tot_slots = c_curr + DIFS_slots + c_back + FRAME_slots + SIFS_slots + ACK_slots;    // move current total slots to C's ACK slot
                    c_success++;    // increment for successful C transmission
                }
            }
        }
    }
    
    // Scenario B, CSMA 1
    
}
