#!/bin/bash

set -e
rm -f fenv_test_TONEAREST.exe
make fenv_test_TONEAREST.exe
sst --add-lib-path=../../build/src/ ../rev-model-options-config.py -- --program=fenv_test_TONEAREST.exe
