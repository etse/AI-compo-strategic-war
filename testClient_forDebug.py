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
            self.send_command({"mode": "standard", "moves": [[21, 21, "east"]]})
        elif self.turn == 2:
            self.send_command({"mode": "standard", "moves": [[22, 21, "east"]]})
        elif self.turn == 3:
            self.send_command({"mode": "standard", "moves": [[23, 21, "east"]]})
        elif self.turn == 4:
            self.send_command({"mode": "standard", "moves": [[21, 21, "east"]]})
        elif self.turn == 5:
            self.send_command({"mode": "standard", "moves": [[22, 21, "east"]]})
        elif self.turn == 20:
            self.send_command({"mode": "standard", "moves": [[23, 21, "east"], [24, 21, "east"]]})
        elif self.turn == 21:
            self.send_command({"mode": "standard", "moves": [[24, 21, "east"], [25, 21, "east"]]})
        elif self.turn == 22:
            self.send_command({"mode": "standard", "moves": [[25, 21, "east"], [26, 21, "east"]]})
        elif self.turn == 23:
            self.send_command({"mode": "standard", "moves": [[26, 21, "east"], [27, 21, "east"]]})
        elif self.turn == 24:
            self.send_command({"mode": "standard", "moves": [[27, 21, "east"], [28, 21, "west"]]})
        elif self.turn == 25:
            self.send_command({"mode": "standard", "moves": [[28, 21, "east"], [27, 21, "west"]]})



if __name__ == '__main__':
    myai = TestAI("TestAI"+str(random.randint(0, 1000)))
    myai.start("127.0.0.1", 5050)
