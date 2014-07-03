from __future__ import print_function, division, unicode_literals

import socket
import pygame
import json
import sys
from functools import partial
from display import Display
from server import *


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


s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
s.connect(("127.0.0.1", 5050))

display = None
board = None
players = None

for line in readline_from_socket(s):
    try:
        data = json.loads(line)
        if not display:
            board = GameBoard(*data["map_size"])
            players = [Player(None, name=player["name"]) for player in data["players"]]
            display = Display(800, 800, board.width, board.height)
            display.init()

        update_board(board, data["map"])
        display.clear()
        display.draw_board(board, players)
        display.update(fps=0)

    except Exception, e:
        print("Error parsing:", e.message)

    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            print("Game terminated by host.")
            sys.exit(0)
