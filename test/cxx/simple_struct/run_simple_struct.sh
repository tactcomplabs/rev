#!/bin/bash

#Build the test
make clean && make

# Check that the exec was built...
if [ -x simple_struct.exe ]; then
	sst --add-lib-path=../../../build/src/ ./rev-test-simple-struct.py
else
	echo "Test SIMPLE_STRUCT: simple_struct.exe not Found - likely build failed"
	exit 1
fi
