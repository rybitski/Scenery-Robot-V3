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

  //Motor Control-------------
  /*so 2400 is the only Baud rate that both controllers will listen to simultaneously.
    The Sabertooth saves the baud rate in EEPROM, the syrens baud is set with the autobaud command

    //    To change your Sabertooth back to its default, you must *be at the baud rate you've
    //    set the Sabertooth to*, and then call ST.setBaudRate(9600)
    SabertoothTXPinSerial.begin(9600);
    ST.setBaudRate(2400);
    SabertoothTXPinSerial.end();

    //    OK, we're at 2400. Let's talk to the Sabertooth at that speed.
    SabertoothTXPinSerial.begin(2400);
  */
  
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
  p.runShellCommandAsynchronously("python /usr/lib/andyCode/Middleware_for_Controller.py");
  Serial.println("command sent!");

  Serial.println("waiting for data influx...");
  digitalWrite(ledG, LOW); //turn LEDs YELLOW while waiting
  while(!p.available());

  digitalWrite(ledR, HIGH);   //turn LEDs GREEN when leaving setup
  Serial.println("setup complete");
}
 
void loop() {
  //check to see if emergency stop was activated (updates flag)
  estopStatus();

  //initialize control values to implicit zeros
  int rightX = 0;
  int rightY = 0;
  int leftY = 0;
  int rightTrigger = 0;
  int dPad = 0;
  int aButton = 0;
  int rightBumper = 0;
  int leftBumper = 0;

  //If we are not in an e-stop condition
  if(eStopFlag == false){
    // GET the values from the python script----------------------------
    if(p.available()){
      rightX = getNextNum();
      Serial.print("rightX: ");
      Serial.println(rightX);
  
      rightY = getNextNum();
      Serial.print("rightY: ");
      Serial.println(rightY);
  
      leftY = getNextNum();
      Serial.print("leftY: ");
      Serial.println(leftY);
  
      rightTrigger = getNextNum();
      Serial.print("rightTrigger: ");
      Serial.println(rightTrigger);
  
      dPad = getNextNum();
      Serial.print("dPad: ");
      Serial.println(dPad);
  
      aButton = getNextNum();
      Serial.print("aButton: ");
      Serial.println(aButton);
  
      rightBumper = getNextNum();
      Serial.print("rightBumper: ");
      Serial.println(rightBumper);
  
      leftBumper = getNextNum();
      Serial.print("leftBumper: ");
      Serial.println(leftBumper);
    }
    

    //if the bumbers are both pressed, explicit break
    if(leftBumper == 1 && rightBumper == 1){
      digitalWrite(brake, LOW);  //remove power to the brakes to engage them.
      ST1.motor(1,0);
      ST1.motor(2,0);
      Serial.println("break condition");
    }
    //otherwise, if we have the right trigger down, execute drive
    else if (rightTrigger == 1){
      digitalWrite(brake, HIGH);  //send power to the brakes to disengauge them.
        ST1.turn(rightY);
        ST1.drive(rightX);
        Serial.println("drive condition");
    }
    //if neither, if aButton is down, execute a lift operation from dPad data
    else if(aButton ==1){
      Serial.println("lift condition");
      if(dPad==1){
        Lift.motor(40);
      }
      else if (dPad == 3){
        Lift.motor(-40);
      }
      else if (dPad == 0){
        Lift.motor(0);
      }
    }
    //if none, implicit break
    else{
      digitalWrite(brake, LOW);  //remove power to the brakes to engage them.
      ST1.motor(1,0);
      ST1.motor(2,0);
      Serial.println("implicit break condition");
    }
  }
  //if E-STOP is triggered, stop immediately
  else{
    digitalWrite(brake, LOW);  //remove power to the brakes to engage them.
    ST1.motor(1,0);
    ST1.motor(2,0);
  }
  //flush remaining data on Bridge if any
  p.flush();
}


void readEncoders() {
  encoder1Reading = Encoder1.readEncoder();         //Read Encoder
  encoder2Reading = Encoder2.readEncoder();           //Read Encoder
  //Encoder1.clearEncoderCount();                 // Clear Encoder
}

//-----------------Check Voltage----------------------------------------------------------------
void checkVoltage() {
  /* Charge|Voltage|ADC
      100%  25.6V   905
       75%  25.2V   891
       50%  24.6V   870
       25%  24.0V   849
  */

  int voltage;
  voltage = analogRead(voltagePin);
  voltage = constrain(voltage, 833, 905);
  chargeLevel = map(voltage, 833, 905, 0, 100);
  // Serial.println(chargeLevel);
}

//-----------------Estop Status----------------------------------------------------------------
void estopStatus() {
  if (digitalRead(eStop) == 1) {
    eStopFlag = false;
  }
  else {
    eStopFlag = true;
  }
}

//-----------------Lift Position----------------------------------------------------------------
void liftPosition() {

  /*
     750 = fully extended
     269 = fully detracted
     2 inch maximum stroke
  */
  int posL;
  posL = analogRead(liftFeedback);
  samplesL++;
  posL = constrain(posL, 269, 750);
  posL = map(posL, 750, 269, 0, 200);
  if (posL < averageL + 30 || posL > averageL - 30) {

    averageL = averageL + posL;
    if (samplesL > 7) {
      liftPos = averageL / samplesL;
      samplesL = 0;
      averageL = 0;
    }
  }
}

//-----------------Lift Move----------------------------------------------------------------
void liftMove() {
  if (liftPos > target - offset && liftPos < target + offset) {
    Lift.motor(0);
  }
  else {
    if (liftPos < target) {
      Lift.motor(-40);
    }

    else if (liftPos > target) {
      Lift.motor(40);
    }

  }
}

//-------------------Get an int off the Bridge-----------------------------------------------
int getNextNum(){
  int res = 0;
  if(p.available()){
    res = p.parseInt()-127; //offset back to +/- range
  }
  return res;
}
