import sys
import model
import util
import math
from pynput import mouse

px_rad = 1100
rad_range = math.pi / 40

def aim_at_closest_enemy():
    enemies = [(id, e) for id, e in model.players.items() if model.is_enemy(e)]
    if not enemies:
        return
    (t, e, id) = min([(util.get_angle_x(e), e, id) for id, e in enemies], key=lambda t: abs(t[0]))
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
        model.parse_players(line, history=0) # todo 
