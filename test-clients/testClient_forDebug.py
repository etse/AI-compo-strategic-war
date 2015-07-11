from __future__ import print_function, division, unicode_literals

import random
from testClient import GameAI

class TestAI(GameAI):
    def __init__(self, name):
        GameAI.__init__(self, name)
        self.turn = 0

    def resolve_round(self):
        self.turn += 1
        if self.turn == 1:
            print("Runde 0:", self.board.units)
            self.send_command({"mode": "harvester", "moves": [[48,80,"north"],[21,21,"east"]]})

        elif self.turn == 2:
            print("Runde 1:", self.board.units)
            self.send_command({"mode": "soldier", "moves": [[48,79,"north"],[22,21,"south"],[48,80,"north"],[21,21,"east"]]})

        elif self.turn == 3:
            print("Runde 2:", self.board.units)
            self.send_command({"mode": "harvester", "moves": [[48,79,"north"],[22,21,"south"],[48,80,"north"],[21,21,"east"]]})

        elif self.turn == 4:
            print("Runde 3:", self.board.units)






if __name__ == '__main__':
    myai = TestAI("TestAI"+str(random.randint(0, 1000)))
    myai.start("127.0.0.1", 5050)
