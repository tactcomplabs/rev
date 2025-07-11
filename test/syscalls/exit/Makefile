#
# Makefile
#
# makefile: exit
#
# Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

TESTNAME=exit

SOURCES = $(TESTNAME).c
INCLUDES=
CCOPTS = -O0 -g3

TARG ?= rev

REV_MACHINE = "CORES:RV64IMAFDC"
REV_VERBOSE?=0

ifndef REVHOME
$(error REVHOME is not defined)
else
$(info REVHOME is $(REVHOME))
endif

include $(REVHOME)/test/syscalls/mkinc/common.mk

.PHONY: clean
clean:
	rm -f $(TESTNAME).exe $(TESTNAME).d StatisticOutput.csv

#-- EOF
