from __future__ import print_function, division, unicode_literals

__author__ = 'Steffen Hageland'

import socket
import threading
import time
import json
import pygame
import random
import math
import os
from argparse import ArgumentParser
from display import Display
from functools import partial
from itertools import chain
from copy import copy


class Unit:
    def __init__(self, owner, position):
        self.position = position
        self.owner = owner
        self.harvest = 1
        self.attack = 3
        self.dead = False
        self.hasMoved = False
        self.attackStrength = 0

    @property
    def type(self):
        return self.__class__.__name__.lower()

    def __str__(self):
        return "<{} at {}>".format(self.type, self.position)

    def __repr__(self):
        return self.__str__()


class Harvester(Unit):
    def __init__(self, owner, position):
        Unit.__init__(self, owner, position)
        self.harvest = 2
        self.attack = 2


class Soldier(Unit):
    def __init__(self, owner, position):
        Unit.__init__(self, owner, position)
        self.harvest = 0
        self.attack = 5


class Spawner(Unit):
    def __init__(self, owner, position):
        Unit.__init__(self, owner, position)


class BoardCell:
    def __init__(self, x, y):
        self.isWall = False
        self.spawner = None
        self.hasFood = False
        self.unit = None
        self.newUnit = None
        self.x = x
        self.y = y

    def as_dict(self):
        d = {"position": (self.x, self.y)}
        if self.isWall:
            d["is_wall"] = True
        if self.hasFood:
            d["has_food"] = True
        if self.spawner:
            d["spawner"] = {"owner": self.spawner.owner, "destroyed": self.spawner.dead}
        if self.unit:
            d["unit"] = {"owner": self.unit.owner, "type": self.unit.type}
        return d

    def empty(self):
        return not (self.isWall or self.spawner or self.hasFood or self.unit or self.newUnit)


