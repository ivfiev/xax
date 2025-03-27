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
        if not es:
            return
        (a, x, y) = min((abs(e[0]), e[0], e[1]) for e in model.players)
        # print(f'{x} {y}', file=sys.stderr)
        # print(f'{x2} {y2}', file=sys.stderr)
        if a <= 120 and y > 0:
            dx = x * 1200 / y
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
    signal.signal(signal.SIGINT, lambda x, y: sys.exit(0))