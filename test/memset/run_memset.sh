#!/bin/bash

#Build the test
make

# Check that the exec was built...
if [ -f memset.exe ]; then
  sst ./memset.py
else
  echo "Test MEMSET: memset.exe not Found - likely build failed"
  exit 1
fi
