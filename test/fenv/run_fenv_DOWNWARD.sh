#!/bin/bash

set -e
make fenv_test_DOWNWARD.exe
sst --add-lib-path=../../build/src/ ../rev-model-options-config.py -- --program=fenv_test_DOWNWARD.exe --verbose=6
