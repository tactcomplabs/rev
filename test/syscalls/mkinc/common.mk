#
# Makefile
#
# makefile: common
#
# Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

ifeq ($(TARG),host)
include $(REVHOME)/test/syscalls/mkinc/host.mk
else ifeq ($(TARG),spike)
include $(REVHOME)/test/syscalls/mkinc/spike.mk
else ifeq ($(TARG),rev)
include $(REVHOME)/test/syscalls/mkinc/rev.mk
else
$(error unknown TARG $(TARG))
endif

.PHONY: clean
clean:
	rm -f $(TESTNAME).exe $(TESTNAME).d StatisticOutput.csv $(TESTNAME).spike

.PHONY: help
help:
	@echo "# REV defaults"
	@echo "make clean & make"
	@echo "make clean run"
	@echo "# Alternate targets"
	@echo "make TARG=host clean run"
	@echo "make TARG=spike clean run"

#-- EOF
