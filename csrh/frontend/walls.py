import util
import signal
import sys
from time import sleep
from pynput import mouse
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
        self.toggle(False)

    def set(self, x, y, h, w, col):
        self.x, self.y, self.h, self.w = x, y, h, w
        self.col = col
        self.root.geometry(f"{round(self.w)}x{round(self.h)}+{round(self.x - self.w/2)}+{round(self.y - self.h/2)}")
        self.canvas.config(width=self.w, height=self.h, bg=col)

    def toggle(self, visible):
        if visible:
            self.root.deiconify()
        else:
            self.root.withdraw()

class Root(tk.Tk):
    def __init__(self, screenName = None, baseName = None, className = "Tk", useTk = True, sync = False, use = None):
        super().__init__(screenName, baseName, className, useTk, sync, use)
        self.thread = None
        self.withdraw()

root = Root()
windows = [MovableWindow(root, 0, 0, 0, 0) for _ in range(65)]

pressed = False
last_press = 0
def setup_mouse():
    def on_click(_, __, ___, press): 
        global last_press, pressed
        last_press = util.utcnow()
        pressed = press
    ml = mouse.Listener(on_click=on_click)
    ml.start()

def fisheye(r):
    if abs(r) <= math.pi / 7: 
        return 1
    return (1 + 0.33 * (abs(r) - math.pi / 7))**2
    
def targets():
    enemies = [(id, e) for id, e in model.players.items() if model.is_enemy(e)]
    boxed = [(int(id), e) for (id, e) in enemies 
                if e.yr > 0 and 
                    abs(util.get_angle_x(e)) < rad_range and 
                    abs(util.get_angle_y(e) + model.me.p) < rad_range / 1.3]
    return boxed

try:
    signal.signal(signal.SIGINT, lambda x, y: sys.exit(0))
    # setup_mouse()
    util.stdin_nonblock()
    while True:
        line = util.read_last()
        if line:
            model.parse_players(line)
            ts = targets()
            active = set()
            visible = not pressed and (util.utcnow() - last_press > 200)

            for t in ts:
                (id, e) = t
                active.add(id)
                win = windows[id]

                dist = util.get_dist(e)
                scale = px_200_100_350 / dist
                offset = 350 * scale
                tx = util.get_angle_x(e)
                ty = -util.get_angle_y(e) - model.me.p
                fish = fisheye(tx)

                h = fish * min(400, max(200 * scale, 16))
                w = fish * min(200, max(100 * scale, 8))
                x = mid_x + fish * tx * px_rad
                y = mid_y + ty * px_rad + offset
                color = 'red' if e.color == 'T' else 'blue'

                win.set(x, y, h, w, color)
                win.toggle(visible)

            for i in range(65):
                if i not in active:
                    windows[i].toggle(False)
            
        root.update_idletasks()
        root.update()
        sleep(0.001)
except:
    traceback.print_exc()
    root.destroy()