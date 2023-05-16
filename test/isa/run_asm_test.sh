#!/bin/bash

#Build the test
make

# Check that the exec was built...
if [ -f $RVASM.exe ]; then
  sst --model-options=$RVASM.exe ./rev-isa-test.py
else
  echo "Test ASM: File not found - likely build failed"
  exit 1
fi
