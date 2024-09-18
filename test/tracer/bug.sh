#!/bin/bash

declare -a opts=("O0" "O1" "O2" "O3")

for opt in "${opts[@]}"; do
    riscv64-unknown-elf-g++ -g -$opt -march=rv64g -DRV64G -I../../common/syscalls -I../include -o bug.$opt.exe bug.cpp
    riscv64-unknown-elf-objdump -D --source bug.$opt.exe > bug.$opt.dis
    ARCH=rv64g REV_EXE=bug.$opt.exe sst --add-lib-path=../../build/src ./rev-test-tracer.py > bug.$opt.log
   ../../scripts/rev-print.py -l bug.$opt.log | tee bug.$opt.runlog
done

