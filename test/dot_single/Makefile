#
# Makefile
#
# makefile: dot_single
#
# Copyright (C) 2017-2020 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

.PHONY: src

EXAMPLE=dot_single
CC=${RVCC}
ARCH=rv64imafd

all: $(EXAMPLE).exe
$(EXAMPLE).exe: $(EXAMPLE).c
	$(CC) -march=$(ARCH) -static -o $(EXAMPLE).exe $(EXAMPLE).c -L$(BLASDIR) -lopenblas
clean:
	rm -Rf $(EXAMPLE).exe

#-- EOF
