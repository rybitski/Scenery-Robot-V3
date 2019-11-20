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
import json

cuesList = []

driveENC = 0
turnENC = 0

acc_end = 0
dec_beg = 0
endpt = 0

endrot = 0

live_vel = 0

acc_metadata = 3
cruise_metadata = 4
dec_metadata = 5

drive_flag = False
turn_flag = False

seed_vel = 1
unit_tick = 1

example = '''
{
    "1": [[1,2,3,4,5],[1,2,3,4,5],[1,2,3,4,5]],
    "2": [[1,2,3,4,5],[1,2,3,4,5],[1,2,3,4,5]],
    "robots": 1
}
'''
class stageVector(object):
    #Constructor
    def __init__(self, dist, theta, accel, maxSpeed, decel):
        self.distance = dist
        self.angle = theta
        self.accel = accel
        self.decel = decel
        self.maxSpeed = maxSpeed
    #Makeshift alternate constructor
    def makeFromPoints(self, x1, y1, x2, y2, acc, dec, vel):
        self.distance = math.sqrt((abs(x2 - x1) ** 2) + abs(y2 - y1) ** 2)
        self.angle = (180 / math.pi) * (math.atan(abs(x2 - x1) / self.distance))
        self.accel = acc
        self.decel = dec
        self.maxSpeed = vel
    #Getters
    def get_distance(self):
        return self.distance
    def get_angle(self):
        return self.angle
    def get_maxSpeed(self):
        return self.maxSpeed
    def get_accel(self):
        return self.accel
    def get_decel(self):
        return self.decel

class stageCue(object):
    #Constructor
    def __init__(self, id):
        self.vectorList = []
        self.cueID = id
    #push a vector to the list
    def appendVector(self, stageVec):
        self.vectorList.append(stageVec)
    #returns a list of the vectors
    def getVectorsInCue(self):
        return self.vectorList
    def popVector(self):
        return self.vectorList.pop()
    def popThisVector(self, loc):
        return self.vectorList.pop(loc)
    def appendVectorList(self, newVecList):
        for sv in newVecList:
            self.vectorList.appendVector(sv)
    def addVectorAtLocation(self, loc, vec):
        self.vectorList.add(loc,vec)
    def getID(self):
        return self.cueID
    def setID(self, newID):
        self.cueID = newID

class stageCueList(object):
    #Constructor
    def __init__(self, id):
        self.cueList = []
        self.listID = id
    def appendCue(self, stageQ):
        self.cueList.append(stageQ)
    def getCuesInList(self):
        return self.cueList
    def popCue(self):
        return self.cueList.pop()
    def popThisCue(self, loc):
        return self.cueList.pop(loc)
    def appendCueList(self, newCueList):
        for q in newCueList:
            self.cueList.appendCue(q)
    def addCueAtLocation(self, loc, q):
        self.cueList.add(loc,q)
    def getCueAtLocation(self, loc):
        allCues = self.cueList
        return allCues[loc]
    def getID(self):
        return self.listID
    def setID(self, newID):
        self.listID = newID

def parseJson(source):
    jsonDict = json.loads(source)
    for key in jsonDict.keys():
        if key != "robots":
            cue = stageCue(int(key))
            for item in jsonDict[key]:
                vector = stageVector(item[0], item[1], item[2], item[3], item[4])
                cue.appendVector(vector)
            cuesList.append(cue)
            #print(cuesList)
    return cuesList

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
    inputA = float(inputA)

    return inputA

def precalcDest(loc):
    return loc+dist

def precalcRot(loc):
    return loc+rot

def convertToLineUnit(units):
    return ((units*387)/(2.559*math.pi))

def convertToCustomUnit(line_units):
    return (line_units*2.559*math.pi/387)

pinFlag = False

parseJson(example)

cue = cuesList.pop()
vect = cue.popVector()

dist = vect.get_distance()
rot = vect.get_angle()
acceleration = vect.get_accel()
deceleration = vect.get_decel()
velocity = vect.get_maxSpeed()

#dist = 120 #in inches
#rot = 90 #in degrees
#acceleration = 3 #inches per second^2
#deceleration = -4
#velocity = 12 #inches per second

sendCommand("4")

while(True):
    if(not pinFlag):
        arduinoReading = getData()

    #if go pin was pressed
    if(arduinoReading==-1):
        pinFlag = True
        drive_flag = True
        driveENC = convertToCustomUnit(getData())
        endpt = precalcDest(driveENC)
        endrot = rot

        acc_end = ((velocity+seed_vel)/2.0)*((velocity-seed_vel)/acceleration)
        dec_beg = endpt-(((0+velocity)/2.0)*((0-velocity)/deceleration))

        arduinoReading=0

        sendCommand([convertToLineUnit(endpt),convertToLineUnit(velocity),convertToLineUnit(endrot), convertToLineUnit(acceleration), convertToLineUnit(deceleration), convertToLineUnit(acc_end), convertToLineUnit(dec_beg),convertToLineUnit(seed_vel)])

    if(pinFlag):
        flag = getData()

        if(flag<0): #indicates that a command was passed, not an encoder!
            if(flag==-2): #completed the drive
                drive_flag = False
                turn_flag = True
                #update the acc and dec with respect to the rotation (THIS IS  A PROBLEM! NO UNIQUE VEL CURVES FOR ROT)
                accTime = velocity / acceleration
                acc_end = (velocity/2) * accTime
                decTime = (-1 * velocity) / deceleration
                dec_beg = rot - ((velocity/2) * decTime)
                live_vel = 0.1
                prev_driveENC = 0
            elif(flag==-3): #completed the turn
                turn_flag = False

                if(len(cue.getVectorsInCue)>0):
                    cue = cuesList.pop()
                    vect = cue.popVector()

                    dist = vect.get_distance()
                    rot = vect.get_angle()
                    acceleration = vect.get_accel()
                    deceleration = vect.get_decel()
                    velocity = vect.get_maxSpeed()
                else:
                    pinFlag = False
                    ddrive_flag = True


        else:
            driveENC = convertToCustomUnit(flag)
            #post driveENC to the server...
