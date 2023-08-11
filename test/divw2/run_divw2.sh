#!/bin/bash

#Build the test
make

# Check that the exec was built...
if [ -f divw2.exe ]; then
  sst --add-lib-path=../../src/ ./rev-divw2.py
else
  echo "Test DIVW: dviw2.exe not Found - likely build failed"
  exit 1
fi
