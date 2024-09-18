#!/bin/bash -x

riscv64-unknown-elf-g++ -g -O0 -march=rv64g -DRV64G -I../../common/syscalls -I../include -o bug-rdtime.rv64g.exe bug-rdtime.cc
riscv64-unknown-elf-objdump -D --source bug-rdtime.rv64g.exe > bug-rdtime.rv64g.dis
ARCH=rv64g REV_EXE=bug-rdtime.rv64g.exe sst --add-lib-path=../../build/src ./rev-test-tracer.py > bug-rdtime.rv64g.log && echo passed for rv64g || exit 1
../../scripts/rev-print.py -l bug-rdtime.rv64g.log

riscv64-unknown-elf-g++ -g -O0 -march=rv32i -mabi=ilp32 -I../../common/syscalls -I../include -o  bug-rdtime.rv32i.exe bug-rdtime.cc
riscv64-unknown-elf-objdump -D --source bug-rdtime.rv32i.exe > bug-rdtime.rv32i.dis
ARCH=rv32i REV_EXE=bug-rdtime.rv32i.exe sst --add-lib-path=../../build/src ./rev-test-tracer.py > bug-rdtime.rv32i.tmplog && echo passed for rv32i || exit 2
../../scripts/rev-print.py -l bug-rdtime.32i.log
