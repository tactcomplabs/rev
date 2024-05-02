#
# Makefile
#
# makefile: ex1
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
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
RISCV_GCC_OPTS ?= -DPREALLOCATE=1 -mcmodel=medany -static -std=gnu99 -O0 -ffast-math -fno-common -fno-builtin-printf -march=$(ARCH) -mabi=lp64d

ifeq "$(RVCC)" "riscv64-unknown-elf-gcc"
	RISCV_GCC_OPTS += -fno-tree-loop-distribute-patterns
endif

all:$(ISA_EXES)
$(ISA_TESTS): $(ISA_EXES)
%.exe:%.c $(ISA_HEADERS)
#	@echo "RISCV_GCC_OPTS = $(RISCV_GCC_OPTS)"
	$(CC) $(RISCV_GCC_OPTS)  -o $@ $< -T ./test.ld -static -nostdlib -lm -Wl,-e,main
clean:
	rm -Rf *.exe

#-- EOF
