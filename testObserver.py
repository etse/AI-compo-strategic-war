from __future__ import print_function, division, unicode_literals

import socket
import pygame
import json
import sys
from display import Display
from server import Player

from testClient import readline_from_socket, GameBoardClient

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
            board = GameBoardClient(*data["map_size"])
            players = [Player(None, name=player["name"]) for player in data["players"]]
            display = Display(600, 600, board.width, board.height)
            display.init()

        board.update(data["map"])
        display.clear()
        display.draw_board(board, players)
        display.update(fps=0)

    except Exception, e:
        print("Error parsing:", e.message)

    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            print("Game terminated by host.")
            sys.exit(0)
