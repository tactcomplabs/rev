#!/bin/bash

#Build the test
make clean && make
REV_SST_CONFIG=${REV_SST_CONFIG-./rev-test-do_single.py}
REV_EXE=dot_single.exe

# Check that the exec was built...
if [ -f ${REV_EXE} ]; then
  sst --add-lib-path=../../build/src/ ./dot_single.py
else
  echo "Test DOT_SINGLE: ${REV_EXE} not Found - likely build failed"
  exit 1
fi 
