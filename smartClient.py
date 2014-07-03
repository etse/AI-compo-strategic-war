from testClient import GameAI
import random

class SmartAI(GameAI):
    def resolve_round(self):
        command = {"mode": random.choice(["standard", "harvester", "soldier"]), "moves": []}
        for unit in filter(lambda u: u.owner == self.my_id, self.board.units):
            x, y = unit.position
            legal_directions = []
            for nx, ny, direction in [(x, y-1, "north"), (x, y+1, "south"), (x-1, y, "west"), (x+1, y, "east")]:
                rx, ry = self.board.get_real_position(nx, ny)
                if not self.board.any_units_on_position((rx, ry)) and not self.board[rx][ry].isWall:
                    legal_directions.append(direction)

            if legal_directions:
                direction = random.choice(legal_directions)
                command["moves"].append([x, y, direction])
                self.board.move_unit(x, y, direction)
        self.send_command(command)