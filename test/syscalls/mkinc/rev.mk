#
# REV make include file: rev.mk
#
# Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

#CC=riscv64-unknown-elf-gcc
CC="${RVCC}"
#CCOPTS += -march=rv64g
CCOPTS += -march=rv64imafdc

REVHOME := $(realpath ../../..)
CCOPTS += -I$(REVHOME)/common/syscalls
CCOPTS += -I$(REVHOME)/test/include

.PHONY: run

all: $(TESTNAME).exe

# STATIC = -static
$(TESTNAME).exe: $(TESTNAME).c
	$(CC) $(CCOPTS) -o $(TESTNAME).exe $(TESTNAME).c $(STATIC)

ifdef RVOBJDUMP
all: $(TESTNAME).d
$(TESTNAME).d: $(TESTNAME).exe
	$(RVOBJDUMP) -dC -Mno-aliases --source $< > $@
endif

MEMH=--enableMemH=1
ifdef ARGS
 PROG_ARGS = --args="$(ARGS)"
endif
run: $(TESTNAME).exe
	sst $(REVHOME)/test/rev-model-options-config.py -- --verbose=5 --trcStartCycle=0 $(MEMH) --program=$(TESTNAME).exe $(PROG_ARGS)

#-- EOF
