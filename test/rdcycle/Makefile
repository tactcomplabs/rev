#
# Makefile
#
# makefile: rdcycle
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

EXAMPLE=rdcycle
CC="${RVCC}"
# ARCH=rv64gc_zicntr
ARCH=rv64gc
ABI=lp64d

CFLAGS=-I ../../common/syscalls -O1

all: $(EXAMPLE).exe
$(EXAMPLE).exe: $(EXAMPLE).c
	$(CC) $(CFLAGS) -march=$(ARCH) -mabi=$(ABI) -static -o $(EXAMPLE).exe $(EXAMPLE).c
clean:
	rm -Rf $(EXAMPLE).exe

#-- EOF
