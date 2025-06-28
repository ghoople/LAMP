/*
 * LAMP_Principal
 *
 *  This is the primary code base that will run on the clear core to control the light tower for LAMP
 * 
 */
#include <main.h>
#include <Arduino.h>
#include <ClearCore.h>

bool debug = true;

//Temporarily Initialize variables for storing DMX values, these will actually be in the DMX code
// This will need to be removed if I get the DMX read to work and integrate it here. 
uint8_t positionChannel = 0; // Position value ranges 0 -> 255
uint8_t speedChannel = 0; // Speed value ranges 0->255

// Variables to store last position
uint8_t positionLast = 0; // Position value ranges 0 -> 255
uint8_t speedLast = 0; // Speed value ranges 0->255


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
    
    // DMX Code will go here, for now hard code: 

        positionChannel = 100; // Position value ranges 0 -> 255
        speedChannel = 50; // Speed value ranges 0->255

    if (positionChannel != positionLast || speedChannel != speedLast) {
        
        int motorPosition = (positionChannel * Top) / 255;
        int motorSpeed = (speedChannel * motorMaxSpeed) / 255;
        
        // Set speed and move (ClearCore will blend with any in-progress move)
        motor.VelMax(motorSpeed);
        motor.Move(motorPosition, MotorDriver::MOVE_TARGET_ABSOLUTE);
        
        // Update tracking variables
        positionLast = positionChannel;
        speedLast = speedChannel;
        
        if(debug) {
            Serial.print("Updated motor commands sent: Position=");
            Serial.print(motorPosition);
            Serial.print(" Speed="); 
            Serial.println(motorSpeed);
        }
    }
}
