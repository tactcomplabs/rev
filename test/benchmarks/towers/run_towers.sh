#!/bin/bash

#Build the test
make clean && make

# Check that the exec was built...
if [ -f towers.exe ]; then
  sst --add-lib-path=../../../src/ ./towers.py
else
  echo "Test TOWERS: big_loop.exe not Found - likely build failed"
  exit 1
fi 
