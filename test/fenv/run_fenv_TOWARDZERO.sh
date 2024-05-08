#!/bin/bash

set -e
rm -f fenv_test_TOWARDZERO.exe
make fenv_test_TOWARDZERO.exe
sst --add-lib-path=../../build/src/ ../rev-model-options-config.py -- --program=fenv_test_TOWARDZERO.exe
