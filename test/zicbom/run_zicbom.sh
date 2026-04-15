#!/bin/bash

#Build the test
make clean && make

# Check that the exec was built...
if [[ -x zicbom.exe ]]; then
    #sst --output-dot=orig.dot --dot-verbosity=7 --add-lib-path="$BUILDDIR"/src/ ./rev-test-zicbom.py
    sst --output-dot=zicbom.dot --dot-verbosity=7 --add-lib-path="$BUILDDIR"/src/ ./rev-mordred-zicbom.py
else
    echo "Test TEST_ZICBOM: zicbom.exe not Found - likely build failed"
    exit 1
fi
