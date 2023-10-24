#!/bin/bash

#Build the test
make clean && make

# Check that the exec was built...
if [ -f dot_single.exe ]; then
  sst --add-lib-path=../../build/src/ ./dot_single.py
else
  echo "Test DOT_SINGLE: dot_single.exe not Found - likely build failed"
  exit 1
fi
