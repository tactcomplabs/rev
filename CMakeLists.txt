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

cmake_minimum_required(VERSION 3.19)
project(revcpu CXX)


#---------------------------------------------------------------------------
#-- CMAKE ARGS
#---------------------------------------------------------------------------
# Tests requiring blas headers - Default: OFF
option(CTEST_BLAS_REQUIRED_TESTS "Enable tests which require BLAS headers" OFF)
# Tests requiring 32-bit enabled compiler - Default: ON
option(CTEST_MULTILIB_TESTS "Enable 32-bit tests" ON)



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

set(REV_BASE "${CMAKE_CURRENT_SOURCE_DIR}")
set(ENV{REV_BASE} "${REV_BASE}")
message(STATUS "REV_BASE set to ${REV_BASE}")

#------------------------------------------------------------------------
#-- DERIVE SST ENV
#------------------------------------------------------------------------
execute_process(COMMAND sst-config --CXX
                OUTPUT_VARIABLE CXX
                OUTPUT_STRIP_TRAILING_WHITESPACE
)
message (CXX="${CXX}")
execute_process(COMMAND sst-config --ELEMENT_CXXFLAGS
  OUTPUT_VARIABLE SST_CXXFLAGS
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
set(CXXFLAGS "${SST_CXXFLAGS}  -fno-stack-protector")
message (CXXFLAGS="${CXXFLAGS}")

execute_process(COMMAND sst-config --ELEMENT_LDFLAGS
  OUTPUT_VARIABLE SST_LDFLAGS
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
set(LDFLAGS "${SST_LDFLAGS}  -fno-stack-protector")
message (LDFLAGS="${LDFLAGS}")

#------------------------------------------------------------------------
#-- DISASM
#------------------------------------------------------------------------
option(REV_TRACER "Enable tracer compilation" ON)
message (REV_TRACER="${REV_TRACER}")
if(REV_TRACER)
  option(REV_USE_SPIKE "Link to spike disasm static library" OFF)
  if(REV_USE_SPIKE)
    find_program(SPIKE spike)
    cmake_path(GET SPIKE PARENT_PATH spike_dir)
    find_library(DISASM_LIB disasm HINT "${spike_dir}/../lib")
    include_directories("${spike_dir}/../include")
    link_directories("${spike_dir}/../lib")
    link_libraries("disasm")
    add_compile_definitions("REV_USE_SPIKE")
  endif()
else()
   # requires cmake 3.12
  add_compile_definitions(NO_REV_TRACER)
endif()

#------------------------------------------------------------------------
#-- COMPILER OPTIONS
#------------------------------------------------------------------------
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  set(WERROR_FLAG "")
  set(FP_MODE_FLAG "-ffp-model=strict")
else()
  set(WERROR_FLAG "-Werror")
  set(FP_MODE_FLAG "-frounding-math")
endif()

set(CMAKE_CXX_FLAGS "-std=c++17 ${FP_MODE_FLAG} -O2 -Wall -Wextra ${WERROR_FLAG} -Wvla -Wuninitialized -Wfloat-conversion -Wdouble-promotion -Wno-unused-parameter -Wno-deprecated-declarations ${CMAKE_CXX_FLAGS} -I./ ${LDFLAGS} ${REVCPU_COMPILER_MACROS}")
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

#------------------------------------------------------------------------
#-- CUSTOM COMMANDS
#------------------------------------------------------------------------
add_custom_target(uninstall COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/scripts/uninstall.sh")

# EOF
