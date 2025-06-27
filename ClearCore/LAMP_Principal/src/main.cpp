/*
 * LAMP_Principal
 *
 *  This is the primary code base that will run on the clear core to control the light tower for LAMP
 * 
 */
#include "main.h"
#include <Arduino.h>
#include <ClearCore.h>

// Define the baud rate for the serial devices
#define usbBaudRate 115200

bool debug = true;

void setup() {
    // Communications Setup
        // Serial Coms to the USB Port (with timeout)
        Serial.begin(usbBaudRate);
        uint32_t timeout = 5000; // 5 second timeout
        uint32_t startTime = millis();
        while (!Serial && millis() - startTime < timeout) {
            continue;
        }

    // Configure the hard stops
        pinMode(BotInterruptPin, INPUT);
        pinMode(TopInterruptPin, INPUT);
        attachInterrupt(digitalPinToInterrupt(BotInterruptPin), BottomHardStop, RISING);
        attachInterrupt(digitalPinToInterrupt(TopInterruptPin), TopHardStop, RISING);
    
    // Motor Setup
        // Sets the input clocking rate.
        MotorMgr.MotorInputClocking(MotorManager::CLOCK_RATE_LOW);

        // Sets all motor connectors into step and direction mode.
        MotorMgr.MotorModeSet(MotorManager::MOTOR_ALL,
                            Connector::CPM_MODE_STEP_AND_DIR);

        // Sets the initial velocity for each move
        motor.VelMax(initialVelocityLimit);

        // Set the acceleration limit for each move
        motor.AccelMax(accelerationLimit);

        // Enables output to the motor (must have, even if not using the enable pin on the motor)
        motor.EnableRequest(true);


        // Home the motor
        if(debug){Serial.println("Homing Motor");}
        motor.MoveVelocity(homingVelocity); // Move down, will stop when the hard stop is tripped.  
        // Wait for interrupt to trigger when hard stop reached, interrupt code will automatically set the 0 position.
        while (!motor.StepsComplete()) {
            continue;
        }
        
        if(debug){Serial.println("Homing Complete");}
    

    delay(1000); // Probably don't need this. But can give it all a second to catchup. 
    if(debug){Serial.println("Setup complete");}
}

void loop() {
    
    // Add Code here to receive DMX Signals and check if I get new values. If i do get new values, then I'd want to do something. 

    // Right now assume they are this: 
    
    uint8_t DMXposition = 100; // Position value ranges 0 -> 255
    uint8_t DMXspeed = 50; // Speed value ranges 0->255

    int position = (DMXposition * Top) / 255;
    int speed = (DMXspeed * motorMaxSpeed) / 255; // Maximum speed for the control wheel was 5000

     // Sets the maximum velocity for this move
    motor.VelMax(speed);

    // Command the move of absolute distance
    motor.Move(position, MotorDriver::MOVE_TARGET_ABSOLUTE);

    /* Waits for all step pulses to output. While waiting it needs to: 
    1. Monitor the DMX signal for position or velocity changes and break out of loop. 
    2. Monitor for hard stop trips, because if it trips it could get stuck in an infinite loop (was a pain to degbug in Penumbra). 
    */

    while (!motor.StepsComplete()) {

        // Add code here to check and see if a new DMX Signal is received (I think, then maybe break out of the loop)  

        // Check for hard stop trips, break out of while loop if one is detected to avoid infinite loop. 
        if(hardStopTrip){
            if(debug){Serial.println("Hard Stop Trip Detected, breaking out of while loop to avoid crash.");}
            hardStopTrip = false; // Reset the global hard stop trip variable
            break; // Break out of the while loop. This is critically important, without it, the ClearCore becomes inconsistenly unresponsive.
        }    
    }
    delay(10);
}
