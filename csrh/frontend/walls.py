import fcntl
import os
import signal
import sys
from time import sleep
import tkinter as tk
import traceback
import model

class MovableWindow:
    def __init__(self, root, x, y, h, w):
        self.root = tk.Toplevel(root)
        self.root.overrideredirect(True)
        self.root.geometry(f"{w}x{h}+{x}+{y}")
        self.root.attributes('-topmost', True)
        self.canvas = tk.Canvas(self.root, width=w, height=h, bg=None, highlightthickness=3)
        self.canvas.pack()
        self.x, self.y = x, y
        self.h, self.w = h, w
        self.col = None
        self.toggle(False)

    def set(self, x, y, h, w, col):
        self.x, self.y, self.h, self.w = x, y, h, w
        self.col = col
        self.root.geometry(f"{self.w}x{self.h}+{self.x}+{self.y}")
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
windows = [
   MovableWindow(root, 0, 0, 0, 0),
   MovableWindow(root, 0, 0, 0, 0),
   MovableWindow(root, 0, 0, 0, 0),
   MovableWindow(root, 0, 0, 0, 0),
   MovableWindow(root, 0, 0, 0, 0),
]

signal.signal(signal.SIGINT, lambda x, y: sys.exit(0))

try:
    fcntl.fcntl(sys.stdin.fileno(), fcntl.F_SETFL, fcntl.fcntl(sys.stdin.fileno(), fcntl.F_GETFL) | os.O_NONBLOCK)
    i = 0
    while True:
        line = sys.stdin.readline()
        model.parse_players(line, False)
        for w in range(5):
            windows[w].set(100 + w * 100 + i * w, 100 + w * 100 + i, 100, 100, 'red' if i % 100 < 50 else 'blue')
            windows[w].toggle(True)
        root.update_idletasks()
        root.update()
        sleep(0.005)
        i += 1
except:
    traceback.print_exc()
    root.destroy()