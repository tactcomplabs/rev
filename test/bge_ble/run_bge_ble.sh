#!/bin/bash

#Build the test
make

# Check that the exec was built...
if [ -f bge_ble.exe ]; then
  sst --add-lib-path=../../src/ ./rev-bge-ble.py
else
  echo "Test BGE_BLE: bge_ble.exe not Found - likely build failed"
  exit 1
fi
