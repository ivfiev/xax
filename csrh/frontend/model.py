import datetime
from math import *

me = None
players = {}
histories = []

class Entity():
    def __init__(self, x, y, t, color):
        self.x = x
        self.y = y
        self.t = t
        self.color = color
        self.xr = None
        self.yr = None

    def rebase(self, x, y, t):
        (self.xr, self.yr) = rebase(self.x, self.y, x, y, t)
    
def parse_players(line, record_history=False, max_records=50):
    global me, players, histories
    if not line.endswith(',1\n'):
        return
    raw_strs = line.split('|')
    new_players = {}
    for str in raw_strs:
        [id, coords, meta] = str.split(':')
        is_me = meta.startswith('1')
        [x, y, yaw] = map(lambda xy: float(xy), coords.split(','))
        color = 'T' if 'T' in meta else 'C'
        t = yaw / 180 * pi - pi / 2.0
        e = Entity(x, y, t, color)
        if is_me:
            me = e
        else:
            new_players[id] = e
    for id, e in new_players.items():
        e.rebase(me.x, me.y, me.t)
    players = new_players
    if record_history:
        histories.append(new_players)
        if len(histories) > max_records:
            histories = histories[1:]

def rebase(x, y, x0, y0, t0):
    (x, y) = (x - x0, y - y0)
    t = -t0
    (x, y) = (x * cos(t) - y * sin(t), x * sin(t) + y * cos(t))
    return (x, y)

def utcnow():
    return int(datetime.now(datetime.UTC).timestamp() * 1000)