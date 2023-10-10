#!/bin/bash

#Build the test
make clean && make

# Check that the exec was built...
if [ -f amoswap_c_memh.exe ]; then
  sst --add-lib-path=../../../build/src/ ./rev-test-amoswap_c_memh.py
else
  echo "Test AMOADD_C_MEMH: amoswap_c_memh.exe not Found - likely build failed"
  exit 1
fi
