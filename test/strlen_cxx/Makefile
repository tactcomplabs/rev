#
# Makefile
#
# makefile: strlen_c
#
# Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

.PHONY: src

EXAMPLE=strlen_cxx
CXX="${RVCXX}"
ARCH?=rv64imafd
ABI?=lp64d

all: $(EXAMPLE).exe
$(EXAMPLE).exe: $(EXAMPLE).cc
	$(CXX) -march=$(ARCH) -mabi=$(ABI) -o $(EXAMPLE).exe $(EXAMPLE).cc -static
clean:
	rm -Rf $(EXAMPLE).exe

#-- EOF
