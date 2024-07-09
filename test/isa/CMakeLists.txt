# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#

#include_directories(${XBGAS_PATH})

file(GLOB TEST_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.c)

#project(asm_tst C)

#set(CMAKE_C_COMPILER "${RISCV_ENV}/bin/riscv64-unknown-elf-gcc")
#set(CMAKE_ASM_COMPILER "${RISCV_ENV}/bin/riscv64-unknown-elf-gcc")
#set(CMAKE_C_FLAGS "-O0 -march=rv64imafdc -static")
#set(CMAKE_OSX_DEPLOYMENT_TARGET "")

set (passRegex "Simulation is complete")

add_test(
  NAME BUILD_CLEAN
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMAND make clean
)

foreach(testSrc ${TEST_SRCS})
  # Extract the file name
  get_filename_component(testName ${testSrc} NAME_WE)

  #COMMAND sst --model-options=${CMAKE_CURRENT_SOURCE_DIR}/${testName} ./rev-isa-test.py)
  # Add the tests for execution
  add_test(NAME ${testName}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND ./run_asm_test.sh)
  set_tests_properties( ${testName}
    PROPERTIES
    ENVIRONMENT "RVASM=${testName}; RVCC=${RVCC}"
    TIMEOUT 30
    LABELS "rv64;isa"
    PASS_REGULAR_EXPRESSION "${passRegex}")
  if(TEST_LEVEL LESS 2)
    set_tests_properties(${testName} PROPERTIES DISABLED TRUE)
  endif()
endforeach(testSrc)
