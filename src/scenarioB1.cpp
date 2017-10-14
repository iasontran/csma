#include "simDefinitions.h"
#include "scenario.h"

void runScenarioB1(std::vector<int> a_arrival, std::vector<int> c_arrival) {
    int a_back = generate_backoff(0, CW_0);
    int c_back = generate_backoff(0, CW_0);
    int a_curr = a_arrival.at(0);
    int c_curr = c_arrival.at(0);
    int temp_collision = 0, tot_slots = 0, j = 0, k = 0, a_success = 0, c_success = 0, collisions = 0, total_iterations = 0, a_slots = 0, c_slots = 0;
    bool send_c = false, send_a = false;
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
            
            // C transmits before A successfully transmits
            if ( c_curr + c_back <= a_curr + a_back ) {
                
                if ( send_c == false ) {
                    tot_slots += c_back + DIFS_slots + FRAME_slots + SIFS_slots + ACK_slots;
                    
                    c_success++;
                    temp_collision = 0;
                    k++;
                }
                
                else {
                    tot_slots += c_back + DIFS_slots + FRAME_slots + SIFS_slots + ACK_slots;
                    send_c = false;
                }
            }
            // A transmits before C successfully
            else if ( a_curr + a_back <= c_curr + c_back ) {
                
                if ( send_a == false ) {
                    tot_slots += a_back + DIFS_slots + FRAME_slots + SIFS_slots + ACK_slots;
                    
                    a_success++;
                    temp_collision = 0;
                    j++;
                }
                
                else {
                    tot_slots += a_back + DIFS_slots + FRAME_slots + SIFS_slots + ACK_slots;
                    send_a = false;
                }
            }
            // A and C transmit at the same time
            else {
                temp_collision++;
                collisions++;
                
                if ( c_curr + c_back < a_curr + a_back ) {
                    tot_slots += c_back + DIFS_slots + FRAME_slots + SIFS_slots + ACK_slots;
                    c_back = generate_backoff(temp_collision, CW_0);
                    send_c = false;
                    send_a = true;
                }
                
                else if ( a_curr + a_back < c_curr + c_back ) {
                    tot_slots += a_back + DIFS_slots + FRAME_slots + SIFS_slots + ACK_slots;
                    a_back = generate_backoff(temp_collision, CW_0);
                    send_a = false;
                    send_c = true;
                }
                
                else {
                    tot_slots += a_back + DIFS_slots + FRAME_slots + SIFS_slots + ACK_slots;
                    a_back = generate_backoff(temp_collision, CW_0);
                    c_back = generate_backoff(temp_collision, CW_0);
                }
            }
        }
        // Given previous if statements, current total slot will be ahead of C arrival, but behind A
        else if ( tot_slots >= c_curr ) {
            // Current total slots is behind A arrival and will have a successful transmission
            if ( tot_slots <= a_curr ) {
                tot_slots += c_back + DIFS_slots + FRAME_slots + SIFS_slots + ACK_slots;
                c_success++;
                temp_collision = 0;
                c_back = generate_backoff(0, CW_0);
                k++;
            }
            // Current total slots potentially will collide if backoff ends up being the same
            else {
                // Backoff freeze from A transmitting first
                if ( a_curr < tot_slots + c_back ) {
                    c_back = ( a_curr + a_back ) - ( tot_slots + c_back );
                }
                
                else {
                    c_back = generate_backoff(temp_collision, CW_0);
                }
                
                tot_slots = a_curr;
            }
        }
        // Based off prior statements, current total slot will be ahead of A arrival, but behind C
        else if ( tot_slots >= a_curr ) {
            // Current total slots is behind C arrival and will have a successful transmission
            if ( tot_slots <= c_curr ) {
                tot_slots += a_back + DIFS_slots + FRAME_slots + SIFS_slots + ACK_slots;
                a_success++;
                temp_collision = 0;
                a_back = generate_backoff(0, CW_0);
                j++;
            }
            // Current total slots potentially will collide if backoff ends up being the same
            else {
                // Backoff freeze from C transmitting first
                if ( c_curr < tot_slots + a_back ) {
                    a_back = ( c_curr + c_back ) - ( tot_slots + a_back );
                }
                
                else {
                    a_back = generate_backoff(temp_collision, CW_0);
                }
                
                tot_slots = c_curr;
            }
        }
        total_iterations++;
        std::cout << total_iterations << ") Current time: " << tot_slots << std::endl;
        std::cout << "Current collisions: " << collisions << std::endl << std::endl;
    }
    std::cout << "\n\nTotal iterations to complete simulation time: " << total_iterations << std::endl;
    std::cout << "Total number of collisions: " << collisions << "\tSuccessful A nodes: " << a_success << "\tSuccessful C nodes: " << c_success << std::endl;
}