class GameBoard:
    def __init__(self, width, height):
        self.width = width
        self.height = height
        self.board = [[BoardCell(x, y) for y in xrange(height)] for x in xrange(width)]
        self.spawners = []
        self.units = []
        self.foodcells = []
        self._offsetcache = {}

    def __getitem__(self, item):
        return self.board[item]

    def get_real_position(self, x, y):
        return x % self.width, y % self.height

    def any_units_on_position(self, position):
        return self.board[position[0]][position[1]].unit is not None

    def add_wall(self, x, y):
        self.board[x][y].isWall = True

    def add_spawner(self, x, y, owner):
        spawner = Spawner(owner, (x, y))
        self.board[x][y].spawner = spawner
        self.spawners.append(spawner)

    def add_unit(self, x, y, unit, owner):
        newUnit = unit(owner, (x, y))
        self.board[x][y].unit = newUnit
        self.units.append(newUnit)

    def spawn_food(self):
        for x in range(10):
            cell = random.choice(random.choice(self.board))
            if cell.empty():
                self.foodcells.append(cell)
                cell.hasFood = True
                return True

    def move_unit(self, x, y, owner, direction):
        unit = self.board[x][y].unit
        if unit is None and VERBOSE:
            print("MOVEMENT: A player tried to move a unit on an empty cell, the cell was ({}, {})".format(x, y))
        elif unit is not None:
            if unit.owner == owner and not unit.hasMoved:
                newX, newY = x, y
                if direction == "north":
                    newY = (y-1) % self.height
                elif direction == "south":
                    newY = (y+1) % self.height
                elif direction == "west":
                    newX = (x-1) % self.width
                elif direction == "east":
                    newX = (x+1) % self.width

                if not (self.board[newX][newY].isWall or self.board[newX][newY].hasFood):
                    unit.hasMoved = True
                    if self.board[newX][newY].newUnit is not None:
                        # Someone else has moved here, kill the unit and do not move
                        self.board[newX][newY].newUnit.dead = True
                        self.units.remove(unit)
                        self.board[x][y].unit = None
                        if VERBOSE:
                            print("COLLISION: Two units moved to ({}, {}), the last unit came from ({}, {})".format(newX, newY, x, y))
                    else:
                        # Lets move the unit
                        self.board[newX][newY].newUnit = unit
                        unit.position = (newX, newY)
                        self.board[x][y].unit = None
                    return True
                elif VERBOSE:
                    print("MOVEMENT: A player tried to move a unit into a cell that contains a wall or food. ({}, {}), with direction: {}.", x, y, direction)
            elif VERBOSE and unit.owner != owner:
                print("MOVEMENT: A Player tried to move a unit that he does not own on cell ({}, {})".format(x, y))
            elif VERBOSE and unit.hasMoved:
                print("MOVEMENT: A Player tried to move a unit that has already moved on cell ({}, {})".format(x, y))
        return False

    def resolve_moves(self):
        # If 2 units moved to the same space, one will still be left
        # So we need to remove that one now
        deadUnits = []
        for unit in self.units:
            if unit.dead:
                deadUnits.append(unit)
                x, y = unit.position
                self.board[x][y].newUnit = None
                self.board[x][y].unit = None
        self.units = filter(lambda unit: unit not in deadUnits, self.units)

        # To resolve the move, units need to be moved from .newUnit to .unit
        # This might cause a conflict if the old unit stood still - in that case
        # .unit is not None and both units should be killed.
        for column in self.board:
            for cell in column:
                if cell.newUnit is not None and cell.unit is not None:
                    if VERBOSE:
                        print("COLLISION: Two units on {}, A unit was already standing on this cell (and did not move) and another unit moved here to it.".format(cell.unit.position))
                    if cell.newUnit in self.units:
                        self.units.remove(cell.newUnit)
                    if cell.unit in self.units:
                        self.units.remove(cell.unit)
                    cell.unit = None
                    cell.newUnit = None

                if cell.newUnit is not None:
                    cell.unit = cell.newUnit
                if cell.unit is not None:
                    cell.unit.hasMoved = False
                cell.newUnit = None

    def calculate_attack_strengths(self):
        for unit in self.units:
            x, y = unit.position
            unit.attackStrength = sum(u.attack for u in self.get_neighbour_enemy_units(x, y, 5, unit.owner))

    def get_offsets(self, distance):
        if distance not in self._offsetcache:
            offsets = []
            radius = int(math.sqrt(distance))
            for x in xrange(-radius, radius+1):
                for y in xrange(-radius, radius+1):
                    if 0 < x**2 + y**2 <= distance:
                        offsets.append((x, y))
            self._offsetcache[distance] = offsets
        return self._offsetcache[distance]

    def get_neighbour_cells(self, x, y, distance):
        return [self.board[(x+dx) % self.width][(y+dy) % self.height] for dx, dy in self.get_offsets(distance)]

    def get_neighbour_enemy_units(self, x, y, distance, player):
        return [cell.unit for cell in filter(lambda c: c.unit is not None and c.unit.owner != player,
                                             self.get_neighbour_cells(x, y, distance))]


class Player(threading.Thread):
    def __init__(self, socket, name="player"):
        threading.Thread.__init__(self)
        self.socket = socket
        self.ready = False
        self.connected = True
        self.daemon = True
        self.name = name
        self.food = 5
        self.mode = Unit
        self.command = {}
        self._latest_command = {}
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

    def start_next_turn(self):
        self.command = self._latest_command
        self._latest_command = {}

    def read_commands(self):
        if not self.connected:
            self._latest_command = {}
            return
        try:
            for line in self.readline_from_socket():
                try:
                    command = json.loads(line)
                    self.send_ok()
                    if type(command) is dict:
                        self._latest_command = command
                except ValueError:
                    self.send_error("Could not parse the command. Probably invalid JSON or command split over multiple lines.")
        except socket.error:
            self.connected = False
            print("Player {} has disconnected...".format(self.name))

    def send_gamestate(self, gamestate):
        self.send_line(json.dumps(gamestate))

    def send_ok(self):
        self.send_line('{"status": "OK"}')

    def send_error(self, msg):
        print("({})Error: {}".format(self.name, msg))
        self.send_line('{{"status": "ERROR", "msg": "{}"}}'.format(msg))

    def send_line(self, line):
        if not self.connected:
            return
        try:
            self.socket.sendall(line+"\n")
        except socket.error:
            self.connected = False
            print("Player {} has disconnected...".format(self.name))

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
        yield self.buffer.rstrip()


