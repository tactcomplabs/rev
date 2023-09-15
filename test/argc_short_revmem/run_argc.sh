#!/bin/bash

#Build the test
make clean && make

# Check that the exec was built...
if [ -f argc.exe ]; then
  sst --add-lib-path=../../build/src/ ./rev-test-argc.py
else
  echo "Test ARGC_REVMEM: argc.exe not Found - likely build failed"
  exit 1
fi
