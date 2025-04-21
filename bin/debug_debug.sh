#!/bin/bash
BIN_DIR="$(dirname -- ${BASH_SOURCE[0]})"
PROJECT_ROOT="$BIN_DIR/.."
cd $PROJECT_ROOT/build/debug \
    && gdb ./stratego \
    || echo "Build the debug project first"
