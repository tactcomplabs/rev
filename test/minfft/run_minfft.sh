#!/bin/bash

#Build the test
make

# Check that the exec was built...
if [ -f minfft.exe ]; then
  sst ./rev-test-minfft.py
else
  echo "Test MINFFT: minfft.exe not Found - likely build failed"
  exit 1
fi 
