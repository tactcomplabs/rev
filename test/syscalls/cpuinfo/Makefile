#
# Makefile
#
# makefile: cpuinfo
#
## See LICENSE in the top level directory for licensing details
#

.PHONY: src

EXAMPLE=cpuinfo
#CC=riscv64-unknown-elf-gcc
CC="${RVCC}"
#ARCH=rv64g
ARCH=rv64imafdc

all: $(EXAMPLE).exe
$(EXAMPLE).exe: $(EXAMPLE).c
	$(CC) -march=$(ARCH) -O0 -o $(EXAMPLE).exe $(EXAMPLE).c -static
clean:
	rm -Rf $(EXAMPLE).exe

#-- EOF
