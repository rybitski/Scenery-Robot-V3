import json

example = '''
{
    "1": [[1,2,3,4,5],[1,2,3,4,5],[1,2,3,4,5]],
    "2": [[1,2,3,4,5],[1,2,3,4,5],[1,2,3,4,5]],
    "robots": 1
}
'''
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
    cuesList = []
    jsonDict = json.loads(source)
    for key in jsonDict.keys():
        if key != "robots":
            cue = stageCue(int(key))
            for item in jsonDict[key]:
                vector = stageVector(item[0], item[1], item[2], item[3], item[4])
                cue.appendVector(vector)
            cuesList.append(cue)
            print(cuesList)
    return cuesList

print(parseJson(example))