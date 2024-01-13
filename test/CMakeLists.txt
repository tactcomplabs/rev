# RevCPU test/CMakeLists.txt
#
# Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

set(RISCV_ENV "$ENV{RISCV}")
if(RISCV_ENV)
  message(STATUS "RISCV environment set to ${RISCV_ENV}")
else()
  message(FATAL_ERROR "RISCV environment variable is not set.")
endif()

set(RISCV_BIN_PATH "$ENV{RISCV}/bin")

# Check for RISCV C Compiler
set(RVCC "$ENV{RVCC}")
if(NOT RVCC)
  file(GLOB RVCC_FILES "${RISCV_BIN_PATH}/*gcc" "${RISCV_BIN_PATH}/*clang")
  list(GET RVCC_FILES 0 RVCC)
endif()
if(RVCC)
  message(STATUS "RVCC set to ${RVCC}")
else()
  message(FATAL_ERROR "RVCC environment varible is not set.")
endif()

set(RVCXX "$ENV{RVCXX}")
if(NOT RVCXX)
  file(GLOB RVCXX_FILES "${RISCV_BIN_PATH}/*++")
  list(GET RVCXX_FILES 0 RVCXX)
endif()
if(RVCXX)
  message(STATUS "RVCXX set to ${RVCXX}")
else()
  message(FATAL_ERROR "RVCXX environment varible is not set.")
endif()

if(NOT RVOBJDUMP)
  file(GLOB RVOBJDUMP_FILES "${RISCV_BIN_PATH}/*objdump")
  list(GET RVOBJDUMP_FILES 0 RVOBJDUMP)
endif()
if(RVOBJDUMP)
  message(STATUS "RVOBJDUMP set to ${RVOBJDUMP}")
else()
  message(FATAL_ERROR "RVOBJDUMP environment varible is not set.")
endif()

set (passRegex "Simulation is complete")


#------- TESTS  ---------------
message(STATUS "CTest Setup")

# Macro to build and configure tests in rev based on the labels the test was specified with
macro(add_rev_test test_name test_dir timeout labels)
  string(TOLOWER ${test_dir} test_dir_lower)
  string(TOLOWER ${test_name} test_name_lower)

  # Parse arguments for an optional script
  set(optional_script "")
  cmake_parse_arguments(ARG "" "SCRIPT" "" ${ARGN})
  if(ARG_SCRIPT)
    set(optional_script ${ARG_SCRIPT})
  endif()

  # Build the test and generate disassembly
  add_custom_target(build_${test_name_lower}
    COMMAND ${CMAKE_MAKE_PROGRAM} clean
    COMMAND ${CMAKE_MAKE_PROGRAM}
    COMMAND ${RVOBJDUMP} -dC -Mno-aliases --source *.exe > ${CMAKE_CURRENT_SOURCE_DIR}/${test_dir_lower}/${test_name_lower}.asm
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${test_dir_lower}
    COMMENT "Building and disassembling ${test_name_lower} test"
  )

  # Initialize configuration variables
  set(machine_args "\"[CORES:RV64GC]\"")
  set(numHartsList 1)
  set(numCoresList 1)
  set(add_memh_test FALSE)
  set(startSymbol "\"[0:main]\"")

  # Adjust parameters based on the labels that are passed
  string(FIND "${labels}" "rv32" rv32_label_index)
  if(NOT ${rv32_label_index} EQUAL -1)
    set(machine_args "\"[CORES:RV32GC]\"")
  endif()

  string(FIND "${labels}" "multihart" multihart_label_index)
  if(NOT ${multihart_label_index} EQUAL -1)
    set(numHartsList 1 2 4)
  endif()

  string(FIND "${labels}" "multicore" multicore_label_index)
  if(NOT ${multicore_label_index} EQUAL -1)
    set(numCoresList 1 2 4)
  endif()

  string(FIND "${labels}" "pthreads" multithreading_label_index)
  if(NOT ${multithreading_label_index} EQUAL -1)
    set(numHartsList 1 2 4)
    set(numCoresList 1 2 4)
  endif()

  string(FIND "${labels}" "memh" memh_label_index)
  if(NOT ${memh_label_index} EQUAL -1)
    set(add_memh_test TRUE)
  endif()

  string(FIND "${labels}" "cpp" cpp_label_index)
  if(NOT ${cpp_label_index} EQUAL -1)
    set(startSymbol "\"[0:_start]\"")
  endif()

  if(NOT optional_script)
    foreach(numHarts IN LISTS numHartsList)
      foreach(numCores IN LISTS numCoresList)

        # Define revmem target with the new naming convention
        add_custom_target(run_${test_name_lower}_revmem_${numHarts}_harts_${numCores}_cores
        COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR}/${test_name_lower} && sst --add-lib-path=${CMAKE_SOURCE_DIR}/build/src/ ${CMAKE_SOURCE_DIR}/test/rev-model-options-config.py -- --program="${test_name_lower}.exe" --numHarts=${numHarts} --numCores=${numCores} --machine "${machine_args}" --startSymbol "${startSymbol}"
        DEPENDS build_${test_name_lower}
        COMMENT "Running ${test_name_lower} test with revmem, numHarts=${numHarts}, numCores=${numCores}"
      )
        add_test(NAME ${test_name_lower}_revmem_${numHarts}_harts_${numCores}_cores COMMAND ${CMAKE_COMMAND} --build . --target run_${test_name_lower}_revmem_${numHarts}_harts_${numCores}_cores)

        # If 'memh' label found, add a memHierarchy test
        if(add_memh_test)
          add_custom_target(run_${test_name_lower}_memh_${numHarts}_harts_${numCores}_cores
          COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR}/${test_name_lower} && sst --add-lib-path=${CMAKE_SOURCE_DIR}/build/src/ ${CMAKE_SOURCE_DIR}/test/rev-model-options-config.py -- --program="${test_name_lower}.exe" --numHarts=${numHarts} --numCores=${numCores} --machine "${machine_args}" --enableMemH=1 --startSymbol "${startSymbol}"
          DEPENDS build_${test_name_lower}
          COMMENT "Running ${test_name_lower} test with memHierarchy enabled, numHarts=${numHarts}, numCores=${numCores}"
        )
          add_test(NAME ${test_name_lower}_memh_${numHarts}_harts_${numCores}_cores COMMAND ${CMAKE_COMMAND} --build . --target run_${test_name_lower}_memh_${numHarts}_harts_${numCores}_cores)
        endif()

        set_tests_properties(${test_name_lower}_revmem_${numHarts}_harts_${numCores}_cores
        PROPERTIES
          ENVIRONMENT "RVCC=${RVCC}"
          TIMEOUT ${timeout}
          PASS_REGULAR_EXPRESSION "${passRegex}"
          LABELS "${labels}"
      )
        if(add_memh_test)
          set_tests_properties(${test_name_lower}_memh_${numHarts}_harts_${numCores}_cores
          PROPERTIES
            ENVIRONMENT "RVCC=${RVCC}"
            TIMEOUT ${timeout}
            PASS_REGULAR_EXPRESSION "${passRegex}"
            LABELS "${labels}"
        )
        endif()
      endforeach()
    endforeach()

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

