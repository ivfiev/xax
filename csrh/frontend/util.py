from datetime import datetime, UTC
from fcntl import fcntl, F_GETFL, F_SETFL
import os
import sys
from math import *

def utcnow():
    return int(datetime.now(UTC).timestamp() * 1000)

def stdin_nonblock():
    fd = sys.stdin.fileno()
    flags = fcntl(fd, F_GETFL)
    fcntl(fd, F_SETFL, flags | os.O_NONBLOCK)

def get_dist(e):
    return sqrt(e.xr**2 + e.yr**2 + e.zr**2)

def get_angle_x(e):
    h = sqrt(e.xr**2 + e.yr**2)
    return asin(e.xr / h)

def get_angle_y(e):
    h = sqrt(e.zr**2 + e.yr**2)
    return asin(e.zr / h)

def get_angle_abc(a, b, c):
    h = sqrt(a**2 + b**2 + c**2)
    return asin(c / h)

def read_last():
    # requires non-blocking stdin!
    last_line = None
    while True:
        tmp = sys.stdin.readline()
        if not tmp:
            return last_line
        last_line = tmp