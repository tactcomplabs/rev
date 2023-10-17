#
# Makefile
#
# makefile: tracer
#
# Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

.PHONY: src

EXAMPLE=tracer
CC=${RVCC}
OBJDUMP=${RISCV}/bin/riscv64-unknown-elf-objdump -D --source 

ARCH=rv32i
ABI=-mabi=ilp32
//ARCH=rv64g

all: $(EXAMPLE).exe $(EXAMPLE).dis
$(EXAMPLE).exe: $(EXAMPLE).c
	$(CC) -g -O0 -march=$(ARCH) $(ABI) -o  $(EXAMPLE).exe $(EXAMPLE).c
$(EXAMPLE).dis: $(EXAMPLE).exe
	$(OBJDUMP) $< > $@

clean:
	rm -Rf $(EXAMPLE).exe $(EXAMPLE).dis

#-- EOF
