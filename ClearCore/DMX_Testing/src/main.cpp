#include <ClearCore.h>
#include <Arduino.h>
#include <dmx_functions.h>

// Variables to store last position
uint8_t positionLast = 0; // Position value ranges 0 -> 255
uint8_t speedLast = 0; // Speed value ranges 0->255

void setup() {
    // Initialize USB Serial for debugging
    Serial.begin(115200);
    
    // Give USB time to initialize
    delay(2000);
    
    Serial.print("=== ClearCore DMX Test Receiver ===\r\n");
    
    char startMsg[100];
    snprintf(startMsg, sizeof(startMsg), 
             "Listening for DMX channels %d (position) and %d (speed)\r\n", 
             DMX_START_ADDRESS, DMX_START_ADDRESS + 1);
    Serial.print(startMsg);
    
    // Configure COM-0 for DMX reception
    setupDMX();
    
    Serial.print("DMX receiver ready! Waiting for signal...\r\n");
}

void loop() {
    // Update DMX receiver
    updateDMXReceiver();
    
    // Process received DMX data, will update positionChannel and speedChannel (extern variables)
    if (dmxFrameReady) {
        processDMXData();
        dmxFrameReady = false;
    }
    
    // Check for DMX timeout and print a warning
    if (millis() - lastFrameTime > DMX_FRAME_TIMEOUT_MS) {
        if (dmxDataValid) {
            Serial.print("*** DMX signal lost! ***\r\n");
            dmxDataValid = false;
        }
    }

    if (positionChannel != positionLast || speedChannel != speedLast) {
        Serial.print("Change Detected: positionChannel=");
        Serial.print(positionChannel);
        Serial.print(", speedChannel="); 
        Serial.println(speedChannel);

        // Update tracking variables
        positionLast = positionChannel;
        speedLast = speedChannel;
    }
}