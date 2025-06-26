#include "main.h"

/* WheelControl() is the function that is called when a user starts to move the wheel. 
It allows the user to control the light for a defined interval of time. 
*/

/* Set the Maximum speeds for mapping the encoder speed to the motor speed. 
  For the Encoder: 
    1 revolution in 5 seconds on the wheel = 200 counts/second
    250 RPM = 4.2 rev/s (Seems hard to imagine going that fast) = 42,000 counts/second  
  For the motor: (Pulses/s output)
    0 - 5000 seems reasonable to start, may want to adjust
*/
#define encoderMaxSpeed 42000 
#define motorMaxSpeed 5000  // 5000 is for the chain system, 3000 better for the wire system. 

// Set the update intervals for the timer loops. 
#define wheelTimeout 10000 // 10 seconds
#define encoderAverageInterval 100 // 100 ms
#define encoderReadInterval 25 // ms
#define motorVelocityReset 2000 // Resets the motor velocity at the end of user interaction. 
unsigned long motorUpdateInterval = 200; // ms interval to update the motor

int loop_count = 0;

void WheelControl() {
    bool debug = false;

    if(debug){
      motorUpdateInterval = 500; // Slow down motor updates. 
      Serial.println("Entering Wheel Control Function");
    }

    // Declare variables needed for this function only. 
    int32_t wheelPosition = 0;
    int32_t lastWheelPosition = 0;
    int32_t wheelVelocity = 0;
    long totalVelocity = 0;
    long readingsCount = 0;
    int userSpeed = 0; 

// Define the DMX brightness for the halogne lights during wheel control mode, Ranges 0 -> 255. 
    int intensityA = maxBright;
    int intensityB = 0; 

    // This timer keeps us in a while loop until wheelTimeout ms after the last time a user moved the wheel above the threshold speed. 
    uint32_t lastWheelMoveTime, encoderStartTime, motorStartTime, encoderReadTime;
    lastWheelMoveTime = encoderStartTime = motorStartTime = encoderReadTime = millis();
    
    while (millis() - lastWheelMoveTime < wheelTimeout) {
      
      if (millis() - encoderReadTime >= encoderReadInterval) {
        wheelPosition = EncoderIn.Position(); // Read the encoder position
        wheelVelocity = EncoderIn.Velocity(); // Read the encoder velocity
        if (wheelVelocity > wheelThreshold) {
          lastWheelMoveTime = millis();
        }
        encoderReadTime = millis();
      }

      totalVelocity += wheelVelocity; // Add the wheel velocity to the totalVelocity variable. 
      readingsCount++; // Increment the number of readings we have taken

      // This timer calculates the average velocity over a window of 0.1 seconds
      if (millis() - encoderStartTime >= encoderAverageInterval) {
        // Calculate average velocity over 0.1 seconds
        float averageVelocity = totalVelocity / readingsCount;
        
        //Convert the averageVelocity (counts/second) to an appropriate motor speed (Pulses/sec).
        userSpeed = map(abs(averageVelocity),0,encoderMaxSpeed,0,motorMaxSpeed);

        // Reset variables for the next interval
        totalVelocity = 0;
        readingsCount = 0;
        encoderStartTime = millis();
      }

      // This timer makes it so the motor gets updated position commands every motorUpdateInterval seconds
      // Going faster seems to cause unwanted behavior.  
      // It also tells the agent where the motor is and that a user is controlling it.
      if (millis() - motorStartTime >= motorUpdateInterval) {
        if(wheelPosition > lastWheelPosition){ // Go UP
          motor.VelMax(userSpeed); // Sets the maximum velocity for this move based on the potentiometer
          motor.Move(Top, MotorDriver::MOVE_TARGET_ABSOLUTE); // Commands the motor move (with no waiting step before commanding additional moves!)
          
          if(debug){
            Serial.print("Moving up at user speed: ");
            Serial.println(userSpeed);
          }

        }
        else{ // Go Down
          motor.VelMax(userSpeed); // Sets the maximum velocity for this move based on the userSpeed
          motor.Move(Bot, MotorDriver::MOVE_TARGET_ABSOLUTE); // Commands the motor move (with no waiting step before commanding additional moves!)
          if(debug){
            Serial.print("Moving down at user speed: ");
            Serial.println(userSpeed);
          }
        }
        lastWheelPosition = wheelPosition;
        motorStartTime = millis();
      
        if(debug){Serial.println("Sending data to agent");}

        // Send the required data to the agent arduino
        int currentPos = motor.PositionRefCommanded();

        Serial1.print(currentPos); // Tell the Agent where the light is. 
        Serial1.print(","); 
        Serial1.print(intensityA);// Tell the agent what the intensity should be for halA
        Serial1.print(","); 
        Serial1.println(intensityB);// Tell the agent what the intensity should be for halB
      }
    }

  // Prepare to exit the wheel control mode
  motor.MoveStopDecel(motorDecel); // Stop the motor at the deceleration limit (set in main.cpp)
  while(!motor.StepsComplete()){ // Wait for the motor to stop
    // Do nothing
  }
  motor.VelMax(motorVelocityReset); // Reset the motor velocity to something reasonable (in case wheel was at 0)
  if(debug){Serial.println("Exiting Wheel Control Mode ");}

}