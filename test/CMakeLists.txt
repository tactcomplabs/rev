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
  message(FATAL_ERROR "RVCC environment variable is not set.")
endif()

if(NOT RVCXX)
  set(RVCXX "$ENV{RVCXX}")
endif()
if(RVCXX)
  message(STATUS "RVCXX set to ${RVCXX}")
else()
  message(FATAL_ERROR "RVCXX environment variable is not set.")
endif()

# Check for custom REV_SST_CONFIG
if(NOT REV_SST_CONFIG)
  set(REV_SST_CONFIG "$ENV{REV_SST_CONFIG}")
endif()
if(REV_SST_CONFIG)
  message(STATUS "REV_SST_CONFIG set to ${REV_SST_CONFIG}")
else()
  message(STATUS "REV_SST_CONFIG is not set, default to per-test folder configs")
endif()

# Check for custom ARCH
if(NOT ARCH)
  set(ARCH "$ENV{ARCH}")
endif()
if(ARCH)
  message(STATUS "ARCH set to ${ARCH}")
else()
  message(STATUS "ARCH is not set, default to makefile settings")
endif()

# Check for custom ABI
if(NOT ABI)
  set(ABI "$ENV{ABI}")
endif()
if(ABI)
  message(STATUS "ABI set to ${ABI}")
else()
  message(STATUS "ABI is not set, default to makefile settings")
endif()

# find_program(RVLINUX riscv64-unknown-linux-gnu-gcc)
set (passRegex "Simulation is complete")
#Program execution complete

#------- TESTS  ---------------
message(STATUS "CTest Setup")

