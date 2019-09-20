#!/usr/bin/python -u
'''
Cue_Control_Middleware.py
Written by Andy Carluccio
University of Virginia

This file is designed to run on a properly configured YUN Linux Environment

Extensive documentation is available at:

Good things to know about communication with the Arduino environment:
1. The first int sent is always the number of ints to follow (accomplished within the sendCommand method)
2. Using sendCommand(-1) alerts for load mode enter
3. Using sendCommand(-2) alerts for load mode exit
4. Using sendCommand(-3) alerts for a manual control packet to follow
5. Using sendCommand(-4) alerts for an incomming cue
6. Using sendCommand(-5) is a short-hand soft stop trigger
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

print("setup completed")
sys.stdout.flush()

#Runs forever (in theory!)
while(True):
    inputA = sys.stdin.readline()
    inputA = inputA.rstrip('\n')
    inputA = inputA.strip()

    inputB = sys.stdin.readline()
    inputB = inputB.rstrip('\n')
    inputB = inputB.strip()

    inputA = int(inputA)
    inputB = int(inputB)

    print(inputA)
    #a = -7
    #print(a)
    sys.stdout.flush()

    #b = -32442
    #print(b)
    print(inputB)
    sys.stdout.flush()

#A message we should never see
print("We're in the endgame now...\n")