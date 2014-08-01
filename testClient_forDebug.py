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
            print(self.board.units)
            self.send_command({"mode": "standard", "moves": [[48,80,"north"],[21,21,"east"]]})

        elif self.turn == 5:
            print(self.board.units)
            self.send_command({"mode": "standard", "moves": [[48,79,"north"],[22,21,"south"],[48,80,"north"],[21,21,"east"]]})

        elif self.turn == 10:
            print(self.board.units)
            self.send_command({"mode": "standard", "moves": [[48,79,"north"],[22,21,"south"],[48,80,"north"],[21,21,"east"]]})

        elif self.turn == 15:
            print(self.board.units)
            self.send_command({"mode": "standard", "moves":  [[48,78,"north"],[22,22,"east"],[48,79,"north"],[22,21,"south"],[48,80,"north"]]})






if __name__ == '__main__':
    myai = TestAI("TestAI"+str(random.randint(0, 1000)))
    myai.start("127.0.0.1", 5050)
