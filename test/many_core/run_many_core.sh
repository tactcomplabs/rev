#!/bin/bash

#Build the test
make clean && make

# Check that the exec was built...
if [ -f many_core.exe ]; then
  sst --add-lib-path=../../src/ ./rev-many-core.py
else
  echo "Test MANY_CORE: many_core.exe not Found - likely build failed"
  exit 1
fi
