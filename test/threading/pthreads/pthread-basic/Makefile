#
# Makefile
#
# makefile: pthread-basic
#
# Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

.PHONY: src
EXAMPLE=pthread-basic

#CC=riscv64-unknown-elf-gcc
CC="${RVCC}"
#ARCH=rv64g
ARCH=rv64g

all: $(EXAMPLE).exe
$(EXAMPLE).exe: $(EXAMPLE).c
	$(CC) -march=$(ARCH) -O0 -o $(EXAMPLE).exe $(EXAMPLE).c  -static
clean:
	rm -Rf $(EXAMPLE).exe

#-- EOF
