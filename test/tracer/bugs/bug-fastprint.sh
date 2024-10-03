#!/bin/bash -x

declare -a opts=("O0" "O1" "O2" "O3")
REVROOT=$(realpath ../../..)
for opt in "${opts[@]}"; do
    riscv64-unknown-elf-g++ -g "-$opt" -march=rv64g -DRV64G -I"$REVROOT"/common/syscalls -I"$REVROOT"/test/include -o "bug-fastprint.$opt.exe" bug-fastprint.cc
    riscv64-unknown-elf-objdump -D --source "bug-fastprint.$opt.exe" > "bug-fastprint.$opt.dis"
    ARCH=rv64g_zicntr "REV_EXE=bug-fastprint.$opt.exe" sst --add-lib-path="$REVROOT"/build/src ../rev-test-tracer.py > "bug-fastprint.$opt.log"
    "$REVROOT"/scripts/rev-print.py -l "bug-fastprint.$opt.log" | tee "bug-fastprint.$opt.revlog"
done
