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
    int lambda_A = 300;	// set of 50, 100, 200, 300 frame/sec
    int lambda_C = 300;	// set of 50, 100, 200, 300 frame/sec
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
    
    int j = 0;
    int k = 0;
    int diff = 0;
    int old_slot = 0;
    
    // Increment every loop to see how many cycles it needed to get to SIM_TIME_slots
    int total_iterations = 0;
    switch (choice) {
        case A_CA:	{// Scenario A, CSMA
            
            a_back = generate_backoff(0, CW_0);
            c_back = generate_backoff(0, CW_0);
            int temp_collision = 0;
            while ( tot_slots < SIM_TIME_slots ) {
                
                if (j != 0 && k != 0 && old_slot == tot_slots) {
                    
                }
                old_slot = tot_slots;
                a_curr = a_arrival.at(j);
                c_curr = c_arrival.at(k);
                
                // When current total slots is behind both A and C arrivals
                if ( tot_slots < a_curr && tot_slots < c_curr ) {
                    // A arrival is the same as C arrival, update total slots to arbitrary arrival A
                    if ( a_curr == c_curr ) {
                        tot_slots = a_curr;
                    }
                    // A arrival is earlier than C, set total slots to A arrival.
                    else if ( a_curr < c_curr ) {
                        tot_slots = a_curr;
                    }
                    // C arrival is earlier than A, set total slots to C arrival.
                    else {
                        tot_slots = c_curr;
                    }
                }
                // When current total slots is ahead both A and C arrivals
                else if ( tot_slots >= a_curr && tot_slots >= c_curr ) {
                    
                    tot_slots += DIFS_slots;
                    
                    // Collision will occur when both backoffs are the same.
                    if ( a_back == c_back ) {
                        tot_slots += a_back + FRAME_slots + SIFS_slots + ACK_slots;
                        temp_collision++;
                        collisions++;
                        a_back = generate_backoff(temp_collision, CW_0);
                        c_back = generate_backoff(temp_collision, CW_0);
                    }
                    // A transmits before C
                    else if ( a_back < c_back ) {
                        tot_slots += a_back + FRAME_slots + SIFS_slots + ACK_slots;
                        
                        a_success++;
                        temp_collision = 0;
                        j++;
                        c_back -= a_back;
                        a_back = generate_backoff(0, CW_0);
                        c_back = generate_backoff(0, CW_0);
                    }
                    // C transmits before A
                    else {
                        tot_slots += c_back + FRAME_slots + SIFS_slots + ACK_slots;
                        
                        c_success++;
                        temp_collision = 0;
                        k++;
                        a_back -= c_back;
                        c_back = generate_backoff(0, CW_0);
                        a_back = generate_backoff(0, CW_0);
                    }
                }
                // Given previous if statements, current total slot will be ahead of C arrival, but behind A
                else if ( tot_slots >= c_curr ) {
                    // C possibly transmits during A's backoff
                    if ( tot_slots + c_back >= a_curr ) {
                        // A and C backoff ends at same slot
                        if ( a_curr + DIFS_slots + a_back == tot_slots + DIFS_slots + c_back ) {
                            tot_slots += DIFS_slots + c_back + FRAME_slots + SIFS_slots + ACK_slots;
                            temp_collision++;
                            collisions++;
                            a_back = generate_backoff(temp_collision, CW_0);
                            c_back = generate_backoff(temp_collision, CW_0);
                        }
                        // A's backoff ends later than C's backoff, A's backoff freezes
                        else if ( a_curr + DIFS_slots + a_back > tot_slots + DIFS_slots + c_back ) {
                            tot_slots += DIFS_slots + c_back + FRAME_slots + SIFS_slots + ACK_slots;
                            
                            c_success++;
                            k++;
                            temp_collision = 0;
                            a_back = ( a_curr + DIFS_slots + a_back ) - ( tot_slots + DIFS_slots + c_back );
                            c_back = generate_backoff(0, CW_0);
                        }
                        // C's backoff ends later than A's backoff, C's backoff freezes
                        else {
                            tot_slots = a_curr + DIFS_slots + a_back + FRAME_slots + SIFS_slots + ACK_slots;
                            
                            a_success++;
                            j++;
                            temp_collision = 0;
                            c_back = ( tot_slots + DIFS_slots + c_back ) - ( a_curr + DIFS_slots + a_back );
                            a_back = generate_backoff(0, CW_0);
                        }
                    }
                    // No possible transmission before A's backoff or collision, C transmits successfully
                    else {
                        tot_slots += DIFS_slots + c_back + FRAME_slots + SIFS_slots + ACK_slots;
                        
                        c_success++;
                        k++;
                        temp_collision = 0;
                        c_back = generate_backoff(0, CW_0);
                    }
                }
                // Based off prior statements, current total slot will be ahead of A arrival, but behind C
                else if ( tot_slots >= a_curr ) {
                    // A possibly transmits during C's backoff
                    if ( tot_slots + c_back >= a_curr ) {
                        // A and C backoff ends at same slot
                        if ( c_curr + DIFS_slots + c_back == tot_slots + DIFS_slots + a_back ) {
                            tot_slots += DIFS_slots + a_back + FRAME_slots + SIFS_slots + ACK_slots;
                            temp_collision++;
                            collisions++;
                            a_back = generate_backoff(temp_collision, CW_0);
                            c_back = generate_backoff(temp_collision, CW_0);
                        }
                        // C's backoff ends later than A's backoff, C's backoff freezes
                        else if ( c_curr + DIFS_slots + c_back > tot_slots + DIFS_slots + a_back ) {
                            tot_slots += DIFS_slots + a_back + FRAME_slots + SIFS_slots + ACK_slots;
                            
                            a_success++;
                            j++;
                            temp_collision = 0;
                            c_back = ( c_curr + DIFS_slots + c_back ) - ( tot_slots + DIFS_slots + a_back );
                            a_back = generate_backoff(0, CW_0);
                        }
                        // A's backoff ends later than C's backoff, A's backoff freezes
                        else {
                            tot_slots = c_curr + DIFS_slots + c_back + FRAME_slots + SIFS_slots + ACK_slots;
                            
                            c_success++;
                            k++;
                            temp_collision = 0;
                            a_back = ( tot_slots + DIFS_slots + a_back ) - ( c_curr + DIFS_slots + c_back );
                            c_back = generate_backoff(0, CW_0);
                        }
                    }
                    // No possible transmission before C's backoff or collision, A transmits successfully
                    else {
                        tot_slots += DIFS_slots + a_back + FRAME_slots + SIFS_slots + ACK_slots;
                        
                        a_success++;
                        j++;
                        temp_collision = 0;
                        a_back = generate_backoff(0, CW_0);
                    }
                }
                total_iterations++;
                std::cout << total_iterations << ") Current time: " << tot_slots << std::endl;
                std::cout << "Current collisions: " << collisions << std::endl << std::endl;
            }
            std::cout << "\n\nTotal iterations to complete simulation time: " << total_iterations << std::endl;
            std::cout << "Total number of collisions: " << collisions << "\tSuccessful A nodes: " << a_success << "\tSuccessful C nodes: " << c_success << std::endl;
            
            //runScenarioA1(a_arrival, c_arrival);
            break;
        }
        case A_VCS:	{// Scenario A, CSMA 2
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
        }
        case B_CA:	{// Scenario B, CSMA 1
            runScenarioB1(a_arrival, c_arrival);
            system("pause");
            break;
        }
        case B_VCS:	{// Scenario B, CSMA 2
			lambda_A = 100;
			lambda_C = 100;
			for (int itr = 0; itr < 20; itr++) {
				a_poisson_set = generate_poisson(lambda_A);
				c_poisson_set = generate_poisson(lambda_C);
				a_arrival = generate_arrival(a_poisson_set);
				c_arrival = generate_arrival(c_poisson_set);
				runScenarioB2(a_arrival, c_arrival);
			}
            break;
        }
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
