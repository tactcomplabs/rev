#!/bin/bash

#Build the test
make clean && make

# Check that the exec was built...
if [ -f strstr.exe ]; then
  sst --add-lib-path=../../build/src/ ./rev-strstr.py
else
  echo "Test STRSTR: strstr.exe not Found - likely build failed"
  exit 1
fi
