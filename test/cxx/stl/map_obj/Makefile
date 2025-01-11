#
# Makefile
#
# makefile: map_obj
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

.PHONY: src

EXAMPLE=map_obj
CXX=${RVCXX}
ARCH=rv64gc

all: $(EXAMPLE).exe
$(EXAMPLE).exe: $(EXAMPLE).cc
	$(CXX) -march=$(ARCH) -O0 -I../../../../common/include -I../../../../common/syscalls -I../../../include -o $(EXAMPLE).exe $(EXAMPLE).cc -static
clean:
	rm -Rf $(EXAMPLE).exe

#-- EOF
