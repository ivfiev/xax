import signal
import tkinter as tk
import threading
import sys
import time
import model
from math import *
from pynput import mouse

px_rad = 1100                                        # TODO decrease a bit
rad_range = pi / 20

def get_dist(e):
    return sqrt(e.xr**2 + e.yr**2)

def get_angle(e):
    h = get_dist(e)
    return asin(e.xr / h)

def on_click(press):
    if not press:
        return
    enemies = [(id, e) for id, e in model.players.items() if e.color != model.me.color]
    if not enemies:
        return
    (t, e, id) = min([(get_angle(e), e, id) for id, e in enemies], key=lambda t: abs(t[0]))
    if abs(t) <= rad_range and e.yr > 0:
        mouse_px = t * px_rad
        print(f'{int(round(mouse_px))} 0')
        sys.stdout.flush() # repeat few times?

class MouseListener():
    def __init__(self):
        self.listener = mouse.Listener(on_click=self.on_click)
        self.listener.start()

    def on_click(self, x, y, _, press):
        on_click(press)

if __name__ == "__main__":
    listener = MouseListener()
    while True:
        line = sys.stdin.readline()
        model.parse_players(line, record_history=True)
