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

void setupDMX() {
    Serial0.begin(DMX_BAUD_RATE, SERIAL_8N2);
    Serial.print("DMX UART configured: 250000 baud, 8N2 on COM-0\r\n");
    dmxState = DMX_IDLE;
    dmxChannelIndex = 0;
    dmxFrameReady = false;
    dmxDataValid = false;
}

void updateDMXReceiver() {
    static uint32_t lastByteTime = 0;
    const uint32_t BREAK_THRESHOLD_US = 100; // 100 microseconds, adjust as needed
    while (Serial0.available()) {
        uint32_t now = micros();
        int receivedChar = Serial0.read();
        if (receivedChar < 0) {
            dmxState = DMX_IDLE;
            dmxDataValid = false; // Invalidate data on error
            continue;
        }
        // Software timing-based BREAK detection
        if (now - lastByteTime > BREAK_THRESHOLD_US) {
            // BREAK detected - this byte should be start code
            dmxState = DMX_IDLE;
            dmxChannelIndex = 0;
            dmxDataValid = false; // Invalidate data on break
        }
        lastByteTime = now;
        switch (dmxState) {
            case DMX_IDLE:
                if (receivedChar == DMX_START_CODE) {
                    dmxState = DMX_RECEIVING_DATA;
                    dmxChannelIndex = 1;
                    dmxBuffer[0] = receivedChar;
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
                        break;
                    }
                    if (dmxChannelIndex > (DMX_START_ADDRESS + DMX_CHANNELS_NEEDED - 1)) {
                        dmxState = DMX_FRAME_COMPLETE;
                        dmxFrameReady = true;
                        lastFrameTime = millis();
                        dmxDataValid = true;
                    }
                } else {
                    dmxState = DMX_IDLE;
                    dmxDataValid = false;
                }
                break;
            case DMX_FRAME_COMPLETE:
                dmxState = DMX_IDLE;
                break;
        }
    }
}

void processDMXData() {
    positionChannel = getDMXChannel(DMX_START_ADDRESS);
    speedChannel = getDMXChannel(DMX_START_ADDRESS + 1);
    static uint32_t frameCount = 0;
    frameCount++;
    char dataMsg[100];
    snprintf(dataMsg, sizeof(dataMsg),
             "Frame %lu: Position=%3d, Speed=%3d\r\n",
             frameCount, positionChannel, speedChannel);
    Serial.print(dataMsg);
    if (frameCount % 50 == 0) {
        Serial.print("--- DMX Reception OK ---\r\n");
    }
}

uint8_t getDMXChannel(uint16_t channel) {
    if (channel >= 1 && channel <= DMX_MAX_CHANNELS && dmxDataValid) {
        return dmxBuffer[channel];
    }
    return 0;
}
