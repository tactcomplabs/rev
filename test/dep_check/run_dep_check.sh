#!/bin/bash

#Build the test
make clean && make

# Check that the exec was built...
if [ -f dep_check.exe ]; then
  sst --add-lib-path=../../build/src/ ./rev-test-dep_check.py
else
  echo "Test DEP_CHECK: File dep_check.exe not found - likely build failed"
  exit 1
fi
