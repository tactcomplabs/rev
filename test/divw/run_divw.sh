#!/bin/bash

#Build the test
make

# Check that the exec was built...
if [ -f divw.exe ]; then
  sst ./rev-divw.py
else
  echo "Test DIVW: dviw.exe not Found - likely build failed"
  exit 1
fi
