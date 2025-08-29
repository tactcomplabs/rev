#
# spike make include file: spike.mk
#
# Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

SPIKE ?= spike

ifeq (, $(shell which $(SPIKE)))
 $(error $(SPIKE) not found)
endif

SPIKE_ARCH ?= rv64imafdc

CC="${RVCC}"
CCOPTS += -march=$(SPIKE_ARCH) $(INCLUDES)
CCOPTS += -I$(REVHOME)/common/syscalls
CCOPTS += -I$(REVHOME)/test/include
CCOPTS += -DSPIKE_TARGET

.PHONY: run

all: $(TESTNAME).exe

STATIC = -static
$(TESTNAME).exe: $(SOURCES)
	$(CC) $(CCOPTS) -o $(TESTNAME).exe $^ $(STATIC)

ifdef RVOBJDUMP
all: $(TESTNAME).d
$(TESTNAME).d: $(TESTNAME).exe
	$(RVOBJDUMP) -dC -Mno-aliases --source $< > $@
endif

#SPIKE_OPTS=

run: $(TESTNAME).spike

ifdef ARGS
 PROG_ARGS = $(ARGS)
endif
$(TESTNAME).spike: $(TESTNAME).exe
	$(SPIKE) $(SPIKE_OPTS) -l --log=$@ --isa=$(SPIKE_ARCH) pk $(TESTNAME).exe $(PROG_ARGS)

#-- EOF
