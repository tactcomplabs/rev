#
# Makefile
#
# makefile: dot_double
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

.PHONY: src

EXAMPLE=dot_double
CC=${RVCC}
ARCH=rv64imafd

all: $(EXAMPLE).exe
$(EXAMPLE).exe: $(EXAMPLE).c
	$(CC) -march=$(ARCH) -static -o $(EXAMPLE).exe $(EXAMPLE).c -L$(BLASDIR) -lopenblas
clean:
	rm -Rf $(EXAMPLE).exe

#-- EOF
