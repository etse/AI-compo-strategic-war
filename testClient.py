from __future__ import print_function, division, unicode_literals

import socket, random
from functools import partial
from itertools import chain
import json
import sys
import pygame
from display import Display
from server import Unit, Harvester, Soldier, GameBoard, Player, Spawner


def readline_from_socket(socket):
    buffer = ""
    for data in iter(partial(socket.recv, 1024), ''):
        buffer += data
        temp = buffer.split("\n")
        buffer = temp.pop()
        for line in temp:
            yield line.rstrip()
    yield buffer.rstrip()


class GameBoardClient(GameBoard):
    def clear(self):
        self.units = []
        self.spawners = []
        self.foodcells = []

        for cell in chain.from_iterable(self.board):
            cell.hasFood = False
            cell.unit = None

    def update(self, map):
        self.clear()
        for cell in iter(map):
            x, y = cell["position"]
            self.board[x][y].unit = None
            if cell.get("spawner", None):
                s = Spawner(cell["spawner"]["owner"], (x, y))
                s.dead = cell["spawner"]["destroyed"]
                self.board[x][y].spawner = s
            if cell.get("unit", None):
                unittype = Unit
                if cell["unit"]["type"] == "harvester":
                    unittype = Harvester
                elif cell["unit"]["type"] == "soldier":
                    unittype = Soldier
                self.add_unit(x, y, unittype, cell["unit"]["owner"])

            self.board[x][y].isWall = cell.get("is_wall", False)
            self.board[x][y].hasFood = cell.get("has_food", False)

    def move_unit(self, x, y, direction):
        unit = self.board[x][y].unit
        if unit is not None:
            newX, newY = x, y
            if direction == "north":
                newY = (y-1) % self.height
            elif direction == "south":
                newY = (y+1) % self.height
            elif direction == "west":
                newX = (x-1) % self.width
            elif direction == "east":
                newX = (x+1) % self.width

            self.board[newX][newY].unit = unit
            unit.position = (newX, newY)


class GameAI:
    def __init__(self, name):
        self.name = name
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.display = None
        self.board = None
        self.players = None
        self.my_id = None

    def sendline(self, line):
        self.sock.sendall(line+"\n")

    def send_command(self, command):
        self.sendline(json.dumps(command))

    def start(self, ip, port):
        self.sock.connect((ip, port))
        self.sendline("name {}".format(self.name))

        for line in readline_from_socket(self.sock):
            try:
                data = json.loads(line)
                if "status" in data:
                    self.handle_status_message(data)
                    continue

                if not self.display:
                    self.board = GameBoardClient(*data["map_size"])
                    self.players = [Player(None, name="Player"+str(i)) for i in xrange(data["num_players"])]
                    self.my_id = data["player_id"]
                    self.players[self.my_id].name = self.name
                    self.display = Display(600, 600, self.board.width, self.board.height)
                    self.display.init()

                self.board.update(data["map"])
                self.display.clear()
                self.display.draw_board(self.board, self.players)
                self.display.update(fps=0)
                self.resolve_round()

                for event in pygame.event.get():
                    if event.type == pygame.QUIT:
                        print("Game terminated by host.")
                        sys.exit(0)

            except (IndexError, ValueError, KeyError), e:
                import traceback
                traceback.print_exc()
                print("Error parsing:", e)

    def handle_status_message(self, command):
        '''
        Implement how to handle status-messages here. The current implementation
        will ignore any "Status OK" and print everything else.

        :param command: The data recieved from server (must be a status message)
        '''
        if command.get("status", "ok").lower() != "ok":
            print("Status: {} - Message: {}".format(command.get("status", "ok"), command.get("msg", "")))

    def resolve_round(self):
        '''
        This function will be called once each turn. Any login with the AI should be implemented here.
        Make sure that this command sends a new command to the server before it returns.

        The map-state is stored in self.board.
        '''

        # Simple implementation - do not move anywhere a unit is currently.
        command = {"mode": random.choice(["standard", "harvester", "soldier"]), "moves": []}
        for unit in filter(lambda u: u.owner == self.my_id, self.board.units):
            x, y = unit.position
            legal_directions = ["north", "south", "west", "east"]
            direction = random.choice(legal_directions)
            command["moves"].append([x, y, direction])
            self.board.move_unit(x, y, direction)
        self.send_command(command)


if __name__ == '__main__':
    myai = GameAI("TestAI"+str(random.randint(0, 1000)))
    myai.start("127.0.0.1", 5050)
