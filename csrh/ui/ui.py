import signal
import tkinter as tk
import threading
import sys
import time
from math import cos, sin, pi
from pynput import mouse


class FadingCircle:
    def __init__(self, canvas, x1, y1, x2, y2, t, col):
        self.canvas = canvas
        self.x0 = 0
        self.y0 = 0
        self.x1 = x1
        self.y1 = y1
        self.t = t
        self.col = col
        self.size = 14
        self.alpha = 1.0
        self.recalc_effective(x2, y2, t)
        self.circle = self.canvas.create_oval(
            self.x0 - self.size / 2,
            self.y0 - self.size / 2,
            self.x0 + self.size / 2,
            self.y0 + self.size / 2,
            fill=self._get_color(), outline=''
        )
        self.fade()

    def _get_color(self):
        col = int(255 * self.alpha)
        if self.col == 'T':
            return f'#{col:02x}0000'
        if self.col == 'C':
            return f'#0000{col:02x}'
        if self.col == 'ME':
            return f'#00{col:02x}00'
        return f'#{0:02x}{0:02x}{0:02x}'

    def fade(self):
        if self.alpha > 0:
            self.alpha -= 0.20
            if self.alpha < 0:
                self.alpha = 0
            self.canvas.itemconfig(self.circle, fill=self._get_color())
            self.canvas.after(100, self.fade)
        else:
            self.canvas.delete(self.circle)

    def recalc_effective(self, x2, y2, t):
        (x, y) = (self.x1 - x2, self.y1 - y2)
        t = -t
        (x, y) = (x * cos(t) - y * sin(t), x * sin(t) + y * cos(t))
        (x, y) = ((x + 4000.0) / 16.0, (4000.0 - y) / 16.0)
        self.x0 = x
        self.y0 = y

    def place(self):
        self.canvas.coords(self.circle,
                           round(self.x0 - self.size / 2),
                           round(self.y0 - self.size / 2),
                           round(self.x0 + self.size / 2),
                           round(self.y0 + self.size / 2))


class Overlay(tk.Tk):
    def __init__(self):
        super().__init__()
        self.prev = (0.0, 0.0)
        self.angle = 0.0
        self.overrideredirect(True)
        self.attributes('-topmost', True)
        self.geometry('500x500+40+40')
        self.config(bg='black')
        self.canvas = tk.Canvas(self, width=500, height=500, bg='black', highlightthickness=0)
        self.canvas.pack()
        self.circles = []
        self.stdin_thread = threading.Thread(target=self.read_stdin, daemon=True)
        self.stdin_thread.start()
        self.hidden = False

    def read_stdin(self):
        while True:
            line = sys.stdin.readline()
            if line:
                if not line.endswith(',1\n'):
                    continue
                me = False
                [coords, meta] = line.split(':')
                if meta.startswith('1'):
                    me = True
                [x, y, yaw] = map(lambda xy: float(xy), coords.split(','))
                col = 'ME' if meta.startswith('1') else 'T' if 'T' in meta else 'C'
                if me:
                    t = yaw / 180 * pi - pi / 2.0
                    self.prev = (x, y)
                    self.angle = t
                    for circle in self.circles:
                        circle.recalc_effective(x, y, t)
                        circle.place()
                    self.draw_circle(x, y, x, y, t, col)
                else:
                    self.draw_circle(x, y, self.prev[0], self.prev[1], self.angle, col)
            self.circles = [c for c in self.circles if c.alpha > 0]
            if not line:
                time.sleep(0.01)

    def draw_circle(self, x1, y1, x2, y2, t, c):
        self.circles.append(FadingCircle(self.canvas, x1, y1, x2, y2, t, c))

    def on_closing(self):
        self.destroy()

    def toggle(self, show):
        if show and self.hidden:
            self.deiconify()
            self.hidden = False
        elif not show and not self.hidden:
            self.withdraw()
            self.hidden = True


class MouseListener():
    def __init__(self, overlay):
        self.overlay = overlay
        self.listener = mouse.Listener(on_move=self.on_move, on_scroll=self.on_scroll)
        self.listener.start()
        self.timer = False

    def on_move(self, x, y):
        if 40 <= x <= 540 and 40 <= y <= 540:
            self.overlay.toggle(False)
        elif not self.timer:
            self.overlay.toggle(True)

    def on_scroll(self, _, __, ___, ____):
        self.temp_hide()

    def temp_hide(self):
        def on_timeout():
            self.overlay.toggle(True)
            self.timer = False

        if not self.timer:
            self.timer = True
            self.overlay.toggle(False)
            threading.Timer(3.0, on_timeout).start()


if __name__ == "__main__":
    overlay = Overlay()
    listener = MouseListener(overlay)
    overlay.protocol("WM_DELETE_WINDOW", overlay.on_closing)
    signal.signal(signal.SIGINT, lambda x, y: overlay.destroy())
    tk_check = lambda: overlay.after(100, tk_check)
    overlay.after(100, tk_check)
    overlay.bind_all("<Control-c>", lambda e: overlay.destroy())
    overlay.mainloop()