class GameServer:
    def __init__(self, port, mapfile, rounds_per_second, w, h, observers):
        self._port = port
        self.players = []
        self.observers = []
        self.numObservers = observers
        self.numPlayers = 0  # Will be overwritten by loadmap. (but included here to make PyCharm happy)
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.bind(('', port))
        self.socket.listen(3)
        self.loadmap(mapfile)
        self.display = Display(w, h, self.board.width, self.board.height)
        self.rounds_per_second = rounds_per_second

    def start(self):
        print("Server started on port {}.".format(self._port))
        self.wait_for_observsers()
        self.wait_for_players()
        print("All players are ready, game is now starting.")
        self.send_gamestate_to_observers(unfiltered=True)

        self.display.init()
        delay = 10
        while True:
            self.display.clear()

            self.start_next_turn()
            self.resolve_food_harvest()
            self.move_and_spawn_units()

            self.resolve_fights()
            self.destroy_spawners()
            if random.randrange(0, 100) < 10:
                self.board.spawn_food()

            self.send_gamestate_to_players()
            self.send_gamestate_to_observers()

            self.display.draw_board(self.board, self.players)
            while delay > 0:
                delay -= 1
                self.display.update(self.rounds_per_second)
                for event in pygame.event.get():
                    if event.type == pygame.QUIT:
                        print("Game terminated by host.")
                        return True
            delay = 1

    def wait_for_observsers(self):
        print("Waiting for {} observer(s) to connect...".format(self.numObservers))
        for _ in xrange(self.numObservers):
            observer = self.get_player_from_socket()
            observer.name = "Observer"
            self.observers.append(observer)

    def wait_for_players(self):
        print("Waiting for {} player(s) to connect...".format(self.numPlayers))
        for _ in xrange(self.numPlayers):
            player = self.get_player_from_socket()
            player.start()
            self.players.append(player)

        while not self.check_players_ready():
            time.sleep(0.5)

    def start_next_turn(self):
        for player in self.players:
            player.start_next_turn()

    def get_player_from_socket(self):
        conn, addr = self.socket.accept()
        print("Recieved new connection from {}:{}".format(*addr))
        return Player(conn)

    def check_players_ready(self):
        for player in self.players:
            if not player.ready:
                return False
        return True

    def move_and_spawn_units(self):
        for playerNum, player in enumerate(self.players):
            # Set new spawning mode for the player
            mode = player.command.get("mode", "standard")
            if mode == "standard":
                player.mode = Unit
            elif mode == "harvester":
                player.mode = Harvester
            elif mode == "soldier":
                player.mode = Soldier

            # move all the units he sent a command for
            legal_moves = []  # Used so we do not send loads of illegal-moves to the observers
            for move in player.command.get("moves", []):
                try:
                    x, y, direction = move
                    if self.board.move_unit(x, y, playerNum, direction):
                        legal_moves.append(move)
                except (IndexError, ValueError, TypeError), e:
                    print("{} sent an invalid move-command: '{}' Exception: {}".format(player.name, move, e.message))
            player.command["moves"] = legal_moves
        self.board.resolve_moves()

        # Spawn new units
        spawners = copy(self.board.spawners)
        random.shuffle(spawners)
        for spawner in filter(lambda s: not s.dead, spawners):
            owner = self.players[spawner.owner]
            if owner.food > 0:
                if not self.board.any_units_on_position(spawner.position):
                    self.board.add_unit(spawner.position[0], spawner.position[1],  owner.mode, spawner.owner)
                    owner.food -= 1

    def resolve_fights(self):
        self.board.calculate_attack_strengths()
        deadUnits = []
        for unit in self.board.units:
            x, y = unit.position
            for enemy in self.board.get_neighbour_enemy_units(x, y, 5, unit.owner):
                if unit.attackStrength >= enemy.attackStrength:
                    unit.dead = True
                    if unit not in deadUnits:
                        deadUnits.append(unit)
                    self.display.draw_attack(enemy, unit)

        for unit in deadUnits:
            x, y = unit.position
            self.board[x][y].unit = None
            self.board.units.remove(unit)

    def destroy_spawners(self):
        for spawner in filter(lambda s: not s.dead, self.board.spawners):
            x, y = spawner.position
            if self.board[x][y].unit is not None:
                if self.board[x][y].unit.owner != spawner.owner:
                    spawner.dead = True

    def resolve_food_harvest(self):
        removedCells = []
        for foodcell in self.board.foodcells:
            players = set()
            harvest = 0
            neighbourCells = self.board.get_neighbour_cells(foodcell.x, foodcell.y, 1)

            for cell in filter(lambda c: c.unit is not None and c.unit.type != "soldier", neighbourCells):
                players.add(cell.unit.owner)
                if cell.unit.harvest > harvest:
                    harvest = cell.unit.harvest
                foodcell.hasFood = False
                removedCells.append(foodcell)
                break

            if len(players) == 1:
                self.players[players.pop()].food += harvest

        for cell in removedCells:
            self.board.foodcells.remove(cell)

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

    def send_gamestate_to_players(self):
        for i, player in enumerate(self.players):
            state = {"map_size": (self.board.width, self.board.height), "player_id": i, "num_players": self.numPlayers}
            units = filter(lambda unit: unit.owner == i, self.board.units)
            cells = set()
            for unit in units:
                x, y = unit.position
                cells.add(self.board[x][y])
                cells |= set(filter(lambda cell: not cell.empty(), self.board.get_neighbour_cells(x, y, 55)))
            state["map"] = [cell.as_dict() for cell in cells]
            player.send_gamestate(state)

    def send_gamestate_to_observers(self, unfiltered=False):
        if self.numObservers == 0:
            return

        players = []
        for i, player in enumerate(self.players):
            players.append({"id": i, "name": player.name, "food": player.food, "command": player.command})
        state = dict(map_size=(self.board.width, self.board.height), players=players)
        cells = filter(lambda c: (not c.empty()) and (unfiltered or not c.isWall), chain.from_iterable(self.board))
        state["map"] = [cell.as_dict() for cell in cells]

        for observer in self.observers:
            observer.send_gamestate(state)


