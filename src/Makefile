#
# src/Makefile
#
# Integrated makefile to build the RevCPU library
#
# Copyright (C) 2017-2020 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

ifeq (, $(shell which sst-config))
 $(error "No sst-config in $(PATH), add `sst-config` to your PATH")
endif

CXX=$(shell sst-config --CXX)
CXXFLAGS=$(shell sst-config --ELEMENT_CXXFLAGS)
LDFLAGS=$(shell sst-config --ELEMENT_LDFLAGS)
CPPFLAGS=-I./
OPTIMIZE_FLAGS=-O3

REV_SOURCES := $(wildcard *.cc)

REV_HEADERS := $(wildcard *.h)

REV_OBJS := $(patsubst %.cc,%.o,$(wildcard *.cc))

all: librevcpu.so
librevcpu.so: $(REV_OBJS)
	$(CXX) $(OPTIMIZE_FLAGS) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ *.o
%.o:%.cc $(REV_HEADERS)
	$(CXX) $(OPTIMIZE_FLAGS) $(CXXFLAGS) $(CPPFLAGS) -c $<
install: librevcpu.so
	sst-register revcpu revcpu_LIBDIR=$(CURDIR)
clean:
	rm -Rf *.o librevcpu.so

#-- EOF
