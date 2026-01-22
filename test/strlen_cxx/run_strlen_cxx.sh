#!/bin/bash

#Build the test
make clean && make

# Check that the exec was built...
if [[ -x strlen_cxx.exe ]]; then
	sst --add-lib-path="$BUILDDIR"/src/ ./strlen_cxx.py
else
	echo "Test STRLEN_CXX: strlen_cxx.exe not Found - likely build failed"
	exit 1
fi
