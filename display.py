from __future__ import print_function, division, unicode_literals

WALLCOLOR = (50, 50, 50)

PLAYERCOLORS = [(255, 255, 255), (255, 50, 150), (150, 50, 255), (255, 255, 0)]
PLAYERCOLORS += [(255, 0, 255), (255, 125, 0), (0, 25, 255)]
PLAYERCOLORS += [(0, 0, 255), (255, 0, 0), (150, 150, 0)]

FOODCOLOR = (140, 40, 40)
LAZERCOLOR = (0, 255, 0)

import pygame

class Display:
    def __init__(self, width, height, board_width, board_height):
        self.width = width
        self.height = height
        self.clock = None
        self.screen = None
        self.cell_h = self.height / board_height
        self.cell_w = self.width / board_width

    def init(self):
        pygame.init()
        self.clock = pygame.time.Clock()
        pygame.display.set_caption("Strategic war")
        self.screen = pygame.display.set_mode((self.width, self.height))
        self.font = pygame.font.SysFont("verdana", 11)

    def clear(self):
        self.screen.fill((0, 0, 0))

    def update(self, fps=30):
        pygame.display.flip()
        self.clock.tick(fps)

    def draw_board(self, board, players):
        h, w = self.cell_h, self.cell_w

        for x in xrange(board.width):
            for y in xrange(board.height):
                if board[x][y].isWall:
                    pygame.draw.rect(self.screen, WALLCOLOR, (w*x, h*y, w+1, h+1))
                if board[x][y].spawner:
                    pygame.draw.rect(self.screen, PLAYERCOLORS[board[x][y].spawner.owner], (w*x, h*y, w+1, h+1), 1)
                    if board[x][y].spawner.dead:
                        x1, x2, y1, y2 = w*x, w*(x+1)-1, h*y, h*(y+1)-1
                        pygame.draw.line(self.screen, PLAYERCOLORS[board[x][y].spawner.owner], (x1, y1), (x2, y2), 1)
                        pygame.draw.line(self.screen, PLAYERCOLORS[board[x][y].spawner.owner], (x1, y2), (x2, y1), 1)
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

        for index, player in enumerate(players):
            label = self.font.render(player.name, 1, PLAYERCOLORS[index])
            self.screen.blit(label, (15, 12+15*index))

    def draw_attack(self, attacker, defender):
        x1 = int(attacker.position[0]*self.cell_w+self.cell_w/2)
        y1 = int(attacker.position[1]*self.cell_h+self.cell_h/2)
        x2 = int(defender.position[0]*self.cell_w+self.cell_w/2)
        y2 = int(defender.position[1]*self.cell_h+self.cell_h/2)
        pygame.draw.line(self.screen, LAZERCOLOR, (x1, y1), (x2, y2), 1)
