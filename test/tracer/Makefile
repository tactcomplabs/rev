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

.PHONY: src

EXAMPLE ?= tracer

ALLTESTS   := $(addprefix $(EXAMPLE),.mem.rv32.c .mem.rv64.c .memh.rv32.c .memh.rv64.c)
ALLTESTS   += $(addprefix $(EXAMPLE),.mem.rv32.cpp .mem.rv64.cpp .memh.rv32.cpp .memh.rv64.cpp)

TLIST = $(ALLTESTS)
# TLIST = $(filter-out %rv32.c %rv32.cpp,$(ALLTESTS))
# TLIST = $(filter %.mem.rv64.cpp %.mem.rv32.cpp,$(ALLTESTS))

EXES   = $(addsuffix .exe,$(TLIST))
DIASMS = $(addsuffix .dis,$(TLIST))
LOGS   = $(addsuffix .log,$(TLIST))
REVLOGS = $(addsuffix .revlog,$(TLIST))

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

# TODO check: compiling with rv32i but running with rv32g
%.rv32.c.exe %.rv32.cpp.exe: ARCH=rv32i
%.rv32.c.exe %.rv32.cpp.exe: OPTS=-mabi=ilp32

%.rv64.c.exe %.rv64.cpp.exe: ARCH=rv64g
%.rv64.c.exe %.rv64.cpp.exe: OPTS=-DRV64G

%.mem.rv32.c.log %.mem.rv32.cpp.log:   REVCFG=./rev-test-tracer.py
%.memh.rv32.c.log %.memh.rv32.cpp.log: REVCFG=./rev-test-tracer-memh.py

%.mem.rv64.c.log %.mem.rv64.cpp.log:   REVCFG=./rev-test-tracer.py
%.memh.rv64.c.log %.memh.rv64.cpp.log: REVCFG=./rev-test-tracer-memh.py

%.exe: $(EXAMPLE).c
	$(GCC) -g -O2 -march=$(ARCH) $(OPTS) $(INCLUDE) -o  $@ $<

%.rv64.c.log %.rv64.cpp.log: ARCH=rv64g
%.rv32.c.log %.rv32.cpp.log: ARCH=rv32g

%.log: %.exe
	@$(eval tmp := $(basename $@).tmplog)
	ARCH=$(ARCH) REV_EXE=$< sst --add-lib-path=../../build/src $(REVCFG) > $(tmp)
	mv $(tmp) $@

%.revlog: %.log
	$(REVPRINT) -l $< > $@

.PHONY: clean

clean:
	rm -f $(TARGS) *.tmplog

#-- EOF
