// Single file DMX receiver - Arduino + native BREAK detection
#include <ClearCore.h>
#include <Arduino.h>

// DMX Configuration
#define DMX_START_ADDRESS       1
#define DMX_CHANNELS_NEEDED     2
#define DMX_FRAME_TIMEOUT_MS    1000
#define DMX_START_CODE          0x00

// DMX State Variables
uint8_t dmxBuffer[513];  // 512 channels + start code
uint8_t positionChannel = 0;
uint8_t speedChannel = 0;
uint8_t positionLast = 0;
uint8_t speedLast = 0;

// Frame tracking
uint32_t frameCount = 0;
uint32_t lastFrameTime = 0;
bool dmxDataValid = false;

// Function declarations
void updateDMXReceiver();
void printStatus();

void setup() {
    // Arduino-style setup
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("=== ClearCore DMX Receiver (Testing BREAK) ===");
    Serial.print("Listening for DMX channels ");
    Serial.print(DMX_START_ADDRESS);
    Serial.print(" (position) and ");
    Serial.print(DMX_START_ADDRESS + 1);
    Serial.println(" (speed)");
    
    // Use Arduino wrapper but try to access native features
    Serial0.begin(250000, SERIAL_8N2);
    
    // Try to configure the underlying connector for BREAK detection
    // Test different mode values that might work
    ConnectorCOM0.Speed(250000);
    ConnectorCOM0.StopBits(2);
    ConnectorCOM0.PortOpen();
    
    // Initialize variables
    memset(dmxBuffer, 0, sizeof(dmxBuffer));
    frameCount = 0;
    lastFrameTime = millis();
    dmxDataValid = false;
    
    Serial.println("Mixed Arduino/Native UART setup complete");
    
    // Print what BREAK_DETECTED value is
    Serial.print("SerialBase::BREAK_DETECTED = 0x");
    Serial.print(SerialBase::BREAK_DETECTED, HEX);
    Serial.print(" (");
    Serial.print(SerialBase::BREAK_DETECTED);
    Serial.println(")");
    
    // Test if EOB constant exists
    Serial.print("SerialBase::EOB = 0x");
    Serial.print(SerialBase::EOB, HEX);
    Serial.print(" (");
    Serial.print(SerialBase::EOB);
    Serial.println(")");
}

void loop() {
    // Update DMX receiver
    updateDMXReceiver();
    
    // Check for timeout
    if (dmxDataValid && (millis() - lastFrameTime > DMX_FRAME_TIMEOUT_MS)) {
        Serial.println("*** DMX signal lost! ***");
        dmxDataValid = false;
    }
    
    // Check for changes
    if (positionChannel != positionLast || speedChannel != speedLast) {
        Serial.print(">>> CHANGE: Position ");
        Serial.print(positionLast);
        Serial.print(" -> ");
        Serial.print(positionChannel);
        Serial.print(", Speed ");
        Serial.print(speedLast);
        Serial.print(" -> ");
        Serial.println(speedChannel);
        
        positionLast = positionChannel;
        speedLast = speedChannel;
    }
    
    // Print status every 5 seconds
    static uint32_t lastStatus = 0;
    if (millis() - lastStatus > 5000) {
        printStatus();
        lastStatus = millis();
    }
}

void updateDMXReceiver() {
    static bool waitingForBreak = true;
    static uint16_t channelIndex = 0;
    static uint32_t debugCount = 0;
    
    // Use native connector that we properly configured
    while (ConnectorCOM0.CharPeek() != SerialBase::EOB) {
        int16_t receivedChar = ConnectorCOM0.CharGet();
        
        // Check for valid character
        if (receivedChar < 0) {
            continue;
        }
        
        debugCount++;
        
        // Debug: Print characters occasionally with BREAK check
        if (debugCount % 30 == 0) {
            Serial.print("Native: 0x");
            Serial.print(receivedChar, HEX);
            Serial.print(" (");
            Serial.print(receivedChar);
            Serial.print(")");
            if (receivedChar == SerialBase::BREAK_DETECTED) {
                Serial.println(" <<< BREAK!");
            } else {
                Serial.println("");
            }
        }
        
        // CRITICAL: Test for hardware BREAK detection
        if (receivedChar == SerialBase::BREAK_DETECTED) {
            Serial.println("*** HARDWARE BREAK DETECTED! ***");
            waitingForBreak = false;
            channelIndex = 0;
            continue;
        }
        
        // Also test for other common BREAK values
        if (receivedChar > 255) {  // BREAK might be > 8-bit value
            Serial.print("*** POSSIBLE BREAK (>255): 0x");
            Serial.print(receivedChar, HEX);
            Serial.println(" ***");
            waitingForBreak = false;
            channelIndex = 0;
            continue;
        }
        
        uint8_t data = (uint8_t)receivedChar;
        
        // If waiting for BREAK, ignore normal data
        if (waitingForBreak) {
            // But if we see a start code, process it anyway
            if (data == DMX_START_CODE) {
                Serial.println("Start code found - forcing frame start");
                waitingForBreak = false;
                channelIndex = 0;
            } else {
                continue;
            }
        }
        
        // DMX frame processing
        if (channelIndex == 0) {
            if (data == DMX_START_CODE) {
                dmxBuffer[0] = data;
                channelIndex = 1;
                if (debugCount % 100 == 0) {
                    Serial.println("✓ Start code");
                }
            } else {
                waitingForBreak = true;
            }
        } 
        else if (channelIndex <= 512) {
            dmxBuffer[channelIndex] = data;
            channelIndex++;
            
            if (channelIndex >= (DMX_START_ADDRESS + DMX_CHANNELS_NEEDED)) {
                frameCount++;
                lastFrameTime = millis();
                dmxDataValid = true;
                
                positionChannel = dmxBuffer[DMX_START_ADDRESS];
                speedChannel = dmxBuffer[DMX_START_ADDRESS + 1];
                
                if (debugCount % 100 == 0) {
                    Serial.print("✓ Frame: P=");
                    Serial.print(positionChannel);
                    Serial.print(" S=");
                    Serial.println(speedChannel);
                }
                
                // Continue collecting more channels but mark as ready
            }
        }
    }
}

void printStatus() {
    Serial.print("=== DMX Status === Frames: ");
    Serial.print(frameCount);
    Serial.print(", Valid: ");
    Serial.print(dmxDataValid ? "YES" : "NO");
    Serial.print(", Position: ");
    Serial.print(positionChannel);
    Serial.print(", Speed: ");
    Serial.println(speedChannel);
    
    // Show first few channels for debugging
    if (dmxDataValid) {
        Serial.print("Debug: Ch1=");
        Serial.print(dmxBuffer[1]);
        Serial.print(" Ch2=");
        Serial.print(dmxBuffer[2]);
        Serial.print(" Ch3=");
        Serial.print(dmxBuffer[3]);
        Serial.print(" Ch4=");
        Serial.print(dmxBuffer[4]);
        Serial.print(" Ch5=");
        Serial.println(dmxBuffer[5]);
    }
}