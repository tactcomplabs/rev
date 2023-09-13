#!/bin/bash

#Build the test
make clean && make
REV_SST_CONFIG=${REV_SST_CONFIG-./rev-test-ex5.py}
REV_EXE=ex5.exe

# Check that the exec was built...
if [ -f ${REV_EXE} ]; then
  REV_EXE=${REV_EXE} sst --add-lib-path=../../build/src/ ${REV_SST_CONFIG}
else
  echo "Test EX5: ${REV_EXE} not Found - likely build failed"
  exit 1
fi 
