#
# Makefile
#
# makefile: ex5
#
# Copyright (C) 2017-2020 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

.PHONY: src

EXAMPLE=math_log
CC=riscv64-unknown-elf-gcc
ARCH=rv64imafd

all: $(EXAMPLE).exe
$(EXAMPLE).exe: $(EXAMPLE).c
	$(CC) -march=$(ARCH) $(EXAMPLE).c -o $(EXAMPLE).exe -lm

clean:
	rm -Rf $(EXAMPLE).exe $(EXAMPLE).asm *.txt

#-- EOF
