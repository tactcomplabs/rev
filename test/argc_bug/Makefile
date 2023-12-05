#
# Makefile
#
# makefile: argc bug
#
# Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

# Python configuration file
REVCFG ?= ./rev-test-basim.py

# Rev toolchain (for host Rev)
CC=${RVCC}
INCLUDE = -I$(REVHOME)/test/include
FLAGS     = -O2 -g -static -lm

OBJDUMP   = ${RISCV}/bin/riscv64-unknown-elf-objdump -D --source -C -l

# REV simulator includes BASim
REVHOME ?= $(realpath ../..)
REVLIBPATH ?= $(REVHOME)/build/src

# configuration
# rev core needs to use rv64imfdc to support compact instructions in stdlib (apparantly)
ARCH             ?= rv64imfd
ABI              ?= -mabi=lp64d
NUM_LANES        ?= 64
NUM_THR_PER_LANE ?= 4
NUM_ELEM_PER_THREAD ?= 2
CHUNK ?= 0

# Targets
TLIST = $(patsubst %.c,%,$(wildcard *.c))
EXES   = $(addsuffix .exe,$(TLIST))
DIASMS = $(addsuffix .dis,$(TLIST))
LOGS   = $(addsuffix .log,$(TLIST))
TARGS  = $(EXES) $(DIASMS) $(LOGS)

# Recipes
all: $(TARGS)

compile: $(EXES) $(DIASMS)

run: $(LOGS)

%.dis: %.exe
	$(OBJDUMP) $< > $@

%.exe: %.c
	$(CC) $(FLAGS) -o $@ $< -march=$(ARCH) $(ABI) $(INCLUDE)

%.log: %.exe $(BINS)
	@$(eval tmp = $(basename $@).tmplog)
	REV_EXE=$< sst --add-lib-path=$(REVLIBPATH) $(REVCFG) > $(tmp)
	mv $(tmp) $@

.PHONY: clean run

clean:
	rm -f $(TARGS) *.tmplog *.csv

.PHONY: help
help:
	@echo make compile
	@echo make run
	@echo make
	@echo environment variables: REVCFG NUM_LANES NUM_THR_PER_LANE NUM_ELEM_PER_THREAD CHUNK


.SECONDARY:

#-- EOF
