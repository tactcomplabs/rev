#
# HOST make include file: host.mk
#
# Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

CC=gcc
CCOPTS += -DHOST_TARGET
CCOPTS += -Wall -Wextra -pedantic-errors -w

.PHONY: run

all: $(TESTNAME).exe
$(TESTNAME).exe: $(TESTNAME).c
	$(CC) $(CCOPTS) -o $(TESTNAME).exe $(TESTNAME).c

all: $(TESTNAME).d
$(TESTNAME).d: $(TESTNAME).exe
	objdump -dC -Mno-aliases --source $< > $@

run: $(TESTNAME).exe
	./$< $(ARGS)

#-- EOF
