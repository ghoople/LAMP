// dmx_functions.cpp - Fixed frame synchronization
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

// Frame tracking
volatile uint32_t lastByteTime = 0;
volatile uint32_t frameCount = 0;
volatile uint32_t errorCount = 0;
volatile uint16_t completedFrameSize = 0; // Size of last completed frame

// Debug variables
volatile uint16_t lastFrameSizes[10];
volatile uint8_t frameSizeIndex = 0;
volatile uint32_t breakGaps[10];
volatile uint8_t breakGapIndex = 0;

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
    completedFrameSize = 0;
    
    // Clear buffers and arrays
    memset(dmxBuffer, 0, sizeof(dmxBuffer));
    memset((void*)lastFrameSizes, 0, sizeof(lastFrameSizes));
    memset((void*)breakGaps, 0, sizeof(breakGaps));
}

void updateDMXReceiver() {
    uint32_t now = micros();
    
    while (Serial0.available()) {
        int receivedByte = Serial0.read();
        
        if (receivedByte < 0) {
            // Read error - reset completely
            dmxState = DMX_IDLE;
            dmxChannelIndex = 0;
            dmxDataValid = false;
            errorCount++;
            continue;
        }
        
        uint8_t data = (uint8_t)receivedByte;
        uint32_t timeSinceLastByte = now - lastByteTime;
        
        // BREAK detection: gap > 150µs indicates new frame
        if (timeSinceLastByte > 150) {
            // Log break gap
            breakGaps[breakGapIndex] = timeSinceLastByte;
            breakGapIndex = (breakGapIndex + 1) % 10;
            
            // Complete previous frame if it had enough data
            if (dmxState == DMX_RECEIVING_DATA && dmxChannelIndex >= (DMX_START_ADDRESS + DMX_CHANNELS_NEEDED)) {
                // Atomically complete the frame
                completedFrameSize = dmxChannelIndex;
                lastFrameSizes[frameSizeIndex] = completedFrameSize;
                frameSizeIndex = (frameSizeIndex + 1) % 10;
                
                dmxFrameReady = true;
                dmxDataValid = true;
                lastFrameTime = millis();
                frameCount++;
            }
            
            // Reset for new frame - ALWAYS reset on BREAK
            dmxState = DMX_IDLE;
            dmxChannelIndex = 0;
        }
        
        lastByteTime = now;
        
        // Process the current byte based on state
        switch (dmxState) {
            case DMX_IDLE:
                // ONLY accept start code after a BREAK
                if (data == DMX_START_CODE) {
                    dmxState = DMX_RECEIVING_DATA;
                    dmxBuffer[0] = data;
                    dmxChannelIndex = 1; // Next byte will be channel 1
                } else {
                    // Not a valid start - stay in IDLE
                    // This prevents false synchronization
                }
                break;
                
            case DMX_RECEIVING_DATA:
                if (dmxChannelIndex <= DMX_MAX_CHANNELS) {
                    dmxBuffer[dmxChannelIndex] = data;
                    dmxChannelIndex++;
                } else {
                    // Buffer overflow - reset
                    dmxState = DMX_IDLE;
                    dmxChannelIndex = 0;
                    dmxDataValid = false;
                    errorCount++;
                }
                break;
                
            case DMX_FRAME_COMPLETE:
                // Should not reach here in this implementation
                dmxState = DMX_IDLE;
                break;
        }
        
        now = micros();
    }
}

void processDMXData() {
    if (!dmxFrameReady || !dmxDataValid) {
        return;
    }
    
    // Clear frame ready flag atomically
    dmxFrameReady = false;
    
    // Use the completed frame size, not the current receiving position
    uint16_t frameSize = completedFrameSize;
    
    // Extract channel data safely
    uint8_t newPosition = getDMXChannel(DMX_START_ADDRESS);
    uint8_t newSpeed = getDMXChannel(DMX_START_ADDRESS + 1);
    
    // Display frame info with correct size
    Serial.print("Frame #"); Serial.print(frameCount);
    Serial.print(" ("); Serial.print(frameSize); Serial.print(" ch): ");
    Serial.print("Ch1="); Serial.print(dmxBuffer[1]);
    Serial.print(" Ch2="); Serial.print(dmxBuffer[2]);
    Serial.print(" Ch3="); Serial.print(dmxBuffer[3]);
    Serial.print(" Ch4="); Serial.print(dmxBuffer[4]);
    Serial.print(" Ch5="); Serial.print(dmxBuffer[5]);
    
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
        dmxDataValid && channel < completedFrameSize) {
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
    Serial.print(", Current RX: "); Serial.print(dmxChannelIndex);
    Serial.print(", Last frame: "); Serial.print(completedFrameSize);
    Serial.print(", Valid: "); Serial.print(dmxDataValid ? "YES" : "NO");
    Serial.print(", Ready: "); Serial.print(dmxFrameReady ? "YES" : "NO");
    Serial.print("\r\nTotal frames: "); Serial.print(frameCount);
    Serial.print(", Errors: "); Serial.print(errorCount);
    
    if (dmxDataValid) {
        Serial.print("\r\nCurrent values: Position="); Serial.print(positionChannel);
        Serial.print(", Speed="); Serial.print(speedChannel);
    }
    
    // Show frame size history
    Serial.print("\r\nLast 10 frame sizes: ");
    for (int i = 0; i < 10; i++) {
        Serial.print(lastFrameSizes[i]); 
        if (i < 9) Serial.print(",");
    }
    
    // Show break gap history  
    Serial.print("\r\nLast 10 break gaps (µs): ");
    for (int i = 0; i < 10; i++) {
        Serial.print(breakGaps[i]); 
        if (i < 9) Serial.print(",");
    }
    
    Serial.print("\r\nLast byte: "); Serial.print((micros() - lastByteTime) / 1000);
    Serial.println("ms ago");
}