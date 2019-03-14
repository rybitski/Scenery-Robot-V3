//Motor Controller-------------------------
#include <Sabertooth.h>
Sabertooth ST1(128);        //Sabertooth 2x60 Drive Motors
Sabertooth Lift(130);        //SyRen 25 Lift Motor


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

// Global Variables--------------------------
byte chargeLevel = 0;    //stores level of charge 0-100%
bool eStopFlag = true;

byte liftPos = 0;   //stores lift position in inches 200=2inches
int averageL;       //Lift stuff
int samplesL;       //Lift stuff

int target = 50;
int offset = 15;    //increase this to stop lift jitter


void setup() {

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
  SabertoothTXPinSerial.begin(2400);
  Sabertooth::autobaud(SabertoothTXPinSerial);


  //Encoder------------------
  SPI.begin();
  Encoder1.initEncoder();
  Encoder2.initEncoder();

  //LED----------------------
  pinMode(ledR, OUTPUT);
  pinMode(ledG, OUTPUT);
  pinMode(ledB, OUTPUT);

  digitalWrite(ledR, HIGH);   //turn LEDs off on startup
  digitalWrite(ledG, HIGH);
  digitalWrite(ledB, HIGH);

  // Misc -------------------
  pinMode(brake, OUTPUT);
  pinMode(eStop, INPUT);
  Serial.begin(9600);


}

void loop() {
  readEncoders();       //updates encoder position
  checkVoltage();       //updates battery charge state %
  estopStatus();        //checks if estop has been activated
  liftPosition();       //updates the position of the lift
  liftMove();           //moves the lift to the target position

  digitalWrite(brake, HIGH);  //send power to the brakes to disengauge them.




  //  ST1.motor(2,20);        //turn on motor2
  // Lift.motor(60);             //60 is the max for the 12v actuator. + is down and - is up

  //------Serial Feedback-----------------------------
  if (eStopFlag == true) {
    Serial.print("Emergency Stop ");
  }
  Serial.print("Charge: ");
  Serial.print(chargeLevel);
  Serial.print("% ");
  Serial.print("Lift Position: ");
  Serial.print(liftPos);
  Serial.print(" Encoder 1: ");
  Serial.print(encoder1Reading);
  Serial.print("  Encoder 2: ");
  Serial.print(encoder2Reading);
  Serial.println("");


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
