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

// Function to initialize the DMX serial port, run in setup once. 
void setupDMX() {
    Serial0.begin(DMX_BAUD_RATE, SERIAL_8N2);
    Serial.print("DMX UART configured: 250000 baud, 8N2 on COM-0\r\n");
    dmxState = DMX_IDLE;
    dmxChannelIndex = 0;
    dmxFrameReady = false;
    dmxDataValid = false;
}

// Function to receive DMX data
void updateDMXReceiver() {
    static uint32_t lastByteTime = 0;
    const uint32_t BREAK_THRESHOLD_US = 100; // 100 microseconds, DMX spec is minimum 88 us, most controllers do 2x that
    static bool waitingForStartCode = true;
    while (Serial0.available()) {
        uint32_t now = micros();
        int receivedChar = Serial0.read();
        if (receivedChar < 0) {
            dmxState = DMX_IDLE;
            dmxDataValid = false; // Invalidate data on error
            waitingForStartCode = true;
            continue;
        }
        // Software timing-based BREAK detection
        if (now - lastByteTime > BREAK_THRESHOLD_US) {
            dmxState = DMX_IDLE;
            dmxChannelIndex = 0;
            dmxDataValid = false; // Invalidate data on break
            waitingForStartCode = true;
        }
        lastByteTime = now;
        switch (dmxState) {
            case DMX_IDLE:
                if (waitingForStartCode) {
                    if (receivedChar == DMX_START_CODE) {
                        dmxState = DMX_RECEIVING_DATA;
                        dmxChannelIndex = 1;
                        dmxBuffer[0] = receivedChar;
                        waitingForStartCode = false;
                    } else {
                        // Ignore bytes until we see a valid start code
                        break;
                    }
                }
                break;
            case DMX_RECEIVING_DATA:
                if (dmxChannelIndex <= DMX_MAX_CHANNELS) {
                    dmxBuffer[dmxChannelIndex] = (uint8_t)receivedChar;
                    dmxChannelIndex++;
                    // Buffer overrun protection
                    if (dmxChannelIndex > DMX_MAX_CHANNELS) {
                        dmxState = DMX_IDLE;
                        dmxDataValid = false;
                        waitingForStartCode = true;
                        break;
                    }
                    // Only process frame if we have enough data for all needed channels
                    if (dmxChannelIndex > (DMX_START_ADDRESS + DMX_CHANNELS_NEEDED - 1)) {
                        dmxState = DMX_FRAME_COMPLETE;
                        dmxFrameReady = true;
                        lastFrameTime = millis();
                        dmxDataValid = true;
                        waitingForStartCode = true;
                    }
                } else {
                    dmxState = DMX_IDLE;
                    dmxDataValid = false;
                    waitingForStartCode = true;
                }
                break;
            case DMX_FRAME_COMPLETE:
                dmxState = DMX_IDLE;
                waitingForStartCode = true;
                break;
        }
    }
}

// Function to extract the relevant information
void processDMXData() {
    positionChannel = getDMXChannel(DMX_START_ADDRESS);
    speedChannel = getDMXChannel(DMX_START_ADDRESS + 1);

    Serial.print("Buffer[1]="); Serial.print(dmxBuffer[1]);
    Serial.print(" Buffer[2]="); Serial.print(dmxBuffer[2]);
    Serial.print(" Buffer[3]="); Serial.print(dmxBuffer[3]);
    Serial.print(" Buffer[4]="); Serial.print(dmxBuffer[4]);
    Serial.print(" Buffer[5]="); Serial.println(dmxBuffer[5]);

}

uint8_t getDMXChannel(uint16_t channel) {
    if (channel >= 1 && channel <= DMX_MAX_CHANNELS && dmxDataValid) {
        return dmxBuffer[channel];
    }
    return 0;
}
