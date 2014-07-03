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


def update_board(board, map):
    board.units = []
    board.spawners = []
    board.foodcells = []

    for cell in chain.from_iterable(board):
        cell.hasFood = False

    for cell in iter(map):
        x, y = cell["position"]
        board[x][y].unit = None
        if cell.get("spawner", None):
            s = Spawner(cell["spawner"]["owner"], (x, y))
            s.dead = cell["spawner"]["destroyed"]
            board[x][y].spawner = s
        if cell.get("unit", None):
            unittype = Unit
            if cell["unit"]["type"] == "harvester":
                unittype = Harvester
            elif cell["unit"]["type"] == "soldier":
                unittype = Soldier
            board.add_unit(x, y, unittype, cell["unit"]["owner"])

        board[x][y].isWall = cell.get("is_wall", False)
        board[x][y].hasFood = cell.get("has_food", False)


class MyGameAI:
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
                    self.board = GameBoard(*data["map_size"])
                    self.players = [Player(None, name="Player"+str(i)) for i in xrange(data["num_players"])]
                    self.my_id = data["player_id"]
                    self.players[self.my_id].name = self.name
                    self.display = Display(600, 600, self.board.width, self.board.height)
                    self.display.init()

                update_board(self.board, data["map"])
                self.resolve_round()
                self.display.clear()
                self.display.draw_board(self.board, self.players)
                self.display.update(fps=0)

                for event in pygame.event.get():
                    if event.type == pygame.QUIT:
                        print("Game terminated by host.")
                        sys.exit(0)

            except (IndexError, ValueError, KeyError), e:
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

        # Current implementation will just send a random command to the server
        command = {"mode": random.choice(["standard", "harvester", "soldier"]), "moves": []}
        for unit in filter(lambda u: u.owner == self.my_id, self.board.units):
            x, y = unit.position
            command["moves"].append([x, y, random.choice(["north", "south", "west", "east"])])
        self.send_command(command)


if __name__ == '__main__':
    myai = MyGameAI("TestAI"+str(random.randint(0, 1000)))
    myai.start("127.0.0.1", 5050)
