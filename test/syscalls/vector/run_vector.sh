#!/bin/bash

#Build the test
make clean && make

if [ -f vector.exe ]; then
	sst --add-lib-path=../../build/src/ ../../rev-model-options-config.py -- --program "vector.exe" --startSymbol "[0:_start]" --enableMemH=0
	echo "Test vector_REVMEM: Completed"
	sst --add-lib-path=../../build/src/ ../../rev-model-options-config.py -- --program "vector.exe" --startSymbol "[0:_start]" --enableMemH=1
	echo "Test vector_MEMH: Completed"
else
	echo "Test vector: vector.exe not Found - likely build failed"
	exit 1
fi
