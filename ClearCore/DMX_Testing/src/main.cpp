#include <ClearCore.h>
#include <Arduino.h>
#include <dmx_functions.h>

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
    
    // Process received DMX data
    if (dmxFrameReady) {
        processDMXData();
        dmxFrameReady = false;
    }
    
    // Check for DMX timeout
    if (millis() - lastFrameTime > DMX_FRAME_TIMEOUT_MS) {
        if (dmxDataValid) {
            Serial.print("*** DMX signal lost! ***\r\n");
            dmxDataValid = false;
            positionChannel = 0;
            speedChannel = 0;
        }
    }

    uint8_t position = getDMXChannel(DMX_START_ADDRESS);
    uint8_t speed = getDMXChannel(DMX_START_ADDRESS + 1);

    Serial.print("Current pos: ");
    Serial.print(position);
    Serial.print(" speed: ");
    Serial.println(speed);
    
    delay(10); // Small delay
}