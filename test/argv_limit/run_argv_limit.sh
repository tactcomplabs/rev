#!/bin/bash

#Build the test
make clean && make

args=()
for arg in {1..4096}
do
    args+=("$arg")
done

IFS=,
argstr="[${args[*]}]"
IFS=

# Check that the exec was built...
if [[ -x argv_limit.exe ]]; then
        set -e
	sst --add-lib-path=../../build/src/ ../rev-model-options-config.py -- --program "argv_limit.exe" --enableMemH=0 --args "$argstr"
	echo "Test ARGV_LIMIT_REVMEM: Completed"
	sst --add-lib-path=../../build/src/ ../rev-model-options-config.py -- --program "argv_limit.exe" --enableMemH=1 --args "$argstr"
	echo "Test ARGV_LIMIT_MEMH: Completed"
else
	echo "Test ARGV_LIMIT: argv_limit.exe not Found - likely build failed"
	exit 1
fi
