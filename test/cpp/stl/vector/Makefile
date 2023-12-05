#
# Makefile
#
# makefile: vector
#
# Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

.PHONY: src

EXAMPLE=vector
ARCH=rv64gc
CC=$(RVCC)
CXX=$(RVCXX)

# Use the git rev-parse command to get the root of the git repository
GIT_ROOT=$(shell git rev-parse --show-toplevel)
REV_INCLUDES=$(GIT_ROOT)/common/rev_includes

all: $(EXAMPLE).exe
$(EXAMPLE).exe: $(EXAMPLE).cc
	$(CXX) -march=$(ARCH) -O0 -I$(REV_INCLUDES) -o $(EXAMPLE).exe $(EXAMPLE).cc -static
clean:
	rm -Rf $(EXAMPLE).exe

#-- EOF
