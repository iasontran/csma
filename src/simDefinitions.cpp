#include "simDefinitions.h"
#include "scenario.h"

/*
Print scenario options
*/
void printOptions() {
	std::cout << "Select a scenario:\n";
	std::cout << "A1: Concurrent Communication with CSMA Collision Avoidance\n";
	std::cout << "A2: Concurrent Communication with CSMA Virtual Carrier Sensing\n";
	std::cout << "B1: Hidden Terminal Communication with CSMA Collision Avoidance\n";
	std::cout << "B2: Hidden Terminal Communication with CSMA Virtual Carrier Sensing\n";
}

/*
Get user input
*/
int getScenario() {
	std::string scenario;
	while (1) {
		std::cout << "Your choice: ";
		std::cin >> scenario;
		if (scenario.compare("A1") == 0 || scenario.compare("a1") == 0) {
			return A_CA;
		}
		else if (scenario.compare("A2") == 0 || scenario.compare("a2") == 0) {
			return A_VCS;
		}
		else if (scenario.compare("B1") == 0 || scenario.compare("b1") == 0) {
			return B_CA;
		}
		else if (scenario.compare("B2") == 0 || scenario.compare("b2") == 0) {
			return B_VCS;
		}
		else {
			std::cout << "\nInvalid option, try again.\n\n";
		}
	}
}

/*
Function to generate the backoff value
*/
int generate_backoff(int collide, double CW) {
	int bound = 0;
	double output;

	if (pow(2, collide) > CW_MAX) {
		bound = (int)pow(2, CW_MAX)*CW;
	}
	else {
		bound = (int)pow(2, collide)*CW;
	}

	output = rand() % bound;

	return output;
}

/*
Generates the poisson set
*/
std::vector<int> generate_poisson(int lambda) {
	std::vector<int> temp_poisson;
	/*
	Poisson output done here, the / 2 part is just to make
	the set smaller. Without it, it seems the largest time
	is roughly 2x more than what is needed within sim time
	*/
	for (int i = 0; i < (SIM_TIME_slots / FRAME_slots); i++) {
		double uniform_rand = 0.0;
		// Mod to get a value between 0-99, / to get a value between 0 to 1
		uniform_rand = (rand() % 100) / 100.0;
		// Calculate the Poisson value, units are sec/frame
		uniform_rand = -(1.0 / lambda)*log(1 - uniform_rand);
		// Convert to slots, sec/frame / slot duration = slot/frame, long integer round
		uniform_rand = lround(uniform_rand / (SLOT * pow(10, -6)));
		// Add to vectors
		temp_poisson.push_back(uniform_rand);
	}
	return temp_poisson;
}

/*
Increments the arrival and stuff
*/
std::vector<int> generate_arrival(std::vector<int> poisson) {
	std::vector<int> temp_arrival;
	temp_arrival.push_back(poisson[0]);
	// Using just a_poisson_set.size() because both a and c should be the same size from the randomizer
	for (int i = 1; i < poisson.size(); i++) {
		int tempSum = 0;
		tempSum = temp_arrival[i - 1] + poisson[i];
		temp_arrival.push_back(tempSum);
	}
	return temp_arrival;
}