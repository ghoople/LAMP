// Penumbra_Principal.h

#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h>
#include <ClearCore.h>

// Configure Interrupt Pins for the hard stops
// Pins that support digital interrupts on clear core are:
// DI-6, DI-7, DI-8, A-9, A-10, A-11, A-12
#define BotInterruptPin A11
#define TopInterruptPin A12

extern bool hardStopTrip; // Global variable to track if a hard stop has been tripped.

// My code (and example code) uses motor instead of the connector name. 
// Options are: ConnectorM0, ConnectorM1, ConnectorM2, or ConnectorM3.
#define motor ConnectorM0
#define motorDecel 4000 //Defines how quickly the motor should decelerate when switching into/out of user mode. 
#define accelerationLimit 9000 // pulses per sec^2
#define wheelThreshold 200 // Define what speed the encoder needs to move at for it to be considered a user input (200 pulses/second) 

// Define the physical relationship between steps and light position
// See "Penumbra Motor Calculations" google sheet for value calculator
#define Bot 0 
#define Top 11732 // This is the number of pulses to get to the top.
#define Mid Top/2
#define Home_Offset 95 // This is 1 inch in pulses (I think)

// Define how long the index is for the halogen lighting array. Can't be an int. 
#define halIndexLength 100

// DMX Brightness Levels, range is 0 -> 255
#define maxBright 255
#define midBright 175
#define lowBright 150
#define off 0

// Function Declarations
void Pause(int pauseTime, int* halA, int* halB);
void MoveTarget(int target, int velocityLimit, int* halA, int* halB);
void BottomHardStop();
void TopHardStop();
void WheelControl();
void Animation(int showNum);

#endif // MAIN_H
