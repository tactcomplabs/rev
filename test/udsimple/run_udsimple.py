#!/bin/bash

#Build the test
make

# Check that the exec was built...
if [ -f ex1.exe ]; then
  sst ./rev-test-udsimple.py
else
  echo "Test UDSIMPLE: File ex1.exe not found - likely build failed"
  exit 1
fi
