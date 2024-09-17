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

TLIST   = $(addprefix $(EXAMPLE),.mem.rv32i.c .mem.rv64g.c .memh.rv32i.c .memh.rv64g.c)
TLIST   += $(addprefix $(EXAMPLE),.mem.rv32i.cpp .mem.rv64g.cpp .memh.rv32i.cpp .memh.rv64g.cpp)

EXES   = $(addsuffix .exe,$(TLIST))
DIASMS = $(addsuffix .dis,$(TLIST))
LOGS   = $(addsuffix .log,$(TLIST))

TARGS = $(EXES) $(DIASMS) $(LOGS)

CC=${RVCC}
CXX=${RVCXX}

OBJDUMP=${RISCV}/bin/riscv64-unknown-elf-objdump -D --source
INCLUDE = -I../../common/syscalls -I../include

all: $(TARGS)
	@echo Done: See $(LOGS)

compile: $(EXES) $(DIASMS)

%.dis: %.exe
	$(OBJDUMP) $< > $@

%.c.exe: GCC = $(CC)
%.cpp.exe: GCC = $(CXX)

%.rv32i.c.exe %.rv32i.cpp.exe: ARCH=rv32i
%.rv32i.c.exe %.rv32i.cpp.exe: OPTS=-mabi=ilp32

%.rv64g.c.exe %.rv64g.cpp.exe: ARCH=rv64g
%.rv64g.c.exe %.rv64g.cpp.exe: OPTS=-DRV64G

%.mem.rv32i.c.log %.mem.rv32i.cpp.log:   REVCFG=./rev-test-tracer.py
%.memh.rv32i.c.log %.memh.rv32i.cpp.log: REVCFG=./rev-test-tracer-memh.py

%.mem.rv64g.c.log %.mem.rv64g.cpp.log:   REVCFG=./rev-test-tracer.py
%.memh.rv64g.c.log %.memh.rv64g.cpp.log: REVCFG=./rev-test-tracer-memh.py

%.exe: tracer.c
	$(GCC) -g -O0 -march=$(ARCH) $(OPTS) $(INCLUDE) -o  $@ $<

%.rv64g.c.log %.rv64g.cpp.log: ARCH=rv64g
%.rv32i.c.log %.rv32i.cpp.log: ARCH=rv32i

%.log: %.exe
	@$(eval tmp := $(basename $@).tmplog)
	ARCH=$(ARCH) REV_EXE=$< sst --add-lib-path=../../build/src $(REVCFG) > $(tmp)
	mv $(tmp) $@

.PHONY: clean

clean:
	rm -f $(EXES) $(DIASMS) $(LOGS) *.log.tmp

#-- EOF
