#!/bin/bash

#Build the test
make clean && make
REV_SST_CONFIG=${REV_SST_CONFIG-./rev-test-do_double.py}
REV_EXE=dot_double.exe

# Check that the exec was built...
if [ -f ${REV_EXE} ]; then
  sst --add-lib-path=../../build/src/ ./dot_double.py
else
  echo "Test DOT_DOUBLE: ${REV_EXE} not Found - likely build failed"
  exit 1
fi 
