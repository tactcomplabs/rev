#
# Makefile
#
# makefile: zfa
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

EXAMPLE=zfa
CXX="${RVCXX}"
ARCH=rv64gc_zfa
ABI=lp64d

CXXFLAGS=-I ../../common/syscalls -O2

ifneq (,$(findstring clang,$(CXX)))
	CXX_FLAGS += -ffp-model=strict
else
	CXX_FLAGS += -fsignaling-nans
endif

CXX_FLAGS += -frounding-math -ftrapping-math -Werror=double-promotion -Werror=conversion -ffp-contract=on

all: $(EXAMPLE).exe
$(EXAMPLE).exe: $(EXAMPLE).cc
	$(CXX) $(CXXFLAGS) -march=$(ARCH) -mabi=$(ABI) -static -o $(EXAMPLE).exe $(EXAMPLE).cc
clean:
	rm -Rf $(EXAMPLE).exe

#-- EOF
