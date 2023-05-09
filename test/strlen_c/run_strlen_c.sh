#!/bin/bash

#Build the test
make

# Check that the exec was built...
if [ -f strlen_c.exe ]; then
  sst ./strlen_c.py
else
  echo "Test STRLEN_C: strlen_c.exe not Found - likely build failed"
  exit 1
fi 
