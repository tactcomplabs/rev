#
# Makefile
#
# makefile: printf
#
# Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

TESTNAME=printf

SOURCES = $(TESTNAME).c
INCLUDES=
CCOPTS = -O0 -g3

TARG ?= rev

REV_VERBOSE?=2

include $(REVHOME)/test/syscalls/mkinc/common.mk

.PHONY: clean
clean:
	rm -f $(TESTNAME).exe $(TESTNAME).d StatisticOutput.csv

#-- EOF
