#!/bin/bash

#Build the test
make

# Check that the exec was built...
if [ -f strlen_cxx.exe ]; then
  sst ./strlen_cxx.py
else
  echo "Test STRLEN_CXX: strlen_cxx.exe not Found - likely build failed"
  exit 1
fi 
