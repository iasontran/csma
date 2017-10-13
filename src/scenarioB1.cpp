#include "simDefinitions.h"
#include "scenario.h"

void runScenarioB1(std::vector<int> a_arrival, std::vector<int> c_arrival) {
    int a_back = generate_backoff(0, CW_0);
    int c_back = generate_backoff(0, CW_0);
    int a_curr = a_arrival.at(0);
    int c_curr = c_arrival.at(0);
    int temp_collision = 0, tot_slots = 0, j = 0, k = 0, a_success = 0, c_success = 0, collisions = 0, total_iterations = 0, diff = 0, a_slots = 0, c_slots = 0;
    
    while ( tot_slots < SIM_TIME_slots ) {
        
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
            
            diff = ( c_curr + c_back ) - ( a_curr + a_back );
            
            if ( diff < 0 ) {
                
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
}
