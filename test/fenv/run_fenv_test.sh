#!/bin/bash

sst --add-lib-path=../../build/src/ ../rev-model-options-config.py -- --program="$1" --verbose=0 | awk 'BEGIN {pass=0} /Simulation is complete/ {pass=1;next} {print} END {exit !pass}'
