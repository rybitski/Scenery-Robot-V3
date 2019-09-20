#!/usr/bin/python

#import statements for required libraries and bridge setup
import sys
import requests

sys.path.insert(0, '/usr/lib/python2.7/bridge')
 
from time import sleep
 
from bridgeclient import BridgeClient as bridgeclient

value = bridgeclient()

oldTime = 0

#for infinity
while(True):
    #scrape the robot move reporting site
    resp = requests.get("http://192.168.1.2:5000/api/xsend")
    #create an array of the movement values
    codeLines = resp.text.split('\n')
    importantLine = codeLines[0]
    chris_list = importantLine[1:-1]
    listVals = chris_list.split(',')

    #implicit break defaults
    char1 = 0
    char2 = 0
    char3 = 0
    char4 = 0
    char5 = 0
    char6 = 0
    char7 = 0
    char8 = 0

    #if the data has not gone stale
    if(int(listVals[8]) - int(oldTime) < 30):
        #first offset values to positive range
        char1 = int(listVals[0])+127
        char2 = int(listVals[1])+127
        char3 = int(listVals[2])+127
        char4 = int(listVals[3])+127
        char5 = int(listVals[4])+127
        char6 = int(listVals[5])+127
        char7 = int(listVals[6])+127
        char8 = int(listVals[7])+127

        if(char1!=127 or char2 !=127 or char3!= 127 or char4!=127 or char5!=127 or char6!=127 or char7!=127 or char8!=127):
            #Print the encoded values, flushing each time
            print(char1)
            sys.stdout.flush()
            print(char2)
            sys.stdout.flush()
            print(char3)
            sys.stdout.flush()
            print(char4)
            sys.stdout.flush()
            print(char5)
            sys.stdout.flush()
            print(char6)
            sys.stdout.flush()
            print(char7)
            sys.stdout.flush()
            print(char8)
            sys.stdout.flush()

    #update stale timer
    oldTime = listVals[8]

    #add just enough delay to keep the processors in sync
    sleep(0.1)

#A message we should never see
print("I hope you enjoyed the show\n")