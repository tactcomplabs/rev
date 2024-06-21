#
# Makefile
#
# makefile: csr
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

EXAMPLE=csr
CXX="${RVCXX}"
ARCH=rv64g
ABI=lp64d

CFLAGS=-I ../../common/syscalls -O1

all: $(EXAMPLE).exe
$(EXAMPLE).exe: $(EXAMPLE).cc
	$(CXX) $(CFLAGS) -march=$(ARCH) -mabi=$(ABI) -static -o $(EXAMPLE).exe $(EXAMPLE).cc
clean:
	rm -Rf $(EXAMPLE).exe

#-- EOF
