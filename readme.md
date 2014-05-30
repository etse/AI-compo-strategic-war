# Strategic war AI-compo
A competition in creating Artificial intelligence based on the [Ants AI challenge](http://ants.aichallenge.org/).

## How to run server
The game server is written using Python 2.7 with PyGame, so make sure you have those installed.
In order to run the server it is as simple as **python server.py simple.map -p 5050**, where simple.map
is the map-file to be used and -p 5050 specifies which port the server will listen for incoming
connections on.

The included test-client is a simple client that sends random movement-commands each turn. This can
be used as a starting point to see how to communicate with the server.

## Problem description
The objective is to create an artificial intelligence that can play the specified game as strategic
as possible. As the competition goes on bugs in the server might be fixed, and libraries for different
languages might be created by either the author or any of the participants. So keep your eyes open
for any updates in the repository.

The game is turn-based and plays out on a simple grid that wraps around on the edges. The means that
any unit walking across the top of the map will appear at the bottom.

The bot will get feeded information visible by his units each turn, and should respond with his
commands for all his units. In order to win the game it is important to make sure that all your
units work together - and a strategic tips: like in most RTS-games establishing map-control is
the easiest way to win.

The 2 main concepts of the game are:

1. Collect food to spawn new units.
2. Hunt down and destroy enemy spawners.

Along the way you will meet enemy units, when this happend you have to choose if you want to fight
or flee. In situations where you end up fighting them the war-turns is resolved using the an
implementation of majority-wins that allows for flanking. For more details read the game specifications.

## Game specifications
The game consists of turns which are resolved in the following order:

1. Harvest any nearby food
2. Move units
3. Spawn new units
4. Resolve battles
5. Destroy spawners
6. Spawn new food

Everything that happens in a specific event is considered to be done simultaneously - in fights this means
that unit **A** can kill unit **B** simultaneously as unit **B** kills unit **A**, resulting in both units
dead.

Using the order of execution we can also see that each turn we can move a nearby unit on top of a spawner
to block it from spawning new units.

In this game *distance* between objects is important, and for combat-range and viewdistance it is important
that both the client and server handles it the same way if you are to expect your bot to behave correctly.
Distances are measured as *squered-distance*.

Eample: Lets say we have a unit A on position (2, 5) and a unit B on position (4, 3). To calculate the
distance between them we would do: ```(4-2)*(4-2) + (3-5)*(3-5) = 8```. As vision-range is set to
55 this means that the units are able to see each other. But with an attack-range of 5 they are not
able to fight.

```
Attack ranges for unit A:
.......    ...9...
..xxx..    .85458.
.xxxxx.    .52125.
.xxAxx.    9410149
.xxxxx.    .52125.
..xxx..    .85458.
.......    ...9...
```

### Movement
Movement in the game can be done in one of four different directions: north, south, east and west.
Each unit can only move 1 tile each turn and is not allowed to move into any block currently containing
either food or a wall.

If 2 or more units move into the same tile they will automatically end up in a big fist-fight. The result
will end with all of them getting killed and removed from the game.

### Harvesting food
In order to harvest food all you have to do is to make sure you have a unit (not a soldier) right
next to in (technically within a range of 1). By doing this you will automatically collect the food
and receive the bonus if it was harvested by a harvester.

If both you and an enemy is standing next to the same block of food you will both try to grab it, which
will result in it breaking and neither getting any food.

### Spawning of new units
In order to spawn new units the following three criteria must be met:

1. The player must have at least 1 unit of food
2. The player must have a spawner that is not destroyed
3. The spawner shall not be blocked by any unit standing on top of it

Each spawner will try to spawn a new unit each turn, where the selected priority is chosen at
random. If you do not want a specific spawner to spawn units you can block it by moving one of
your own units on top of it.

There are three different types of units that can be spawned, chosen by the player.

1. **Standard:** This is the most basic unit. This unit can collect food and has an attack-strength
of 4. Identified by a a circle on the map.
2. **Harvester:** This unit will get double value for each food collected, but has weaker attack with
a strength of 2. This unit has a vertical line through it
3. **Soldier:** This unit is a pure fighting unit with an attack-strength of 5, but he is incapable
of collecting any food. This unit has a horizontal line through its body.

### Battle resolvement
During the battle-resolvement step the game will check which units will die, and which get to survive.
This is done using a simple algorithm:

For each unit the game calculates the collective strength of all units inside his attack-radius (currently
set to 5). If any unit inside his attack radius has lower or equal attack-strength of his combined
enemies, the unit will be marked for death. As all fights happen simultaneously no units will actually
die before all fights have been resolved.

In very simple psaudo-code it is something like this:
```python
for unit on board:
    unit.score = sum(enemy.attackstrength for enemy in attackradius)
    if unit.score >= min(enemy.score for enemy in attackradius):
        unit.dead = True
```

Lets take an example, with Alice and Bob. For the sake of simplicity lets consider that they are
both using a simple unit with 1 in attack.

They got their units in following confrontation;
```
.b.
...
.a.
```
In this setup both Bob and Alice will get attacked by a strength of 1. This means both their units die.

Now look at an example of a wall of 5 units against another wall of 5 units:
```
..bbbbb..
.........
..aaaaa..
```
Now lets calculate the attack-values on all of the units:
```
..233332..
..........
..233332..
```
Again, everyone dies. Even though a few units have lower attack-strength, every unit has at least
1 unit inside his attack-radius which has lower or equal strength. For the ones that got a score of 2
the other unit with 2 standing straight in front of him will kill him.

Now let us take a look at a big wall-push. A good way to push into enemy-ranks if you manage to flank
them. In this case A is flanking B with a big wall-punch
```
AAAAAAAAA    013565310    AAxxxxxAA
...BBB... -> ...555... -> ...xxx...
...BBB...    ...333...    ...xBx...
```
* The B ant lives because it is only attacked by the 3 center A ants, and each of those ants are more occupied.
    In turn, it participates in the death of the 3 center A ants.
* The A ants on the end live because they are only attacked by 1 B ant which is much more occupied.

In our examples we have only used simple scenarioes with units with same attach-strength, but the idea
is pretty much the same even with different kinds of units. For instance a soldier with his strength of
5 kill kill a standard unit with an attack of 3, but if 2 standard units join up they will get a combined
strength of 6 and be able to kill a soldier without any casualties.

### Destroying spawners
Destroying an enemy spawner will restrict it from spawning any more units throughout the game. This is
a very good way to establish map-control and restrict possible attack vectors from you enemy. To do this
all you have to do is move one of your units on top of the spawner and make sure it survives to the end
of turn.

## The protocol
Each command sent between client and server shall always be 1 line, and **must** end with a linux-style
newline: \n. All commands but the first, which you use to register a name with the server must be in
valid JSON.

The server will always respond with a status-message if you send a command. This can be either
status OK, which means the server successfully parsed your command or status ERROR, which usually
means you sent invalid JSON or forgot a newline.

### Registering with server
When the client connects to the server the first thing it should do is register a name to tell the
server you are ready. This is done by sending the command following command:
```name [yourname]```. As allways, remember to end it with a newline.

```
Example:
>>> name compoBot\n
<<< {"status": "ok"}\n
```

### Sending commands to the server
The commands sent to the server should be in JSON of the following formatting:
```
{"mode": spawn_mode, "moves": [[x, y, direction], [x, y, direction] ...]}\n
```
Mode refers to which mode you want the spawners to be in. This can be set to *standard*, *soldier* or
*harvester*. All units spawned this turn will be of the specified type.

Moved should be a list of all moved you want to do this turn. X and Y refers to the coordinates
of the unit you want to move while direction is in which direction that unit will be moved. (This should
be a string with the value *north*, *south*, *west* or *east*.

If you try to move the same unit twice, or try to move an unit you do not own, the specific move is
ignored by the server - while the others are executed as normal.

### States recieved by the server
Each turn the game will send you the current state of the game, from your perspective. This means that it
will only send you data visible by any of your units. Keep in mind that the server will not send you information about your own spawners unless you have visibility
of it. To remember to save the location if you move your units away. (if you leave your
spawner outside of vision you have no way of knowing if it has been destroyed).

In order to save some network-traffic (and to lower the amount of JSON that has to be parsed by
clients - and cell that is empty, and in vision range, will not be sent. It is up to the client
to understand that this implicitly means that the cell is empty.

Any information about what a cell isn't is left out. That means there will not be any information
telling you that a cell is **not** a well, instead the client can assume that a cell is not a wall
unless stated otherwise. 

Additionally it will include your player-id and the total size of the map.
```
{"map_size": [10, 10], "player_id": 0, "map": [...]}\n
```
*map_size* is the width and height of the current map, and *player_id* is your ID in the game. This is used
to be able to distinguish your units from enemy units on the map.

*map* is a list of all cells currently visible by one of your units. Each cell has the format shown below
```
With a spawner and unit:
{"position": [x, y], "spawner": {"owner": 0, "destroyed": False}, "unit": {"owner": 0, "type": "standard"}}

No unit and no spawner, but is a wall:
 {"position": [x, y], "is_wall": True}

A cell that has food:
 {"position": [x, y], "has_food": True}
```


## Getting started
Some good starting tips is to start by trying to implement the following features:

1. Make sure your units does not collide with each other.
2. Get them to explore and create a map of your surroundings.
3. Get the units to collect any food it finds.

When those 3 features are implemented you should have a good starting point for trying to implement
more advanced tactics and strategies.

## Creating maps
A map is created in simple ASCII where the first line conists of meta-data about the map in the
following format: ``` width height numberOfPlayers ```

The rest of the map will be an ASCII-art representing the layout of the map, using the following
symbols:

* **#:## A wall in the game.
* **.:** An empty cell.
* **0-9:** A number represents a spawner owned by the player with specified ID. 

Remember that the upper-left corner has the coordinates (0,0), just like normal screen coordinates.

## TODO:
* Add animations
    * Movement
    * Death-animation
* Create python library

