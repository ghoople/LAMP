/* DMX Signal Generator

This code will send a DMX signal from the Arduino to a DMX device for testing. 

*/ 

bool debug = false; // Set to true to print debug messages to the serial monitor.

// Dependencies
#include <Arduino.h>
#include <DmxSimple.h>

// Define Hardware Pins
// 0 Should not be used, reserved for USB Coms
// 1 Sould not be used, reserved for USB Coms
#define dmxDePin 2 // Must be pin 2 based on board.
//#define dmxRxPin 3 // I don't need to use this pin in transmit only mode. Signals from shield might conflict with the debugger.  
#define dmxTxPin 4 // Must be pin 4 based on jumpers on the DMX board

// DMX Variables
// Set DMX shield to master mode
int channels = 10; // Number of DMX Channels needed for the test
int DmxStartAddress = 1; // 
int pos = 0;
int speed = 125;

// Serial Coms 
#define usbBaudRate 115200 // This can be slow.  

void setup() {

  // DMX Setup
    // Set the DMX to Principal (Master) mode
    pinMode(dmxDePin,OUTPUT);
    digitalWrite(dmxDePin,HIGH);

    // Set the pin the module will use to transmit data to the DMX Cable
    DmxSimple.usePin(dmxTxPin);

    // Set the maximum number of channels I have on my system. 
    DmxSimple.maxChannel(channels);

  // Serial Coms
    Serial.begin(usbBaudRate); // Hardware Serial on USB port
    if(debug){Serial.println("Setup complete");}
}

void loop() {
  if(pos>250){pos = 0;}
  if(speed>250){speed = 0;}

  DmxSimple.write(DmxStartAddress,pos);
  DmxSimple.write(DmxStartAddress+1,speed);

  pos++;
  speed++;

  delay(2000);
}

