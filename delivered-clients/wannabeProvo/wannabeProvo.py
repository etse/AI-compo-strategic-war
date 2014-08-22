from __future__ import print_function, division, unicode_literals

import socket, random
from functools import partial
import json
from display import Display

seed = random.randint(0, 999)
logGame = False

turn = 0
myID = 0


moves = []
foodTargets = []

emptyList = []
wallList = []
foodList = []
myUnits = []
enemyUnits = []
mySpawners = []
enemySpawners = []



DIRECTIONS = {"north": (0, -1),
              "east": (1, 0),
              "south": (0, 1),
              "west": (-1, 0)}

ATTACKRADIUS = 5
MAPSIZE = [0, 0]
mode = "Harvester"
maxHarvesters = 10



def init():
    global moves
    del moves[:]
    del mySpawners[:]
    del wallList[:]
    del enemyUnits[:]
    del enemySpawners[:]
    del foodList[:]


    update()



    main()
    if logGame == True:
        logInfo()
    executeMoves()



def update():
    nonWalls = []


    for i in d["map"]:
        #Walls
        if "is_wall" in i and i["is_wall"] == True:
            wallList.append(i)
        else:
            nonWalls.append(i)
            #Spawners
            if "spawner" in i:
                if i["spawner"]["owner"] == myID:
                    mySpawners.append(i)
                else:
                    enemySpawners.append(i)
            #Enemy units
            if "unit" in i and i["unit"]["owner"] != myID:
                enemyUnits.append(i)
            #Food
            if "has_food" in i and i["has_food"] == True:
                foodList.append(i)



    #Spawn units
    for spawner in mySpawners:
        if "unit" in spawner and spawner["unit"]["owner"] == myID:
            for unit in myUnits:
                if unit.position == spawner["position"]:
                    break
            else:
                myUnits.append(Unit(spawner["position"], spawner["unit"]["type"]))

    #Dead units
    for unit in myUnits:
        for i in nonWalls:
            if "unit" in i and i["unit"]["owner"] == myID:
                if unit.position == i["position"]:
                    break
        else:
            myUnits.remove(unit)

    #Food Targets
    for food in foodList:
        for foodTarget in foodTargets:
            if food["position"] == foodTarget["foodLocation"]:
                break
        else:
            foodTargets.append({"foodLocation": food["position"], "gatherer": None})




def logInfo():
    log("")
    log("-----------------------")
    log("Turn: " + str(turn) + ": ")
    log("")


    log("mySpawners:")
    for i in mySpawners:
        log(str(i))
    log("")

    #log("enemySpawners:")
    #for i in enemySpawners:
    #    log(str(i))
    #log("")

    log("myUnits: ")
    for i in myUnits:
        log(str(i))
    log("")

    log("enemyUnits: ")
    for i in enemyUnits:
        log(str(i))
    log("")

    log("food:")
    for i in foodList:
        log(str(i))
    log("")

    log("foodTargets:")
    for i in foodTargets:
        log("FoodLocation: " + str(i["foodLocation"]) + ", gatherer: " + str(i["gatherer"]))
    log("")



    #for i in d["map"]:
        #log2(str(i))

    #log2("walls: ")
    #for i in wallList:
        #log2(str(i))
    #log2("")



def main():
    global mode
    for food in foodTargets:
        if food["gatherer"] == None:
            assignGatherer(food)

    for target in foodTargets:
        if target["gatherer"] != None:
            gatherer = target["gatherer"]
            if gatherer.plannedMoves == [] and distance2(gatherer.position, target["foodLocation"]) < 100:
                gatherer.plannedMoves = AStar(gatherer.position, target["foodLocation"])
            if gatherer.plannedMoves == []:
                foodTargets.remove(target)
                gatherer.mission = None
            else:
                if not willCollide(gatherer, gatherer.destinationDir):
                    gatherer.plannedMoves.pop(0)
                    if gatherer.plannedMoves == []:
                        foodTargets.remove(target)
                        gatherer.mission = None
                    try:
                        gatherer.destination = gatherer.plannedMoves[0]
                    except:
                        pass
                    if gatherer.position != gatherer.destination:
                        gatherer.destinationDir = getDirection(gatherer.position, gatherer.destination)


                    

    numHarvesters = 0
    for unit in myUnits:
        if unit.mission == None:
            explore(unit)
        if unit.unitType == "harvester":
            numHarvesters += 1

    if numHarvesters >= maxHarvesters:
        mode = "Soldier"



