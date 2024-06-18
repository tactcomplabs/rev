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
CC=${RVCC}
ARCH=rv64gc
INCLUDE=-I../../common/syscalls -I../include

all: $(EXAMPLE).exe
$(EXAMPLE).exe: $(EXAMPLE).cpp
	$(CC) -march=$(ARCH) -static $(INCLUDE) -o $(EXAMPLE).exe $(EXAMPLE).cpp
clean:
	rm -Rf $(EXAMPLE).exe

#-- EOF
