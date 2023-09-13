#!/bin/bash

#Build the test
make clean && make
REV_SST_CONFIG=${REV_SST_CONFIG-./strlen_cxx.py}
REV_EXE=strlen_cxx.exe

# Check that the exec was built...
if [ -f ${REV_EXE} ]; then
  REV_EXE=${REV_EXE} sst --add-lib-path=../../build/src/ ${REV_SST_CONFIG}
else
  echo "Test STRLEN_CXX: ${REV_EXE} not Found - likely build failed"
  exit 1
fi 
