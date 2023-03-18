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

EXAMPLE=minfft
CC=riscv64-unknown-elf-gcc
ARCH=rv64imfd
#ARCH=rv32i
#ABI=ilp32

all: $(EXAMPLE).exe
$(EXAMPLE).exe: $(EXAMPLE).c
	#$(CC) -O1 -mcmodel=medlow -march=$(ARCH) -mabi=lp64d -o $(EXAMPLE).exe minfft.c $(EXAMPLE).c -lm
	$(CC) -O2 -mno-explicit-relocs -mno-save-restore -march=$(ARCH) -mabi=lp64d -o $(EXAMPLE).exe $(EXAMPLE).c ex1.c -lm
clean:
	rm -Rf $(EXAMPLE).exe

#-- EOF