# add_rev_test(test_name test_dir timeout labels)
add_rev_test(EX1 ex1 30 "all;memh;rv32")
add_rev_test(EX2 ex2 30 "all;memh;rv64")
add_rev_test(EX3 ex3 30 "all;memh;rv32")
add_rev_test(EX4 ex4 30 "all;memh;rv32")
add_rev_test(EX5 ex5 30 "all;memh;rv64")
add_rev_test(EX6 ex6 45 "all;memh;rv64")
add_rev_test(EX7 ex7 30 "all;memh;rv64")
add_rev_test(BIG_LOOP big_loop 100 "all;rv64;benchmark")
add_rev_test(LARGE_BSS large_bss 60 "all;memh;rv64")
add_rev_test(DEP_CHECK dep_check 30 "all;memh;rv32")
add_rev_test(CACHE_1 cache_1 30 "all;memh;rv32" SCRIPT "run_cache_1.sh")
add_rev_test(CACHE_2 cache_2 30 "all;memh;rv64" SCRIPT "run_cache_2.sh")
add_rev_test(STRLEN_C strlen_c 30 "all;memh;rv64")
add_rev_test(STRLEN_CXX strlen_cxx 30 "all;memh;rv64;cxx")
add_rev_test(STRSTR strstr 30 "all;memh;rv64")
add_rev_test(MEMSET memset 30 "all;memh;rv64")
add_rev_test(MEMSET_2 memset_2 60 "all;memh;rv64")
add_rev_test(MANY_CORE many_core 30 "all;memh;rv64" SCRIPT "run_many_core.sh")
add_rev_test(DIVW divw 30 "all;memh;rv64")
add_rev_test(DIVW2 divw2 30 "all;memh;rv64")
add_rev_test(X0 x0 30 "all;memh;rv64")
add_rev_test(ARGC argc 30 "all;memh;rv64;loader;" SCRIPT "run_argc.sh")
add_rev_test(ARGC_SHORT argc 30 "all;memh;rv64;loader;" SCRIPT "run_argc_short.sh")
add_rev_test(COPROC_EX coproc_ex 30 "all;memh;rv64;coproc" SCRIPT "run_coproc_ex.sh")
add_rev_test(ZICBOM zicbom 45 "all;memh;rv64" SCRIPT "run_zicbom.sh")
add_rev_test(BACKINGSTORE backingstore 100 "all;rv64" SCRIPT "run_backingstore.sh")
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
