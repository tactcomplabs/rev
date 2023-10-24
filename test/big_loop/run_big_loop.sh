#!/bin/bash

#Build the test
make clean && make

# Check that the exec was built...
if [ -f big_loop.exe ]; then
  sst --add-lib-path=../../build/src/ ./big_loop.py
else
  echo "Test BIG_LOOP: big_loop.exe not Found - likely build failed"
  exit 1
fi
