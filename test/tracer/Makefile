#
# Makefile
#
# makefile: tracer
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

EXAMPLE ?= tracer
REVCFG = $(realpath ../rev-model-options-config.py)
NUMCORES ?= 1
NUMHARTS ?= 1

# Set to compile and simulate for spike
# USE_SPIKE := 1
ifdef USE_SPIKE
 SPIKE = /opt/homebrew/bin/spike
 OPTS += -DSPIKE
endif

# C tests
ALLTESTS   := $(addprefix $(EXAMPLE),.mem.rv32.c .mem.rv64.c .memh.rv32.c .memh.rv64.c)
# C++ tests
ALLTESTS   += $(addprefix $(EXAMPLE),.mem.rv32.cpp .mem.rv64.cpp .memh.rv32.cpp .memh.rv64.cpp)

TLIST = $(ALLTESTS)
# TLIST = $(filter-out %rv32.c %rv32.cpp,$(ALLTESTS))
# TLIST = $(filter %.mem.rv64.cpp %.mem.rv32.cpp,$(ALLTESTS))

EXES   = $(addsuffix .exe,$(TLIST))
DIASMS = $(addsuffix .dis,$(TLIST))
ifndef USE_SPIKE
LOGS   = $(addsuffix .log,$(TLIST))
REVLOGS = $(addsuffix .revlog,$(TLIST))
else

endif

TARGS = $(EXES) $(DIASMS) $(LOGS) $(REVLOGS)

CC=${RVCC}
CXX=${RVCXX}
REVPRINT=$(realpath ../../scripts/rev-print.py)

OBJDUMP=${RISCV}/bin/riscv64-unknown-elf-objdump -D --source
INCLUDE = -I../../common/syscalls -I../include

all: $(TARGS)
	@echo Done: See $(LOGS)

compile: $(EXES) $(DIASMS)

%.dis: %.exe
	$(OBJDUMP) $< > $@

%.c.exe: GCC = $(CC)
%.cpp.exe: GCC = $(CXX)

%.rv32.c.exe %.rv32.cpp.exe: ARCH=rv32i
%.rv32.c.exe %.rv32.cpp.exe: OPTS += -mabi=ilp32

%.rv64.c.exe %.rv64.cpp.exe: ARCH=rv64gc
%.rv64.c.exe %.rv64.cpp.exe: OPTS +=-DRV64G

%.mem.rv32.c.log %.mem.rv32.cpp.log:   MEMH=0
%.memh.rv32.c.log %.memh.rv32.cpp.log: MEMH=1

%.mem.rv64.c.log %.mem.rv64.cpp.log:   MEMH=0
%.memh.rv64.c.log %.memh.rv64.cpp.log: MEMH=1

%.exe: $(EXAMPLE).c
	$(GCC) -g -O2 -march=$(ARCH) $(OPTS) $(INCLUDE) -o  $@ $<

%.rv32.c.log %.rv32.cpp.log: ARCH=rv32g
%.rv64.c.log %.rv64.cpp.log: ARCH=rv64gc

%.log: %.exe
	@$(eval tmp := $(basename $@).tmplog)
	sst --add-lib-path=../../build/src ../rev-model-options-config.py -- \
	 --enableMemH=$(MEMH) --trcStartCycle=1 --verbose=5 --program=$< --machine=CORES:$(ARCH)_zicntr \
	 --numCores=$(NUMCORES) --numHarts=$(NUMHARTS) > $(tmp)
	mv $(tmp) $@

%.revlog: %.log
	$(REVPRINT) -l $< > $@

.PHONY: clean help

clean:
	rm -f $(TARGS) *.tmplog

help:
	@echo make
	@echo make compile
	@echo make NUMHARTS=2
	@echo make NUMCORES=2
	@echo make USE_SPIKE=1
	@echo make TLIST=\"tracer.mem.rv32.c tracer.mem.rv64.c\"
	@echo available tests:
	@echo $(ALLTESTS)

#-- EOF
