#!/bin/bash

#Build the test
make clean && make

# Check that the exec was built...
if [ -f strlen_c.exe ]; then
  sst --add-lib-path=../../build/src/ ./strlen_c.py
else
  echo "Test STRLEN_C: strlen_c.exe not Found - likely build failed"
  exit 1
fi
