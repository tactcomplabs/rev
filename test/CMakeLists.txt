# RevCPU test/CMakeLists.txt
#
# Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

set( RISCV_ENV "$ENV{RISCV}" )
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

# find_program(RVLINUX riscv64-unknown-linux-gnu-gcc)
set (passRegex "Simulation is complete")
#Program execution complete


#------- TESTS  ---------------
message(STATUS "CTest Setup")

# Macro for adding tests
macro(add_old_sst_test test_name test_dir timeout labels)
  add_test(NAME ${test_name} COMMAND RUN_${test_name}.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${test_dir}" )
  set_tests_properties(${test_name}
    PROPERTIES
      ENVIRONMENT "RVCC=${RVCC}"
      TIMEOUT ${timeout}
      PASS_REGULAR_EXPRESSION "${passRegex}"
      LABELS "${labels}"
  )
endmacro()


# Macro to build and configure a test with customizable timeout
# macro(add_rev_test test_name test_dir timeout labels)
#   # Custom target for building the test
#   add_custom_target(build_${test_name}
#     COMMAND ${CMAKE_MAKE_PROGRAM} clean
#     COMMAND ${CMAKE_MAKE_PROGRAM}
#     WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${test_dir}
#     COMMENT "Building ${test_name} test"
#   )
#
#   # Custom target for running the test
#   add_custom_target(run_${test_name}
#     COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_SOURCE_DIR}/${test_dir}
#             $<IF:$<BOOL:$<TARGET_EXISTS:${test_name}.exe>>,
#                ${CMAKE_COMMAND} -E echo "Running ${test_name} test" && sst --add-lib-path=../../build/src/ ./${test_name}.py,
#                ${CMAKE_COMMAND} -E echo "Test ${test_name}: ${test_name}.exe not Found - likely build failed" && false>
#     DEPENDS build_${test_name}
#     COMMENT "Running ${test_name} test"
#   )
#
#   # Add the custom target as a test
#   add_test(NAME ${test_name} COMMAND ${CMAKE_COMMAND} --build . --target run_${test_name})
#
#   # Set test properties
#   set_tests_properties(${test_name}
#     PROPERTIES
#       ENVIRONMENT "RVCC=${RVCC}"
#       TIMEOUT ${timeout}
#       PASS_REGULAR_EXPRESSION "${passRegex}"
#       LABELS "${labels}"
#   )
# endmacro()
# Example usage of the macro
# add_rev_test(TEST_EX5 ex5 30)
# add_rev_test(TEST_EX6 ex6 45)
# Macro to build and configure a test with customizable timeout

macro(add_rev_test test_name test_dir timeout labels)
  # Custom target for building the test
  add_custom_target(build_${test_name}
    COMMAND ${CMAKE_MAKE_PROGRAM} clean
    COMMAND ${CMAKE_MAKE_PROGRAM}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${test_dir}
    COMMENT "Building ${test_name} test"
  )

  # Custom target to run the test with --enableMemH=1
  add_custom_target(run_${test_name}_memh
    COMMAND cd ${CMAKE_SOURCE_DIR}/test/${test_name} && sst --add-lib-path=../../build/src/ ${CMAKE_SOURCE_DIR}/test/rev-basic-config.py -- --program="${test_name}.exe" --numHarts=1 --numCores=1 --enableMemH=1
    DEPENDS build_${test_name}
    COMMENT "Running ${test_name} test with memHierarchy enabled"
  )

  # Custom target to run the test without --enableMemH
  add_custom_target(run_${test_name}_revmem
    COMMAND cd ${CMAKE_SOURCE_DIR}/test/${test_name} && sst --add-lib-path=../../build/src/ ${CMAKE_SOURCE_DIR}/test/rev-basic-config.py -- --program="${test_name}.exe" --numHarts=1 --numCores=1
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
endmacro()

# Use the macro to add tests with specific timeouts
add_rev_test(EX1 ex1 30 "all;rv32")
add_rev_test(EX2 ex2 30 "all;rv64")
add_rev_test(EX3 ex3 30 "all;rv32")
add_rev_test(EX4 ex4 30 "all;rv32")
add_rev_test(EX5 ex5 30 "all;rv64")
add_rev_test(EX6 ex6 45 "all;rv64")
add_rev_test(BIG_LOOP big_loop 100 "all;rv64;benchmark")
add_rev_test(LARGE_BSS large_bss 30 "all;rv64")

# AMO Instructions
# TODO: Add MemH versions of these tests
# TODO: Add dcmp test
# TODO: Add bge_ble test
# TODO: Add tracer
# TODO: Add pan tests
# TODO: Add syscalls
# DONE: Pass exe as model option as well as verbosity
# TODO: Make the pass regex based on the last PC of the main function
add_rev_test(AMO_ADD_C amo/amoadd_c 30 "all;rv64;amo")
add_rev_test(AMO_ADD_CXX amo/amoadd_cxx 30 "all;rv64;amo")
add_rev_test(AMO_SWAP_C amo/amoswap_c 30 "all;rv64;amo")

# blas-required
# add_rev_test(DOT_SINGLE dot_single 30 "all;rv64;blas-required")
# add_rev_test(DOT_DOUBLE dot_double 30 "all;rv64;blas-required")

# dependency check
add_rev_test(DEP_CHECK dep_check 30 "all;rv32;")

# cache tests
add_rev_test(CACHE_1 cache_test1 30 "all;rv32;")
add_rev_test(CACHE_2 cache_test2 30 "all;rv64;")

add_rev_test(STRLEN_C strlen_c 30 "all;rv64")
add_rev_test(STRLEN_CXX strlen_cxx 30 "all;rv64;cxx")
add_rev_test(STRSTR strstr 30 "all;rv64")
add_rev_test(MEMSET memset 30 "all;rv64")
add_rev_test(MEMSET_2 memset_2 30 "all;rv64")
add_rev_test(MANY_CORE many_core 30 "all;rv64")

add_rev_test(DIVW divw 30 "all;rv64")
add_rev_test(DIVW2 divw2 30 "all;rv64")

add_rev_test(TOWERS benchmarks/towers 30 "all;rv64")
add_rev_test(QSORT benchmarks/qsort 30 "all;rv64")
add_rev_test(MEMCPY benchmarks/memcpy 30 "all;rv64;benchmark")
add_rev_test(X0 x0 30 "all;rv64")
add_rev_test(ARGC argc 30 "all;rv64;loader")

add_rev_test(ARGC_MEMH argc_memh 30 "all;rv64;loader;")
add_rev_test(ARGC_SHORT_REVMEM argc_short_revmem 30 "all;rv64;loader;")
add_rev_test(ARGC_SHORT_MEMH argc_short_memh 30 "all;rv64;loader;")
add_rev_test(COPROC_EX coproc_ex 30 "all;rv64;coproc")

# -- PROCESS CTest Config Variables
# if(NOT CTEST_BLAS_REQUIRED_TESTS)
#   set_tests_properties (DOT_SINGLE PROPERTIES DISABLED True)
#   set_tests_properties (DOT_DOUBLE PROPERTIES DISABLED True)
#   message(NOTICE "   * Tests requiring BLAS headers are disabled (ie. -DCTEST_BLAS_REQUIRED_TESTS=OFF)
#     - If you'd like to still run these tests run: `ctest -L blas_required`")
#   message(VERBOSE "   TEST_DOT_SINGLE - Disabled")
#   message(VERBOSE "   TEST_DOT_DOUBLE - Disabled")
# endif()

# if(NOT CTEST_MULTILIB_TESTS)
#   set_tests_properties (TEST_EX1 PROPERTIES DISABLED True)
#   set_tests_properties (TEST_EX3 PROPERTIES DISABLED True)
#   set_tests_properties (TEST_EX4 PROPERTIES DISABLED True)
#   message(NOTICE "   * RV32 tests disabled (ie. -DCTEST_MULTILIB_TESTS=OFF)
#     - If you'd like to still run these tests run: `ctest -L rv32`")
#   message(VERBOSE "TEST_EX1  - Disabled")
#   message(VERBOSE "TEST_EX3  - Disabled")
#   message(VERBOSE "TEST_EX4  - Disabled")
# endif()

add_subdirectory(isa)

# EOF
