
if [[ "$1" == "--dm" ]]; then
  export DEATHMATCH=1
fi

. ./frontend/.venv/bin/activate
sudo ./build/xax csrh | python frontend/walls.py # tee >(python frontend/radar.py) | python frontend/aim.py | sudo ./build/mouse
