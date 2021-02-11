#
# Makefile
#
# Top-level Rev makefile
#
# Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

.PHONY: src

all: src
doc:
	doxygen ./doxygen/Rev.conf
src:
	$(MAKE) -C src
install:
	$(MAKE) -C src install
test:
	$(MAKE) -C test
clean:
	$(MAKE) -C src clean && rm -Rf ./doxygen/html ./doxygen/latex ./doxygen/man ./doxygen/rtf ./doxygen/xml

#-- EOF
