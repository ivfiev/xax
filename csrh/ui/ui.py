import signal
import tkinter as tk
import threading
import sys
import time
import model
from pynput import mouse

class FadingCircle:
    def __init__(self, canvas, x, y, col):
        self.canvas = canvas
        self.x0 = x
        self.y0 = y
        self.x1 = None
        self.y1 = None
        self.col = col
        self.size = 14
        self.alpha = 1.0
        self.circle = None
        self.canvas.after(100, self.fade)

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
            self.alpha -= 0.51
            if self.alpha < 0:
                self.alpha = 0
            self.canvas.itemconfig(self.circle, fill=self._get_color())
            self.canvas.after(100, self.fade)
        else:
            self.canvas.delete(self.circle)

    def rebase(self, x0, y0, t0):
        (self.x1, self.y1) = model.rebase(self.x0, self.y0, x0, y0, t0)
        (self.x1, self.y1) = ((self.x1 + 4000.0) / 16.0, (4000.0 - self.y1) / 16.0)

    def place(self):
        if self.circle:
            self.canvas.coords(self.circle,
                round(self.x1 - self.size / 2),
                round(self.y1 - self.size / 2),
                round(self.x1 + self.size / 2),
                round(self.y1 + self.size / 2))
        else:
            self.circle = self.canvas.create_oval(
                self.x1 - self.size / 2,
                self.y1 - self.size / 2,
                self.x1 + self.size / 2,
                self.y1 + self.size / 2,
                fill=self._get_color(), outline='')   

class Overlay(tk.Tk):
    def __init__(self):
        super().__init__()
        self.circles = []
        self.overrideredirect(True)
        self.attributes('-topmost', True)
        self.geometry('500x500+40+40')
        self.config(bg='black')
        self.canvas = tk.Canvas(self, width=500, height=500, bg='black', highlightthickness=0)
        self.canvas.pack()
        self.stdin_thread = threading.Thread(target=self.read_stdin, daemon=True)
        self.stdin_thread.start()
        self.hidden = False

    def read_stdin(self):
        while True:
            line = sys.stdin.readline()
            if line:
                model.parse_players(line)
                for id, p in model.players.items():
                    self.circles.append(FadingCircle(self.canvas, p.x, p.y, p.color))
                self.circles.append(FadingCircle(self.canvas, model.me.x, model.me.y, 'ME'))
                for c in self.circles:
                    c.rebase(model.me.x, model.me.y, model.me.t)
                    c.place()
            else:
                time.sleep(0.01)
            self.circles = [c for c in self.circles if c.alpha > 0]

    def on_closing(self):
        self.destroy()

    def toggle(self, show):
        if show and self.hidden:
            self.deiconify()
            self.hidden = False
        elif not show and not self.hidden:
            self.withdraw()
            self.hidden = True


class Aim():
    def __init__(self, get_enemies):
        self.get_enemies = get_enemies

    def on_click(self, press):
        if not press:
            return
        es = self.get_enemies()
        # print(es, file=sys.stderr)
        if not es:
            return
        (a, x, y) = min((abs(e[0]), e[0], e[1]) for e in es)
        # print(f'{x} {y}', file=sys.stderr)
        # print(f'{x2} {y2}', file=sys.stderr)
        if a <= 120 and y > 0:
            dx = x * 1200 / y
            print(f'{int(round(dx))} 0')
            sys.stdout.flush()


class MouseListener():
    def __init__(self, overlay, aim):
        self.overlay = overlay
        self.aim = aim
        self.listener = mouse.Listener(on_move=self.on_move, on_scroll=self.on_scroll, on_click=self.on_click)
        self.listener.start()
        self.timer = False

    def on_move(self, x, y):
        if 40 <= x <= 540 and 40 <= y <= 540:
            self.overlay.toggle(False)
        elif not self.timer:
            self.overlay.toggle(True)

    def on_scroll(self, _, __, ___, ____):
        self.temp_hide()

    def on_click(self, x, y, _, press):
        ... # self.aim.on_click(press)

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
    aim = Aim(lambda: overlay.enemies)
    listener = MouseListener(overlay, aim)
    overlay.protocol("WM_DELETE_WINDOW", overlay.on_closing)
    signal.signal(signal.SIGINT, lambda x, y: overlay.destroy())
    tk_check = lambda: overlay.after(100, tk_check)
    overlay.after(100, tk_check)
    overlay.bind_all("<Control-c>", lambda e: overlay.destroy())
    overlay.mainloop()
