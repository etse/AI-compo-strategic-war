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

### Destroying spawners

## The protocol

## Creating maps

## TODO:
* Make it possible to see type of unit
* Complete the fight-resolvements
* Send data to user
    * Based on visibility

* Add animation
    * Movement
    * Death-animation
    * Shooting
* Create python library
* Complete the readme-file

