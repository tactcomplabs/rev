#!/bin/bash

#Build the test
make clean && make

# Check that the exec was built...
if [[ -x basic.exe ]]; then
	sst --add-lib-path=../../build/src/ ./mem_dump.py
else
	echo "Test mem_dump (python config options - not syscall version): basic.exe not Found - likely build failed"
	exit 1
fi
