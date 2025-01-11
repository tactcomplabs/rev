#
# Makefile
#
# makefile: big_loop.c
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

.PHONY: src

EXAMPLE=memcpy
CC=${RVCC}
ARCH=rv64imafd

src_dir = .
COMMON_RISCV_OPTS ?= -DPREALLOCATE=1 -mcmodel=medany -static -std=gnu99 -O2 -ffast-math -fno-common -fno-builtin-printf -march=$(ARCH) -mabi=lp64d
incs  += -I$(src_dir)/../env -I$(src_dir)/../common $(addprefix -I$(src_dir)/, $(bmarks))

compiler=$(findstring clang,${RVCC})
ifeq ($(compiler),clang)
  RISCV_OPTS ?= $(COMMON_RISCV_OPTS)
  RISCV_LINK_OPTS ?= -static -e main
else
  RISCV_OPTS ?= $(COMMON_RISCV_OPTS) -fno-tree-loop-distribute-patterns
  RISCV_LINK_OPTS ?= -static -nostdlib --entry main
endif

all: $(EXAMPLE).exe
$(EXAMPLE).exe: $(EXAMPLE).c
	$(CC) $(incs) $(RISCV_OPTS) -o $(EXAMPLE).exe $(EXAMPLE).c $(RISCV_LINK_OPTS)
clean:
	rm -Rf $(EXAMPLE).exe

#-- EOF
