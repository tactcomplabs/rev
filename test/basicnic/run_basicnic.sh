#!/bin/bash

#Build the test
make clean && make

# Check that the exec was built...
if [ -f basicnic.exe ]; then
  sst --add-lib-path=../../build/src/ ./rev-basicnic.py
else
  echo "Test BASICNIC: basicnic.exe not Found - likely build failed"
  exit 1
fi
