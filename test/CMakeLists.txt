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
if(RVCC)
  message(STATUS "RVCXX set to ${RVCXX}")
else()
  message(FATAL_ERROR "RVCXX environment varible is not set.")
endif()

set (passRegex "Simulation is complete")


#------- TESTS  ---------------
message(STATUS "CTest Setup")

# Macro to build and configure a test with customizable timeout
macro(add_rev_test test_name test_dir timeout labels)
  # Default script name is empty
  set(optional_script "")

  # Parse arguments to find if a script name is provided
  cmake_parse_arguments(ARG "" "SCRIPT" "" ${ARGN})
  if(ARG_SCRIPT)
    set(optional_script ${ARG_SCRIPT})
  endif()

  # Build the test (ie. run make)
  add_custom_target(build_${test_name}
    COMMAND ${CMAKE_MAKE_PROGRAM} clean
    COMMAND ${CMAKE_MAKE_PROGRAM}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${test_dir}
    COMMENT "Building ${test_name} test"
  )

  # If no optional script is specified, run with basic config and add revmem/memH tests
  if(NOT optional_script)
    # Add memH test
    add_custom_target(run_${test_name}_memh
      COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR}/${test_name} && sst --add-lib-path=../../build/src/ ${CMAKE_SOURCE_DIR}/test/rev-model-options-config.py -- --program="${test_name}.exe" --numHarts=1 --numCores=1 --enableMemH=1
      DEPENDS build_${test_name}
      COMMENT "Running ${test_name} test with memHierarchy enabled"
    )

    # Add revmem test
    add_custom_target(run_${test_name}_revmem
      COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR}/${test_name} && sst --add-lib-path=../../build/src/ ${CMAKE_SOURCE_DIR}/test/rev-model-options-config.py -- --program="${test_name}.exe" --numHarts=1 --numCores=1
      DEPENDS build_${test_name}
      COMMENT "Running ${test_name} test with revmem"
    )

    # Add the custom targets as tests
    add_test(NAME ${test_name}_memh COMMAND ${CMAKE_COMMAND} --build . --target run_${test_name}_memh)
    add_test(NAME ${test_name}_revmem COMMAND ${CMAKE_COMMAND} --build . --target run_${test_name}_revmem)

    # Set test properties for both tests
    set_tests_properties(${test_name}_memh ${test_name}_revmem
      PROPERTIES
        ENVIRONMENT "RVCC=${RVCC}"
        TIMEOUT ${timeout}
        PASS_REGULAR_EXPRESSION "${passRegex}"
        LABELS "${labels}"
    )
  else()
    # Custom target for running the specified script
    add_custom_target(run_${test_name}_script
      COMMAND ${CMAKE_COMMAND} -E echo "Executing script ${optional_script}"
      COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR}/${test_name} && bash ${optional_script}
      DEPENDS build_${test_name}
      COMMENT "Running script for ${test_name} test"
    )

    # Add the custom target as a test
    add_test(NAME ${test_name}_script COMMAND ${CMAKE_COMMAND} --build . --target run_${test_name}_script)

    # Set test properties for the script test
    set_tests_properties(${test_name}_script
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
add_rev_test(BIG_LOOP big_loop 100 "all;rv64;benchmark")
add_rev_test(LARGE_BSS large_bss 30 "all;rv64")
add_rev_test(DEP_CHECK dep_check 30 "all;rv32;")
add_rev_test(CACHE_1 cache_1 30 "all;rv32;" SCRIPT "run_cache_1.sh")
add_rev_test(CACHE_2 cache_2 30 "all;rv64;" SCRIPT "run_cache_2.sh")
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
