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

CC_OPTS = -ftrapping-math -fno-signaling-nans -fno-associative-math -fno-tree-loop-distribute-patterns -Werror=double-promotion -Werror=conversion


ifeq "$(RVCC)" "riscv64-unknown-elf-gcc"
	CC_OPTS += -frounding-math
else
	CC_OPTS += -ffp-model=strict
endif

all: fenv.exe

fenv.exe: fenv.c
	$(CC) $(CC_OPTS) -O2 -static -mcmodel=medany -march=$(ARCH) -mabi=$(ABI) -o fenv.exe fenv.c

clean:
	rm -Rf *.exe

#-- EOF
