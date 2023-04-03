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

EXAMPLE=add
CC="${RVCC}"
#CC=riscv64-unknown-linux-gnu-gcc
ARCH=rv64imafdc
#ABI=lp64

all: $(EXAMPLE).exe
$(EXAMPLE).exe: $(EXAMPLE).c
	#$(CC) -march=$(ARCH) -mabi=$(ABI) -o $(EXAMPLE).exe $(EXAMPLE).c
	$(CC) -march=$(ARCH) -O0 -o $(EXAMPLE).exe $(EXAMPLE).c -static
clean:
	rm -Rf $(EXAMPLE).exe

#-- EOF
