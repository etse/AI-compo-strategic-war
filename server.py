from __future__ import print_function, division, unicode_literals

__author__ = 'Steffen'

import socket
import threading
import time
import json
import pygame
import random
from argparse import ArgumentParser
from display import Display
from functools import partial
from copy import copy


class BoardCell:
    def __init__(self):
        self.isWall = False
        self.spawner = None
        self.hasFood = False
        self.unit = None
        self.newUnit = None


class GameBoard:
    def __init__(self, width, height):
        self.width = width
        self.height = height
        self.board = [[BoardCell() for _ in xrange(height)] for _ in xrange(width)]
        self.spawners = []
        self.units = []
        self.food = []

    def __getitem__(self, item):
        return self.board[item]

    def any_units_on_position(self, position):
        return self.board[position[0]][position[1]].unit is not None

    def add_wall(self, x, y):
        self.board[x][y].isWall = True

    def add_spawner(self, x, y, owner):
        spawner = Spawner(owner, (x, y))
        self.board[x][y].spawner = spawner
        self.spawners.append(spawner)

    def add_unit(self, x, y, owner):
        newUnit = Unit(owner, (x, y))
        self.board[x][y].unit = newUnit
        self.units.append(newUnit)

    def spawn_food(self):
        # TODO: Add spawning food
        pass

    def move_unit(self, x, y, owner, direction):
        if self.board[x][y].unit is not None:
            if self.board[x][y].unit.owner == owner and not self.board[x][y].unit.hasMoved:
                newX, newY = x, y
                if direction == "north":
                    newY = (y-1) % self.height
                elif direction == "south":
                    newY = (y+1) % self.height
                elif direction == "west":
                    newX = (x-1) % self.width
                elif direction == "east":
                    newX = (x+1) % self.width

                if not self.board[newX][newY].isWall:
                    self.board[x][y].unit.hasMoved = True
                    if self.board[newX][newY].newUnit is not None:
                        # Someone else has moved here, kill the unit and do not move
                        self.board[newX][newY].newUnit.dead = True
                        self.board[x][y].unit.dead = True
                        self.units.remove(self.board[x][y].unit)
                    else:
                        # Lets move the unit
                        self.board[newX][newY].newUnit = self.board[x][y].unit
                        self.board[newX][newY].newUnit.position = (newX, newY)
                        self.board[x][y].unit = None

    def resolve_moves(self):
        # If 2 units moved to the same space, one will still be left
        # So we need to remove that one now
        deadUnits = []
        for unit in self.units:
            if unit.dead:
                deadUnits.append(unit)
                x, y = unit.position
                self.board[x][y].newUnit = None
        self.units = [unit for unit in self.units if unit not in deadUnits]

        # To resolve the move, units need to be moved from .newUnit to .unit
        # This might cause a conflict if the old unit stood still - in that case
        # .unit is not None and both units should be killed.
        for column in self.board:
            for cell in column:
                if cell.newUnit is not None and cell.unit is not None:
                    try:
                        self.units.remove(cell.newUnit)
                        self.units.remove(cell.unit)
                    except ValueError:
                        pass

                    cell.newUnit = None
                    cell.unit = None
                if cell.newUnit is not None:
                    cell.unit = cell.newUnit
                    cell.newUnit = None
                    cell.unit.hasMoved = False


class Unit:
    def __init__(self, owner, position):
        self.position = position
        self.owner = owner
        self.harvest = 1
        self.attack = 4
        self.dead = False
        self.hasMoved = False
        self.numAttackers = 0


class Harvester(Unit):
    def __init__(self, owner, position):
        Unit.__init__(self, owner, position)
        self.harvest = 2
        self.attack = 3


class Soldier(Unit):
    def __init__(self, owner, position):
        Unit.__init__(self, owner, position)
        self.harvest = 0
        self.attack = 5


class Spawner(Unit):
    def __init__(self, owner, position):
        Unit.__init__(self, owner, position)
        self.destroyed = False


