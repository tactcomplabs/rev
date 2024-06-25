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

buildtests: fenv_test.makefile
	make -j4 -f fenv_test.makefile CXX="$(CXX)" CXX_OPTS="$(CXX_OPTS)" ARCH="$(ARCH)" ABI="$(ABI)" TONEAREST UPWARD DOWNWARD TOWARDZERO

runtests: fenv_test.makefile
	make -j4 -f fenv_test.makefile CXX="$(CXX)" CXX_OPTS="$(CXX_OPTS)" ARCH="$(ARCH)" ABI="$(ABI)" RUN_TONEAREST RUN_UPWARD RUN_DOWNWARD RUN_TOWARDZERO
	echo "Simulation is complete"

fenv_test.makefile: fenv_gentests
	printf '%%.exe: %%.cc\n\t$$(CXX) $$(CXX_OPTS) -static -mcmodel=medany -march=$$(ARCH) -mabi=$$(ABI) -lm $$< -o $$@\n' > fenv_test.makefile
	./fenv_gentests TONEAREST  >> fenv_test.makefile
	./fenv_gentests UPWARD     >> fenv_test.makefile
	./fenv_gentests DOWNWARD   >> fenv_test.makefile
	./fenv_gentests TOWARDZERO >> fenv_test.makefile

# build test generator on local host
fenv_gentests: fenv_test.h fenv_gentests.cc
	g++ $(CXX_OPTS) fenv_gentests.cc -o $@

clean:
	rm -Rf *.asm *.exe *.csv fenv_test.makefile TONEAREST_*.cc UPWARD_*.cc DOWNWARD_*.cc TOWARDZERO_*.cc fenv_gentests

.PHONY: buildtests runtests clean TONEAREST UPWARD DOWNWARD TOWARDZERO

#-- EOF
