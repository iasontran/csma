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
    int lambda_A = 200;	// set of 50, 100, 200, 300 frame/sec
    int lambda_C = 200;	// set of 50, 100, 200, 300 frame/sec
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
    
    /*
     Selection of scenario
     */
    
    //printOptions();
    //choice = getScenario();
    
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
    
    // Increment every loop to see how many cycles it needed to get to SIM_TIME_slots
    int total_iterations = 0;
	choice = A_VCS;
    switch (choice) {
        case A_CA:	{// Scenario A, CSMA
            lambda_A = 50;
            lambda_C = 50;
            for (int itr = 0; itr < 20; itr++) {
                a_poisson_set = generate_poisson(lambda_A);
                c_poisson_set = generate_poisson(lambda_C);
                a_arrival = generate_arrival(a_poisson_set);
                c_arrival = generate_arrival(c_poisson_set);
                runScenarioA1(a_arrival, c_arrival);
            }
            break;
        }
        case A_VCS:	{// Scenario A, CSMA 2
			int lambdaTemp[] = { 50, 100, 200, 300 };
			for (int i = 0; i < 4; i++) {
				// Testing purposes
				lambda_A = 2 * lambdaTemp[i];
				lambda_C = 1 * lambdaTemp[i];
				//Adding to file, Testing purposes
				std::ofstream tempFile;
				tempFile.open("a2out.txt", std::ios::app);
				tempFile << "\nLambda: " << lambdaTemp[i] << "\tLambda A: " << lambda_A << "\tLambda C: " << lambda_C << std::endl;
				tempFile.close();
				// Close file, start testing. Loop of 20 just to get an average read.
				for (int itr = 0; itr < 20; itr++) {
					a_poisson_set = generate_poisson(lambda_A);
					c_poisson_set = generate_poisson(lambda_C);
					a_arrival = generate_arrival(a_poisson_set);
					c_arrival = generate_arrival(c_poisson_set);
					runScenarioA2(a_arrival, c_arrival);
				}
			}
            break;
        }
        case B_CA:	{// Scenario B, CSMA 1
            runScenarioB1(a_arrival, c_arrival);
            break;
        }
        case B_VCS:	{// Scenario B, CSMA 2
			int lambdaTemp[] = { 50, 100, 200, 300 };
			for (int i = 0; i < 4; i++) {
				// Testing purposes
				lambda_A = 2 * lambdaTemp[i];
				lambda_C = 1 * lambdaTemp[i];
				//Adding to file, Testing purposes
				std::ofstream tempFile;
				tempFile.open("b2out.txt", std::ios::app);
				tempFile << "\nLambda: " << lambdaTemp[i] << "\tLambda A: " << lambda_A << "\tLambda C: " << lambda_C << std::endl;
				tempFile.close();
				// Close file, start testing. Loop of 20 just to get an average read.
				for (int itr = 0; itr < 20; itr++) {
					a_poisson_set = generate_poisson(lambda_A);
					c_poisson_set = generate_poisson(lambda_C);
					a_arrival = generate_arrival(a_poisson_set);
					c_arrival = generate_arrival(c_poisson_set);
					runScenarioB2(a_arrival, c_arrival);
				}
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
