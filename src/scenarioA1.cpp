#include "simDefinitions.h"
#include "scenario.h"

void runScenarioA1(std::vector<int> a_arrival, std::vector<int> c_arrival) {
    int a_back = generate_backoff(0, CW_0);
    int c_back = generate_backoff(0, CW_0);
    int a_curr = a_arrival.at(0);
    int c_curr = c_arrival.at(0);
    int temp_collision = 0, tot_slots = 0, a_index = 0, c_index = 0, a_success = 0, c_success = 0, collisions = 0, total_iterations = 0;
    while ( tot_slots < SIM_TIME_slots ) {
        // Every new slot transmission will have at least DIFS, calling it running because current is being used.
        int running_slot = 0;
        
        /*
         If total slot is currently idle, and no queue, jump ahead.
         Ex1: tot_slots = 0, a_curr = 130 and c_curr = 180, this will jump to 130.
         Ex2: tot_slots = 512, a_curr = 921 and c_curr = 802, this will jump to 802.
         */
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
}
