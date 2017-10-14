#include "simDefinitions.h"
#include "scenario.h"

void runScenarioB1(std::vector<int> a_arrival, std::vector<int> c_arrival) {
    int a_back = generate_backoff(0, CW_0);
    int c_back = generate_backoff(0, CW_0);
    int a_curr = a_arrival.at(0);
    int c_curr = c_arrival.at(0);
    int temp_collision = 0, tot_slots = 0, j = 0, k = 0, a_success = 0, c_success = 0, collisions = 0, total_iterations = 0, a_slots = 0, c_slots = 0, a_ref = 0, c_ref = 0, send_a = 0, send_c = 0;
    std::ofstream myFile;
    myFile.open("b1out.txt", std::ios::app);
    
    while ( tot_slots < SIM_TIME_slots ) {
        total_iterations++;
        // When current total slots is behind A and C arrival times.
        if ( tot_slots < a_curr && tot_slots < c_curr ) {
            // Check which station arrives earlier (if both, push current slot to arbitrary time)
            if(a_curr < c_curr){
                tot_slots = a_curr;
            }
            else if(a_curr > c_curr){
                tot_slots = c_curr;
            }
            else{
                tot_slots = a_curr;
            }
        }
        // When current total slots is ahead A and C arrival times.
        else if ( tot_slots >= a_curr && tot_slots >= c_curr ) {
            // When A's current slot isn't reset and C's slot needs to be reset
            if( a_slots > 0 && c_slots == 0 ){
                c_back = generate_backoff(temp_collision, CW_0) + DIFS_slots;
                c_slots = tot_slots + c_back + FRAME_slots + SIFS_slots + ACK_slots;
                // Update A reference when needed
                if(a_ref == 0){
                    a_ref = tot_slots + a_back;
                }
                c_ref = tot_slots + c_back;
            }
            // When C's current slots isn't reset and A's slots need to be reset
            else if( c_slots > 0 && a_slots == 0 ){
                a_back = generate_backoff(temp_collision, CW_0) + DIFS_slots;
                // Update a station's current slot
                a_slots = tot_slots + a_back + FRAME_slots + SIFS_slots + ACK_slots;
                // Update A reference
                a_ref = tot_slots + a_back;
                // Update reference variable for C when needed after successful C transmission
                if( c_ref == 0 ){
                    c_ref = tot_slots + c_back;
                }
            }
            // A and C transmit at the same time ** COLLISION **
            else {
                c_slots = tot_slots + c_back + FRAME_slots + SIFS_slots + ACK_slots;
                a_slots = tot_slots + a_back + FRAME_slots + SIFS_slots + ACK_slots;
                c_back = generate_backoff(temp_collision, CW_0) + DIFS_slots;
                a_back = generate_backoff(temp_collision, CW_0) + DIFS_slots;
                c_ref = tot_slots + c_back;
                a_ref = tot_slots + a_back;
            }
            
            // Check C's transmitting status in comparison to A's backoff
            if ( c_slots <= a_ref ){
                // If C needs to be retransmitted
                if ( send_c == 1) {
                    tot_slots = c_slots;
                    c_slots = 0;
                    c_ref = 0;
                    c_back = 0;
                    send_c = 0;
                }
                // C transmits successfully
                else {
                    tot_slots = c_slots;
                    c_success++;
                    k++;
                    temp_collision = 0;
                    c_ref = 0;
                    c_slots = 0;
                }
            }
            // Check A's transmitting status in comparison to C's backoff
            else if (a_slots <= c_ref){
                // If A needs to be retransmitted
                if ( send_a == 1 ) {
                    tot_slots = a_slots;
                    a_slots = 0;
                    a_ref = 0;
                    a_back = 0;
                    send_a = 0;
                }
                // A transmits successfully
                else {
                    tot_slots = a_slots;
                    a_slots = 0;
                    a_success++;
                    temp_collision = 0;
                    a_ref = 0;
                    j++;
                }
            }
            // Collision will occur
            else{
                // Update collision variables
                temp_collision++;
                collisions++;
                // If C's transmission arrives earlier
                if(c_slots < a_slots){
                    tot_slots = c_slots;
                    c_slots = 0;
                    send_c = 0;
                    send_a = 1;
                    c_ref = 0;
                    c_back = 0;
                }
                // If A's transmission arrives earlier
                else if(a_slots < c_slots){
                    tot_slots = a_slots;
                    a_slots = 0;
                    send_a = 0;
                    send_c = 1;
                    a_ref = 0;
                    a_back = 0;
                }
                // If A and C transmissions arrive at the same time
                else{
                    tot_slots = a_slots;
                    a_slots = 0;
                    c_slots = 0;
                    a_back = 0;
                    c_back = 0;
                    a_ref = 0;
                    c_ref = 0;
                }
            }
        }
        // Current time slot is ahead of A and behind C
        else if(tot_slots >= a_curr){
            // Check for need of A station slot update
            if(a_slots == 0){
                a_back = generate_backoff(temp_collision, CW_0) + DIFS_slots;
                a_slots = tot_slots + a_back + FRAME_slots + SIFS_slots + ACK_slots;
            }
            // Update C arrival time here
            int c_curr = c_arrival.at(k);
            // Check A's backoff freeze
            if ( a_slots > c_curr ) {
                
                if ( c_curr < tot_slots + a_back ) {
                    a_back -= c_curr - tot_slots;
                }
                
                if ( c_curr < a_ref ) {
                    a_back = a_back - ( c_curr - ( a_ref - a_back ) );
                }
                
                else {
                    a_back = 0;
                }
                // Current slot is moved to C arrival slot
                tot_slots = c_curr;
            }
            // When A transmits without possible collision/backoff freeze
            else {
                tot_slots = a_slots;
                temp_collision = 0;
                a_back = 0;
                a_slots = 0;
                a_ref = 0;
                a_success++;
                j++;
            }
        }
        // When current time slot is behind C but ahead of A
        else if ( tot_slots >= c_curr ) {
            // When C's current time slot needs to be reset
            if( c_slots == 0 ) {
                c_back = generate_backoff(temp_collision, CW_0) + DIFS_slots;
                c_slots = tot_slots + c_back + FRAME_slots + SIFS_slots + ACK_slots;
            }
            // A's arrival time updates here
            int a_curr = a_arrival.at(j);
            
            // Backoff freeze calculation
            if ( c_slots > a_curr ) {
                
                if ( a_curr < tot_slots + c_back ) {
                    c_back -= a_curr - tot_slots;
                }
                
                else if ( a_curr < c_ref ) {
                    c_back = c_back - ( a_curr - ( c_ref - c_back ) );
                }
                
                else{
                    c_back = 0;
                }
                // Current slot is moved to A arrival slot
                tot_slots = a_curr;
            }
            // When C transmits without possible collision/backoff freeze
            else {
                tot_slots = c_slots;
                temp_collision = 0;
                c_ref = 0;
                c_back = 0;
                c_slots = 0;
                c_success++;
                k++;
            }
        }
    }
    myFile << total_iterations << "\t" << collisions << "\t" << a_success << "\t" << c_success << std::endl;
    myFile.close();
    std::cout << "\n\nTotal iterations to complete simulation time: " << total_iterations << std::endl;
    std::cout << "Total number of collisions: " << collisions << "\tSuccessful A nodes: " << a_success << "\tSuccessful C nodes: " << c_success << std::endl;
}
