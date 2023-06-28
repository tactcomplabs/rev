#!/bin/bash

#Build the test
make

# Check that the exec was built...
if [ -f towers.exe ]; then
  sst ./towers.py
else
  echo "Test TOWERS: big_loop.exe not Found - likely build failed"
  exit 1
fi 
