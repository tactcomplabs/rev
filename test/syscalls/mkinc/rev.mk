#
# REV make include file: rev.mk
#
# Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

ifndef REVHOME
$(error REVHOME not defined)
endif

REV_ARCH ?= rv64imafdc
REV_VERBOSE ?= 1
REV_ENABLE_MEMH ?= 0
REV_MACHINE ?= "[CORES:RV64GC]"

REV_SDL ?= $(REVHOME)/test/rev-model-options-config.py
REV_SDL_PARAMS = --verbose=$(REV_VERBOSE) --enableMemH=$(REV_ENABLE_MEMH) --machine=$(REV_MACHINE)

CC="${RVCC}"
CCOPTS += -march=$(REV_ARCH) $(INCLUDES)
CCOPTS += -I$(REVHOME)/common/syscalls
CCOPTS += -I$(REVHOME)/test/include

.PHONY: run

all: $(TESTNAME).exe

# STATIC = -static
$(TESTNAME).exe: $(SOURCES)
	$(CC) $(CCOPTS) -o $(TESTNAME).exe $^ $(STATIC)

ifdef RVOBJDUMP
all: $(TESTNAME).d
$(TESTNAME).d: $(TESTNAME).exe
	$(RVOBJDUMP) -dC -Mno-aliases --source $< > $@
endif


ifdef ARGS
 PROG_ARGS = --args="$(ARGS)"
endif
run: $(TESTNAME).exe
	sst $(REV_SDL) -- $(REV_SDL_PARAMS) --program=$(TESTNAME).exe $(PROG_ARGS)

#-- EOF
