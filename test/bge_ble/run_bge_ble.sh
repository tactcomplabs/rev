#!/bin/bash

#Build the test
make clean && make

# Check that the exec was built...
if [[ -x bge_ble.exe ]]; then
	sst --add-lib-path=../../build/src/ ./rev-bge-ble.py
else
	echo "Test BGE_BLE: bge_ble.exe not Found - likely build failed"
	exit 1
fi