def firstTurn():
    global myID
    global MAPSIZE
    myID = d["player_id"]
    MAPSIZE = d["map_size"]

    if logGame == True:
        log("ID: " + str(myID))
        log("Seed: " + str(seed))
        log("Map size: " + str(MAPSIZE))



def explore(unit):
    x = unit.position[0]
    y = unit.position[1]

    extra1 = random.choice(["north", "east", "west", "south"])
    extra2 = random.choice(["north", "east", "west", "south"])

    closestDistance = 999
    for otherUnit in myUnits:
        currentDistance = distance(unit.position, otherUnit.position)
        if currentDistance < closestDistance and currentDistance != 0:
            closestUnit = otherUnit
            closestDistance = currentDistance


    if closestDistance < 12:  
        if x - closestUnit.position[0] > 0:
            extra1 = "east"
        elif x - closestUnit.position[0] < 0:
            extra1 = "west"


        if y - closestUnit.position[1] > 0:
            extra2 = "south"
        elif y - closestUnit.position[1] < 0:
            extra2 = "north"

    log3(str(extra1))

    possibleDirection = random.choice(["north", "east", "west", "south", extra1, extra1, extra2, extra2])
    if not willCollide(unit, possibleDirection):
        unit.destination = getDestination(x, y, possibleDirection)
        unit.destinationDir = possibleDirection
        return True
    return False


def willCollide(unit, direction):
    global moves
    x = unit.position[0]
    y = unit.position[1]
    destination = getDestination(x, y, direction)
    for other in myUnits:
        if destination == other.destination:
            return True

    for i in wallList:
        if destination == i["position"]:
            return True
    return False

def getDestination(x, y, direction):
    return map(sum, zip([x, y], DIRECTIONS[direction]))


def executeMoves():
    global moves
    for unit in myUnits:
        if unit.position != unit.destination and unit.destinationDir != None:
            x = unit.position[0]
            y = unit.position[1]
            moves.append([x, y, unit.destinationDir])


            if unit.destination[0] > MAPSIZE[0] - 1:
                unit.destination[0] = 0

            if unit.destination[1] > MAPSIZE[1] - 1:
                unit.destination[1] = 0

            if unit.destination[0] < 0:
                unit.destination[0] = MAPSIZE[0] - 1

            if unit.destination[1] < 0:
                unit.destination[1] = MAPSIZE[1] - 1

            unit.position = unit.destination[:]




                    



def assignGatherer(food):
    potentialUnits = []
    for unit in myUnits:
        if unit.mission == None and (unit.unitType == "harvester" or unit.unitType == "unit"):
            potentialUnits.append(unit)
    if len(potentialUnits) != 0:
        chosenUnit = getClosestUnit(potentialUnits, food["foodLocation"])
        chosenUnit.mission = "gatherFood"
        food["gatherer"] = chosenUnit


def getClosestUnit(units, destination):
    closestDistance = 999
    for unit in units:
        currentDistance = distance(unit.position, destination)
        if currentDistance < closestDistance:
            closestUnit = unit
            closestDistance = currentDistance
    return closestUnit

def distance(position1, position2):
    x = abs(position1[0] - position2[0])
    y = abs(position1[1] - position2[1])
    return x + y

def distance2(position1, position2):
    x = abs(position1[0] - position2[0])
    y = abs(position1[1] - position2[1])
    return x**2 + y**2

def getDirection(position, destination):
    x = destination[0] - position[0]
    y = destination[1] - position[1]

    if x == 1 and y == 0:
        return "east"
    elif x == -1 and y == 0:
        return "west"
    elif x == 0 and y == 1:
        return "south"
    elif x == 0 and y == -1:
        return "north" 


def log(string):
        f = open('log.txt', 'a')
        f.write(string + "\n")
        f.close()

def log2(string):
        f = open('log2.txt', 'a')
        f.write(string + "\n")
        f.close()

