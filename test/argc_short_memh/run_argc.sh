#!/bin/bash

#Build the test
make clean && make
REV_SST_CONFIG=${REV_SST_CONFIG-./rev-test-argc.py}
REV_EXE=argc.exe

# Check that the exec was built...
if [ -f ${REV_EXE} ]; then
  REV_EXE_ARGS=${REV_EXE_ARGS} REV_EXE=${REV_EXE} sst --add-lib-path=../../build/src/ ${REV_SST_CONFIG}
else
  echo "Test ARGC_SHORT_MEMH: ${REV_EXE} not Found - likely build failed"
  exit 1
fi