#!/bin/bash

#Build the test
make clean && make

# Check that the exec was built...
if [ -x cache_2.exe ]; then
	sst --add-lib-path=../../build/src/ ./rev-test-cache2.py
else
	echo "Test TEST_CACHE2: cache_test2.exe not Found - likely build failed"
	exit 1
fi
