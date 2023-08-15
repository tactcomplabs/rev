#!/bin/bash

#Build the test
make clean && make

# Check that the exec was built...
echo $REV_BASE
if [ -f tracer.exe ]; then
  sst --add-lib-path=../../build/src/ ./rev-test-tracer.py
else
  echo "Test TRACER: File tracer.exe not found - likely build failed"
  exit 1
fi
