#
# Makefile
#
# makefile: open
#
# Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

TESTNAME=open

SOURCES = $(TESTNAME).c
INCLUDES=
CCOPTS = -O0 -g3

TARG ?= rev
REV_MACHINE = "[CORES:RV64IMAFDC]"

include $(REVHOME)/test/syscalls/mkinc/common.mk

.PHONY: clean
clean:
	rm -f $(TESTNAME).exe $(TESTNAME).d StatisticOutput.csv data.bin data.csv *.tmp

#-- EOF
