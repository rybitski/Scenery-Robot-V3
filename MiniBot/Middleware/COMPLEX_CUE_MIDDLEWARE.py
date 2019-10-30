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
#import requests
import math
#sys.path.insert(0, '/usr/lib/python2.7/bridge')
from time import sleep
#from bridgeclient import BridgeClient as bridgeclient
#value = bridgeclient()

dist = 60 #in inches
rot = 90 #in degrees
acceleration = 3 #inches per second^2
deceleration = -8
velocity = 12 #inches per second

driveENC = 0

acc_end = 0
dec_beg = 0
endpt = 0

live_vel = 0

acc_metadata = 3
cruise_metadata = 4
dec_metadata = 5

drive_flag = False
turn_flag = False

unit_tick = 1
line_tick = 34/2.559*math.pi

def sendCommand(cmd):
    chars = []

    #offset values to positive range
    for instr in cmd:
        chars.append(int((float(instr))*100))

    #the console is what the Ardunio Processor reads from, must flush each time

    #the first message must always be the number of messages to follow
    print(len(chars)*100)
    sys.stdout.flush()

    for msg in chars:
        print(msg)
        sys.stdout.flush()

def getData():
    inputA = sys.stdin.readline()
    #inputA = inputA.rstrip('\n')
    #inputA = inputA.strip()
    inputA = float(inputA)

    return inputA

def precalcDest(loc):
    return loc+dist

def getAccelVel(accel, vel_now, enc_now, point):
    new_vel = math.sqrt(vel_now**2.0 + 2.0*accel*(point-enc_now)/point)
    #new_vel = vel_now+1
    return new_vel

def getDecVel(accel, vel_now, enc_now, point):
    new_vel = math.sqrt(vel_now**2.0 + 2.0*accel*(((dist-point)-(enc_now-point))/(dist-point)))
    #new_vel = vel_now+1
    return new_vel

def convertToLineUnit(units):
    return ((units*374)/(2.559*math.pi))

def convertToCustomUnit(line_units):
    return (line_units*2.559*math.pi/374)

pinFlag = False


sendCommand("4")

while(True):
    if(not pinFlag):
        arduinoReading = getData()

    #if go pin was pressed
    if(arduinoReading==-1):
        pinFlag = True
        drive_flag = True
        sendCommand([convertToLineUnit(dist),convertToLineUnit(velocity),convertToLineUnit(rot)])
        driveENC = convertToCustomUnit(getData())
        endpt = precalcDest(driveENC)

        accTime = velocity / acceleration
        acc_end = (velocity/2) * accTime 

        decTime = velocity / deceleration*-1
        dec_beg = dist - ((velocity/2) * decTime) #mult by 2 could be an issue...
        arduinoReading=0

    if(pinFlag):
        driveENC = convertToCustomUnit(getData())
        if(driveENC<0): #indicates that a command was passed, not an encoder!
            if(driveENC==-2): #completed the drive
                drive_flag = False
                turn_flag = True
                #update the acc and dec with respect to the rotation (THIS IS  A PROBLEM! NO UNIQUE VEL CURVES FOR ROT)
                accTime = velocity / acceleration
                acc_end = (velocity/2) * accTime
                decTime = (-1 * velocity) / deceleration
                dec_beg = rot - ((velocity/2) * decTime)
                live_vel = 0.1
                prev_driveENC = 0
            elif(driveENC==-3): #completed the turn
                turn_flag = False
                pinFlag = False

        #Send velocities for a linear drive
        elif(drive_flag and driveENC<acc_end):
            #accelerate
            live_vel = getAccelVel(acceleration, live_vel, driveENC, acc_end)
            sendCommand([acc_metadata,convertToLineUnit(live_vel)])
            prev_driveENC = driveENC
        elif(drive_flag and driveENC>acc_end and driveENC<dec_beg):
            #cruise
            sendCommand([cruise_metadata, convertToLineUnit(velocity)])
            prev_driveENC = driveENC
            live_vel=velocity
        elif(drive_flag and driveENC>dec_beg and driveENC<dist):
            #deccelerate 
            live_vel = getDecVel(deceleration, live_vel, driveENC, dec_beg)
            sendCommand([dec_metadata,convertToLineUnit(live_vel)])
            prev_driveENC = driveENC
        elif(drive_flag and driveENC>dist):
            velocity=0
            sendCommand([dec_metadata, convertToLineUnit(velocity)])
            live_vel=velocity
            prev_driveENC = driveENC


        #Send velocities for a turn
        elif(turn_flag and driveENC<acc_end):
            #accelerate
            live_vel = getAccelVel(acceleration, live_vel, driveENC, acc_end)
            sendCommand([acc_metadata,convertToLineUnit(live_vel)])
            prev_driveENC = driveENC
        elif(turn_flag and driveENC>acc_end and driveENC<dec_beg):
            #cruise
            sendCommand([cruise_metadata, convertToLineUnit(velocity)])
            prev_driveENC = driveENC
            live_vel=velocity
        elif(drive_flag and driveENC>dec_beg and driveENC<dist):
            #deccelerate 
            live_vel = getDecVel(deceleration, live_vel, driveENC, dec_beg)
            sendCommand([dec_metadata,convertToLineUnit(live_vel)])
            prev_driveENC = driveENC
        else:
            velocity=0
            sendCommand([dec_metadata, convertToLineUnit(velocity)])
            live_vel=velocity
            prev_driveENC = driveENC



