import signal
import tkinter as tk
import threading
import sys
import time
import model
from pynput import mouse

class Aim():
    def on_click(self, press):
        if not press:
            return
        if not model.players:
            return
        (a, x, y) = min((abs(e.xr), e.xr, e.yr) for _, e in model.players.items())
        print(f'{x} {y}', file=sys.stderr)
        if a <= 120 and y > 0:
            dx = x * 800 / y
            print(f'{int(round(dx))} 0')
            sys.stdout.flush()

class MouseListener():
    def __init__(self, aim):
        self.aim = aim
        self.listener = mouse.Listener(on_click=self.on_click)
        self.listener.start()

    def on_click(self, x, y, _, press):
        self.aim.on_click(press)

if __name__ == "__main__":
    aim = Aim()
    listener = MouseListener(aim)
    while True:
        line = sys.stdin.readline()
        model.parse_players(line, True)
