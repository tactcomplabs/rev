#!/bin/bash

#Build the test
make

# Check that the exec was built...
if [ -f ex2.exe ]; then
  sst ./rev-test-ex2.py
else
  echo "Test EX2: ex2.exe not Found - likely build failed"
  exit 1
fi 
