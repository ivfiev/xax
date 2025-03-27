. ./frontend/.venv/bin/activate
sudo ./build/csrh csrh | tee >(python frontend/radar.py) | python frontend/aim.py | sudo ./build/mouse
