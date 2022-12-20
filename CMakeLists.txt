# RevCPU Top-Level CMake
#
# Copyright (C) 2017-2022 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "DO NOT BUILD in-tree.")
endif()

cmake_minimum_required(VERSION 3.0)
project(revcpu CXX)

#---------------------------------------------------------------------------
#-- SETUP PATHS - This will be relevant as soon as SST supports sane CMake
#---------------------------------------------------------------------------
# Setup the path to the SST installation directory
#set(SST_INSTALL_DIR "" CACHE PATH "SST installation directory")
execute_process(COMMAND sst-config --prefix 
                OUTPUT_VARIABLE SST_INSTALL_DIR
                OUTPUT_STRIP_TRAILING_WHITESPACE
                )
set(SST_INCLUDE_DIR "${SST_INSTALL_DIR}/include")
if(NOT (EXISTS "${SST_INSTALL_DIR}"))
  message(FATAL_ERROR " SST_INSTALL_DIR (${SST_INSTALL_DIR}) is invalid.")
endif()

include_directories(SST_SRC ${SST_INSTALL_DIR})
# find_package(SST REQUIRED CONFIG) # NOTE: Pipe dream for when SST has coherent CMake support

#------------------------------------------------------------------------
#-- LOAD THE CONFIGURATION
#------------------------------------------------------------------------
# Sanity check the path
find_program(SST sst)
if(NOT SST)
  message(FATAL_ERROR "No sst binary found in path")
endif()

find_program(SST_CONFIG sst-config)
if(NOT SST_CONFIG)
  message(FATAL_ERROR "No sst-config binary found in path")
endif()


#------------------------------------------------------------------------
#-- DERIVE SST ENV 
#------------------------------------------------------------------------
execute_process(COMMAND sst-config --CXX
                OUTPUT_VARIABLE CXX
  )

execute_process(COMMAND sst-config --ELEMENT_CXXFLAGS -fno-stack-protector
  OUTPUT_VARIABLE CXXFLAGS
  )

execute_process(COMMAND sst-config --ELEMENT_LDFLAGS -fno-stack-protector
  OUTPUT_VARIABLE LDFLAGS
  )

#------------------------------------------------------------------------
#-- COMPILER OPTIONS
#------------------------------------------------------------------------
# Require C++14
# TODO: This might not be necessary
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -std=c++14 -Wall ${REVCPU_COMPILER_MACROS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -Wall ${REVCPU_COMPILER_MACROS}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -Wall ${REVCPU_COMPILER_MACROS}")

#------------------------------------------------------------------------
#-- FIND PACKAGES
#------------------------------------------------------------------------
# Find SST packages
find_package(SST REQUIRED CONFIG)
message(STATUS "Found SST execute_process(COMMAND sst --version)")
# message(STATUS "Using SSTConfig.cmake in: ${SST_DIR}")
include_directories(${SST_INCLUDE_DIRS})
add_definitions(${SST_DEFINITIONS})

# Find Doxygen packages
option(BUILD_DOCUMENTATION "Create and install the doxygen-ized API documentation (requires Doxygen)" ${DOXYGEN_FOUND})

if(BUILD_DOCUMENTATION)
  find_package(Doxygen)
  if(NOT DOXYGEN_FOUND)
    message(FATAL_ERROR "Doxygen is required to build the documentation.")
  endif()

  set(doxyfile ${CMAKE_CURRENT_SOURCE_DIR}/doxygen/RevCPU.conf)

  add_custom_target(doc
    COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
    COMMAND ${BASH_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/cleanmanpages.sh ${CMAKE_CURRENT_SOURCE_DIR}/doxygen/man/man3
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM)

  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/doxygen/html DESTINATION share/doc)
  # install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/doxygen/man DESTINATION share/man) # NOTE: No 




endif()
  # find_package(DOXYGEN Doxygen)
# option(BUILD_DOCUMENTATION "Create and install the doxygen-ized API documentation (requires Doxygen)" ${DOXYGEN_FOUND})


#------------------------------------------------------------------------
#-- REVCPU PATHS
#------------------------------------------------------------------------
# RevCPU include path
if(NOT REVCPU_INCLUDE_PATH)
  set(REVCPU_INCLUDE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/include")
  message(STATUS "REVCPU_INCLUDE_PATH set to ${REVCPU_INCLUDE_PATH}")
endif()

# RevCPU instructions path
if(NOT INSTRUCTIONS_PATH)
  set(INSTRUCTIONS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/include/insns")
  message(STATUS "INSTRUCTIONS_PATH set to ${INSTRUCTIONS_PATH}")
endif()


#------------------------------------------------------------------------
#-- TESTING
#------------------------------------------------------------------------
# Enable Testing
option(REV_ENABLE_TESTING "Enable Testing")
enable_testing()


add_subdirectory(src)
add_subdirectory(include)
add_subdirectory(test)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install (CODE "execute_process(COMMAND sst-register revcpu -revcpu_LIBDIR=${CMAKE_INSTALL_PREFIX}/lib)")

# EOF
