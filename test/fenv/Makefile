#
# Makefile
#
# makefile: ex1
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

CC=${RVCC}
ARCH=rv64imafdc
ABI=lp64d

CC_OPTS = -xc++ -O2

ifneq (,$(findstring clang,$(RVCC)))
	CC_OPTS += -ffp-model=strict
else
	CC_OPTS += -fsignaling-nans
endif

CC_OPTS += -frounding-math -ftrapping-math -Werror=double-promotion -Werror=conversion -ffp-contract=on

all: fenv_test.exe

fenv_test.exe: fenv_test.cc
	$(CC) $(CC_OPTS) -static -mcmodel=medany -march=$(ARCH) -mabi=$(ABI) fenv_test.cc -lm -o $@

fenv_test.cc: fenv_gentests.exe
	./fenv_gentests.exe FE_TONEAREST > $@

# build test generator on local host
fenv_gentests.exe: fenv_test.h fenv_gentests.cc
	g++ $(CC_OPTS) fenv_gentests.cc -o $@

clean:
	rm -Rf *.exe

#-- EOF
