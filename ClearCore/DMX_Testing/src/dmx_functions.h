#ifndef DMX_FUNCTIONS_H
#define DMX_FUNCTIONS_H

#include <stdint.h>
#include <Arduino.h>

// DMX Protocol Constants
#define DMX_BAUD_RATE           250000
#define DMX_MAX_CHANNELS        512
#define DMX_START_CODE          0x00
#define DMX_FRAME_TIMEOUT_MS    1000
#define DMX_START_ADDRESS       1
#define DMX_CHANNELS_NEEDED     2

// DMX Reception State Machine
enum DMXState {
    DMX_IDLE,
    DMX_RECEIVING_DATA,
    DMX_FRAME_COMPLETE
};

// Global DMX variables (extern for use in main.cpp)
extern uint8_t dmxBuffer[DMX_MAX_CHANNELS + 1];
extern volatile DMXState dmxState;
extern volatile uint16_t dmxChannelIndex;
extern volatile bool dmxFrameReady;
extern volatile uint32_t lastFrameTime;
extern volatile bool dmxDataValid;
extern uint8_t positionChannel;
extern uint8_t speedChannel;

void setupDMX();
void updateDMXReceiver();
void processDMXData();
uint8_t getDMXChannel(uint16_t channel);

#endif // DMX_FUNCTIONS_H
