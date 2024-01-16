# RevCPU src CMakeLists.txt
# Copyright (C) 2017-2022 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

set(RevCPUSrcs
  RevCPU.cc
  RevExt.cc
  RevFeature.cc
  RevLoader.cc
  RevMem.cc
  RevMemCtrl.cc
  RevNIC.cc
  RevOpts.cc
  RevProc.cc
  RevTracer.cc
  librevcpu.cc
  RevPrefetcher.cc
  RevCoProc.cc
  RevRegFile.cc
  RevThread.cc
  )

set(RevPyInc
  "${REVCPU_INCLUDE_PATH}/pyrevcpu.inc"
  )

set(ODSHELL "${CMAKE_CURRENT_SOURCE_DIR}/../scripts/build_python_inc.sh")

add_custom_command(OUTPUT ${RevPyInc}
  COMMAND ${ODSHELL} ${CMAKE_CURRENT_SOURCE_DIR}/pyrevcpu.py ${RevPyInc}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Building Rev python include: ${RevPyInc}"
  VERBATIM
  )

add_custom_target(BuildRevPyInc ALL
  DEPENDS ${RevPyInc}
  VERBATIM
  )

add_subdirectory(../common common)
add_library(revcpu SHARED ${RevCPUSrcs} ${RevPyInc})
target_include_directories(revcpu PRIVATE ${REVCPU_INCLUDE_PATH} PUBLIC ${SST_INSTALL_DIR}/include)

install(TARGETS revcpu DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})
install(CODE "execute_process(COMMAND sst-register revcpu revcpu_LIBDIR=${CMAKE_CURRENT_SOURCE_DIR})")
if( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  install(CODE "execute_process(COMMAND_ERROR_IS_FATAL ANY COMMAND_ECHO STDERR COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/test_undefined_symbols.sh ${CMAKE_CURRENT_SOURCE_DIR}/librevcpu.dylib)")
else()
  install(CODE "execute_process(COMMAND_ERROR_IS_FATAL ANY COMMAND_ECHO STDERR COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/test_undefined_symbols.sh ${CMAKE_CURRENT_SOURCE_DIR}/librevcpu.so)")
endif()
# EOF
