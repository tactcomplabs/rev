#!/bin/bash

#Build the test
make clean && make

# Check that the exec was built...
if [ -f large_bss.exe ]; then
  sst --add-lib-path=../../build/src/ ./rev-large-bss.py
else
  echo "Test LARGE-BSS: large_bss.exe not Found - likely build failed"
  exit 1
fi
