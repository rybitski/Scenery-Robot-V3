#include <Boards.h>
#include <Firmata.h>
#include <FirmataConstants.h>
#include <FirmataDefines.h>
#include <FirmataMarshaller.h>
#include <FirmataParser.h>

//FIRMWARE FOR UVA STAGE ROBOT

//Software Serial Setup------------------------------------
#include <SoftwareSerial.h>

SoftwareSerial SWSerial(5,4); // Available software serial pins



//Motor Controller-------------------------
#include <Sabertooth.h>
       //Sabertooth 2x60 Drive Motors
Sabertooth Lift(130, SWSerial);        //SyRen 25 Lift Motor

//KANG---------
#include <Kangaroo.h>
KangarooSerial  K(SWSerial);
KangarooChannel K1(K, '1');
KangarooChannel K2(K, '2');
//Sabertooth ST1(128, SWSerial); 

//Encoder Buffer---------------------------
//#include <SPI.h>

//#define EncoderCS1 7        //Slave Select for encoder1
//#define EncoderCS2 8        //Slave Select for encoder2

long encoder1Reading = 0;
long encoder2Reading = 0;

//Encoder_Buffer Encoder1(EncoderCS1);
//Encoder_Buffer Encoder2(EncoderCS2);

//I/O----------------------------------------
int voltagePin = A0;      //voltage divider to monitor battery state
int liftFeedback = A1;    //potintiometer connected to linear actuator
int eStop = 2;            //checks estop status from estop controller
int brake = 3;            //controls motor brakes
int ledR = 9;             //common anode LED high=off low=on
int ledG = 10;
int ledB = 11;

//int buttonPin = 9;

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

bool buttonPressed = true;
int buttonPin = 12;

bool switch1 = false;

bool goFlag = false;

int destination = 0;
int velocity = 0;

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

  //KANG------
  K1.start();
  K2.start();

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
  pinMode(buttonPin, INPUT);
  
  // Start using the Bridge and Python script
  Serial.println("Starting bridge...");
  Bridge.begin();

  Serial.println("running python script...");
  p.runShellCommandAsynchronously("python -u /usr/lib/andyCode/COMPLEX_CUE_MIDDLEWARE.py");
  Serial.println("command sent!");

  Serial.println("waiting for data influx...");
  digitalWrite(ledG, LOW); //turn LEDs YELLOW while waiting
  while(!p.available());

  digitalWrite(ledR, HIGH);   //turn LEDs GREEN when leaving setup
  Serial.println("setup complete");
}

/*
 * WHAT THE LOOP DOES
 * 
 * First, check the eStopStatus
 * If not in eStop scenerio, check Bridge for a command code
 * Run the proper subroutine for the active command code
 * 
 */
void loop() {
  //check to see if emergency stop was activated (updates flag)
  estopStatus();

  if(!eStopFlag){ //if we are not in an emergency condition
    //Serial.println("no emergency here boss");
    if(true){  //If a new CMD code was parsed (note that updateCMDCode also strips the length bit)
      //Do any cleanup required when switching between operation modes
      //TODO ---> add a previous command variable and clean based on "from / to" logic
      updateCMDCode();
    }
    //trigger subroutines based on the type of mode we are in
    if(commandCode==1){ //enter load mode
      
    }
    else if(commandCode==2){  //exit load mode
      
    }
    else if(commandCode==3){  //manual control mode
      //manualMode();
    }
    else if(commandCode==4){  //cue control mode
      cueControlMode();
    }
    else if(commandCode==5){  //soft stop trigger mode
      
    }
  }
  else{ //emergency condition
    
  }
}


void readEncoders() {
  //encoder1Reading = Encoder1.readEncoder();         //Read Encoder
  //encoder2Reading = Encoder2.readEncoder();           //Read Encoder
  //Encoder1.clearEncoderCount();                 // Clear Encoder
  encoder1Reading = K1.getP().value();
  encoder2Reading = K2.getP().value();
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

//-------------------Get an int off the Bridge-----------------------------------------------
int getNextNum(){
  int res = 0;
  if(p.available()){
    res = p.parseInt(); //offset back to +/- range
  }
  return res;
}

//----------------------Update Size and CMD Code---------------------------------------------
bool updateCMDCode(){
  //Serial.println("Running update");
  if(p.available()){
    packetSize = p.parseInt();
  }
  if(packetSize==1){ //command packets have length 1 after size
    if(p.available()){
      commandCode = p.parseInt();
      Serial.println("updated command code");
      return true;
    }
  }
  return false;
}

//-------------------CONTROL MODE SUBROUTINES---------------------------------------------

void cueControlMode(){
  readEncoders();
  encoder1Reading = -1*encoder1Reading;
  if(digitalRead(buttonPin)==HIGH){
    p.println(-1);
    goFlag = true;
    //switch1 = true;
    while(!p.available()){}
    getNextNum();
    destination = getNextNum();
    velocity = getNextNum();
  }
  if(goFlag){
    K1.s(velocity);
    K1.p(destination);
    K2.s(velocity);
    K2.p(destination);
  }
}


/*
void cueControlMode2(){
  //Serial.println("cue control");
  readEncoders();
  encoder1Reading = -1*encoder1Reading;
  if(digitalRead(buttonPin)==HIGH){
    p.println(-1);
    //p.println(encoder1Reading);
    //p.println(encoder2Reading);
    //Serial.println(encoder1Reading);
    //Serial.println(encoder2Reading);
    goFlag = true;
    //Serial.println("hit button!");
    switch1 = true;
  }
  
  if(goFlag){
      if(p.available()){
     //bridge stuff
      int rightWheel = getNextNum();
      int leftWheel = getNextNum();
      Serial.print("Bridge Right wheel: ");
      Serial.println(rightWheel);
      Serial.print("Bridge Left wheel: ");
      Serial.println(leftWheel);
      //drive stuff
      digitalWrite(brake, HIGH);
      
      ST1.motor(1,rightWheel *-1);
      ST1.motor(2,leftWheel);
      
      if(rightWheel==0 && leftWheel==0){
        goFlag = false;
        digitalWrite(brake, LOW);
      }
    }
  }
  if(switch1 && goFlag){
    p.println(encoder1Reading);
    p.println(encoder2Reading);
    p.println(encoder1Reading);
    p.println(encoder2Reading);
    Serial.println(encoder1Reading);
    Serial.println(encoder2Reading);
    Serial.println(encoder1Reading);
    Serial.println(encoder2Reading);
    switch1=false;
  }
  else if (!switch1 && goFlag){
    p.println(encoder1Reading);
    p.println(encoder2Reading);
    Serial.println(encoder1Reading);
    Serial.println(encoder2Reading);
  }
}
*/
/*
 * CODE GRAVEYARD
 * IN HONOR OF TIM
 */
