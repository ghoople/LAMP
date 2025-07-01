// main.h

#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h>
#include <ClearCore.h>

// Define the baud rate for the USD serial
#define usbBaudRate 115200
#define agentBaudRate 38400

// Configure Interrupt Pins for the hard stops
// Pins that support digital interrupts on clear core are:
// DI-6, DI-7, DI-8, A-9, A-10, A-11, A-12
#define BotInterruptPin A11
#define TopInterruptPin A12

extern volatile bool hardStopTrip; // Global variable to track if a hard stop has been tripped.

// My code (and example code) uses motor instead of the connector name. 
// Options are: ConnectorM0, ConnectorM1, ConnectorM2, or ConnectorM3.
#define motor ConnectorM0

// Define motor speed parameters
#define accelerationLimit 9000 // pulses per sec^2 , worked well in Penumbra
#define motorMaxSpeed 5000 // 5000 pulses per sec worked well for the max speed when on wheel control, could experiment with higher
#define initialVelocityLimit 2000 // pulses per sec, sets the initial motor speed after homing (but will get overwritten). 

// Homing velocity might need to have the sign changed so that it goes the correct direction. 
#define homingVelocity -500 //Velocity to home the motor. 


// Define the physical relationship between steps and light position
// See "Penumbra Motor Calculations" google sheet for value calculator
#define Bot 0 
#define Top 11732 // This is the number of pulses to get to the top.
#define Mid Top/2
#define Home_Offset 95 // This is 1 inch in pulses (I think)

// Function Declarations
void BottomHardStop();
void TopHardStop();

#endif // MAIN_H
