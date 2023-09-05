#
# Makefile
#
# makefile: ex2
#
# Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

.PHONY: src

EXAMPLE=argc
CC=${RVCC}
#ARCH=rv64g
ARCH=rv64imafd

all: $(EXAMPLE).exe
$(EXAMPLE).exe: $(EXAMPLE).c
	$(CC) -march=$(ARCH) -o $(EXAMPLE).exe $(EXAMPLE).c -static
clean:
	rm -Rf $(EXAMPLE).exe

#-- EOF
