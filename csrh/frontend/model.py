from math import *
import os

DEATHMATCH = os.getenv('DEATHMATCH') == '1'

class Entity():
    def __init__(self, x, y, z, t, p, color):
        self.x = x
        self.y = y
        self.z = z
        self.t = t
        self.p = p
        self.color = color
        self.xr = None
        self.yr = None
        self.zr = None

    def rebase(self, x, y, z, t):
        (self.xr, self.yr) = rebase(self.x, self.y, x, y, t)
        self.zr = self.z - z
    
me = Entity(0, 0, 0, 0, 0, '')
players = {}
histories = []

def parse_players(line, history=0):
    global me, players, histories
    if not line.endswith(',1\n'):
        return False
    raw_strs = line.split('|')
    new_players = {}
    for str in raw_strs:
        [id, coords, meta] = str.split(':')
        is_me = meta.startswith('1')
        [x, y, z, yaw, pitch] = map(lambda f: float(f), coords.split(','))
        color = 'T' if 'T' in meta else 'C'
        t = yaw / 180 * pi - pi / 2.0
        p = pitch / 180 * pi
        e = Entity(x, y, z, t, p, color)
        if is_me:
            me = e
        else:
            new_players[id] = e
    for id, e in new_players.items():
        e.rebase(me.x, me.y, me.z, me.t)
    players = new_players
    if history > 0:
        histories.append(new_players)
        if len(histories) > history:
            histories = histories[len(histories) - history:]
    return True

def rebase(x, y, x0, y0, t0):
    (x, y) = (x - x0, y - y0)
    t = -t0
    (x, y) = (x * cos(t) - y * sin(t), x * sin(t) + y * cos(t))
    return (x, y)

def is_enemy(e):
    return DEATHMATCH or e.color != me.color