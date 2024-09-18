#!/bin/bash -x

riscv64-unknown-elf-g++ -g -O0 -march=rv64g -DRV64G -I../../common/syscalls -I../include -o bug-rdtime.rv64.exe bug-rdtime.cc || exit 1
riscv64-unknown-elf-objdump -D --source bug-rdtime.rv64.exe > bug-rdtime.rv64.dis || exit 2
ARCH=rv64g REV_EXE=bug-rdtime.rv64.exe sst --add-lib-path=../../build/src ./rev-test-tracer.py > bug-rdtime.rv64.log && echo passed for rv64 || exit 3
../../scripts/rev-print.py -l bug-rdtime.rv64.log

riscv64-unknown-elf-g++ -g -O0 -march=rv32i -mabi=ilp32 -I../../common/syscalls -I../include -o  bug-rdtime.rv32.exe bug-rdtime.cc || exit 10
riscv64-unknown-elf-objdump -D --source bug-rdtime.rv32.exe > bug-rdtime.rv32.dis || exit 11
ARCH=rv32g REV_EXE=bug-rdtime.rv32.exe sst --add-lib-path=../../build/src ./rev-test-tracer.py > bug-rdtime.rv32.log && echo passed for rv32 || exit 12
../../scripts/rev-print.py -l bug-rdtime.rv32.log
