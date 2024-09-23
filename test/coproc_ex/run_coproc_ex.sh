#!/bin/bash

#Build the test
make

# Check that the exec was built...
if [[ -x coproc_ex.exe ]]; then
	sst-info revcpu
	sst --add-lib-path=../../build/src/ ./rev-test-coproc_ex.py
else
	echo "Test COPROC: File ex1.exe not found - likely build failed"
	exit 1
fi
