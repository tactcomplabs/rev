#!/bin/bash

#Build the test
make

# Check that the exec was built...
if [ -x $RVASM.exe ]; then
	sst --add-lib-path=../../build/src/ --model-options=$RVASM.exe ./rev-isa-test.py
else
	echo "Test $RVASM ASM: File not found - likely build failed"
	exit 1
fi
