#
# Makefile
#
# makefile: lwsp
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

.PHONY: src
EXAMPLE=lwsp
CXX=${RVCXX}
ARCH=rv64gc
INCLUDE=-I../../common/syscalls -I../include

all: $(EXAMPLE).exe
$(EXAMPLE).exe: $(EXAMPLE).cc
	$(CXX) -march=$(ARCH) -static $(INCLUDE) -o $(EXAMPLE).exe $(EXAMPLE).cc
clean:
	rm -Rf $(EXAMPLE).exe

#-- EOF
