#
# Makefile
#
# makefile: ex1
#
# Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

.PHONY: src

CC=${RVCC}
ARCH=rv64imafdc

ISA_SOURCES := $(wildcard *.c)
ISA_HEADERS := $(wildcard *.h)
ISA_EXES=$(ISA_SOURCES:.c=.exe)
RISCV_GCC_OPTS ?= -DPREALLOCATE=1 -mcmodel=medany -static -std=gnu99 -O0 -ffast-math -fno-common -fno-builtin-printf -fno-tree-loop-distribute-patterns -march=$(ARCH) -mabi=lp64d

all:$(ISA_EXES)
$(ISA_TESTS): $(ISA_EXES)
%.exe:%.c $(ISA_HEADERS)
	#$(CC) -march=$(ARCH) -O0 -o $@ $< -T ./test.ld -static -lm
	$(CC) $(RISCV_GCC_OPTS)  -o $@ $< -T ./test.ld -static -nostdlib -lm --entry main
clean:
	rm -Rf *.exe

#-- EOF
