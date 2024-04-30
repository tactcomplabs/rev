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

EXAMPLE=minfft
CC=${RVCC}
ARCH=rv64imfd
#ARCH=rv32i
#ABI=ilp32

all: $(EXAMPLE).exe
$(EXAMPLE).exe: $(EXAMPLE).c
	#$(CC) -O1 -mcmodel=medlow -march=$(ARCH) -mabi=lp64d -o $(EXAMPLE).exe minfft.c $(EXAMPLE).c -lm
	#$(CC) -O2 -mno-explicit-relocs -mno-save-restore -march=$(ARCH) -mabi=lp64d -static -o $(EXAMPLE).exe $(EXAMPLE).c ex1.c -lm
	$(CC) -O2 -march=$(ARCH) -mabi=lp64d -static -o $(EXAMPLE).exe minfft_new.cc -lm
clean:
	rm -Rf $(EXAMPLE).exe

#-- EOF
