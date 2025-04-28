#!/bin/bash

while true; do
  if ! pgrep -f "pre_t2.py" >/dev/null; then
    python3 pre_t2.py >> log 2>&1 &
  fi
  sleep 60
done
