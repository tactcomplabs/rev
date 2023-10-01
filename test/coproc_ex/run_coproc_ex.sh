#!/bin/bash

#Build the test
make

# Check that the exec was built...
if [ -f ex1.exe ]; then
  sst ./rev-test-coproc_ex.py
else
  echo "Test COPROC: File ex1.exe not found - likely build failed"
  exit 1
fi
