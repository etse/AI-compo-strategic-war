from __future__ import print_function, division, unicode_literals

WALLCOLOR = (50, 50, 50)
PLAYERCOLORS = [(150, 0, 150), (255, 150, 0), (0, 255, 100), (255, 255, 0)]
FOODCOLOR = (140, 40, 40)

import pygame
from pygame.locals import *

class Display:
    def __init__(self, width, height):
        self.width = width
        self.height = height
        self.clock = None
        self.screen = None

    def init(self):
        pygame.init()
        self.clock = pygame.time.Clock()
        self.screen = pygame.display.set_mode((self.width, self.height))

    def clear(self):
        self.screen.fill((0, 0, 0))

    def update(self, fps=30):
        pygame.display.flip()
        self.clock.tick(fps)

    def draw_board(self, board):
        h = self.height / board.height
        w = self.width / board.width
        for x in xrange(board.width):
            for y in xrange(board.height):
                if board[x][y].isWall:
                    pygame.draw.rect(self.screen, WALLCOLOR, (w*x, h*y, w+1, h+1))
                if board[x][y].spawner:
                    pygame.draw.rect(self.screen, PLAYERCOLORS[board[x][y].spawner.owner], (w*x, h*y, w+1, h+1), 1)
                if board[x][y].hasFood:
                    pygame.draw.rect(self.screen, FOODCOLOR, (w*x, h*y, w+1, h+1))

        for unit in board.units:
            px, py = int(unit.position[0]*w+w/2), int(unit.position[1]*h+h/2)
            pygame.draw.circle(self.screen, PLAYERCOLORS[unit.owner], (px, py), int(w/2), 0)
            if unit.type == "harvester":
                start = (px-1, py-int(h/2))
                end = (px-1, py+int(h/2))
                pygame.draw.line(self.screen, (0, 0, 0), start, end, 2)
            elif unit.type == "soldier":
                start = (px-int(w/2), py-1)
                end = (px+int(w/2), py-1)
                pygame.draw.line(self.screen, (0, 0, 0), start, end, 2)
