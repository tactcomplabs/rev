#!/bin/bash

#Build the test
make clean && make

# Check that the exec was built...
if [ -f ex3.exe ]; then
  sst --add-lib-path=../../build/src/ ./rev-test-ex3.py
else
  echo "Test EX3: ex3.exe not Found - likely build failed"
  exit 1
fi
