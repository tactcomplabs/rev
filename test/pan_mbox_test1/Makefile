#
# Makefile
#
# makefile: pan_test
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

.PHONY: src

EXAMPLE=pan_test
SPIN=pan_spin
CC=${RVCC}
ARCH=rv64g

all: $(EXAMPLE).exe $(SPIN).exe
$(SPIN).exe: $(SPIN).c
	$(CC) -march=$(ARCH) -o $(SPIN).exe $(SPIN).c
$(EXAMPLE).exe: $(EXAMPLE).c
	$(CC) -march=$(ARCH) -o $(EXAMPLE).exe $(EXAMPLE).c
clean:
	rm -Rf $(EXAMPLE).exe $(SPIN).exe

#-- EOF
