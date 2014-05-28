# Strategic war AI-compo
A competition in creating Artificial intelligence based on the [Ants AI challenge](http://ants.aichallenge.org/).

## How to run server
The game server is written using Python 2.7 with PyGame, so make sure you have those installed.
In order to run the server it is as simple as "python server.py simple.map -p 5050", where simple.map
is the map-file to be used and -p 5050 specifies which port the server will listen for incoming
connections on.

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

## Creating maps

## The protocol

## TODO:
* Add animation
    * Movement
    * Death-animation
    * Shooting
* Make it possible to see type of unit
* Create python library
* Complete the readme-file
* Complete the fight-resolvements
* Complete food-gathering
* Add food spawning
