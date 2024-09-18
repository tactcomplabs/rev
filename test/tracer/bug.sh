#!/bin/bash

/opt/homebrew/bin/riscv64-unknown-elf-g++ -g -O2 -march=rv64g -DRV64G -I../../common/syscalls -I../include -o  bug.mem.rv64g.cpp.exe bug.c
/opt/homebrew/bin/riscv64-unknown-elf-objdump -D --source bug.mem.rv64g.cpp.exe > bug.mem.rv64g.cpp.dis
ARCH=rv64g REV_EXE=bug.mem.rv64g.cpp.exe sst --add-lib-path=../../build/src ./rev-test-tracer.py > bug.log

# echo postprocessing for rev-fast-printf messages
#/Users/kgriesser/work/rev/scripts/rev-print.py -l log

