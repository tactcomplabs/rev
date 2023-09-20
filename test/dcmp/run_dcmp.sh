#!/bin/bash

#Build the test
make

# Check that the exec was built...
if [ -f dcmp.exe ]; then
  sst --add-lib-path=../../src/ ./rev-dcmp.py
else
  echo "Test DCMP: dcmp.exe not Found - likely build failed"
  exit 1
fi
