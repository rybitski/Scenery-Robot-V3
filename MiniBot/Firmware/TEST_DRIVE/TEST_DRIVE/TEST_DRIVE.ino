#include <Boards.h>
#include <Firmata.h>
#include <FirmataConstants.h>
#include <FirmataDefines.h>
#include <FirmataMarshaller.h>
#include <FirmataParser.h>

// Mixed Mode Sample for Kangaroo
// Copyright (c) 2013 Dimension Engineering LLC
// See license.txt for license details.

#include <SoftwareSerial.h>
#include <Kangaroo.h>

// Arduino TX (pin 11) goes to Kangaroo S1
// Arduino RX (pin 10) goes to Kangaroo S2
// Arduino GND         goes to Kangaroo 0V
// Arduino 5V          goes to Kangaroo 5V (OPTIONAL, if you want Kangaroo to power the Arduino)
#define TX_PIN 5
#define RX_PIN 8

// Mixed mode channels on Kangaroo are, by default, 'D' and 'T'.
SoftwareSerial  SerialPort(RX_PIN, TX_PIN);
KangarooSerial  K(SerialPort);
KangarooChannel Drive(K, 'D');
KangarooChannel Turn(K, 'T');

int ledR = 11;             //common anode LED high=off low=on
int ledG = 10;
int ledB = 9;

void setup()
{
  SerialPort.begin(9600);
  SerialPort.listen();

    //LED----------------------
  pinMode(ledR, OUTPUT);
  pinMode(ledG, OUTPUT);
  pinMode(ledB, OUTPUT);

  digitalWrite(ledR, LOW);   //turn LEDs RED on startup
  digitalWrite(ledG, HIGH);
  digitalWrite(ledB, HIGH);
  
  Drive.start();
  Turn.start();

  
  digitalWrite(ledR, HIGH);   //turn LEDs RED on startup
  digitalWrite(ledG, LOW);
  digitalWrite(ledB, LOW);
  
  Drive.si(0);
  Turn.si(0);
}

void loop()
{
  digitalWrite(ledR, HIGH);   //turn LEDs RED on startup
  digitalWrite(ledG, HIGH);
  digitalWrite(ledB, HIGH);
  
  // Drive 500 ticks (relative to where we are right now), then wait 1 second.
  Drive.p(374).wait();

  digitalWrite(ledR, LOW);   //turn LEDs RED on startup
  digitalWrite(ledG, LOW);
  digitalWrite(ledB, LOW);
  
  delay(1000);
}
