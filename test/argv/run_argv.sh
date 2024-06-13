#!/bin/bash

#Build the test
make clean && make

# Check that the exec was built...
if [[ -x argv.exe ]]; then
        set -e
	sst --add-lib-path=../../build/src/ ../rev-model-options-config.py -- --program "argv.exe" --args "one" --enableMemH=0
	echo "Test ARGV_REVMEM: Completed"
	sst --add-lib-path=../../build/src/ ../rev-model-options-config.py -- --program "argv.exe" --args "one" --enableMemH=1
	echo "Test ARGV_MEMH: Completed"
else
	echo "Test ARGV: argv.exe not Found - likely build failed"
	exit 1
fi
