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
  RevNIC.cc
  RevOpts.cc
  RevProc.cc
  libRevComponent.cc
  )

add_library(${PROJECT_NAME} SHARED ${RevCPUSrcs})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}../include)
target_include_directories(${PROJECT_NAME} PUBLIC ${SST_INSTALL_DIR}/include)
# target_compile_options(${PROJECT_NAME} 
# include_directories(revcpu SHARED ${CMAKE_CURRENT_SOURCE_DIR}../include/insns)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}../include/insns/)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR})

