#!/bin/bash

#Build the test
make clean && make

# Check that the exec was built...
if [ -f ex5.exe ]; then
  sst --add-lib-path=../../build/src/ ./rev-test-ex5.py
else
  echo "Test EX5: ex5.exe not Found - likely build failed"
  exit 1
fi
