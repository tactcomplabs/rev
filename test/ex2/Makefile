#
# Makefile
#
# makefile: ex2
#
# Copyright (C) 2017-2020 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

.PHONY: src

EXAMPLE=ex2
CC=riscv64-unknown-elf-gcc
ARCH=rv64imac
ABI=lp64

all: $(EXAMPLE).exe
$(EXAMPLE).exe: $(EXAMPLE).c
	$(CC) -march=$(ARCH) -mabi=$(ABI) -o $(EXAMPLE).exe $(EXAMPLE).c
clean:
	rm -Rf $(EXAMPLE).exe

#-- EOF
