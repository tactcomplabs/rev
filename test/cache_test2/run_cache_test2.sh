#!/bin/bash

#Build the test
make

# Check that the exec was built...
if [ -f cache_test2.exe ]; then
  sst ./rev-test-cache2.py
else
  echo "Test CACHE-TEST2: cache_test2.exe not Found - likely build failed"
  exit 1
fi 
