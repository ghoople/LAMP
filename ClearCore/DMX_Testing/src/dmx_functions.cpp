// dmx_functions.cpp - Simplified robust version
#include <dmx_functions.h>
#include <ClearCore.h>
#include <Arduino.h>

// DMX variables
uint8_t dmxBuffer[DMX_MAX_CHANNELS + 1];
volatile DMXState dmxState = DMX_IDLE;
volatile uint16_t dmxChannelIndex = 0;
volatile bool dmxFrameReady = false;
volatile uint32_t lastFrameTime = 0;
volatile bool dmxDataValid = false;
uint8_t positionChannel = 0;
uint8_t speedChannel = 0;

// Simple timing for BREAK detection
volatile uint32_t lastByteTime = 0;
volatile uint32_t frameCount = 0;
volatile uint32_t errorCount = 0;

void setupDMX() {
    Serial0.begin(DMX_BAUD_RATE, SERIAL_8N2);
    Serial.print("DMX UART configured: 250000 baud, 8N2 on COM-0\r\n");
    
    dmxState = DMX_IDLE;
    dmxChannelIndex = 0;
    dmxFrameReady = false;
    dmxDataValid = false;
    lastByteTime = micros();
    frameCount = 0;
    errorCount = 0;
    
    // Clear buffer
    memset(dmxBuffer, 0, sizeof(dmxBuffer));
}

void updateDMXReceiver() {
    uint32_t now = micros();
    
    while (Serial0.available()) {
        int receivedByte = Serial0.read();
        
        if (receivedByte < 0) {
            // Read error
            dmxState = DMX_IDLE;
            dmxDataValid = false;
            errorCount++;
            continue;
        }
        
        uint8_t data = (uint8_t)receivedByte;
        uint32_t timeSinceLastByte = now - lastByteTime;
        
        // Simple BREAK detection: gap > 150µs indicates new frame
        if (timeSinceLastByte > 150) {
            // Potential BREAK detected - reset for new frame
            dmxState = DMX_IDLE;
            dmxChannelIndex = 0;
        }
        
        lastByteTime = now;
        
        switch (dmxState) {
            case DMX_IDLE:
                // Look for start code
                if (data == DMX_START_CODE) {
                    dmxState = DMX_RECEIVING_DATA;
                    dmxBuffer[0] = data;
                    dmxChannelIndex = 1;
                    // Don't mark as valid yet - need more data
                }
                break;
                
            case DMX_RECEIVING_DATA:
                if (dmxChannelIndex <= DMX_MAX_CHANNELS) {
                    dmxBuffer[dmxChannelIndex] = data;
                    dmxChannelIndex++;
                    
                    // Mark frame as ready when we have our required channels
                    if (dmxChannelIndex > (DMX_START_ADDRESS + DMX_CHANNELS_NEEDED - 1)) {
                        if (!dmxFrameReady) { // Only count new frames
                            dmxFrameReady = true;
                            dmxDataValid = true;
                            lastFrameTime = millis();
                            frameCount++;
                        }
                    }
                } else {
                    // Buffer overflow
                    dmxState = DMX_IDLE;
                    dmxDataValid = false;
                    errorCount++;
                }
                break;
                
            case DMX_FRAME_COMPLETE:
                // This state isn't used in this simplified version
                dmxState = DMX_RECEIVING_DATA;
                break;
        }
        
        now = micros(); // Update time for next iteration
    }
}

void processDMXData() {
    if (!dmxFrameReady || !dmxDataValid) {
        return;
    }
    
    // Clear frame ready flag
    dmxFrameReady = false;
    
    uint8_t newPosition = getDMXChannel(DMX_START_ADDRESS);
    uint8_t newSpeed = getDMXChannel(DMX_START_ADDRESS + 1);
    
    // Always show what we received
    Serial.print("Frame #"); Serial.print(frameCount);
    Serial.print(": Ch1="); Serial.print(dmxBuffer[1]);
    Serial.print(" Ch2="); Serial.print(dmxBuffer[2]);
    Serial.print(" Ch3="); Serial.print(dmxBuffer[3]);
    Serial.print(" Ch4="); Serial.print(dmxBuffer[4]);
    Serial.print(" Ch5="); Serial.print(dmxBuffer[5]);
    Serial.print(" ("); Serial.print(dmxChannelIndex); Serial.print(" total)");
    
    // Check for changes
    if (newPosition != positionChannel || newSpeed != speedChannel) {
        Serial.print(" >>> CHANGE: Pos="); Serial.print(newPosition);
        Serial.print(" Speed="); Serial.print(newSpeed);
        
        positionChannel = newPosition;
        speedChannel = newSpeed;
    }
    
    Serial.println();
}

uint8_t getDMXChannel(uint16_t channel) {
    if (channel >= 1 && channel <= DMX_MAX_CHANNELS && 
        dmxDataValid && channel < dmxChannelIndex) {
        return dmxBuffer[channel];
    }
    return 0;
}

void printDMXStatus() {
    Serial.print("=== DMX Status ===\r\n");
    Serial.print("State: ");
    switch(dmxState) {
        case DMX_IDLE: Serial.print("IDLE"); break;
        case DMX_RECEIVING_DATA: Serial.print("RECEIVING"); break;
        case DMX_FRAME_COMPLETE: Serial.print("COMPLETE"); break;
    }
    Serial.print(", Channels received: "); Serial.print(dmxChannelIndex);
    Serial.print(", Data valid: "); Serial.print(dmxDataValid ? "YES" : "NO");
    Serial.print(", Frame ready: "); Serial.print(dmxFrameReady ? "YES" : "NO");
    Serial.print("\r\nTotal frames: "); Serial.print(frameCount);
    Serial.print(", Errors: "); Serial.print(errorCount);
    
    if (dmxDataValid) {
        Serial.print("\r\nCurrent values: Position="); Serial.print(positionChannel);
        Serial.print(", Speed="); Serial.print(speedChannel);
    }
    
    Serial.print("\r\nLast byte time: "); Serial.print((micros() - lastByteTime) / 1000);
    Serial.println("ms ago");
}