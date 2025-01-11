#
# Makefile
#
# makefile: strlen_c
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

.PHONY: src

EXAMPLE=amoadd_cxx
CC=${RVCC}
#ARCH=rv64g
ARCH=rv64imafdc

all: $(EXAMPLE).exe
$(EXAMPLE).exe: $(EXAMPLE).cc
	$(CC) -march=$(ARCH) -o $(EXAMPLE).exe $(EXAMPLE).cc -static
clean:
	rm -Rf $(EXAMPLE).exe

#-- EOF
