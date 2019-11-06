#include <Boards.h>
#include <Firmata.h>
#include <FirmataConstants.h>
#include <FirmataDefines.h>
#include <FirmataMarshaller.h>
#include <FirmataParser.h>

//FIRMWARE FOR UVA MINI STAGE ROBOT

//Software Serial Setup------------------------------------
#include <SoftwareSerial.h>
SoftwareSerial SWSerial(8,5); // Available software serial pins

//Motor Controller-------------------------
#include <Sabertooth.h>

//KANG---------
#include <Kangaroo.h>
KangarooSerial  K(SWSerial);
KangarooChannel K1(K, 'T');
KangarooChannel K2(K, 'D');

long turnReading = 0;
long driveReading = 0;

//I/O----------------------------------------
int brake = 3;            //controls motor brakes
int ledR = 11;             //common anode LED high=off low=on
int ledG = 10;
int ledB = 9;

int buttonPin = 12;


float commandCode = 0.0;
float packetSize = 0.0;
bool buttonPressed = false;
bool driveFlag = false;
bool turnFlag = false;

int destination = 0;
int max_velocity = 0;
int rotation = 0;
int acceleration = 0;
int deceleration = 0;
int seed_velocity = 0;

int velocity = 0;
int acc_end = 0;
int dec_beg = 0;

int enc_reading = 0;
int prev_enc_reading = 0;

int prev_velocity = 0;
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
  
  SWSerial.begin(9600);             //start sabertooth with software serial
  Sabertooth::autobaud(SWSerial);

  
  //LED----------------------
  pinMode(ledR, OUTPUT);
  pinMode(ledG, OUTPUT);
  pinMode(ledB, OUTPUT);

  digitalWrite(ledR, LOW);   //turn LEDs RED on startup
  digitalWrite(ledG, HIGH);
  digitalWrite(ledB, HIGH);

  
  //KANG------
  K1.start();
  K2.start();

  K1.streaming(true);
  K2.streaming(true);
  
  K1.si(0);
  K2.si(0);

  K1.p(0);
  K2.p(0);

  //K2.units(1, (34/2.559*3.14159));

  Serial.println("Finished KANG Setup");

  // Misc -------------------
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
    updateCMDCode();
    
    //trigger subroutines based on the type of mode we are in
    if(commandCode==1.0){ //enter load mode
      
    }
    else if(commandCode==2.0){  //exit load mode
      
    }
    else if(commandCode==3.0){  //manual control mode
      //manualMode();
    }
    else if(commandCode==4.0){  //cue control mode
      cueControlMode();
    }
    else if(commandCode==5.0){  //soft stop trigger mode
      
    }
}


void readEncoders() {
  //encoder1Reading = Encoder1.readEncoder();         //Read Encoder
  //encoder2Reading = Encoder2.readEncoder();           //Read Encoder
  //Encoder1.clearEncoderCount();                 // Clear Encoder
  turnReading = K1.getP().value();
  driveReading = K2.getP().value();
}

//-------------------Get an int off the Bridge-----------------------------------------------
float getNextNum(){
  float res = -1;
  if(p.available()){
    res = p.parseInt()/100.0; //offset back to +/- range
  }
  return res;
}

//----------------------Update Size and CMD Code---------------------------------------------
bool updateCMDCode(){
  //Serial.println("Running update");
  if(p.available()){
    packetSize = p.parseInt()/100.0;
  }
  if(packetSize==1){ //command packets have length 1 after size
    if(p.available()){
      commandCode = p.parseInt()/100.0;
      Serial.println("updated command code");
      return true;
    }
  }
  return false;
}


float getAccelVel(acc, vel, enc){
  int min_vel = seed_velocity;
  float term = (vel*vel)+2.0*acc*enc;
  if(term==0){
    return min_vel;  
  }
  else{
    return sqrt(term);
  }
}


float getDecVel(acc, vel, enc){
  int min_vel = vel;
  float term = (vel*vel)+2.0*acc*enc;
  if(term<0){
    return min_vel;  
  }
  else{
    return sqrt(term);
  }
}

//-------------------CONTROL MODE SUBROUTINES---------------------------------------------

void cueControlMode(){
  readEncoders();
  int metadata = 0;
  if(digitalRead(buttonPin)==HIGH){
    p.println(-1);
    
    driveFlag = true;
    turnFlag = false;
    while(!p.available()){Serial.print("waiting...");}
    getNextNum(); //flush the length out
    destination = getNextNum();
    max_velocity = getNextNum();
    velocity = 0; 
    rotation = getNextNum();
    acceleration = getNextNum();
    deceleration = getNextNum();
    acc_end = getNextNum();
    dec_beg = getNextNum();
    seed_velocity = getNextNum();
    
    Serial.print("Destination is: ");
    Serial.print(destination);
    Serial.println(" ticks");
    
    //K2.s(velocity);
    K2.p(destination,velocity);
    digitalWrite(ledR, HIGH);  
    digitalWrite(ledG, HIGH);
    digitalWrite(ledB, HIGH);
  }
  if(driveFlag){
    p.println(driveReading);
    Serial.print("Encoder position: ");
    Serial.println(driveReading);

    if(driveReading < acc_end){
      velocity = getAccVel(acceleration, velocity, driveReading);      
      K2.p(destination,velocity);
    }
    else if(driveReading > acc_end && driveReading < dec_beg){
      velocity = max_velocity;
      K2.p(destination, velocity);
    }
    else if(driveReading > dec_beg){
      velocity = getDecVel(decerlation, velocity, driveReading);
      K2.p(destination, velocity);
    }
    else if(driveReading >= destination){
      velocity = 0;
      driveFlag = false;
      turnFlag = true;
      Serial.println("drive is complete");
      p.println(-2);
    }
    
    //diagnostics:
    if(velocity > prev_velocity){
      digitalWrite(ledR, LOW);  
      digitalWrite(ledG, HIGH);
      digitalWrite(ledB, HIGH);
      Serial.println("Accelerating!");
    }
    else if( velocity == prev_velocity){
      digitalWrite(ledR, HIGH);  
      digitalWrite(ledG, LOW);
      digitalWrite(ledB, HIGH);
      Serial.println("Crusing!");
    }
    else{
      digitalWrite(ledR, HIGH);  
      digitalWrite(ledG, HIGH);
      digitalWrite(ledB, LOW);
      Serial.println("Decelerating!");
    }
    prev_velocity = velocity;
  } 
  else if(turnFlag){
    Serial.println("turning...");
    K1.p(rot).wait(); //THIS IS VERY VBRY BAD!!!
    Serial.println("turn is complete");
    turnFlag = false;
    p.println(-3);
 }
}

/*
 * CODE GRAVEYARD
 * IN HONOR OF TIM
 */