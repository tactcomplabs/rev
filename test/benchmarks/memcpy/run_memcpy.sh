#!/bin/bash

#Build the test
make

# Check that the exec was built...
if [ -f memcpy.exe ]; then
  sst --add-lib-path=../../../src/ ./memcpy.py
else
  echo "Test MEMCPY: memcpy.exe not Found - likely build failed"
  exit 1
fi 
