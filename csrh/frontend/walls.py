import util
import signal
import sys
from time import sleep
import tkinter as tk
import traceback
import model
import math

px_rad = 1500
rad_range = math.pi / 3.5
mid_y = 2160.0 / 2.0
mid_x = 3840.0 / 2.0
px_200_100_350 = 180.0

class MovableWindow:
    def __init__(self, root, x, y, h, w):
        self.root = tk.Toplevel(root)
        self.root.overrideredirect(True)
        self.root.geometry(f"{w}x{h}+{x}+{y}")
        self.root.attributes('-topmost', True)
        self.canvas = tk.Canvas(self.root, width=w, height=h, bg=None, highlightthickness=0)
        self.canvas.pack()
        self.x, self.y = x, y
        self.h, self.w = h, w
        self.col = None
        self.visible = None
        self.toggle(False)

    def set(self, x, y, h, w, col):
        self.x, self.y, self.h, self.w = x, y, h, w
        self.col = col
        self.root.geometry(f"{round(self.w)}x{round(self.h)}+{round(self.x - self.w/2)}+{round(self.y - self.h/2)}")
        self.canvas.config(width=self.w, height=self.h, bg=col)

    def toggle(self, visible):
        if self.visible == visible:
            return
        self.visible = visible
        if visible:
            self.root.deiconify()
        else:
            self.root.withdraw()

class Root(tk.Tk):
    def __init__(self, screenName = None, baseName = None, className = "Tk", useTk = True, sync = False, use = None):
        super().__init__(screenName, baseName, className, useTk, sync, use)
        self.withdraw()

root = Root()
windows = [MovableWindow(root, 0, 0, 0, 0) for _ in range(65)]

def fisheye(r):
    if abs(r) <= math.pi / 7: 
        return 1
    return (1 + 0.33 * (abs(r) - math.pi / 7))**2
    
def targets():
    enemies = [(int(id), e) for id, e in model.players.items() if model.is_enemy(e)]
    return enemies

try:
    signal.signal(signal.SIGINT, lambda x, y: sys.exit(0))
    util.stdin_nonblock()
    while True:
        line = util.read_last()
        if line:
            model.parse_players(line)
            ts = targets()
            active = set()

            for t in ts:
                (id, e) = t
                active.add(id)
                win = windows[id]
                
                dist = util.get_dist(e)
                if dist == 0:
                    continue
                scale = px_200_100_350 / dist
                offset = 350 * scale
                tx = util.get_angle_x(e)
                ty = -util.get_angle_y(e) - model.me.p

                visible = abs(tx) < rad_range and abs(ty) < rad_range / 1.3 and e.yr > 0

                fish = fisheye(tx)
                h = fish * min(400, max(200 * scale, 16))
                w = fish * min(200, max(100 * scale, 8))
                x = mid_x + fish * tx * px_rad
                y = mid_y + ty * px_rad + offset

                if not visible:
                    h /= fish
                    w /= fish
                    x = mid_x * 2 - w/5 if x > mid_x else 0 + w/5
                    y = mid_y

                color = 'red' if e.color == 'T' else 'blue'
                win.set(x, y, h, w, color)

                if not win.visible:
                    sleep(0.001)
                    win.toggle(True)

            for i in range(65):
                if i not in active:
                    windows[i].toggle(False)
        
        root.update_idletasks()
        root.update()
        sleep(0.001)
except:
    traceback.print_exc()
    root.destroy()
