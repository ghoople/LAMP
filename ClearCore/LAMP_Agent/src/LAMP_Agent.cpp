/* LAMP Agent
This code is designed to receive a DMX signal and then transmit it to the clear core. 
*/ 

// Dependencies
#include <Arduino.h>
#include <DMXSerial.h> // Use DMXSerial for DMX receive
#include <SoftwareSerial.h> // Add SoftwareSerial

// Define Hardware Pins

// NEED TO DOUBLE CHECK AND DEBUG THESE

// 0 Should not be used, reserved for USB Coms
// 1 Sould not be used, reserved for USB Coms
#define dmxDePin 2 // Not needed for receive mode, but kept for reference
#define dmxRxPin 3 // DMX receive pin (hardware RX)
#define dmxTxPin 4 // Not used in receive mode
#define softTxPin 7 // SoftwareSerial TX pin

// DMX Variables
int channels = 2; // Number of DMX Channels needed for the test
int DmxStartAddress = 1; // 
int lastPos = -1;
int lastSpeed = -1;

// Create SoftwareSerial object for transmitting on pin 7
SoftwareSerial softSerial(-1, softTxPin); // RX not used, TX = 7

void setup() {
  // DMX Setup
    // Initialize DMXSerial in receive mode
    DMXSerial.init(DMXReceiver);

  // SoftwareSerial setup
    softSerial.begin(38400); // Set baud rate as needed
  
  // Might need this, but the jumper probably does it for me.
  // pinMode(dmxDePin, OUTPUT);
  // digitalWrite(dmxDePin, LOW);    // LOW = receive mode (CRITICAL!)
}

void loop() {
  // Read DMX values from the specified start address
  int pos = DMXSerial.read(DmxStartAddress);
  int speed = DMXSerial.read(DmxStartAddress + 1);

  if (pos != lastPos || speed != lastSpeed) {
    softSerial.print("P:");
    softSerial.print(pos);
    softSerial.print(" S:");
    softSerial.println(speed);

    lastPos = pos;
    lastSpeed = speed;
  }

  delay(20); // Don't need to run the loop at max speed. 
}