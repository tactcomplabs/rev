#!/bin/bash

#Build the test
make clean && make

# Check that the exec was built...
if [[ -x argv_layout.exe ]]; then
    set -e
    sst --add-lib-path=../../build/src/ ../rev-model-options-config.py -- --program "argv_layout.exe" --args "test test" --enableMemH=0
    sst --add-lib-path=../../build/src/ ../rev-model-options-config.py -- --program "argv_layout.exe" --args "test test" --enableMemH=1
else
    echo "Test ARGV_LAYOUT: argv_layout.exe not Found - likely build failed"
    exit 1
fi
