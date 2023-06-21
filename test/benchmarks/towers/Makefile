#
# Makefile
#
# makefile: big_loop.c
#
# Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

.PHONY: src

EXAMPLE=towers
CC=${RVCC}
ARCH=rv64imafd

src_dir = .
RISCV_GCC_OPTS ?= -DPREALLOCATE=1 -mcmodel=medany -static -std=gnu99 -O2 -ffast-math -fno-common -fno-builtin-printf -fno-tree-loop-distribute-patterns -march=$(ARCH) -mabi=lp64d
RISCV_LINK ?= $(RISCV_GCC) -T $(src_dir)/common/test.ld $(incs)
RISCV_LINK_OPTS ?= -static -nostdlib 

incs  += -I$(src_dir)/../env -I$(src_dir)/../common $(addprefix -I$(src_dir)/, $(bmarks))

all: $(EXAMPLE).exe
$(EXAMPLE).exe: $(EXAMPLE).c
	$(CC) $(incs) $(RISCV_GCC_OPTS) -o $(EXAMPLE).exe $(EXAMPLE).c $(RISCV_LINK_OPTS)
clean:
	rm -Rf $(EXAMPLE).exe

#-- EOF
