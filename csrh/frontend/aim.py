import signal
import tkinter as tk
import threading
import sys
import time
import model
from pynput import mouse

class Aim():
    def on_click(self, press):      # TODO active while pressed/tracker button
        if not press:
            return
        enemies = [id, e for id, e in model.players.items() if e.color != model.me.color]
        if not enemies:
            return
        (a, x, y, id) = min((abs(e.xr), e.xr, e.yr, id) for id, e in enemies)
        # print(len(model.histories), file=sys.stderr)
        if a <= 90 and y > 0:
            dx, dy = 0.0, 0.0
            for i in range(len(model.histories) - 3, len(model.histories) - 1):
                dx += model.histories[i + 1][id].xr - model.histories[i][id].xr
                dy += model.histories[i + 1][id].yr - model.histories[i][id].yr
            x_target = x + 1.5 * dx
            y_target = y + 1.5 * dy
            mouse_px = x_target * 800 / y_target # TODO use angles instead
            print(f'{int(round(mouse_px))} 0')
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
        model.parse_players(line, record_history=True)
