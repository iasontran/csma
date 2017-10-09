#ifndef SIM_DEFINITIONS_H
#define SIM_DEFINITIONS_H

/*
All the required includes
*/
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <cstdio>
#include <cstdlib>

/*
Simulation Parameters
*/
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

/*
Define four states, using arbitrary negatives to not conflict with anything else
*/
#define A_CA -10
#define A_VCS -11
#define B_CA -12
#define B_VCS -13

/*
Converting all units into SLOT time. We can increment by doing:
tot_slot += DIFS_slots + (a_back or c_back) + FRAME_slots + SIFS_slots + ACK_slots;
*/
#define SIM_TIME_slots	(SIM_TIME / (SLOT * pow(10, -6)))
#define DIFS_slots		ceil((double)DIFS / SLOT)
#define SIFS_slots		ceil((double)SIFS / SLOT);
#define FRAME_slots		((FRAME * 8.0) / (TRANSMISSION_RATE * pow(10, 6)) / (SLOT * pow(10, -6)))
#define ACK_slots		(ACK * 8 / (TRANSMISSION_RATE * pow(10, 6)) / (SLOT * pow(10, -6)))
#define RTS_slots		(RTS * 8 / (TRANSMISSION_RATE * pow(10, 6)) / (SLOT * pow(10, -6)))
#define CTS_slots		(CTS * 8 / (TRANSMISSION_RATE * pow(10, 6)) / (SLOT * pow(10, -6)))

void printOptions();
int getScenario();
int generate_backoff(int collide, double CW);
std::vector<int> generate_poisson(int lambda);
std::vector<int> generate_arrival(std::vector<int> poisson);

#endif //SIM_DEFINITIONS_H