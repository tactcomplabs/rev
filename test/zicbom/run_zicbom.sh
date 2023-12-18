#!/bin/bash

#Build the test
make clean && make

# Check that the exec was built...
if [ -x zicbom.exe ]; then
	sst --add-lib-path=../../build/src/ ./rev-test-zicbom.py
else
	echo "Test TEST_ZICBOM: zicbom.exe not Found - likely build failed"
	exit 1
fi
