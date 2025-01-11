#!/bin/bash

STORE="backingstore.bin"

#Build the test
make clean && make

# Check that the exec was built...
if [[ -x backingstore.exe ]]; then
        dd if=/dev/zero of=$STORE bs=64M count=16
        SHA1BEFORE=$(sha1sum $STORE | awk '{print $1}')
	sst --add-lib-path=../../build/src/ ./rev-backingstore.py
        SHA1AFTER=$(sha1sum $STORE | awk '{print $1}')
        if test "$SHA1BEFORE" = "$SHA1AFTER"
        then
          echo "FAILED: HASHES ARE THE SAME"
          exit 1
        else
          echo "SUCCESS"
        fi

else
	echo "Test BACKINGSTORE: backingstore.exe not Found - likely build failed"
	exit 1
fi
