#
#/bin/bash

while true; do
  if ! pgrep -f "sztest.py" >/dev/null; then
    python3 sztest.py >> log 2>&1 &
  fi
  sleep 60
done
