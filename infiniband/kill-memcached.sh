#!/bin/bash

pids=$(pgrep -u "$USER" -x memcached || true)
if [ -n "$pids" ]; then
  kill $pids
fi
