# Python configuration file
# REVCFG ?= ./rev-memh.py
REVCFG ?= ../rev-model-options-config.py

# Rev toolchain
CC=riscv64-unknown-elf-g++
#CC=riscv64-unknown-elf-gcc
REVHOME ?= $(realpath ../../)
REVLIBPATH ?= $(REVHOME)/build/src
OBJDUMP   = ${RISCV}/bin/riscv64-unknown-elf-objdump -S -dC -Mno-aliases

# Rev runtime includes
INCLUDE  := -I$(REVHOME)/common/include
INCLUDE  := -I$(REVHOME)/common/syscalls
INCLUDE  += -I$(REVHOME)/test/include

# configuration
CCOPT ?= -O0
# WFLAGS    := -Werror -Wall -Wextra -Wuninitialized -pedantic-errors
FLAGS     := $(CCOPT) -g -static -lm -fpermissive $(WFLAGS)
ARCH      := rv64gc
ABI       := -mabi=lp64d

# Targets
TLIST ?= $(patsubst %.cpp,%,$(wildcard *.cpp))
EXES   = $(addsuffix .exe,$(TLIST))
DIASMS = $(addsuffix .dis,$(TLIST))
LOGS   = $(addsuffix .log,$(TLIST))
TARGS  = $(EXES) $(DIASMS) $(LOGS)
TMPS   = $(addsuffix .tmplog,$(TLIST))

# Recipes
all: $(TARGS)

compile: $(EXES)

run: $(LOGS)

%.dis: %.exe
	$(OBJDUMP) $< > $@

%.exe: %.cpp
	$(CC) $(FLAGS) -o $@ $< -march=$(ARCH) $(ABI) $(INCLUDE) $(LIBS)

# If test fails the log file will be in testname.tmplog
%.log: %.exe
	@$(eval tmp = $(basename $@).tmplog)
	sst --add-lib-path=$(REVLIBPATH) $(REVCFG) -- --program=$< > $(tmp)
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
