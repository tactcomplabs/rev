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

CC_OPTS = -std=gnu++1z -O2 -I ../../common/include -I ../../common/syscalls

ifneq (,$(findstring clang,$(CXX)))
	CC_OPTS += -ffp-model=strict
else
	CC_OPTS += -fsignaling-nans
endif

CC_OPTS += -frounding-math -ftrapping-math -Werror=double-promotion -Werror=conversion -ffp-contract=on

all: executables

fenv_test.makefile: fenv_gentests.exe
	printf '%%.exe: %%.cc\n\t$$(CXX) $$(CC_OPTS) -static -mcmodel=medany -march=$$(ARCH) -mabi=$$(ABI) -lm $$< -o $$@\nEXECUTABLES=' > fenv_test.makefile
	./fenv_gentests.exe FE_TONEAREST  >> fenv_test.makefile
	./fenv_gentests.exe FE_UPWARD     >> fenv_test.makefile
	./fenv_gentests.exe FE_DOWNWARD   >> fenv_test.makefile
	./fenv_gentests.exe FE_TOWARDZERO >> fenv_test.makefile
	printf '\nall: $$(EXECUTABLES)\n\t$$(foreach exe,$$(EXECUTABLES),./run_fenv_test.sh $$(exe) &&) true' >> fenv_test.makefile

executables: fenv_test.makefile
	$(MAKE) -j8 -f fenv_test.makefile all CXX="$(CXX)" CC_OPTS="$(CC_OPTS)" ARCH="$(ARCH)" ABI="$(ABI)"

# build test generator on local host
fenv_gentests.exe: fenv_test.h fenv_gentests.cc
	g++ $(CC_OPTS) fenv_gentests.cc -o $@

clean:
	rm -Rf *.exe fenv_test.makefile FE_TONEAREST_*.cc FE_UPWARD_*.cc FE_DOWNWARD_*.cc FE_TOWARDZERO_*.cc

.PHONY: all clean executables

#-- EOF
