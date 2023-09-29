#!/bin/bash

#Build the test
make clean && make

# Check that the exec was built...
if [ -f dot_double.exe ]; then
  sst --add-lib-path=../../build/src/ ./dot_double.py
else
  echo "Test DOT_DOUBLE: dot_double.exe not Found - likely build failed"
  exit 1
fi
