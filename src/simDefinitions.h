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
extern int CW_MAX;// 1024 // in slots
extern int CW_0;// 4 // in slots
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
extern int SIM_TIME_slots;
extern int DIFS_slots;		
extern int SIFS_slots;		
extern int FRAME_slots; 
extern int ACK_slots; 
extern int RTS_slots; 
extern int CTS_slots; 

void printOptions();
int getScenario();
int generate_backoff(int collide, double CW);
std::vector<int> generate_poisson(int lambda);
std::vector<int> generate_arrival(std::vector<int> poisson);

#endif //SIM_DEFINITIONS_H