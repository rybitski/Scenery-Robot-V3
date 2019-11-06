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

  Serial.begin(115200);

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
  Drive.p(0);
  Turn.p(0);

  Drive.p(2903,500);
}

void loop()
{
  Serial.println(Drive.getP().value());
}
