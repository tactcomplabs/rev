#!/bin/bash

#Build the test
make

# Check that the exec was built...
if [ -f strstr.exe ]; then
  sst ./rev-strstr.py
else
  echo "Test STRSTR: strstr.exe not Found - likely build failed"
  exit 1
fi 
