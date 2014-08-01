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
            self.send_command({"mode": "standard", "moves": [[48,80,"north"]]})

        elif self.turn == 5:
            print(self.board.units)
            self.send_command({"mode": "standard", "moves": [[48,79,"north"],[48,80,"north"]]})

        elif self.turn == 10:
            print(self.board.units)
            self.send_command({"mode": "standard", "moves": [[48,79,"north"],[48,80,"north"]]})

        elif self.turn == 14:
            print(self.board.units)





if __name__ == '__main__':
    myai = TestAI("TestAI"+str(random.randint(0, 1000)))
    myai.start("127.0.0.1", 5050)
