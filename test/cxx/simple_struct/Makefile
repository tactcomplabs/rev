#
# Makefile
#
# makefile: simple_struct
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

.PHONY: src

EXAMPLE=simple_struct
CC=riscv64-unknown-elf-g++
#ARCH=rv64g
ARCH=rv64gc

all: $(EXAMPLE).exe
$(EXAMPLE).exe: $(EXAMPLE).cc
	$(CC) -march=$(ARCH) -I../../../common/syscalls -o $(EXAMPLE).exe $(EXAMPLE).cc -static
clean:
	rm -Rf $(EXAMPLE).exe

#-- EOF