# -- Add tests
# TEST DCMP
add_test(NAME TEST_DCMP COMMAND run_dcmp.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/dcmp" )
set_tests_properties(TEST_DCMP
  PROPERTIES
    ENVIRONMENT "RVCC=${RVCC}; RVCXX=${RVCXX}; REV_SST_CONFIG=${REV_SST_CONFIG}; ARCH=${ARCH}; ABI=${ABI};"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv64"
)

# TEST EX1
add_test(NAME TEST_EX1 COMMAND run_ex1.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/ex1" )
set_tests_properties(TEST_EX1
  PROPERTIES
    ENVIRONMENT "RVCC=${RVCC}; RVCXX=${RVCXX}; REV_SST_CONFIG=${REV_SST_CONFIG}; ARCH=${ARCH}; ABI=${ABI};"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv32"
)

add_test(NAME TEST_EX2 COMMAND run_ex2.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/ex2" ) # ex2
set_tests_properties(TEST_EX2
  PROPERTIES
    ENVIRONMENT "RVCC=${RVCC}; RVCXX=${RVCXX}; REV_SST_CONFIG=${REV_SST_CONFIG}; ARCH=${ARCH}; ABI=${ABI};"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv64"
)

add_test(NAME TEST_EX3 COMMAND run_ex3.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/ex3" ) # ex3
set_tests_properties(TEST_EX3
  PROPERTIES
    ENVIRONMENT "RVCC=${RVCC}; RVCXX=${RVCXX}; REV_SST_CONFIG=${REV_SST_CONFIG}; ARCH=${ARCH}; ABI=${ABI};"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv32"
)

add_test(NAME TEST_EX4 COMMAND run_ex4.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/ex4" ) # ex4
set_tests_properties(TEST_EX4
  PROPERTIES
    ENVIRONMENT "RVCC=${RVCC}; RVCXX=${RVCXX}; REV_SST_CONFIG=${REV_SST_CONFIG}; ARCH=${ARCH}; ABI=${ABI};"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv32"
)

add_test(NAME TEST_EX5 COMMAND run_ex5.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/ex5" ) # ex5
set_tests_properties(TEST_EX5
  PROPERTIES
    ENVIRONMENT "RVCC=${RVCC}; RVCXX=${RVCXX}; REV_SST_CONFIG=${REV_SST_CONFIG}; ARCH=${ARCH}; ABI=${ABI};"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv64"
)

add_test(NAME TEST_EX6 COMMAND run_ex6.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/ex6" ) # ex6
set_tests_properties(TEST_EX6
  PROPERTIES
    ENVIRONMENT "RVCC=${RVCC}; RVCXX=${RVCXX}; REV_SST_CONFIG=${REV_SST_CONFIG}; ARCH=${ARCH}; ABI=${ABI};"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv64"
)

add_test(NAME TEST_BGE_BLE COMMAND run_bge_ble.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bge_ble" ) # bge_ble
set_tests_properties(TEST_BGE_BLE
  PROPERTIES
    ENVIRONMENT "RVCC=${RVCC}; RVCXX=${RVCXX}; REV_SST_CONFIG=${REV_SST_CONFIG}; ARCH=${ARCH}; ABI=${ABI};"
    TIMEOUT 90
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv64"
)
add_test(NAME TEST_BIG_LOOP COMMAND run_big_loop.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/big_loop" ) # big_loop
set_tests_properties(TEST_BIG_LOOP
  PROPERTIES
    ENVIRONMENT "RVCC=${RVCC}; RVCXX=${RVCXX}; REV_SST_CONFIG=${REV_SST_CONFIG}; ARCH=${ARCH}; ABI=${ABI};"
    TIMEOUT 90
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv64"
)

add_test(NAME TEST_MINFFT COMMAND run_minfft.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/minfft" ) # minfft
set_tests_properties(TEST_MINFFT
  PROPERTIES
    ENVIRONMENT "RVCC=${RVCC}; RVCXX=${RVCXX}; REV_SST_CONFIG=${REV_SST_CONFIG}; ARCH=${ARCH}; ABI=${ABI};"
    TIMEOUT 45
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv64"
)

add_test(NAME TEST_LARGE_BSS COMMAND run_large_bss.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/large_bss" ) # large_bss
set_tests_properties(TEST_LARGE_BSS
  PROPERTIES
    ENVIRONMENT "RVCC=${RVCC}; RVCXX=${RVCXX}; REV_SST_CONFIG=${REV_SST_CONFIG}; ARCH=${ARCH}; ABI=${ABI};"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv64"
)

add_test(NAME TEST_DOT_SINGLE COMMAND run_dot_single.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/dot_single" ) # dot_single
set_tests_properties(TEST_DOT_SINGLE
  PROPERTIES
    ENVIRONMENT "RVCC=${RVCC}; RVCXX=${RVCXX}; REV_SST_CONFIG=${REV_SST_CONFIG}; ARCH=${ARCH}; ABI=${ABI};"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv64;blas-required"
)

add_test(NAME TEST_DOT_DOUBLE COMMAND run_dot_double.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/dot_double" ) # dot_double
set_tests_properties(TEST_DOT_DOUBLE
  PROPERTIES
    ENVIRONMENT "RVCC=${RVCC}; RVCXX=${RVCXX}; REV_SST_CONFIG=${REV_SST_CONFIG}; ARCH=${ARCH}; ABI=${ABI};"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv64;blas-required"
)

add_test(NAME TEST_DEP_CHECK COMMAND run_dep_check.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/dep_check" ) # dep_check
set_tests_properties(TEST_DEP_CHECK
  PROPERTIES
    ENVIRONMENT "RVCC=${RVCC}; RVCXX=${RVCXX}; REV_SST_CONFIG=${REV_SST_CONFIG}; ARCH=${ARCH}; ABI=${ABI};"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv32"
)

add_test(NAME TEST_CACHE_1 COMMAND run_cache_test1.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/cache_test1" ) # cache_test1
set_tests_properties(TEST_CACHE_1
  PROPERTIES
    ENVIRONMENT "RVCC=${RVCC}; RVCXX=${RVCXX}; REV_SST_CONFIG=${REV_SST_CONFIG}; ARCH=${ARCH}; ABI=${ABI};"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv32"
)

add_test(NAME TEST_CACHE_2 COMMAND run_cache_test2.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/cache_test2" ) # cache_test
set_tests_properties(TEST_CACHE_2
  PROPERTIES
    ENVIRONMENT "RVCC=${RVCC}; RVCXX=${RVCXX}; REV_SST_CONFIG=${REV_SST_CONFIG}; ARCH=${ARCH}; ABI=${ABI};"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv64"
)

add_test(NAME TEST_STRLEN_C COMMAND run_strlen_c.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/strlen_c" ) # strlen_c
set_tests_properties(TEST_STRLEN_C
  PROPERTIES
    ENVIRONMENT "RVCC=${RVCC}; RVCXX=${RVCXX}; REV_SST_CONFIG=${REV_SST_CONFIG}; ARCH=${ARCH}; ABI=${ABI};"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv64"
)

add_test(NAME TEST_STRLEN_CXX COMMAND run_strlen_cxx.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/strlen_cxx" ) # strlen_cxx
set_tests_properties(TEST_STRLEN_CXX
  PROPERTIES
    ENVIRONMENT "RVCC=${RVCC}; RVCXX=${RVCXX}; REV_SST_CONFIG=${REV_SST_CONFIG}; ARCH=${ARCH}; ABI=${ABI};"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv64"
)

add_test(NAME TEST_STRSTR COMMAND run_strstr.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/strstr" ) # strlen_cxx
set_tests_properties(TEST_STRSTR
  PROPERTIES
    ENVIRONMENT "RVCC=${RVCC}; RVCXX=${RVCXX}; REV_SST_CONFIG=${REV_SST_CONFIG}; ARCH=${ARCH}; ABI=${ABI};"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv64"
)

add_test(NAME TEST_MEMSET COMMAND run_memset.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/memset" ) # memset
set_tests_properties(TEST_MEMSET
  PROPERTIES
    ENVIRONMENT "RVCC=${RVCC}; RVCXX=${RVCXX}; REV_SST_CONFIG=${REV_SST_CONFIG}; ARCH=${ARCH}; ABI=${ABI};"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv64"
)

add_test(NAME TEST_MEMSET_2 COMMAND run_mem.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/memset_2" ) # memset
set_tests_properties(TEST_MEMSET_2
  PROPERTIES
    ENVIRONMENT "RVCC=${RVCC}; RVCXX=${RVCXX}; REV_SST_CONFIG=${REV_SST_CONFIG}; ARCH=${ARCH}; ABI=${ABI};"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv64"
)

add_test(NAME MANY_CORE COMMAND run_many_core.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/many_core" ) # memset
set_tests_properties(MANY_CORE
  PROPERTIES
    ENVIRONMENT "RVCC=${RVCC}; RVCXX=${RVCXX}; REV_SST_CONFIG=${REV_SST_CONFIG}; ARCH=${ARCH}; ABI=${ABI};"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv64"
)

add_test(NAME DIVW COMMAND run_divw.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/divw" ) # divw
set_tests_properties(DIVW
  PROPERTIES
    ENVIRONMENT "RVCC=${RVCC}; RVCXX=${RVCXX}; REV_SST_CONFIG=${REV_SST_CONFIG}; ARCH=${ARCH}; ABI=${ABI};"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv64"
)

add_test(NAME DIVW2 COMMAND run_divw2.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/divw2" ) # divw2
set_tests_properties(DIVW2
  PROPERTIES
    ENVIRONMENT "RVCC=${RVCC}; RVCXX=${RVCXX}; REV_SST_CONFIG=${REV_SST_CONFIG}; ARCH=${ARCH}; ABI=${ABI};"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv64"
)

add_test(NAME TOWERS COMMAND run_towers.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/towers" ) # towers
set_tests_properties(TOWERS
  PROPERTIES
    ENVIRONMENT "RVCC=${RVCC}; RVCXX=${RVCXX}; REV_SST_CONFIG=${REV_SST_CONFIG}; ARCH=${ARCH}; ABI=${ABI};"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv64"
)

add_test(NAME QSORT COMMAND run_qsort.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/qsort" ) # qsort
set_tests_properties(QSORT
  PROPERTIES
    ENVIRONMENT "RVCC=${RVCC}; RVCXX=${RVCXX}; REV_SST_CONFIG=${REV_SST_CONFIG}; ARCH=${ARCH}; ABI=${ABI};"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv64"
)

add_test(NAME MEMCPY COMMAND run_memcpy.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/memcpy" ) # memcpy
set_tests_properties(MEMCPY
  PROPERTIES
    ENVIRONMENT "RVCC=${RVCC}; RVCXX=${RVCXX}; REV_SST_CONFIG=${REV_SST_CONFIG}; ARCH=${ARCH}; ABI=${ABI};"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv64"
)

add_test(NAME X0 COMMAND run_x0.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/x0" ) # x0
set_tests_properties(X0
  PROPERTIES
    ENVIRONMENT "RVCC=${RVCC}; RVCXX=${RVCXX}; REV_SST_CONFIG=${REV_SST_CONFIG}; ARCH=${ARCH}; ABI=${ABI};"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv64"
)

add_test(NAME AMOADD_C COMMAND run_amoadd_c.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/amo/amoadd_c" )
set_tests_properties(AMOADD_C
  PROPERTIES
    ENVIRONMENT "RVCC=${RVCC}; RVCXX=${RVCXX}; REV_SST_CONFIG=${REV_SST_CONFIG}; ARCH=${ARCH}; ABI=${ABI};"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv64"
)

add_test(NAME AMOADD_CXX COMMAND run_amoadd_cxx.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/amo/amoadd_cxx" )
set_tests_properties(AMOADD_CXX
  PROPERTIES
    ENVIRONMENT "RVCC=${RVCC}; RVCXX=${RVCXX}; REV_SST_CONFIG=${REV_SST_CONFIG}; ARCH=${ARCH}; ABI=${ABI};"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv64"
)

add_test(NAME AMOADD_C_MEMH COMMAND run_amoadd_c_memh.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/amo/amoadd_c_memh" )
set_tests_properties(AMOADD_C_MEMH
  PROPERTIES
    ENVIRONMENT "RVCC=${RVCC}; RVCXX=${RVCXX}; REV_SST_CONFIG=${REV_SST_CONFIG}; ARCH=${ARCH}; ABI=${ABI};"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv64"
)

add_test(NAME AMOADD_CXX_MEMH COMMAND run_amoadd_cxx_memh.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/amo/amoadd_cxx_memh" )
set_tests_properties(AMOADD_CXX_MEMH
  PROPERTIES
    ENVIRONMENT "RVCC=${RVCC}; RVCXX=${RVCXX}; REV_SST_CONFIG=${REV_SST_CONFIG}; ARCH=${ARCH}; ABI=${ABI};"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv64"
)

# This shows how arguments can be passed to the .py file looking up environment variables through os.env
add_test(NAME ARGC_REVMEM COMMAND run_argc.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/argc_revmem")
set_tests_properties(ARGC_REVMEM
  PROPERTIES
    ENVIRONMENT "REV_EXE_ARGS=one two three; RVCC=${RVCC}; RVCXX=${RVCXX}; REV_SST_CONFIG=${REV_SST_CONFIG}; ARCH=${ARCH}; ABI=${ABI};"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv64"
)

add_test(NAME ARGC_MEMH COMMAND run_argc.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/argc_memh")
set_tests_properties(ARGC_MEMH
  PROPERTIES
    ENVIRONMENT "REV_EXE_ARGS=one two three; RVCC=${RVCC}; RVCXX=${RVCXX}; REV_SST_CONFIG=${REV_SST_CONFIG}; ARCH=${ARCH}; ABI=${ABI};"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv64"
)

add_test(NAME TLS COMMAND run-basic-tls.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/tls" ) # tls
set_tests_properties(TLS
  PROPERTIES
    ENVIRONMENT "RVCC=${RVCC}; RVCXX=${RVCXX}; REV_SST_CONFIG=${REV_SST_CONFIG}; ARCH=${ARCH}; ABI=${ABI};"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv64"
)

add_test(NAME COPROC_EX COMMAND run_coproc_ex.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/coproc_ex" ) # coproc_ex
set_tests_properties(COPROC_EX
  PROPERTIES
    ENVIRONMENT "RVCC=${RVCC}; RVCXX=${RVCXX}; REV_SST_CONFIG=${REV_SST_CONFIG}; ARCH=${ARCH}; ABI=${ABI};"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv64"
)

add_test(NAME ARGC_SHORT_REVMEM COMMAND run_argc.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/argc_short_revmem")
set_tests_properties(ARGC_SHORT_REVMEM
  PROPERTIES
    ENVIRONMENT "REV_EXE_ARGS=one; RVCC=${RVCC}; RVCXX=${RVCXX}; REV_SST_CONFIG=${REV_SST_CONFIG}; ARCH=${ARCH}; ABI=${ABI};"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all"
)

add_test(NAME ARGC_SHORT_MEMH COMMAND run_argc.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/argc_short_memh")
set_tests_properties(ARGC_SHORT_MEMH
  PROPERTIES
    ENVIRONMENT "REV_EXE_ARGS=one; RVCC=${RVCC}; RVCXX=${RVCXX}; REV_SST_CONFIG=${REV_SST_CONFIG}; ARCH=${ARCH}; ABI=${ABI};"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv64"
)

add_test(NAME MEMH_1 COMMAND run_memh_1.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/memh_1" ) # memh_1
set_tests_properties(MEMH_1
  PROPERTIES
    ENVIRONMENT "RVCC=${RVCC}; RVCXX=${RVCXX}; REV_SST_CONFIG=${REV_SST_CONFIG}; ARCH=${ARCH}; ABI=${ABI};"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all"
)

add_test(NAME MEMH_2 COMMAND run_memh_2.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/memh_2" ) # memh_2
set_tests_properties(MEMH_2
  PROPERTIES
    ENVIRONMENT "RVCC=${RVCC}; RVCXX=${RVCXX}; REV_SST_CONFIG=${REV_SST_CONFIG}; ARCH=${ARCH}; ABI=${ABI};"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv64"
)

# TEST CHDIR
add_test(NAME TEST_CHDIR COMMAND run_chdir.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/syscalls/chdir" )
set_tests_properties(TEST_CHDIR
  PROPERTIES
    ENVIRONMENT "RVCC=${RVCC}; RVCXX=${RVCXX}; REV_SST_CONFIG=${REV_SST_CONFIG}; ARCH=${ARCH}; ABI=${ABI};"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv64"
)

# TEST FILE
add_test(NAME TEST_FILE_IO COMMAND run_file-io.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/syscalls/file-io" )
set_tests_properties(TEST_FILE_IO
  PROPERTIES
    ENVIRONMENT "RVCC=${RVCC}; RVCXX=${RVCXX}; REV_SST_CONFIG=${REV_SST_CONFIG}; ARCH=${ARCH}; ABI=${ABI};"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv64"
)

# TEST FORK
add_test(NAME TEST_FORK COMMAND run_fork.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/syscalls/fork" )
set_tests_properties(TEST_FORK
  PROPERTIES
    ENVIRONMENT "RVCC=${RVCC}; RVCXX=${RVCXX}; REV_SST_CONFIG=${REV_SST_CONFIG}; ARCH=${ARCH}; ABI=${ABI};"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv64"
)

# TEST GETCWD
add_test(NAME TEST_GETCWD COMMAND run_getcwd.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/syscalls/getcwd" )
set_tests_properties(TEST_GETCWD
  PROPERTIES
    ENVIRONMENT "RVCC=${RVCC}; RVCXX=${RVCXX}; REV_SST_CONFIG=${REV_SST_CONFIG}; ARCH=${ARCH}; ABI=${ABI};"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv64"
)

# TEST GETPID
add_test(NAME TEST_GETPID COMMAND run_getpid.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/syscalls/getpid" )
set_tests_properties(TEST_GETPID
  PROPERTIES
    ENVIRONMENT "RVCC=${RVCC}; RVCXX=${RVCXX}; REV_SST_CONFIG=${REV_SST_CONFIG}; ARCH=${ARCH}; ABI=${ABI};"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv64"
)

# TEST MUNMAP
add_test(NAME TEST_MUNMAP COMMAND run_munmap.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/syscalls/munmap" )
set_tests_properties(TEST_MUNMAP
  PROPERTIES
    ENVIRONMENT "RVCC=${RVCC}; RVCXX=${RVCXX}; REV_SST_CONFIG=${REV_SST_CONFIG}; ARCH=${ARCH}; ABI=${ABI};"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv64"
)

# TEST VECTOR
add_test(NAME TEST_VECTOR COMMAND run_vector.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/syscalls/vector" )
set_tests_properties(TEST_VECTOR
  PROPERTIES
    ENVIRONMENT "RVCC=${RVCC}; RVCXX=${RVCXX}; REV_SST_CONFIG=${REV_SST_CONFIG}; ARCH=${ARCH}; ABI=${ABI};"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv64"
)

# TEST WRITE
add_test(NAME TEST_WRITE COMMAND run_write.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/syscalls/write" )
set_tests_properties(TEST_WRITE
  PROPERTIES
    ENVIRONMENT "RVCC=${RVCC}; RVCXX=${RVCXX}; REV_SST_CONFIG=${REV_SST_CONFIG}; ARCH=${ARCH}; ABI=${ABI};"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv64"
)


# -- PROCESS CTest Config Variables
if(NOT CTEST_BLAS_REQUIRED_TESTS)
  set_tests_properties (TLS PROPERTIES DISABLED True)
  set_tests_properties (TEST_DOT_SINGLE PROPERTIES DISABLED True)
  set_tests_properties (TEST_DOT_DOUBLE PROPERTIES DISABLED True)
  message(NOTICE "   * Tests requiring BLAS headers are disabled (ie. -DCTEST_BLAS_REQUIRED_TESTS=OFF)
    - If you'd like to still run these tests run: `ctest -L blas_required`")
  message(VERBOSE "   TEST_DOT_SINGLE - Disabled")
  message(VERBOSE "   TEST_DOT_DOUBLE - Disabled")
endif()

if(NOT CTEST_MULTILIB_TESTS)
  set_tests_properties (TEST_EX1 PROPERTIES DISABLED True)
  set_tests_properties (TEST_EX3 PROPERTIES DISABLED True)
  set_tests_properties (TEST_EX4 PROPERTIES DISABLED True)
  message(NOTICE "   * RV32 tests disabled (ie. -DCTEST_MULTILIB_TESTS=OFF)
    - If you'd like to still run these tests run: `ctest -L rv32`")
  message(VERBOSE "TEST_EX1  - Disabled")
  message(VERBOSE "TEST_EX3  - Disabled")
  message(VERBOSE "TEST_EX4  - Disabled")
endif()

add_subdirectory(isa)

# EOF
