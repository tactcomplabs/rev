#!/bin/bash

set -e
rm -f fenv_test_UPWARD.exe
make fenv_test_UPWARD.exe
sst --add-lib-path=../../build/src/ ../rev-model-options-config.py -- --program=fenv_test_UPWARD.exe
