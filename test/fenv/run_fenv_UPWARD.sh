#!/bin/bash

set -e
make fenv_test_UPWARD.exe
sst --add-lib-path=../../build/src/ ../rev-model-options-config.py -- --program=fenv_test_UPWARD.exe --verbose=6