class Player(threading.Thread):
    def __init__(self, socket):
        threading.Thread.__init__(self)
        self.socket = socket
        self.ready = False
        self.daemon = True
        self.name = ""
        self.score = 0
        self.food = 5
        self.mode = Unit
        self.latest_command = {}
        self.buffer = ""

    def run(self):
        self.name = self.get_playername()
        print("{} is ready.".format(self.name))
        self.send_ok()
        self.ready = True
        self.read_commands()

    def get_playername(self):
        for line in self.readline_from_socket():
            try:
                if line.lower().startswith("name "):
                    return line.split(" ")[1].rstrip()
                else:
                    self.send_error("Invalid command, first command should be: NAME [YOURNAME]")
            except IndexError:
                self.send_error("Could not parse the command. Did it follow protocol?")

    def read_commands(self):
        for line in self.readline_from_socket():
            try:
                command = json.loads(line)
                self.send_ok()
                self.latest_command = command
            except ValueError:
                self.send_error("Could not parse the command. Probably invalid JSON or command split over multiple lines.")

    def send_gamestate(self, gamestate):
        self.socket.send("{}\n".format(json.dumps(gamestate)))

    def send_ok(self):
        self.socket.sendall('{"status": "OK"}\n')

    def send_error(self, msg):
        print("({})Error: {}".format(self.name, msg))
        self.socket.sendall('{{"status": "ERROR", "msg": "{}"}}\n'.format(msg))

    def readline_from_socket(self):
        temp = self.buffer.split("\n")
        temp.pop()
        for line in temp:
            self.buffer = "\n".join(self.buffer.split("\n")[1:])
            yield line.rstrip()

        for data in iter(partial(self.socket.recv, 1024), ''):
            self.buffer += data.lower()
            temp = self.buffer.split("\n")
            temp.pop()
            for line in temp:
                self.buffer = "\n".join(self.buffer.split("\n")[1:])
                yield line.rstrip()
        yield buffer.rstrip()


class GameServer:
    def __init__(self, port, mapfile):
        self._port = port
        self.players = []
        self.numPlayers = 2
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.bind(('', port))
        self.socket.listen(3)
        self.display = Display(640, 640)
        self.loadmap(mapfile)

    def start(self):
        print("Server started on port {}.".format(self._port))
        self.wait_for_players()
        print("All players are ready, game is now starting.")

        self.display.init()
        while True:
            self.resolve_food_harvest()
            self.move_and_spawn_units()
            self.resolve_fights()

            if random.randrange(0, 10) < 3:
                self.board.spawn_food()
                self.board.spawn_food()

            self.display.clear()
            self.display.draw_board(self.board)

            # Ugly way to keep 30 fps while pretending to be 1 fps
            for _ in range(6):
                self.display.update(30)
                for event in pygame.event.get():
                    if event.type == pygame.QUIT:
                        print("Game terminated by host.")
                        return True

    def wait_for_players(self):
        print("Waiting for {} players to connect...".format(self.numPlayers))
        for _ in xrange(self.numPlayers):
            conn, addr = self.socket.accept()
            print("Recieved new connection from {}:{}".format(*addr))
            player = Player(conn)
            player.start()
            self.players.append(player)

        while not self.check_players_ready():
            time.sleep(0.5)

    def check_players_ready(self):
        for player in self.players:
            if not player.ready:
                return False
        return True

    def move_and_spawn_units(self):
        for playerNum, player in enumerate(self.players):
            # Set new spawning mode for the player
            mode = player.latest_command.get("mode", "standard")
            if mode == "standard":
                player.mode = Unit
            elif mode == "harvester":
                player.mode = Harvester
            elif mode == "soldier":
                player.mode = Soldier

            # move all the units he sent a command for
            for x, y, direction in player.latest_command.get("moves", []):
                try:
                    self.board.move_unit(x, y, playerNum, direction)
                except IndexError:
                    print("{} sent an invalid move-command: {}, {}, {}".format(player.name, x, y, direction))
            player.latest_command = {}
        self.board.resolve_moves()

        # Spawn new units
        spawners = copy(self.board.spawners)
        random.shuffle(spawners)
        for spawner in filter(lambda s: not s.destroyed, spawners):
            if self.players[spawner.owner].food > 0:
                if not self.board.any_units_on_position(spawner.position):
                    self.board.add_unit(spawner.position[0], spawner.position[1], spawner.owner)
                    self.players[spawner.owner].food -= 1

    def resolve_fights(self):
        # TODO: Implement resolvement of fights
        pass

    def resolve_food_harvest(self):
        # TODO: How to resolve gathering food?
        pass

    def loadmap(self, mapfile_path):
        with open(mapfile_path) as mapfile:
            line = mapfile.readline().rstrip().split(" ")
            width, height, numPlayers = int(line[0]), int(line[1]), int(line[2])
            self.numPlayers = numPlayers
            self.board = GameBoard(width, height)
            for y in xrange(height):
                line = mapfile.readline().rstrip()
                for x in xrange(width):
                    if line[x] == "#":
                        self.board.add_wall(x, y)
                    if line[x].isdigit():
                        self.board.add_spawner(x, y, int(line[x]))


def readCommandlineArguments():
    parser = ArgumentParser(description="Runs a simple server for the compo")
    parser.add_argument("mapfile", help="The map to be used for the game.")
    parser.add_argument('-p', '--port', type=int, help='The port to listen for incoming connections.', default=5050)
    return vars(parser.parse_args())


if __name__ == '__main__':
    random.seed(42)
    args = readCommandlineArguments()
    server = GameServer(args['port'], args['mapfile'])
    server.start()

