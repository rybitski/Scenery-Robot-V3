#!/usr/bin/python -u
'''
COMPLEX_CUE_MIDDLEWARE.py
Written by Andy Carluccio
University of Virginia

This file is designed to run on a properly configured YUN Linux Environment

Extensive documentation is available at:

Good things to know about communication with the Arduino environment:
1. The first int sent is always the number of ints to follow (accomplished within the sendCommand method)
2. Using sendCommand(1) alerts for load mode enter
3. Using sendCommand(2) alerts for load mode exit
4. Using sendCommand(3) alerts for a manual control packet to follow
5. Using sendCommand(4) alerts for an incomming cue
6. Using sendCommand(5) is a short-hand soft stop trigger
'''

#import statements for required libraries and bridge
#setup------------------------
import sys
import os
import requests
import math
sys.path.insert(0, '/usr/lib/python2.7/bridge')
from time import sleep
from bridgeclient import BridgeClient as bridgeclient
value = bridgeclient()

#dist = 5 #in feet
#velocity = 4 #in per second

def sendCommand(cmd):
    chars = []

    #offset values to positive range
    for instr in cmd:
        chars.append(int(instr))

    #the console is what the Ardunio Processor reads from, must flush each time

    #the first message must always be the number of messages to follow
    print(len(chars))
    sys.stdout.flush()

    for msg in chars:
        print(msg)
        sys.stdout.flush()

def getData():
    inputA = sys.stdin.readline()
    #inputA = inputA.rstrip('\n')
    #inputA = inputA.strip()
    inputA = int(inputA)

    return inputA

pinFlag = False


sendCommand("4")
encoder1 = 0
encoder2 = 0

encoderFinish1 = 0
encoderFinish2 = 0

#make sure to multiply by pi
def preCalcEnc(feet):
    inches = feet*12

   # endLoc = 2.5*3.14159*inches*2400+enc
    endLoc = ((inches*2400)/(2.5*3.14159))

    return endLoc

#unit of inches per second
def calcVelocity(speed):
    encoder = speed / (2.5*3.14159)
    ticks = 2400 * encoder
    return ticks

while(True):
    if(not pinFlag):
        arduinoReading = getData()

    #if go pin was pressed
    if(arduinoReading==-1):
        pinFlag = True
        #encoder1 = getData()
        #encoder2 = getData()
        #encoderFinish1 = preCalcEnc(encoder1,dist)
        #encoderFinish2 = preCalcEnc(encoder2,dist)
        distance = preCalcEnc(5)
        speed = calcVelocity(4)
        sendCommand([distance,speed])

        arduinoReading=0

   # if(pinFlag):
       # encoder1 = getData()
       # encoder2 = getData()
       # if(encoder1<encoderFinish1 and encoder2<encoderFinish2):
        #    sendCommand([127,127]) #tells the robot to go full speed forward
        #else:
          #  sendCommand([0,0]) #stop the robot
          #  pinFlag = False
