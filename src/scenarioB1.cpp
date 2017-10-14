#include "simDefinitions.h"
#include "scenario.h"

void runScenarioB1(std::vector<int> a_arrival, std::vector<int> c_arrival) {
    int a_back = generate_backoff(0, CW_0);
    int c_back = generate_backoff(0, CW_0);
    int a_curr = a_arrival.at(0);
    int c_curr = c_arrival.at(0);
    int temp_collision = 0, tot_slots = 0, j = 0, k = 0, a_success = 0, c_success = 0, collisions = 0, total_iterations = 0, a_slots = 0, c_slots = 0, a_ref = 0, c_ref = 0;
    bool send_c = false, send_a = false, c_reset = false, a_reset = false;
    std::ofstream myFile;
    myFile.open("b1out.txt", std::ios::app);
    
    while ( tot_slots < SIM_TIME_slots ) {
        // When current total slots is ahead A and C arrival times.
        if( tot_slots >= a_curr && tot_slots >= c_curr ){
            // When C's current slots aren't reset and A's slots need to be reset
            if( c_reset != true && a_reset == true ){    // c_slots, a_slots
                a_back = generate_backoff(temp_collision, CW_0) + DIFS_slots;
                a_slots = tot_slots + a_back + FRAME_slots + SIFS_slots + ACK_slots;
                // Update reference variable for C when needed
                if( c_ref == 0 ){
                    c_ref = tot_slots + c_back;
                }
                
                a_ref = tot_slots + a_back;
            }
            //
            else if( a_reset != true && c_reset == true ){
                c_back = generate_backoff(temp_collision, CW_0) + DIFS_slots;
                c_slots = tot_slots + c_back + FRAME_slots + SIFS_slots + ACK_slots;
                // Update A reference when needed
                if(a_ref == 0){
                    a_ref = tot_slots + a_back;
                }
                c_ref = tot_slots + c_back;
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
            
            // C firsts transmits before A (no backoff freeze)
            if ( c_slots <= a_ref ){
                
                if ( send_c == true ){
                    tot_slots = c_slots;
                    c_success++;
                    k++;
                    temp_collision = 0;
                    c_ref = 0;
                    c_slots = 0;
                }
                else {
                    tot_slots = c_slots;
                    c_slots = 0;
                    c_ref = 0;
                    c_back = 0;
                    send_c = 0;
                }
            }
            else if(a_slots <= c_ref){
                
                if(send_a == 0){
                    tot_slots = a_slots;
                    a_slots = 0;
                    a_success++;
                    temp_collision = 0;
                    a_ref = 0;
                    j++;
                }
                else{
                    tot_slots = a_slots;
                    a_slots = 0;
                    a_ref = 0;
                    a_back = 0;
                    send_a = 0;
                }
            }
            
            else{
                
                temp_collision++;
                collisions++;
                
                if(a_slots < c_slots){
                    tot_slots = a_slots;
                    a_slots = 0;
                    a_ref = 0;
                    a_back = 0;
                    send_a = 0;
                    send_c = 1;
                }
                
                else if(c_slots < a_slots){
                    tot_slots = c_slots;
                    c_slots = 0;
                    c_ref = 0;
                    c_back = 0;
                    send_c = 0;
                    send_a = 1;
                }
                
                else{
                    tot_slots = c_slots;
                    a_slots = 0;
                    c_slots = 0;
                    a_ref = 0;
                    c_ref = 0;
                    a_back = 0;
                    c_back = 0;
                }
            }
        }
        
        
        else if(tot_slots >= a_curr){
            
            if(a_slots == 0){
                a_back = generate_backoff(temp_collision, CW_0) + DIFS_slots;
                a_slots = tot_slots + a_back + FRAME_slots + SIFS_slots + ACK_slots;
            }
            
            int c_curr = c_arrival.at(k);
            
            
            if(a_slots > c_curr){
                
                if(c_curr < a_ref){
                    a_back = a_back - (c_curr - (a_ref - a_back));
                }
                
                else if(c_curr < tot_slots + a_back){
                    a_back -= c_curr - tot_slots;
                }
                
                else{
                    a_back = 0;
                }
                tot_slots = c_curr;
            }
            else{
                tot_slots = a_slots;
                a_slots = 0;
                a_success++;
                a_ref = 0;
                temp_collision = 0;
                a_back = 0;
                j++;
            }
        }
        
        
        else if(tot_slots >= c_curr){
            
            if(c_slots == 0){
                c_back = generate_backoff(temp_collision, CW_0) + DIFS_slots;
                c_slots = tot_slots + c_back + FRAME_slots + SIFS_slots + ACK_slots;
            }
            int a_curr = a_arrival.at(j);
            
            
            if(c_slots > a_curr){
                
                if(a_curr < c_ref){
                    c_back = c_back - (a_curr - (c_ref - c_back));
                }
                
                else if(a_curr < tot_slots + c_back){
                    c_back -= a_curr - tot_slots;
                }
                
                else{
                    c_back = 0;
                }
                tot_slots = a_curr;
            }
            
            else{
                tot_slots = c_slots;
                c_slots = 0;
                c_success++;
                temp_collision = 0;
                c_ref = 0;
                c_back = 0;
                k++;
            }
        }
        
        
        else{
            
            if(a_curr < c_curr){
                tot_slots += (a_curr - tot_slots);
            }
            
            else if(a_curr > c_curr){
                tot_slots += (c_curr - tot_slots);
            }
            
            else{
                tot_slots += (a_curr - tot_slots);
            }
        }
        total_iterations++;
    }
    myFile << total_iterations << "\t" << collisions << "\t" << a_success << "\t" << c_success << std::endl;
    myFile.close();
    std::cout << "\n\nTotal iterations to complete simulation time: " << total_iterations << std::endl;
    std::cout << "Total number of collisions: " << collisions << "\tSuccessful A nodes: " << a_success << "\tSuccessful C nodes: " << c_success << std::endl;
}
