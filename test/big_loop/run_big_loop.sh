#!/bin/bash

#Build the test
make clean && make

# Check that the exec was built...
if [ -x big_loop.exe ]; then
	sst --add-lib-path=../../build/src/ ../rev-model-options-config.py -- --program="big_loop.exe" --numHarts=1 --numCores=1
else
	echo "Test BIG_LOOP: big_loop.exe not Found - likely build failed"
	exit 1
fi
