#Andy Carluccio
import math

#A stage vector is a pair of distance and rotation values
class stageVector(object):
    #Constructor
    def __init__(self, dist, theta):
        self.distance = dist
        self.angle = theta
    def makeFromPoints(self, x1, y1, x2, y2):
        self.distance = math.sqrt((abs(x2-x1)**2)+abs(y2-y1)**2)
        self.angle = (180/math.pi)*(math.atan(abs(x2-x1)/self.distance))
    def get_distance(self):
        return self.distance
    def get_angle(self):
        return self.angle

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
        self.cueID=newID

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
    def getID(self):
        return self.listID
    def setID(self, newID):
        self.listID=newID
    #provided a file name, this will create a csv style save file for the cue list
    def saveToFile(self, fileName):
        file = open(fileName,"w")
        file.write(self.listID +","+'\n')
        for c in self.getCuesInList():
            file.write(c.getID() +",")
            for v in c.getVectorsInCue():
                file.write(str(v.get_distance()) + "," + str(v.get_angle()) + ",")
            file.write('\n')
        
    
print ("Welcome to the Cue Data Structure Demo Tool")
print ("Written by Andy Carluccio")

print ("In this tutorial, we will demo how to read/write cues and vectors to make a cue list.")

print ("Begin by creating a cue list...")
name = str(input("First, supply an identifying label for this cue list:"))
cueList = stageCueList(name)

print ("Let's add a cue to the cue list")
name = str(input("First, supply an identifying label for this cue:"))
cue = stageCue(name)

print ("Now, we need some vectors to populate our first cue!")
print ("Vectors consist of distances and angles, which we will measure in meters and degrees for this example.")
dist1 = float(input("Supply a distance for the first vector:"))
angle1 = float(input("Supply an angle for the first vector:"))
vec1 = stageVector(dist1,angle1)
cue.appendVector(vec1)

print ("THIS IS A SPECIAL CASE TEST OF ORDERED PAIR POINT ENTRY VECTOR TRANSLATION")
x1 = float(input("x1:"))
y1 = float(input("y1:"))
x2 = float(input("x2:"))
y2 = float(input("y2:"))
vecxy = stageVector(0,0)
vecxy.makeFromPoints(x1,y1,x2,y2)
cue.appendVector(vecxy)

print ("A stage movement will likely consist of a set of many vectors in succession, so let's add more...")
loop = True
while (loop):
    yn = str(input("Add another vector? [y/n]:"))
    if(yn=="y"):
        dist = float(input("Supply a distance for the vector:"))
        angle = float(input("Supply an angle for the vector:"))
        vec = stageVector(dist,angle)
        cue.appendVector(vec)
    else:
        loop=False
cueList.appendCue(cue)


print ("A cue list will likely have several cues, so let's add more...")
cueLoop = True
while(cueLoop):
    cyn = str(input("Add cue to this list? [y/n]:"))
    if(cyn=="y"):
        name = str(input("Supply an identifying label for this cue:"))
        cue = stageCue(name)
        vec = True
        while (vec):
            yn = str(input("Add vector to this cue? [y/n]:"))
            if(yn=="y"):
                dist = float(input("Supply a distance for the vector:"))
                angle = float(input("Supply an angle for the vector:"))
                vec = stageVector(dist,angle)
                cue.appendVector(vec)
            else:
                vec=False
        cueList.appendCue(cue)
    else:
        cueLoop=False

print ("You have now made a cue list!")
print ("Printing...")
print()

print("The cue list was named "+ cueList.getID())

for c in cueList.getCuesInList():
    print("There was a cue named " + c.getID())
    print("Containing vectors: ")
    for v in c.getVectorsInCue():
        print (str(v.get_distance()) + "," + str(v.get_angle()))


print ("Saving...")
cueList.saveToFile("testFile.csv")

print ("Ending program...")