import tkinter as tk


class MovableWindow:
    def __init__(self, x, y, size, color):
        self.root = tk.Tk()
        self.root.overrideredirect(True)
        self.root.geometry(f"{size}x{size}+{x}+{y}")
        self.root.attributes('-topmost', True)

        self.canvas = tk.Canvas(self.root, width=size, height=size, bg=color, highlightthickness=3)
        self.canvas.pack()

        self.x, self.y = x, y
        self.dx, self.dy = 5, 5
        self.size = size
        self.ds = 1

    def move(self):
        self.x += self.dx
        self.y += self.dy

        if self.x <= 0 or self.x + self.size >= 3840:
            self.dx *= -1
        if self.y <= 0 or self.y + self.size >= 2160:
            self.dy *= -1

        self.ds = -1 if self.size == 150 else 1 if self.size == 50 else self.ds
        self.size += self.ds

        self.root.geometry(f"{self.size}x{self.size}+{self.x}+{self.y}")
        self.canvas.config(width=self.size, height=self.size)
        self.root.after(15, self.move)


windows = [
    MovableWindow(100, 100, 100, "red"),
    MovableWindow(300, 300, 100, "red"),
    MovableWindow(200, 700, 100, "blue"),
    MovableWindow(2200, 700, 100, "blue"),
    MovableWindow(1300, 1300, 100, "red"),
]

for window in windows:
    window.move()

for window in windows:
    window.root.mainloop()
