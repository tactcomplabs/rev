#
# src/Makefile
#
# Integrated makefile to build the RevCPU library
#
# Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

ifeq (, $(shell which sst-config))
 $(error "No sst-config in $(PATH), add `sst-config` to your PATH")
endif

CXX=$(shell sst-config --CXX)
CXXFLAGS=$(shell sst-config --ELEMENT_CXXFLAGS) -fno-stack-protector
LDFLAGS=$(shell sst-config --ELEMENT_LDFLAGS) -fno-stack-protector
CPPFLAGS=-I./
OPTIMIZE_FLAGS=-O2

REV_SOURCES := $(wildcard *.cc)

REV_HEADERS := $(wildcard *.h)

REV_OBJS := $(patsubst %.cc,%.o,$(wildcard *.cc))

REV_INCS := $(patsubst %.py,%.inc,$(wildcard *.py))

all: librevcpu.so

debug: CXXFLAGS += -DDEBUG -g
debug: librevcpu.so

librevcpu.so: $(REV_INCS) $(REV_OBJS)
	$(CXX) $(OPTIMIZE_FLAGS) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ *.o
%.o:%.cc $(REV_HEADERS)
	$(CXX) $(OPTIMIZE_FLAGS) $(CXXFLAGS) $(CPPFLAGS) -c $<
%.inc:%.py
	od -v -t x1 < $< | sed -e 's/^[^ ]*[ ]*//g' -e '/^\s*$$/d' -e 's/\([0-9a-f]*\)[ $$]*/0x\1,/g' > $@
uninstall:
	sst-register -u revcpu
install: librevcpu.so
	sst-register revcpu revcpu_LIBDIR=$(CURDIR)
clean:
	rm -Rf *.o *.inc librevcpu.so

#-- EOF
