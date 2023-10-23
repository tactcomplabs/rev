#
# Makefile
#
# makefile: simple_pthreads
#
# Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

.PHONY: src

TEST_NAME = simple_pthreads
REV_ROOT=../../../..
CC=${RVCC}
OBJDUMP=${RISCV}/bin/riscv64-unknown-elf-objdump -D --source 
INCLUDE = -I$(REV_ROOT)/common/syscalls -I$(REV_ROOT)/test/include

# permute architectures
ARCHS = rv32i rv64g
# ARCHS = rv64g
TLIST  = $(addprefix $(TEST_NAME).,$(ARCHS))
EXES   = $(addsuffix .exe,$(TLIST))
DIASMS = $(addsuffix .dis,$(TLIST))

# permute hardware
HARTS = 1 2 4
LOGS := $(addprefix $(TEST_NAME).,$(HARTS))
LOGS := $(foreach a,$(ARCHS),$(addsuffix .$(a).log,$(LOGS)))

TARGS = $(EXES) $(DIASMS) $(LOGS)

all: $(TARGS)

compile: $(EXES) $(DIASMS)

%.dis: %.exe
	$(OBJDUMP) $< > $@

%.rv32i.exe: ARCH=rv32i
%.rv32i.exe: CFLAGS=-mabi=ilp32
%.rv64g.exe: ARCH=rv64g

%.exe: $(TEST_NAME).c
	$(CC) -g -O0 -march=$(ARCH) $(CFLAGS) $(INCLUDE) -o  $@ $<

#
# Testing section
#

%.rv64g.log: ARCH=rv64g
%.rv32i.log: ARCH=rv32i

%.rv64g.log: $(TEST_NAME).rv64g.exe
	@$(eval tmp = $(basename $@).tmplog)
	@$(eval params = $(subst ., ,$@))
	@$(eval harts = $(word 2, $(params)))
	NUM_HARTS=$(harts) ARCH=$(ARCH) REV_EXE=$< sst --add-lib-path=$(REV_ROOT)/build/src ./rev-test-pthread.py > $(tmp)
	mv $(tmp) $@

# sadly repeating for now
%.rv32i.log: $(TEST_NAME).rv32i.exe
	@$(eval tmp = $(basename $@).tmplog)
	@$(eval params = $(subst ., ,$@))
	@$(eval harts = $(word 2, $(params)))
	NUM_HARTS=$(harts) ARCH=$(ARCH) REV_EXE=$< sst --add-lib-path=$(REV_ROOT)/build/src ./rev-test-pthread.py > $(tmp)
	mv $(tmp) $@

.PHONY: clean

clean:
	rm -f $(EXES) $(DIASMS) $(LOGS) *.tmplog

#-- EOF
