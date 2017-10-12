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
    int lambda_A = 100;	// set of 50, 100, 200, 300 frame/sec
    int lambda_C = 100;	// set of 50, 100, 200, 300 frame/sec
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
            a_curr = a_arrival.at(0);
            c_curr = c_arrival.at(0);
            int temp_collision = 0;
            while ( tot_slots < SIM_TIME_slots ) {
                // Every new slot transmission will have at least DIFS, calling it running because current is being used.
                int running_slot = 0;
                
             
                 // If total slot is currently idle, and no queue, jump ahead.
                 // Ex1: tot_slots = 0, a_curr = 130 and c_curr = 180, this will jump to 130.
                 // Ex2: tot_slots = 512, a_curr = 921 and c_curr = 802, this will jump to 802.
             
                if (tot_slots < a_curr && tot_slots < c_curr) {
                    (a_curr <= c_curr) ? (tot_slots = a_curr) : (tot_slots = c_curr);
                }
                
                // In the event the current time is ahead of the a_arrival, we just update a_curr
                a_curr = (a_arrival.at(a_index) > tot_slots) ? a_arrival.at(a_index) : tot_slots;
                c_curr = (c_arrival.at(c_index) > tot_slots) ? c_arrival.at(c_index) : tot_slots;
                
                // Event when both arrivals are the same
                if (a_arrival.at(a_index) == c_arrival.at(c_index)) {
                    // When both backoffs are the same, the RTS collides, so collision incremented, generate new backoff
                    if (a_back == c_back) {
                        temp_collision++;
                        collisions++;
                        running_slot += DIFS_slots + a_back + FRAME_slots + SIFS_slots + ACK_slots;
                        a_back = generate_backoff(temp_collision, CW_0);
                        c_back = generate_backoff(temp_collision, CW_0);
                        std::cout << "!!!!COLLISION!!!!" << std::endl;
                    }
                    else {
                        int low_back, high_back;
                        // Whichever backoff is less, assign it as low. (Avoids re-writing code again for a_back < c_back and c_back < a_back
                        if (a_back < c_back) {
                            low_back = a_back;
                            high_back = c_back;
                        }
                        else {
                            low_back = c_back;
                            high_back = a_back;
                        }
                        // First station starts transmitting, but second station starts transmitting before first finishes.
                        // Case: c_back < a_back && a_back < c_back + DIFS_slots
                        // Case: a_back < c_back && c_back < a_back + DIFS_slots
                        if ( (low_back < high_back) && (high_back < (DIFS_slots + low_back)) ) {
                            temp_collision++;
                            collisions++;
                            running_slot += high_back + DIFS_slots;
                            a_back = generate_backoff(temp_collision, CW_0);
                            c_back = generate_backoff(temp_collision, CW_0);
                            std::cout << "!!!!COLLISION!!!!" << std::endl;
                        }
                        else {
                            // Check which one hit 0 first, update and generate new backoff
                            if (a_back < c_back) {
                                c_back -= a_back;
                                a_back = generate_backoff(0, CW_0);
                                a_index++;
                                a_success++;
                            }
                            else {
                                a_back -= c_back;
                                c_back = generate_backoff(0, CW_0);
                                c_index++;
                                c_success++;
                            }
                            temp_collision = 0;
                            running_slot += DIFS_slots + low_back + FRAME_slots + SIFS_slots + ACK_slots;
                        }
                    }
                }
                else if (a_arrival.at(a_index) < c_arrival.at(c_index)) {
                    // Case: a starts transmitting before c but both end backoff at same time
                    if ( c_curr + c_back == a_curr + a_back ) {
                        temp_collision++;
                        collisions++;
                        running_slot += DIFS_slots + a_back + FRAME_slots + SIFS_slots + ACK_slots;
                        a_back = generate_backoff(temp_collision, CW_0);
                        c_back = generate_backoff(temp_collision, CW_0);
                        std::cout << "!!!!COLLISION!!!!" << std::endl;
                    }
                    // Case: a starts transmitting, c finishes transmitting before a
                    else if ( c_curr + c_back < a_curr + a_back) {
                        running_slot += DIFS_slots + c_back + FRAME_slots + SIFS_slots + ACK_slots;
                        a_back = (a_curr + a_back) - (c_curr + c_back);
                        c_back = generate_backoff(0, CW_0);
                        temp_collision = 0;
                        c_index++;
                        c_success++;
                        if (a_back < 0) {
                            // should not be less than 0, for debug only
                        }
                    }
                    // Case: a starts transmitting and finishes before c starts
                    else {
                        temp_collision = 0;
                        running_slot += DIFS_slots + a_back + FRAME_slots + SIFS_slots + ACK_slots;
                        c_back = (c_curr + c_back) - (a_curr + a_back);
                        a_back = generate_backoff(0, CW_0);
                        a_index++;
                        a_success++;
                        if (c_back < 0) {
                            // debug only
                        }
                    }
                }
                else {
                    // Case: c starts transmitting before a but both end backoff at same time
                    if ( a_curr + a_back == c_curr + c_back ) {
                        temp_collision++;
                        collisions++;
                        running_slot += DIFS_slots + c_back + FRAME_slots + SIFS_slots + ACK_slots;
                        a_back = generate_backoff(temp_collision, CW_0);
                        c_back = generate_backoff(temp_collision, CW_0);
                        std::cout << "!!!!COLLISION!!!!" << std::endl;
                    }
                    // Case: c starts transmitting, a starts transmitting before c finishes
                    if (a_curr + a_back < c_curr + c_back) {
                        running_slot += DIFS_slots + a_back + FRAME_slots + SIFS_slots + ACK_slots;
                        a_back = generate_backoff(0, CW_0);
                        c_back = (c_curr + c_back) - (a_curr + a_back);
                        temp_collision = 0;
                        a_index++;
                        a_success++;
                        if (c_back < 0) {
                            // debug
                        }
                    }
                    // Case: c starts transmitting and finishes before a starts
                    else {
                        temp_collision = 0;
                        running_slot += DIFS_slots + c_back + FRAME_slots + SIFS_slots + ACK_slots;
                        c_back = generate_backoff(0, CW_0);
                        a_back = (a_curr + a_back) - (c_curr + c_back);
                        c_index++;
                        c_success++;
                        if (a_back < 0) {
                            // debug
                        }
                    }
                }
                
                // Update tot_slot, a_curr, c_curr
                tot_slots += running_slot;	// Add the running slot to the total slow
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
            runScenarioB2(a_arrival, c_arrival);
            system("pause");
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
