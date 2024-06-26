#!/bin/sh
set -e

make clean
make

# Unknown extension
sst --add-lib-path=../../build/src/ ../rev-model-options-config.py -- --program "ext_version.exe" --args "one" --enableMemH=0 --machine="[CORES:RV64XGC]" 2>&1 | grep -q 'Error: failed to parse the machine model: RV64XGC'

# Out of order extension
sst --add-lib-path=../../build/src/ ../rev-model-options-config.py -- --program "ext_version.exe" --args "one" --enableMemH=0 --machine="[CORES:RV64GVC]" 2>&1 | grep -q 'Error: failed to parse the machine model: RV64GVC'

# Incomplete version string
sst --add-lib-path=../../build/src/ ../rev-model-options-config.py -- --program "ext_version.exe" --args "one" --enableMemH=0 --machine="[CORES:RV64XGC2P]" 2>&1 | grep -q 'Error: failed to parse the machine model: RV64XGC2P'

# Unsupported version
sst --add-lib-path=../../build/src/ ../rev-model-options-config.py -- --program "ext_version.exe" --args "one" --enableMemH=0 --machine="[CORES:RV64GCV0p7]" 2>&1 | grep -q 'Error: Version 0.7 of V extension is not supported'

# Supported version
sst --add-lib-path=../../build/src/ ../rev-model-options-config.py -- --program "ext_version.exe" --args "one" --enableMemH=0 --machine="[CORES:rv64i2p0m2a2p0fd2p0c]" 2>&1 | grep -q 'Simulation is complete'

# Supported version
sst --add-lib-path=../../build/src/ ../rev-model-options-config.py -- --program "ext_version.exe" --args "one" --enableMemH=0 --machine="[CORES:rv64i2p0m2a2p0fd2p0c2_p]" 2>&1 | grep -q 'Simulation is complete'

echo 'Simulation is complete'
