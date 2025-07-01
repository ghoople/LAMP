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
int positionChannel = 0; // Position value ranges 0 -> 255
int speedChannel = 0; // Speed value ranges 0->255

// Variables to store last position
int positionLast = 0; // Position value ranges 0 -> 255
int speedLast = 0; // Speed value ranges 0->255


void setup() {
    // Communications Setup
        // Serial Coms to the USB Port (with timeout)
        Serial.begin(usbBaudRate);
        uint32_t timeout = 5000; // 5 second timeout
        uint32_t startTime = millis();
        while (!Serial && millis() - startTime < timeout) {
            continue;
        }
    
    // Serials Coms to the agent arduino via Serial 0 port. 
        Serial0.begin(agentBaudRate);
        Serial0.ttl(true); // Tells serial one to use TTL logiv (5V signals)
        startTime = millis();
        while (!Serial0 && millis() - startTime < timeout) {
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


// TEMPORARILY DISABLE HOMING FOR TESTING COMS> MUST RE-ENABLE

    //   // Home the motor
    //   if(debug){Serial.println("Homing Motor");}
    //     motor.MoveVelocity(homingVelocity); // Move down, will stop when the hard stop is tripped.  
    //     // Wait for interrupt to trigger when hard stop reached, interrupt code will automatically set the 0 position.
    //     while (!motor.StepsComplete()) {
    //         continue;
    //     }
        
    //     if(debug){Serial.println("Homing Complete");}
    

    delay(1000); // Probably don't need this. But can give it all a second to catchup. 
    if(debug){Serial.println("Setup complete");}
}

void loop() {
    
    if(Serial0.available() > 0){
        //If new data is available over the serial port, read it. 
        String message = Serial0.readStringUntil('\n'); 
        // Parse the message format "P:{position} S:{speed}"
        int pos = 0, speed = 0;
        if(sscanf(message.c_str(), "P:%d S:%d", &pos, &speed) == 2) {
            positionChannel = pos;
            speedChannel = speed;
            if(debug){
                Serial.print("Received: P:");
                Serial.print(positionChannel);
                Serial.print(" S:");
                Serial.println(speedChannel);
            }
        } else if(debug) {
            Serial.println("Failed to parse message: " + message);
        }
    }

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
