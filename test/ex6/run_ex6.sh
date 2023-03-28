#!/bin/bash

#Build the test
make

# Check that the exec was built...
if [ -f ex6.exe ]; then
  sst ./rev-test-ex6.py
else
  echo "Test EX6: ex6.exe not Found - likely build failed"
  exit 1
fi 