def readCommandlineArguments():
    parser = ArgumentParser(description="Runs a simple server for the compo.")
    parser.add_argument("mapfile", help="The map to be used for the game.")
    parser.add_argument('-p', '--port', type=int, help='The port to listen for incoming connections.', default=5050)
    parser.add_argument('-v', '--verbose', help='Turns on verbose printing', default=False, action='store_true')
    parser.add_argument("--force-onscreen", type=bool, help="Try to force the windows to spawn on screen.", default=False)
    parser.add_argument("-r", "--resolution", help="Resoltuion given in the format x,y.", default="800,800")
    parser.add_argument("-o", "--observers", type=int, help="Number of observers to use.", default=0)
    parser.add_argument("-f", "--fps", type=int,
                        help="The update frequency of the game. Each frame is 1 round in the game.", default=4)
    return vars(parser.parse_args())


if __name__ == '__main__':
    random.seed(42)
    args = readCommandlineArguments()
    VERBOSE = args['verbose']
    if args["force_onscreen"]:
        os.environ['SDL_VIDEO_WINDOW_POS'] = "50,50"

    try:
        res = args["resolution"].split(",")
        w, h = int(res[0]), int(res[1])
    except IndexError, ValueError:
        print("Invalid argument given as resolution.")
    else:
        server = GameServer(args['port'], args['mapfile'], args["fps"], w, h, observers=args["observers"])
        server.start()

