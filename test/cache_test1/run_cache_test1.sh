#!/bin/bash

#Build the test
make

# Check that the exec was built...
if [ -f cache_test1.exe ]; then
  sst ./rev-test-cache1.py
else
  echo "Test CACHE-TEST1: cache_test1.exe not Found - likely build failed"
  exit 1
fi 
