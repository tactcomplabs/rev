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

CC=${RVCC}
CXX=${RVCXX}
ARCH=rv64imafdc
ABI=lp64d

CC_OPTS = -O2 -I ../../common/include -I ../../common/syscalls

ifneq (,$(findstring clang,$(RVCC)))
	CC_OPTS += -ffp-model=strict
else
	CC_OPTS += -fsignaling-nans
endif

CC_OPTS += -frounding-math -ftrapping-math -Werror=double-promotion -Werror=conversion -ffp-contract=on

all: fenv_test_TONEAREST.exe fenv_test_UPWARD.exe fenv_test_DOWNWARD.exe fenv_test_TOWARDZERO.exe

fenv_test_TONEAREST.exe: fenv_test.h fenv_test.cc fenv_test_TONEAREST.cc
	$(CXX) $(CC_OPTS) -static -mcmodel=medany -march=$(ARCH) -mabi=$(ABI) fenv_test.cc fenv_test_TONEAREST.cc -lm -o $@

fenv_test_UPWARD.exe: fenv_test.h fenv_test.cc fenv_test_UPWARD.cc
	$(CXX) $(CC_OPTS) -static -mcmodel=medany -march=$(ARCH) -mabi=$(ABI) fenv_test.cc fenv_test_UPWARD.cc -lm -o $@

fenv_test_DOWNWARD.exe: fenv_test.h fenv_test.cc fenv_test_DOWNWARD.cc
	$(CXX) $(CC_OPTS) -static -mcmodel=medany -march=$(ARCH) -mabi=$(ABI) fenv_test.cc fenv_test_DOWNWARD.cc -lm -o $@

fenv_test_TOWARDZERO.exe: fenv_test.h fenv_test.cc fenv_test_TOWARDZERO.cc
	$(CXX) $(CC_OPTS) -static -mcmodel=medany -march=$(ARCH) -mabi=$(ABI) fenv_test.cc fenv_test_TOWARDZERO.cc -lm -o $@

fenv_test_TONEAREST.cc: fenv_gentests.exe
	./fenv_gentests.exe FE_TONEAREST > $@

fenv_test_UPWARD.cc: fenv_gentests.exe
	./fenv_gentests.exe FE_UPWARD > $@

fenv_test_DOWNWARD.cc: fenv_gentests.exe
	./fenv_gentests.exe FE_DOWNWARD > $@

fenv_test_TOWARDZERO.cc: fenv_gentests.exe
	./fenv_gentests.exe FE_TOWARDZERO > $@

# build test generator on local host
fenv_gentests.exe: fenv_test.h fenv_gentests.cc
	g++ $(CC_OPTS) fenv_gentests.cc -o $@

clean:
	rm -Rf *.exe

#-- EOF
