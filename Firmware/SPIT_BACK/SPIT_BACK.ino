#include <Boards.h>
#include <Firmata.h>
#include <FirmataConstants.h>
#include <FirmataDefines.h>
#include <FirmataMarshaller.h>
#include <FirmataParser.h>

//FIRMWARE FOR UVA STAGE ROBOT
//REMOTE CONTROL MODE

//Software Serial Setup------------------------------------
#include <SoftwareSerial.h>
SoftwareSerial SWSerial(5,4); // Available software serial pins

//Motor Controller-------------------------
#include <Sabertooth.h>
Sabertooth ST1(128, SWSerial);        //Sabertooth 2x60 Drive Motors
Sabertooth Lift(130, SWSerial);        //SyRen 25 Lift Motor

//Encoder Buffer---------------------------
#include <SPI.h>
#include <Encoder_Buffer.h>

#define EncoderCS1 7        //Slave Select for encoder1
#define EncoderCS2 8        //Slave Select for encoder2

long encoder1Reading = 0;
long encoder2Reading = 0;

Encoder_Buffer Encoder1(EncoderCS1);
Encoder_Buffer Encoder2(EncoderCS2);

//I/O----------------------------------------
int voltagePin = A0;      //voltage divider to monitor battery state
int liftFeedback = A1;    //potintiometer connected to linear actuator
int eStop = 2;            //checks estop status from estop controller
int brake = 3;            //controls motor brakes
int ledR = 9;             //common anode LED high=off low=on
int ledG = 10;
int ledB = 11;

// Motor Global Variables--------------------------
byte chargeLevel = 0;    //stores level of charge 0-100%
bool eStopFlag = true;

byte liftPos = 0;   //stores lift position in inches 200=2inches
int averageL;       //Lift stuff
int samplesL;       //Lift stuff

int target = 50;
int offset = 15;    //increase this to stop lift jitter

int commandCode = 0;
int packetSize = 0;

//Linux Support ---------------------------------- 
#include <Bridge.h>
#include <stdio.h>
#include <Process.h>

Process p;
 
void setup() {
  //Serial configuration
  Serial.begin(115200);
  //while(!Serial); //comment this line to remove the pause
  Serial.println("starting setup...");

  SWSerial.begin(2400);             //start sabertooth with software serial
  Sabertooth::autobaud(SWSerial);

  //Encoder------------------
  SPI.begin();
  Encoder1.initEncoder();
  Encoder2.initEncoder();

  //LED----------------------
  pinMode(ledR, OUTPUT);
  pinMode(ledG, OUTPUT);
  pinMode(ledB, OUTPUT);

  digitalWrite(ledR, LOW);   //turn LEDs RED on startup
  digitalWrite(ledG, HIGH);
  digitalWrite(ledB, HIGH);

  // Misc -------------------
  pinMode(brake, OUTPUT);
  pinMode(eStop, INPUT);

  // Start using the Bridge and Python script
  Serial.println("Starting bridge...");
  Bridge.begin();

  Serial.println("running python script...");
  p.runShellCommandAsynchronously("python -u /usr/lib/andyCode/SPIT_OUT.py");
  Serial.println("command sent!");

  Serial.println("waiting for data influx...");
  digitalWrite(ledG, LOW); //turn LEDs YELLOW while waiting
  while(!p.available());

  digitalWrite(ledR, HIGH);   //turn LEDs GREEN when leaving setup
  Serial.println("setup complete");
}

void loop() {
  readEncoders();
  //p.println("i'm a freakin pizza");
  //p.flush();
  //String enc1_str = String(encoder1Reading);
  //const char* enc1 = enc1_str.c_str();
  //String enc2_str = String(encoder2Reading);
  //const char* enc2 = enc2_str.c_str();
  p.println(encoder1Reading);
  p.println(encoder2Reading);
  if(p.available()){
    Serial.print("Bridge encoder 1: ");
    Serial.println(getNextNum());
    Serial.print("Bridge encoder 2: ");
    Serial.println(getNextNum());
  }
  Serial.print("Local encoder 1: ");
  Serial.println(encoder1Reading);
  Serial.print("Local encoder 2: ");
  Serial.println(encoder2Reading);
  p.flush();
}

//-------------------Get an int off the Bridge-----------------------------------------------
int getNextNum(){
  int res = 0;
  if(p.available()){
    res = p.parseInt(); //offset back to +/- range
  }
  return res;
}
void readEncoders() {
  encoder1Reading = Encoder1.readEncoder();         //Read Encoder
  encoder2Reading = Encoder2.readEncoder();           //Read Encoder
  //Encoder1.clearEncoderCount();                 // Clear Encoder
}
