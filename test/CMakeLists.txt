# RevCPU test/CMakeLists.txt
#
# Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

set( RISCV_ENV "$ENV{RISCV}")
if(RISCV_ENV)
  message(STATUS "RISCV environment set to ${RISCV_ENV}")
else()
  message(FATAL_ERROR "RISCV environment variable is not set.")
endif()

set(RISCV_BIN_PATH "$ENV{RISCV}/bin")

file(GLOB RVOBJDUMP_FILES "${RISCV_BIN_PATH}/*objdump")
list(GET RVOBJDUMP_FILES 0 RVOBJDUMP)
file(GLOB RVCC_FILES "${RISCV_BIN_PATH}/*cc")
list(GET RVCC_FILES 0 RVCC)
file(GLOB RVCXX_FILES "${RISCV_BIN_PATH}/*++")
list(GET RVCXX_FILES 0 RVCXX)

message(STATUS "Found RVOBJDUMP: ${RVOBJDUMP}")
message(STATUS "Found RVCC: ${RVCC}")
message(STATUS "Found RVCXX: ${RVCXX}")


# Check for RISCV Compiler
if(NOT RVCC)
  set(RVCC "$ENV{RVCC}")
endif()
if(RVCC)
  message(STATUS "RVCC set to ${RVCC}")
else()
  message(FATAL_ERROR "RVCC environment varible is not set.")
endif()

if(NOT RVCXX)
  set(RVCXX "$ENV{RVCXX}")
endif()
if(RVCXX)
  message(STATUS "RVCXX set to ${RVCXX}")
else()
  message(FATAL_ERROR "RVCXX environment varible is not set.")
endif()

set (passRegex "Simulation is complete")


#------- TESTS  ---------------
message(STATUS "CTest Setup")

# Macro to build and configure a test with customizable timeout
macro(add_rev_test test_name test_dir timeout labels)
  string(TOLOWER ${test_dir} test_dir_lower)
  string(TOLOWER ${test_name} test_name_lower)
  # Default script name is empty
  set(optional_script "")

  # Parse arguments to find if a script name is provided
  cmake_parse_arguments(ARG "" "SCRIPT" "" ${ARGN})
  if(ARG_SCRIPT)
    set(optional_script ${ARG_SCRIPT})
  endif()

  # Build the test (ie. run make)
  add_custom_target(build_${test_name_lower}
    COMMAND ${CMAKE_MAKE_PROGRAM} clean
    COMMAND ${CMAKE_MAKE_PROGRAM}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${test_dir_lower}
    COMMENT "Building ${test_name_lower} test"
    # Disassemble the test executable
    COMMAND ${RVOBJDUMP} -dC -Mno-aliases --source *.exe > ${CMAKE_CURRENT_SOURCE_DIR}/${test_dir_lower}/${test_name_lower}.asm
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${test_dir_lower}
    COMMENT "Building and disassembling ${test_name_lower} test"
  )

  # If no optional script is specified, run with basic config and add revmem/memH tests
  if(NOT optional_script)

    # Check if 'rv32' label is present
    string(FIND "${labels}" "rv32" label_index)
    set(machine_arg "")
    if(NOT ${label_index} EQUAL -1)
      set(machine_arg "--machine \"[0:RV32GC]\"")
    endif()

    # Check if 'multithread' label is present
    string(FIND "${labels}" "multithreading" label_index)
    set(machine_arg "")
    if(NOT ${label_index} EQUAL -1)
      set(machine_arg "--numHarts=\"4\"")
    endif()

    # If no optional script is specified, run with basic config and add revmem/memH tests
    if(NOT optional_script)
      # Add memH test
      add_custom_target(run_${test_name_lower}_memh
    COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR}/${test_name_lower} && sst --add-lib-path=../../build/src/ ${CMAKE_SOURCE_DIR}/test/rev-model-options-config.py -- --program="${test_name_lower}.exe" ${machine_arg} --numCores=1 --enableMemH=1
    DEPENDS build_${test_name_lower}
    COMMENT "Running ${test_name_lower} test with memHierarchy enabled"
  )

      # Add revmem test
      add_custom_target(run_${test_name_lower}_revmem
    COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR}/${test_name_lower} && sst --add-lib-path=../../build/src/ ${CMAKE_SOURCE_DIR}/test/rev-model-options-config.py -- --program="${test_name_lower}.exe" ${machine_arg} --numCores=1
    DEPENDS build_${test_name_lower}
    COMMENT "Running ${test_name_lower} test with revmem"
  )

    endif()

    # Add the custom targets as tests
    add_test(NAME ${test_name_lower}_memh COMMAND ${CMAKE_COMMAND} --build . --target run_${test_name_lower}_memh)
    add_test(NAME ${test_name_lower}_revmem COMMAND ${CMAKE_COMMAND} --build . --target run_${test_name_lower}_revmem)

    # Set test properties for both tests
    set_tests_properties(${test_name_lower}_memh ${test_name_lower}_revmem
      PROPERTIES
        ENVIRONMENT "RVCC=${RVCC}"
        TIMEOUT ${timeout}
        PASS_REGULAR_EXPRESSION "${passRegex}"
        LABELS "${labels}"
    )
  else()
    # Custom target for running the specified script
    add_custom_target(run_${test_name_lower}_script
      COMMAND ${CMAKE_COMMAND} -E echo "Executing script ${optional_script}"
      COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR}/${test_name_lower} && bash ${optional_script}
      DEPENDS build_${test_name_lower}
      COMMENT "Running script for ${test_name_lower} test"
    )

    # Add the custom target as a test
    add_test(NAME ${test_name_lower}_script COMMAND ${CMAKE_COMMAND} --build . --target run_${test_name_lower}_script)

    # Set test properties for the script test
    set_tests_properties(${test_name_lower}_script
      PROPERTIES
        ENVIRONMENT "RVCC=${RVCC}"
        TIMEOUT ${timeout}
        PASS_REGULAR_EXPRESSION "${passRegex}"
        LABELS "${labels}"
    )
  endif()
