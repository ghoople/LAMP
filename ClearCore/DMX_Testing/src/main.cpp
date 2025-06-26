/*
 * Penumbra_Principal
 *
 *  This is the primary code base that will run on the clear core.
 * 
 */
#include "main.h"

// Define the baud rate for the serial devices
#define usbBaudRate 9600
#define agentBaudRate 9600

// Define the initial velocity limit, acceleration limit, and commanded deceleration to be used for default moves
#define initialVelocityLimit 2000 // pulses per sec

// Homing velocity might need to have the sign changed so that it goes the correct direction. 
#define homingVelocity -500 //Velocity to home the motor. 

void setup() {
    bool debug = true;
    // Communications Setup
        // Serial Coms to the USB Port (with timeout)
        Serial.begin(usbBaudRate);
        uint32_t timeout = 5000; // 5 second timeout
        uint32_t startTime = millis();
        while (!Serial && millis() - startTime < timeout) {
            continue;
        }

        // Serials Coms to the agent arduino via Serial 1 port. 
        Serial1.begin(agentBaudRate);
        Serial1.ttl(true); // Tells serial one to use TTL logiv (5V signals)
        startTime = millis();
        while (!Serial1 && millis() - startTime < timeout) {
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
    
    // Encoder Setup
        EncoderIn.Enable(true); // Enable the encoder input feature      
        EncoderIn.SwapDirection(false); // Swap direction if you want for the encoder
        EncoderIn.IndexInverted(false); // Set the sense of index detection (true = rising edge, false = falling edge)
        EncoderIn.Position(0); // Zero the encoder position to match the zero of the motor. 

    delay(1000); // Probably don't need this. But can give it all a second to catchup. 
    if(debug){Serial.println("Setup complete");}
}

void loop() {
    //Animation Order Defined Here

/*
    // Might want to move the light carriage to the bottom, this is where all animations are assumed to start (in case there was an error)
    motor.Move(Bot, MotorDriver::MOVE_TARGET_ABSOLUTE);
    while (!motor.StepsComplete()) {
        continue;}
*/

    Animation(1);
    Animation(2);
    //Animation(3);

    Serial.println("Animation Loop Complete");
    delay(1000);
}
