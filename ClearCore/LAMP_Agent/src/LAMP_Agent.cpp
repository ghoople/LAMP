/* LAMP Agent
This code is designed to receive a DMX signal and then transmit it to the clear core. 
*/ 

// Dependencies
#include <Arduino.h>
#include <DMXSerial.h> // Use DMXSerial for DMX receive
#include <SoftwareSerial.h> // Add SoftwareSerial

// CTC-DRA-10-1 NON ISOLATED DMX-RDM SHIELD JUMPER INSTRUCTIONS
// JUMPER 1: EN        " Shield is enabled (Need to swap to !EN to program it via USB)
// JUMPER 2: SLAVE     " Set to "Slave" side, puts the shield in receive mode. 
// JUMPER 3: TX-uart   " Use UART for transmitting data
// JUMPER 4: RX-uart   " Use UART for receiving data

// Hardware Pins
// Pin 0 RX - by default for DMX Serial Library, uses hardware UART pins
// Pin 1 TX - by default for DMX Serial Library, uses hardware UART pins
#define softTxPin 7 // SoftwareSerial TX pin

// DMX Variables
int channels = 2; // Number of DMX Channels to read
int DmxStartAddress = 1; // First channel address
int lastPos = -1;
int lastSpeed = -1;

// Create SoftwareSerial object for transmitting on pin 7
SoftwareSerial softSerial(-1, softTxPin); // RX not used, TX = 7
#define agentBaudRate 38400

static unsigned long lastStatusTime = 0;

void setup() {
  // Initialize DMXSerial in receive mode
    DMXSerial.init(DMXReceiver); 

  // SoftwareSerial setup
    softSerial.begin(agentBaudRate); // Set baud rate as needed  
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