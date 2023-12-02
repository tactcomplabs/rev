#!/bin/bash

#Build the test
make clean && make

# Check that the exec was built...
if [ -x argc.exe ]; then
	sst --add-lib-path=../../build/src/ ../rev-model-options-config.py -- --program "argc.exe" --args "one" --enableMemH=0
	echo "Test ARGC_SHORT_REVMEM: Completed"
	sst --add-lib-path=../../build/src/ ../rev-model-options-config.py -- --program "argc.exe" --args "one" --enableMemH=1
	echo "Test ARGC_SHORT_MEMH: Completed"
else
	echo "Test ARGC_SHORT: argc.exe not Found - likely build failed"
	exit 1
fi
