# Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#

#include_directories(${XBGAS_PATH})

file(GLOB TEST_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.c)

if(BUILD_ASM_TESTING)
project(asm_tst C)
foreach(testSrc ${TEST_SRCS})
  # Extract the file name
  get_filename_component(testName ${testSrc} NAME_WE)

  # Add compile target
  add_executable(${testName} ${testSrc})

  # Override the compiler
  set(CMAKE_C_COMPILER "${RISCV_ENV}/bin/riscv64-unknown-elf-gcc")
  set(CMAKE_ASM_COMPILER "${RISCV_ENV}/bin/riscv64-unknown-elf-gcc")
  set(CMAKE_C_LINK_EXECUTABLE "${RISCV_ENV}/bin/riscv64-unknown-elf-gcc")
  set(CMAKE_C_FLAGS "-O0 -march=rv64imafdc -static")
  set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
  set(CMAKE_OSX_DEPLOYMENT_TARGET "")

  #set_target_properties(${testName} PROPERTIES LINKER_LANGUAGE C)

  set_target_properties(${testName} PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

  # Add the tests for execution
  add_test(NAME ${testName}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND sst --model-options=${CMAKE_CURRENT_SOURCE_DIR}/${testName} ./rev-isa-test.py)
  set_tests_properties( ${testName} 
	PROPERTIES 
		PASS_REGULAR_EXPRESSION "")
endforeach(testSrc)
endif()
