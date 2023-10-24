#
# Makefile
#
# makefile: tracer
#
# Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

.PHONY: src

EXAMPLE = tracer
TLIST  = $(addprefix $(EXAMPLE),.rv32i .rv64g)
EXES   = $(addsuffix .exe,$(TLIST))
DIASMS = $(addsuffix .dis,$(TLIST))
LOGS   = $(addsuffix .log,$(TLIST))

TARGS = $(EXES) $(DIASMS) $(LOGS)

CC=${RVCC}
OBJDUMP=${RISCV}/bin/riscv64-unknown-elf-objdump -D --source 
INCLUDE = -I../../common/syscalls -I../include

all: $(TARGS)
	@echo Done: See $(LOGS)

compile: $(EXES) $(DIASMS)

%.dis: %.exe
	$(OBJDUMP) $< > $@

%.rv32i.exe: ARCH=rv32i
%.rv32i.exe: OPTS=-mabi=ilp32

%.rv64g.exe: ARCH=rv64g
%.rv64g.exe: OPTS=-DRV64G

%.exe: tracer.c
	$(CC) -g -O0 -march=$(ARCH) $(OPTS) $(INCLUDE) -o  $@ $<

%.rv64g.log: ARCH=rv64g
%.rv32i.log: ARCH=rv32i
%.log: %.exe
	@$(eval tmp := $(basename $@).tmplog)
	ARCH=$(ARCH) REV_EXE=$< sst --add-lib-path=../../build/src ./rev-test-tracer.py > $(tmp)
	mv $(tmp) $@

.PHONY: clean

clean:
	rm -f $(EXES) $(DIASMS) $(LOGS) *.log.tmp

#-- EOF
