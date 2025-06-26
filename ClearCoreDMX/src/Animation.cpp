#include "main.h"

/* Key Positions, defined in the main.h file as we need them in multiple places.
Bot ; this is when the primary lightthe bottom (0) position 
Top ; this is when the primary light is all the way at the top
Mid ; this is defined as halfway to the top
*/
// Additional Positions used only in animations, may want to add more. 
#define oneThird Top/3
#define twoThirds 2*Top/3

// Velocities, in pulses/second, for the light movements. 
#define fast 1500
#define med 500
#define slow 250

/* SEE main.h for actual levels
// DMX Brightness Levels, range is 0 -> 255
#define maxBright 255
#define midBright 150
#define lowBright 80
#define off 0
*/

// By convention, all animations should end at the bottom (So you can assume you are starting from the bottom) 
// After homing the motor will start at the bottom. 

void Animation(int showNum) {
bool debug = false;
// Define variables
int moveVel; // How fast is the move
int halA [halIndexLength];
int halB [halIndexLength];

    switch (showNum) {
    case 1: // Fast move, Light A on the whole time. 
        if(debug){Serial.println("Running Animation 1");}
        
        moveVel = med;
        // Step 1: Move to the top, lights A on
        for (int i = 0; i < halIndexLength; i++) {
            halA[i] = maxBright;
            halB[i] = 0;
        }
        MoveTarget(Top,moveVel,halA,halB);
        
        // Step 2: Move to the bottom, light A stays on. 
        for (int i = 0; i < halIndexLength; i++) {
            halA[i] = maxBright;
            halB[i] = 0;
        }
        MoveTarget(Bot,moveVel,halA,halB);

        break;
    case 2: // Lights are on when they are moving up only. 
        if(debug){Serial.println("Running Animation 2");}

        moveVel = slow;
        // Step 1: Move to the top, halA on
        for (int i = 0; i < halIndexLength; i++) {
            halA[i] = midBright;
            halB[i] = off;
        }
       
        MoveTarget(Top,moveVel,halA,halB);

        // Step 2: Move to the bottom, halB on
        for (int i = 0; i < halIndexLength; i++) {
            halA[i] = off;
            halB[i] = midBright;
        }

        MoveTarget(Bot,moveVel,halA,halB);

        break;
    case 3: // Ramp the brightness of the lights as they move up 
        if(debug){Serial.println("Running Animation 1");}

        moveVel = slow;

        // Step 1: Move to the top with the lights on
        for (int i = 0; i < halIndexLength; i++) {
            halA[i] = (float)i/halIndexLength * maxBright; // HalA ramps up. 
            halB[i] = off; // Turn off
        }

        MoveTarget(Top,moveVel,halA,halB);

        // Step 2: Move to the bottom, swap the lights
        // Turn halA off for entire move
        for (int i = 0; i < halIndexLength; i++) {
            halA[i] = off;
            halB[i] = (float)i/halIndexLength * maxBright; // Start off, ramp up to full
        }

        MoveTarget(Bot,moveVel,halA,halB);

        break;

    case 4: // Random code I was messing iwth. 
        moveVel = slow;
        // Step 1: Move to the top with the lights on
        // Ramp halA and halB for entire move
        for (int i = 0; i < halIndexLength; i++) {
            halA[i] = i/halIndexLength * midBright;
            halB[i] = i/halIndexLength * midBright; // Ramp up halB as well
        }
        MoveTarget(Top,moveVel,halA,halB);

        // Step 2: Move to the bottom, keep ramping the lights
        // Ramp halA and halB for entire move
        for (int i = 0; i < halIndexLength; i++) {
            halA[i] = midBright + i/halIndexLength * midBright; // Ramp up halA as well
            halB[i] = midBright + i/halIndexLength * midBright; // Continue ramping up halB
        }
        MoveTarget(Bot,moveVel,halA,halB);

        break;
    case 5: // Some simple debug code Light A on for up, off for down. 
        moveVel = fast;
        
        // Step 1: Move to Top, A on, B off.
            for (int i = 0; i < halIndexLength; i++) {
                halA[i] = maxBright;
                halB[i] = off; 
            }
            MoveTarget(Top,moveVel,halA,halB);
            
        // Step 2: Pause
            for (int i = 0; i < halIndexLength; i++) {
                halA[i] = off;
                halB[i] = off;
            }
            Pause(2,halA,halB); // pause for 2 seconds
            
        // Step 3: Move to the bottom, A off, B on. 
            for (int i = 0; i < halIndexLength; i++) {
                halA[i] = off;
                halB[i] = maxBright;
            }
            
            MoveTarget(Bot,moveVel,halA,halB);

        break;
    case 6:
        // statements
         // Step 2: Pause
            for (int i = 0; i < halIndexLength; i++) {
                halA[i] = maxBright;
                halB[i] = maxBright;
            }
            if(debug){Serial.println("Target: Pause, A: Max, B: Max");}
            Pause(2,halA,halB); // pause for 2 seconds
            

        break;
    default:
        // statements
        break;
    }





}

