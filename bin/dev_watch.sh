#!/bin/bash
BIN_DIR="$(dirname -- ${BASH_SOURCE[0]})"
PROJECT_ROOT="$BIN_DIR/.."
cd $PROJECT_ROOT

while true
do
  watch -d -t -g ls --full-time -lR src \
      && ./bin/debug_build.sh
done
