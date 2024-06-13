#!/bin/bash

sst --add-lib-path=../../build/src/ ../rev-model-options-config.py -- --program="$1" --verbose=0
