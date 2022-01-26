#
# src/Makefile
#
# Integrated makefile to build the RevCPU library
#
# Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
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
OPTIMIZE_FLAGS=-O3

SYSCALLS_SOURCES := $(wildcard *.cc)

SYSCALLS_HEADERS := $(wildcard *.h)

SYSCALLS_OBJS := $(patsubst %.cc,%.o,$(wildcard *.cc))

all: syscalls.o

debug: CXXFLAGS += -DDEBUG -g
debug: syscalls.o 

syscalls.o: $(SYSCALLS_OBJS)
	$(CXX) $(OPTIMIZE_FLAGS) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ *.o
%.o:%.cc $(SYSCALLS_HEADERS)
	$(CXX) $(OPTIMIZE_FLAGS) $(CXXFLAGS) $(CPPFLAGS) -c $<
clean:
	rm -Rf *.o syscalls.o

#-- EOF
