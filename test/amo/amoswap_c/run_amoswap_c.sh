#!/bin/bash

#Build the test
make clean && make

# Check that the exec was built...
if [ -f amoswap_c.exe ]; then
  sst --add-lib-path=../../../build/src/ ./rev-test-amoswap_c.py
else
  echo "Test AMOADD_C: amoswap_c.exe not Found - likely build failed"
  exit 1
fi
