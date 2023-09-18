#!/bin/bash

#Build the test
make clean && make

# Check that the exec was built...
if [ -f mem.exe ]; then
  sst --add-lib-path=../../build/src/ ./mem.py
else
  echo "Test MEMSET_2: mem.exe not Found - likely build failed"
  exit 1
fi
