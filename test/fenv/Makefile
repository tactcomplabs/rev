#
# Makefile
#
# makefile: fenv_test
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

CXX=${RVCXX}
ARCH=rv64imafdc
ABI=lp64d

CXX_OPTS = -std=gnu++1z -I ../../common/include -I ../../common/syscalls

ifneq (,$(findstring clang,$(CXX)))
	CXX_OPTS += -ffp-model=strict
else
	CXX_OPTS += -fsignaling-nans
endif

CXX_OPTS += -frounding-math -ftrapping-math -Werror=double-promotion -Werror=conversion -ffp-contract=on

all: TONEAREST UPWARD DOWNWARD TOWARDZERO

TONEAREST UPWARD DOWNWARD TOWARDZERO: fenv_test.makefile
	make -f fenv_test.makefile CXX="$(CXX)" CXX_OPTS="$(CXX_OPTS)" ARCH="$(ARCH)" ABI="$(ABI)" $@

fenv_test.makefile: fenv_gentests.exe
	printf '%%.exe: %%.cc\n\t$$(CXX) $$(CXX_OPTS) -static -mcmodel=medany -march=$$(ARCH) -mabi=$$(ABI) -lm $$< -o $$@\n' > fenv_test.makefile
	./fenv_gentests.exe TONEAREST  >> fenv_test.makefile
	./fenv_gentests.exe UPWARD     >> fenv_test.makefile
	./fenv_gentests.exe DOWNWARD   >> fenv_test.makefile
	./fenv_gentests.exe TOWARDZERO >> fenv_test.makefile

# build test generator on local host
fenv_gentests.exe: fenv_test.h fenv_gentests.cc
	g++ $(CXX_OPTS) fenv_gentests.cc -o $@

clean:
	rm -Rf *.exe *.csv fenv_test.makefile TONEAREST_*.cc UPWARD_*.cc DOWNWARD_*.cc TOWARDZERO_*.cc

.PHONY: all clean TONEAREST UPWARD DOWNWARD TOWARDZERO

#-- EOF
