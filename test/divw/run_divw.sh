#!/bin/bash

#Build the test
make clean && make

# Check that the exec was built...
if [ -f divw.exe ]; then
  sst --add-lib-path=../../build/src/ ./rev-divw.py
else
  echo "Test DIVW: dviw.exe not Found - likely build failed"
  exit 1
fi
