#
# Makefile
#
# makefile: chdir
#
# Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

.PHONY: src

EXAMPLE=chdir
CC=$(RVCC)
CXX=$(RVCXX)
ARCH=rv64gc

# Use the git rev-parse command to get the root of the git repository
REV_ROOT=$(shell git rev-parse --show-toplevel)
REV_INCLUDES=$(REV_ROOT)/common/rev_includes

all: $(EXAMPLE).exe
$(EXAMPLE).exe: $(EXAMPLE).c
	$(CC) -march=$(ARCH) -I$(REV_INCLUDES) -O0 -o $(EXAMPLE).exe $(EXAMPLE).c -static
clean:
	rm -Rf $(EXAMPLE).exe

#-- EOF
