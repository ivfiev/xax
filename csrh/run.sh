#!/bin/bash

dm=false
aim=false

for arg in "$@"; do
  if [[ $arg == "--dm" ]]; then
    dm=true
  fi
  if [[ $arg == "--aim" ]]; then
    aim=true
  fi
done

if [[ $dm == true ]]; then
  export DEATHMATCH=1
fi

if [[ $aim == true ]]; then
  . ./frontend/.venv/bin/activate
  sudo ./build/xax csrh | tee >(python frontend/walls.py) | python frontend/aim.py | sudo ./build/mouse
else
  sudo ./build/xax csrh | python frontend/walls.py
fi
