// Interrupt functions that are called when a sensor at either hardstop is tripped. 
#include "main.h"

volatile bool hardStopTrip = false; // Variable to track if a hard stop has been tripped.

void BottomHardStop() {
    // Called via interrupt when the hard stop pin is tripped. 
    
    // Detach the hard stop pin so it doesn't trip twice (had this problem, known as "bouncing")
    detachInterrupt(digitalPinToInterrupt(BotInterruptPin)); // Detach the interrupt
    
    motor.MoveStopAbrupt(); // Stop the motor

    delay(1000); // Give the motor time to stop and the interrupt time to reset.

    Serial.println("Bottom Hard Stop Triggered");
    
    // Move away from the hard stop
    motor.VelMax(2000); // Increase speed to 2000 pulse/sec for the move away from the hard stop

    motor.Move(Home_Offset);
        Serial.println("Moving away from the hard stop... Waiting for the step output to finish...");
    while (!motor.StepsComplete()) {
        continue;
    }
    
    // Zero the motor's reference position after homing to allow for accurate absolute position moves

    motor.PositionRefSet(0);    
    
    // Reattach the interrupt
    attachInterrupt(digitalPinToInterrupt(BotInterruptPin), BottomHardStop, RISING);

    hardStopTrip = true; // Set the global hard stop trip variable to true
}

void TopHardStop() {
    // Called via interrupt when the hard stop pin is tripped. 

    // Detach the hard stop pin so it doesn't trip twice (had this problem)
    detachInterrupt(digitalPinToInterrupt(TopInterruptPin)); // Detach the interrupt
    
    motor.MoveStopAbrupt();

    delay(1000); // Give the motor time to stop and the interrupt time to reset.

    Serial.println("Top Hard Stop Triggered");
    
    // Move away from the hard stop

    motor.VelMax(2000); // Move at 2000 pulse/sec away from hard stop

    motor.Move(-1*Home_Offset);
        Serial.println("Moving away from the hard stop... Waiting for the step output to finish...");
    while (!motor.StepsComplete()) {
        continue;
    }

    // Zero the motor's reference position after homing to allow for accurate absolute position moves

    motor.PositionRefSet(Top);   //Defined in global variables.  

    attachInterrupt(digitalPinToInterrupt(TopInterruptPin), TopHardStop, RISING);

    hardStopTrip = true; // Set the global hard stop trip variable to true
}