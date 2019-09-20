#!/usr/bin/python
'''
Cue_Control_Middleware.py
Written by Andy Carluccio
University of Virginia

This file is designed to run on a properly configured YUN Linux Environment

Extensive documentation is available at:

Good things to know about communication with the Arduino environment:
1. The first int sent is always the number of ints to follow (accomplished within the sendCommand method)
2. Using sendCommand([1]) alerts for load mode enter
3. Using sendCommand([2]) alerts for load mode exit
4. Using sendCommand([3]) alerts for a manual control packet to follow
5. Using sendCommand([4]) alerts for an incomming cue
6. Using sendCommand([5]) is a short-hand soft stop trigger
'''

#import statements for required libraries and bridge
#setup------------------------
import sys
import requests
import math
sys.path.insert(0, '/usr/lib/python2.7/bridge')
from time import sleep
from bridgeclient import BridgeClient as bridgeclient
value = bridgeclient()

#global varaibles and
#flags-------------------------------------------------------
oldTime = 0
controlCode = 0
loadFlag = False
cueFlag = False
manualFlag = False
runningCue = False


#URLs of server web
#pages---------------------------------------------------------
mainServerPage = "http://192.168.1.2:5000/api/home"
manualControlPage = "http://192.168.1.2:5000/api/xsend"
responsePage = "http://192.168.1.2:5000/api/encoder"
cuePage = "http://192.168.1.2:5000/api/cue"

#Stage Cue Data
#Structure---------------------------------------------------------

#A stage vector is a pair of distance and rotation values with acceleration,
#deceleration, and a maximum speed
#There is a way to create the vector using ordered pairs and acceleration,
#deceleration, and a maximum speed
class stageVector(object):
    #Constructor
    def __init__(self, dist, theta, accel, maxSpeed, deccel):
        self.distance = dist
        self.angle = theta
        self.accel = accel
        self.deccel = deccel
        self.maxSpeed = maxSpeed
    #Makeshift alternate constructor
    def makeFromPoints(self, x1, y1, x2, y2, acc, decc, vel):
        self.distance = math.sqrt((abs(x2 - x1) ** 2) + abs(y2 - y1) ** 2)
        self.angle = (180 / math.pi) * (math.atan(abs(x2 - x1) / self.distance))
        self.accel = acc
        self.deccel = dec
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
    def get_deccel(self):
        return self.deccel


#A Stage Cue is a list of stage vectors
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

#A stage cue list is, well, a list of stage cues!
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
    #provided a file name, this will create a csv style save file for the cue
    #list
    def saveToFile(self, fileName):
        file = open(fileName,"w")
        file.write(self.listID + "," + '\n')
        for c in self.getCuesInList():
            file.write(c.getID() + ",")
            for v in c.getVectorsInCue():
                file.write(str(v.get_distance()) + "," + str(v.get_angle()) + "," + str(v.get_accel()) + "," + str(v.get_maxSpeed()) + "," + str(v.get_deccel()))
            file.write('\n')

#function for sending encoder data to the
#server------------------------------------------
def sendEncoderVals():
    encoderValues = {"encoder1data": leftEncoderRunningSum, "encoder2data": rightEncoderRunningSum}
    #should change the encoderReadings to the numbers
    r = requests.post(responsePage, data = encoderValues)

#function for sending other data to the
#server-------------------------------------------
def sendEncoderVals(str):
    encoderValues = {"message": str}
    #should change the encoderReadings to the numbers
    r = requests.post(responsePage, data = encoderValues)

#function for sending data to YUN Arduino
#Processor--------------------------------------
def sendCommand(cmd):
    chars = []
    length = 0

    #offset values to positive range
    for instr in cmd:
        chars.append(int(instr) + 127)
        length+=1

    #the console is what the Ardunio Processor reads from, must flush each time

    #the first message must always be the number of messages to follow
    print(length)
    sys.stdout.flush()

    loc = 0
    while(loc < length):
        print(chars[loc])
        sys.stdout.flush()
        loc+=1

#function for pulling cues down from the web page and converting them into cues
#in local memory--------------------
def loadCues():
    #Notify YUN Arduino Processor that new cues are being loaded (sets LED and
    #causes a stop)
    notifyLoadCommand = [-1]
    sendCommand(notifyLoadCommand)

    #This is where the code for loading cues into the data structure will go
    cuesString = requests.get(cuePage)

    #Tell the server we have completed the load
    msg = {"message": str}
    #should change the encoderReadings to the numbers
    r = requests.post(responsePage, data = msg)

    #Notify the YUN Arduino Processor that the new cues are loaded and it is
    #safe to continue (sets LED, no drive)
    notifyLoadCompleteCommand = [-2]
    sendCommand(notifyLoadCompleteCommand)

def startCue(number):
    #Update global variable
    runningCue = True
    currentCueNum = number
    notifyIncommingCue = [-4]
    sendCommand(notifyIncommingCue)

    #calculate the cue's effective values
    cueToRun = cueList.getCueAtLocation(number)
    #TODO: DO SOME FANCY MATH TO MAKE A CONTROL ARRAY
    fancyArray = []
    #we also need to figure out what it means to go backwards a cue, or to skip
    #a cue, or to do anything nonlinear!
    cueControlArray = fancyArray

    #set current and target encoder values

    sendCommand(cueControlArray)

    '''
    How to run a cue:
    set the flag
    warn the arduino
    calculate the cue's effective values and update global control array
    calculate the target values for the encoders to report
    send the command for the first time

    check in the main loop for the flag
    if the flag is set, 
        check the encoders current values against the targets
        update the global encoder values
        send the encoder values to the web
        if we have not reached the destination yet
            send the stored message of the current cue again
        otherwise
            turn off the running a cue flag
            tell website we completed
            tell the arduino to stop
        continue to check the server's status throughout for a network stop command
    '''

#Get updated encoder data from the Arduino's latest bridge report
def updateEncoderValues():
    #TODO: Implement the receiving of the encoder data over Bridge
    updateLeft = 0
    updateRight = 0

    #TODO: what happens when a wheel spins backwards?
    leftEncoderRunningSum = int(updateLeft - leftEncoderRunningSum) + int(leftEncoderRunningSum)
    rightEncoderRunningSum = int(updateRight - rightEncoderRunningSum) + int(rightEncoderRunningSum)

def handleRunningCue():
    updateEncoderValues()

    if(abs(leftEncoderRunningSum - leftEncoderTarget) > encoderErrorMargin and abs(rightEncoderRunningSum - rightEncoderTarget) > encoderErrorMargin):
        sendCommand(cueControlArray)
    else:
        #TODO: notify server
        runningCue = False
        stop = [-5]
        sendCommand(stop)

#MAIN CONTROL ALGORITHM----------------------------------------------------
lastCompletedCueNum = 0
currentCueNum = 0
cueControlArray = []
cueList = stageCueList("Current YUN-Loaded Cue List")
leftEncoderTarget = 0
rightEncoderTarget = 0
leftEncoderRunningSum = 0
rightEncoderRunningSum = 0
encoderErrorMargin = 1

cmd = [4]
sendCommand(cmd)

#Runs forever (in theory!)
while(True):
    cmd2 = [50,50]
    sendCommand(cmd2)
    encoder1 = value.get("A0")
    encoder2 = value.get("A1")

#A message we should never see
print("We're in the endgame now...\n")