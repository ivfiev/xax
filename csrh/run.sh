. ./frontend/.venv/bin/activate
sudo ./build/xax csrh | tee >(python frontend/radar.py) | python frontend/aim.py | sudo ./build/mouse
