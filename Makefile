#
# Makefile
#
# Top-level Rev makefile
#
# Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

.PHONY: src

all: deprecation
deprecation:
	@echo "------------------------------------------"
	@echo "!!! GNU MAKEFILE SUPPORT IS DEPRECATED !!!"
	@echo "!!! Please switch to CMake             !!!"
	@echo "------------------------------------------"
doc:
	doxygen ./doxygen/Rev.conf
debug:
	$(MAKE) debug -C src
src:
	$(MAKE) -C src
install:
	$(MAKE) -C src install
test:
	$(MAKE) -C test
clean:
	$(MAKE) -C src clean && rm -Rf ./doxygen/html ./doxygen/latex ./doxygen/man ./doxygen/rtf ./doxygen/xml

#-- EOF
