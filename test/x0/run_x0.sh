#!/bin/bash

#Build the test
make clean && make

# Check that the exec was built...
if [ -f x0.exe ]; then
  sst --add-lib-path=../../build/src/ ./rev-test-x0.py
else
  echo "Test X0: x0.exe not Found - likely build failed"
  exit 1
fi
