#
# Makefile
#
# makefile: udruntime/test
# This area checks out runtime on REV  without any dependencies
# on updown install or lib
#
# Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#
ifdef GEM5_MODE
$(error Cannot set GEM5_MODE)
endif

ifdef FASTSIM
$(error Cannot set FASTSIM)
endif

# Python configuration file
REVCFG ?= ./rev-memh.py
#REVCFG ?= ./rev-mem.py

# Rev toolchain
CC=riscv64-unknown-elf-g++
# CC=riscv64-unknown-elf-gcc
REVHOME ?= $(realpath ../../../)
REVLIBPATH ?= $(REVHOME)/build/src
OBJDUMP   = ${RISCV}/bin/riscv64-unknown-elf-objdump -S -dC -Mno-aliases

# Rev runtime includes
INCLUDE  := -I$(REVHOME)/common/include
INCLUDE  := -I$(REVHOME)/common/syscalls
INCLUDE  += -I$(REVHOME)/test/include
INCLUDE  += -I../include
INCLUDE  += -I../src

# configuration
CCOPT ?= -O0
# CCOPT ?= -O2
# debug flags require custom syscalls in ud-sys-dev branch
# DEBUG_FLAGS += -DDEBUG_MODE
FLAGS     := $(CCOPT) -DASST $(DEBUG_FLAGS) -g -static -lm -fpermissive
ARCH      := rv64gc
ABI       := -mabi=lp64d

# Targets
TLIST ?= $(patsubst %.cpp,%,$(wildcard *.cpp))
EXES   = $(addsuffix .exe,$(TLIST))
DIASMS = $(addsuffix .dis,$(TLIST))
LOGS   = $(addsuffix .log,$(TLIST))
TARGS  = $(EXES) $(DIASMS) $(LOGS)
# BINS   = $(addsuffix .bin,$(TLIST))
TMPS   = $(addsuffix .tmplog,$(TLIST))

# Recipes
all: $(TARGS)

#compile: $(EXES) $(DIASMS)
compile: $(EXES)

run: $(LOGS)

%.dis: %.exe
	$(OBJDUMP) $< > $@

%.exe: %.cpp
	$(CC) $(FLAGS) -o $@ $< -march=$(ARCH) $(ABI) $(INCLUDE) $(LIBS)

# Append 'c' to ARCH since stdlib includes compact instructions
# If test fails the log file will be in testname.tmplog
%.log: %.exe
	@$(eval tmp = $(basename $@).tmplog)
	REV_EXE=$<
	REV_EXE=$< ARCH=$(ARCH) sst --add-lib-path=$(REVLIBPATH) $(REVCFG) > $(tmp)
	mv $(tmp) $@

.PHONY: clean run

clean:
	rm -f $(TARGS) $(TMPS)

.PHONY: help
help:
	@echo make compile
	@echo make run
	@echo make


.SECONDARY:

#-- EOF
