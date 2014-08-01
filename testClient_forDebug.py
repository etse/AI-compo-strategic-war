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
            print(1)
            self.send_command({"mode": "standard", "moves": [[21, 21, "east"]]})

        elif self.turn == 20:
            print(2)
            self.send_command({"mode": "standard", "moves": [[22, 21, "south"], [21, 21, "east"]]})

        elif self.turn == 40:
            print(3)
            self.send_command({"mode": "standard", "moves": [[22, 21, "south"], [21, 21, "east"]]})





if __name__ == '__main__':
    myai = TestAI("TestAI"+str(random.randint(0, 1000)))
    myai.start("127.0.0.1", 5050)