endmacro()

add_rev_test(EX1 ex1 30 "all;rv32")
add_rev_test(EX2 ex2 30 "all;rv64")
add_rev_test(EX3 ex3 30 "all;rv32")
add_rev_test(EX4 ex4 30 "all;rv32")
add_rev_test(EX5 ex5 30 "all;rv64")
add_rev_test(EX6 ex6 45 "all;rv64")
# BigLoop uses a script solely to not run with memH
add_rev_test(BIG_LOOP big_loop 100 "all;rv64;benchmark" SCRIPT "run_big_loop.sh")
add_rev_test(LARGE_BSS large_bss 30 "all;rv64")
add_rev_test(DEP_CHECK dep_check 30 "all;rv32")
add_rev_test(CACHE_1 cache_1 30 "all;rv32" SCRIPT "run_cache_1.sh")
add_rev_test(CACHE_2 cache_2 30 "all;rv64" SCRIPT "run_cache_2.sh")
add_rev_test(STRLEN_C strlen_c 30 "all;rv64")
add_rev_test(STRLEN_CXX strlen_cxx 30 "all;rv64;cxx")
add_rev_test(STRSTR strstr 30 "all;rv64")
add_rev_test(MEMSET memset 30 "all;rv64")
add_rev_test(MEMSET_2 memset_2 30 "all;rv64")
add_rev_test(MANY_CORE many_core 30 "all;rv64" SCRIPT "run_many_core.sh")
add_rev_test(DIVW divw 30 "all;rv64")
add_rev_test(DIVW2 divw2 30 "all;rv64")
add_rev_test(X0 x0 30 "all;rv64")
add_rev_test(ARGC argc 30 "all;rv64;loader;" SCRIPT "run_argc.sh")
add_rev_test(ARGC_SHORT argc 30 "all;rv64;loader;" SCRIPT "run_argc_short.sh")
add_rev_test(COPROC_EX coproc_ex 30 "all;rv64;coproc" SCRIPT "run_coproc_ex.sh")
# add_rev_test(TRACER tracer 30 "all;rv64;tracer")
# add_rev_test(PAN_TEST1 pan_test1 30 "all;rv64;pan")
# add_rev_test(PAN_TEST2 pan_test2 30 "all;rv64;pan")
# add_rev_test(DOT_SINGLE dot_single 30 "all;rv64;blas-required")
# add_rev_test(DOT_DOUBLE dot_double 30 "all;rv64;blas-required")
# add_rev_test(BGE_BLE bge_ble 30 "all;rv64")

add_subdirectory(isa)
add_subdirectory(amo)
add_subdirectory(benchmarks)
add_subdirectory(syscalls)
add_subdirectory(threading)

# EOF
