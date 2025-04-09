import sys
import model
from math import *
from pynput import mouse

px_rad = 1100
rad_range = pi / 20

def get_dist(e):
    return sqrt(e.xr**2 + e.yr**2)

def get_angle(e):
    h = get_dist(e)
    return asin(e.xr / h)

def aim_at_closest_enemy():
    enemies = [(id, e) for id, e in model.players.items() if e.color != model.me.color]
    if not enemies:
        return
    (t, e, id) = min([(get_angle(e), e, id) for id, e in enemies], key=lambda t: abs(t[0]))
    if abs(t) <= rad_range and e.yr > 0:
        mouse_px = t * px_rad
        print(f'{int(round(mouse_px))} 0')
        sys.stdout.flush()

def on_click(_, __, ___, press):
    if not press:
        return
    aim_at_closest_enemy()

if __name__ == "__main__":
    ml = mouse.Listener(on_click=on_click)
    ml.start()
    while True:
        line = sys.stdin.readline()
        model.parse_players(line, record_history=False) # todo 
