from __future__ import print_function, division, unicode_literals

import socket
from functools import partial
import json


def readline_from_socket(socket):
    buffer = ""
    for data in iter(partial(socket.recv, 1024), ''):
        buffer += data
        temp = buffer.split("\n")
        buffer = temp.pop()
        for line in temp:
            yield line.rstrip()
    yield buffer.rstrip()

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
s.connect(("127.0.0.1", 5050))

for line in readline_from_socket(s):
    try:
        data = json.loads(line)
        for player in data["players"]:
            print("Player{id}: name={name}, food={food}, numMoves={numcmd}".format(id=player["id"], name=player["name"], food=player["food"], numcmd=len(player["command"]["moves"])))
    except Exception, e:
        print("Error parsing:", e.message)
