# RevCPU src CMakeLists.txt
# Copyright (C) 2017-2022 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

set(RevCPUSrcs
  PanExec.cc
  PanNet.cc
  RevCPU.cc
  RevExt.cc
  RevFeature.cc
  RevLoader.cc
  RevMem.cc
  RevMemCtrl.cc
  RevNIC.cc
  RevOpts.cc
  RevProc.cc
  librevcpu.cc
  RevPrefetcher.cc
  )

add_library(revcpu SHARED ${RevCPUSrcs})
target_include_directories(revcpu PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}../include)
target_include_directories(revcpu PUBLIC ${SST_INSTALL_DIR}/include)

install(TARGETS revcpu DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/src)
install (CODE "execute_process(COMMAND sst-register revcpu -revcpu_LIBDIR=${CMAKE_CURRENT_SOURCE_DIR}/src)")
