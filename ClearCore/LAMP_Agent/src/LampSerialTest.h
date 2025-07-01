/* LAMP Agent
This code is designed to receive a DMX signal and then transmit it to the clear core. 
*/ 

// Dependencies
#include <Arduino.h>
#include <SoftwareSerial.h> // Add SoftwareSerial

// Hardware Pins
// Pin 0 RX - by default for DMX Serial Library, uses hardware UART pins
// Pin 1 TX - by default for DMX Serial Library, uses hardware UART pins
#define softTxPin 7 // SoftwareSerial TX pin
#define softRxPin 8 // SoftwareSerial RX pin (unused but must be valid)

// DMX Variables

// Create SoftwareSerial object for transmitting on pin 7
SoftwareSerial softSerial(softRxPin, softTxPin); // RX = 8 (unused), TX = 7

static unsigned long lastStatusTime = 0;
#define agentBaudRate 38400
int pos = 1;
int speed = 1;


void setup() {
  // SoftwareSerial setup
    softSerial.begin(agentBaudRate); // Set baud rate as needed  
    Serial.begin(115200);
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastStatusTime >= 1000) {
//    softSerial.println("Serial communication Debugging Test");
    lastStatusTime = currentMillis;
    Serial.println("Agent Loop Active");
    softSerial.print("P:");
    softSerial.print(pos);
    softSerial.print(" S:");
    softSerial.println(speed);
    pos++;
    speed++;
  }
  delay(20); // Don't need to run the loop at max speed. 
}