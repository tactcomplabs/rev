#!/bin/bash
#
# scripts/slurm/build-ci-gcc13.2.0-sst14.0.0.sh
#
# Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# This file is a part of the Rev package.  For license
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
source /etc/qlustar/common/skel/bash/bashrc
module load riscv/14.2.0 sst/14.0.0
export CC=gcc
export CXX=g++
export RVCC=riscv64-unknown-elf-gcc

exec >> "rev.jenkins.${SLURM_JOB_ID}.out" 2>&1
sst --version
sst-info revcpu

#-- Stage 2: setup the build directories
mkdir -p build
cd build || exit
rm -Rf ./*

#-- Stage 3: initiate the build
cmake -DCMAKE_BUILD_TYPE=Debug -DRVCC=${RVCC} ../
make clean
make uninstall
make -j
#make install

#-- Stage 4: test everything
make test

#-- EOF
