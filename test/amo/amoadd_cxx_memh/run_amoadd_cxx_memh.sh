#!/bin/bash

#Build the test
make clean && make

# Check that the exec was built...
if [ -f amoadd_cxx_memh.exe ]; then
  sst --add-lib-path=../../../build/src/ ./rev-test-amoadd_cxx_memh.py
else
  echo "Test AMOADD_CXX_MEMH: amoadd_cxx.exe not Found - likely build failed"
  exit 1
fi
