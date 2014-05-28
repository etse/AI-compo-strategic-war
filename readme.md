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

### Harvesting food

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

Lets take an example, with Alice and Bob. They are currently only using normal units with an attack
of 2. They got their units in following confrontation;
```
.b.
...
.a.
```
In this setup both Bob and Alice will get attacked by a strength of 2. This means both their units die.

Now look at an example of a wall of 5 units against another wall of 5 units:
```
..bbbbb..
.........
..aaaaa..
```
Now lets calculate the attack-values on all of the units:
```
..466664..
..........
..466664..
```

### Destroying spawners

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
How to send commands

### States recieved by the server
Each turn the game will send you the current state of the game, from your perspective. This means that it
will only send you data visible by any of your units. Keep in mind that the server will not send you information about your own spawners unless you have visibility
of it. To remember to save the location if you move your units away. (Additionally: if you leave your
spawner outside of vision you have no way of knowing if it has been destroyed.

Additionally it will include your player-id and the total size of the map.

## Getting started
Some good starting tips is to start by trying to implement the following features:

1. Make sure your units does not collide with each other.
2. Get them to explore and create a map of your surroundings.
3. Get the units to collect any food it finds.

When those 3 features are implemented you should have a good starting point for trying to implement
more advanced tactics and strategies.

## Creating maps
Will write this later. For now: just use the standard map, or create one by using it as example.

## TODO:
* Send data to user
    * Based on visibility

* Add animation
    * Movement
    * Death-animation
* Create python library
* Complete the readme-file

