#!/bin/bash
BIN_DIR="$(dirname -- ${BASH_SOURCE[0]})"
PROJECT_ROOT="$BIN_DIR/.."
cd $PROJECT_ROOT/build/release \
    && ./stratego \
    || echo "Build the release project first"