def log3(string):
        f = open('log3.txt', 'a')
        f.write(string + "\n")
        f.close()



class Node:
    def __init__(self, position, destination, startNode, parent):
        self.position = position
        self.destination = destination
        self.startNode = startNode
        self.parent = parent

        if startNode != None:
            self.initialDistance = distance(self.startNode.position, self.position)
            self.finalDistance = distance(self.position, self.destination)
            self.cost = self.initialDistance + self.finalDistance
        else:
            initialDistance = 0
            self.cost = 0

    def __str__(self):
        if self.startNode != None:
            return "Position: " + str(self.position) + ", Cost: " + str(self.cost) + ", initialDistance: " + str(self.initialDistance) + ", finalDistance: " + str(self.finalDistance)
        else:
            return "Position: " + str(self.position)



    #def updateCost(self, destination):




def AStar(position, destination):
    openList = []
    closedList = []

    startNode = Node(position, destination, None, None)
    openList.append(startNode)

    if logGame == True:
        log2("AStar: Starting position: " + str(position)  + ", Destination: " + str(destination))

    #while len(openList) > 0:
    for u in range(20):
        node = chooseNode(openList)


        if node.position == destination:
            path = []
            while node != None:
                path.append(node.position)
                node = node.parent
            path.reverse()
            log2("PAAAAAATH: " + str(path))
            return path[0:len(path)-1]

        openList.remove(node)
        closedList.append(node)

        adjacentNodesDistances = getAdjacentNodes(node, startNode)
        adjacentNodes = []
        for i in adjacentNodesDistances:
            adjacentNodes.append(Node(i, destination, startNode, node))


        newReachable = []
        for adjacentNode in adjacentNodes:
            for closedNode in closedList:
                if adjacentNode.position == closedNode.position:
                    break
            else:
                newReachable.append(adjacentNode)

        if logGame == True:
            log2("")
            log2("OpenList:")
            for i in openList:
                log2(str(i))
            log2("")
            log2("ClosedList")
            for i in closedList:
                log2(str(i))
            log2("")
            log2("newReachable")
            for i in newReachable:
                log2(str(i))
            log2("-----------")



        for adjacent in newReachable:
            for openNode in openList:
                if adjacent.position == openNode.position:
                    break
            else:
                openList.append(adjacent)

            #if node. + 1 < adjacent.cost:
             #   adjacent.parent = node
              #  adjacent.cost = node.cost + 1
    return []


def chooseNode(openList):
    minCost = 999
    bestNode = None

    for node in openList:
        if node.cost < minCost:
            minCost = node.cost
            bestNode = node
    return bestNode

def getAdjacentNodes(node, startNode):
    adjacentNodes = []
    for direction in DIRECTIONS:
        destination = getDestination(node.position[0], node.position[1], direction)
        for wall in wallList:
            if destination == wall["position"]:
                break
        else:
            adjacentNodes.append(destination)

    return adjacentNodes    



def readline_from_socket(socket):
    buffer = ""
    for data in iter(partial(socket.recv, 1024), ''):
        buffer += data
        temp = buffer.split("\n")
        buffer = temp.pop()
        for line in temp:
            yield line.rstrip()
    yield buffer.rstrip()


class Unit():
    
    def __init__ (self, position, unitType):
        self.position = position
        self.destination = position
        self.destinationDir = ""
        self.unitType = unitType
        self.mission = None
        self.plannedMoves = []


    def __str__(self):
        return "Type: " + self.unitType + ", Position: " + str(self.position)  + ", Mission: " + str(self.mission) + ", Planned moves: " + str(self.plannedMoves)




s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
s.connect(("127.0.0.1", 5050))
name = "WannabeProvo"
s.sendall("name {}\n".format(name))

random.seed(seed)
for line in readline_from_socket(s):
    try:
        d = json.loads(line)
        if "status" in d:
            continue

        sentMoves = {}
        
        if turn != 0:
            if turn == 1:
                firstTurn()
            init()
            sentMoves = {"mode": mode, "moves": moves}

            if logGame == True:
                log("Sent moves: ")
                log(str(sentMoves))

        turn += 1
        
        s.sendall(json.dumps(sentMoves)+"\n")
    except Exception, e:
        print("Error parsing:", e)
