/* LAMP Agent
This code is designed to receive a DMX signal and then transmit it to the clear core. 
*/ 

// Dependencies
#include <Arduino.h>
#include <DMXSerial.h> // Use DMXSerial for DMX receive
#include <SoftwareSerial.h> // Add SoftwareSerial

// Define Hardware Pins

// NEED TO DOUBLE CHECK AND DEBUG THESE


// #define dmxDePin 2 // I think this is jumper set, should be able to delete this line. 

#define dmxRxPin 0 // DMX receive pin (hardware RX)
#define dmxTxPin 1 // Not used in receive mode, but noted for reference

#define softTxPin 7 // SoftwareSerial TX pin

// DMX Variables
int channels = 2; // Number of DMX Channels needed for this test
int DmxStartAddress = 1; // Channel 1 address
int lastPos = -1;
int lastSpeed = -1;

// Create SoftwareSerial object for transmitting on pin 7
SoftwareSerial softSerial(-1, softTxPin); // RX not used, TX = 7
#define agentBaudRate 38400

static unsigned long lastStatusTime = 0;

void setup() {
  // DMX Setup
    // Initialize DMXSerial in receive mode
    DMXSerial.init(DMXReceiver);

  // SoftwareSerial setup
    softSerial.begin(agentBaudRate); // Set baud rate as needed
  
  // Might need this, but the jumper probably does it for me.
  // pinMode(dmxDePin, OUTPUT);
  // digitalWrite(dmxDePin, LOW);    // LOW = receive mode (CRITICAL!)
}

void loop() {
  // Read DMX values from the specified start address
  int pos = DMXSerial.read(DmxStartAddress);
  int speed = DMXSerial.read(DmxStartAddress + 1);

  unsigned long currentMillis = millis();
  if (currentMillis - lastStatusTime >= 3000) {
    softSerial.println("Serial communication Debugging Test");
    lastStatusTime = currentMillis;
  }

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