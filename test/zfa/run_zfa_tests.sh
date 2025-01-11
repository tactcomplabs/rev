#!/bin/bash

set -e
sst --add-lib-path=../../build/src/ ../rev-model-options-config.py -- --program "zfa.exe" --machine="[CORES:RV64GC_Zfa]"   --enableMemH=0
