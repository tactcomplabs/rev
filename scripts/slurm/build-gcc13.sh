#!/bin/bash
#
# scripts/slurm/build-gcc13.sh
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# This file is a part of the PAN-RUNTIME package.  For license
# information, see the LICENSE file in the top level directory of
# this distribution.
#
#
# Sample SLURM batch script
#
# Usage: sbatch -N1 build.sh
#
# This command requests 1 nodes for execution
#

#-- Stage 1: load the necessary modules
source /etc/profile.d/modules.sh
module load riscv/gcc/13.2.0 cmake/3.23.0 sst/13.0.0
export CC=gcc-11
export CXX=g++-11
export RVCC=riscv64-unknown-elf-gcc

exec >> "rev.jenkins.${SLURM_JOB_ID}.out" 2>&1
sst --version
sst-info revcpu

#-- Stage 2: setup the build directories
mkdir -p build
cd build || exit
rm -Rf ./*

#-- Stage 3: initiate the build
cmake -DBUILD_ASM_TESTING=ON -DCMAKE_BUILD_TYPE=Debug -DRVCC=${RVCC} ../
make clean
make uninstall
make -j
#make install

#-- Stage 4: test everything
make test

#-- EOF
