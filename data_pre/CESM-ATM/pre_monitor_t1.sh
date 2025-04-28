#!/bin/bash

while true; do
  if ! pgrep -f "pre_t1.py" >/dev/null; then
    python3 pre_t1.py >> log 2>&1 &
  fi
  sleep 60
done
