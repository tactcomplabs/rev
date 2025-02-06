#!/bin/bash

# 64-bit
riscv64-unknown-elf-g++ -g -O0 -mabi=ilp32 -march=rv32i -I../../common/syscalls -I../include -o probe.exe probe.c || exit 1
riscv64-unknown-elf-objdump -D --source probe.exe > probe.dis || exit 2

ARCH=rv64g_zicntr REV_EXE=probe.exe sst --add-lib-path=../../build/src ./rev-probe.py | tee probe.log
echo
echo "######### console messages ########"
../../scripts/rev-print.py -l probe.log
