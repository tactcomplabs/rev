#!/bin/bash

#Build the test
make
REV_SST_CONFIG=${REV_SST_CONFIG-./rev-isa-test.py}
REV_EXE=$RVASM.exe

# Check that the exec was built...
if [ -f ${REV_EXE} ]; then
  REV_EXE=${REV_EXE} sst --add-lib-path=../../build/src/ --model-options=$RVASM.exe ${REV_SST_CONFIG}
else
  echo "Test $RVASM ASM: File${REV_EXE} not found - likely build failed"
  exit 1
fi
