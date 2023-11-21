#!/bin/bash

#Build the test
make clean && make

# Check that the exec was built...
if [ -f argc.exe ]; then
	sst --add-lib-path=../../build/src/ ../rev-basic-config.py -- --program "argc.exe" --args "one two three" --enableMemH=0
	echo "Test ARGC_REVMEM: Completed"
	sst --add-lib-path=../../build/src/ ../rev-basic-config.py -- --program "argc.exe" --args "one two three" --enableMemH=1
	echo "Test ARGC_MEMH: Completed"
else
	echo "Test ARGC_REVMEM: argc.exe not Found - likely build failed"
	exit 1
fi
