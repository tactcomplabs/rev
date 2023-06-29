#!/bin/bash

#Build the test
make

# Check that the exec was built...
if [ -f qsort.exe ]; then
  sst ./qsort.py
else
  echo "Test QSORT: qsort.exe not Found - likely build failed"
  exit 1
fi 
