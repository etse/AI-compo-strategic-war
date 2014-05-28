from __future__ import print_function, division, unicode_literals

import socket, random
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
name = "hahaha{}".format(random.randint(0, 1000))
s.sendall("name {}\n".format(name))
s.sendall(json.dumps({"mode": "standard", "moves": [(x, 2, "east") for x in xrange(15)]})+"\n")

for line in readline_from_socket(s):
    try:
        print(line)
        d = json.loads(line)
        if "status" in d:
            continue
        s.sendall(json.dumps({"mode": "standard", "moves": [(x, 2, "east") for x in xrange(15)]})+"\n")
    except Exception, e:
        print("Error parsing:", line, e)
