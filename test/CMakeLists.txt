set( RISCV_ENV "$ENV{RISCV}" )
if(RISCV_ENV)
  message(STATUS "RISCV environment set to ${RISCV_ENV}")
else()
  message(FATAL_ERROR "RISCV environment variable is not set.")
endif()

# Check for RISCV Compiler
find_program(RVCC NAMES riscv64-unknown-elf-gcc
                        riscv64-unknown-linux-gnu-gcc 
                        riscv64-unknown-linux-musl-gcc)

if(RVCC-NOTFOUND)
    message(FATAL_ERROR "No riscv64 compatible gcc (riscv64-unknown-elf-gcc || riscv64-unknown-linux-gnu-gcc || riscv64-unknown-linux-musl-gcc) binary found in path")
endif()

# find_program(RVLINUX riscv64-unknown-linux-gnu-gcc)
set (passRegex "Simulation is complete")
#Program execution complete


#------- TESTS  ---------------
message(STATUS "CTest Setup")

# -- Add tests

# TEST EX1
add_test(NAME TEST_EX1 COMMAND run_ex1.sh WORKING_DIRECTORY ../test/ex1 ) # ex1
set_tests_properties(TEST_EX1 
  PROPERTIES 
    ENVIRONMENT "RVCC=${RVCC}"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv32"
)

add_test(NAME TEST_EX2 COMMAND run_ex2.sh WORKING_DIRECTORY ../test/ex2 ) # ex2
set_tests_properties(TEST_EX2
  PROPERTIES 
    ENVIRONMENT "RVCC=${RVCC}"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv64"
)

add_test(NAME TEST_EX3 COMMAND run_ex3.sh WORKING_DIRECTORY ../test/ex3 ) # ex3
set_tests_properties(TEST_EX3
  PROPERTIES 
    ENVIRONMENT "RVCC=${RVCC}"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv32"
)

add_test(NAME TEST_EX4 COMMAND run_ex4.sh WORKING_DIRECTORY ../test/ex4 ) # ex3
set_tests_properties(TEST_EX4
  PROPERTIES 
    ENVIRONMENT "RVCC=${RVCC}"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv32"
)

add_test(NAME TEST_EX5 COMMAND run_ex5.sh WORKING_DIRECTORY ../test/ex5 ) # ex4
set_tests_properties(TEST_EX5
  PROPERTIES 
    ENVIRONMENT "RVCC=${RVCC}"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv64"
)

add_test(NAME TEST_EX6 COMMAND run_ex6.sh WORKING_DIRECTORY ../test/ex6 ) # ex5
set_tests_properties(TEST_EX6
  PROPERTIES 
    ENVIRONMENT "RVCC=${RVCC}"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv64"
)

add_test(NAME TEST_BIG_LOOP COMMAND run_big_loop.sh WORKING_DIRECTORY ../test/big_loop ) # big_loop
set_tests_properties(TEST_BIG_LOOP
  PROPERTIES 
    ENVIRONMENT "RVCC=${RVCC}"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv64"
)

add_test(NAME TEST_MINFFT COMMAND run_minfft.sh WORKING_DIRECTORY ../test/minfft ) # minfft
set_tests_properties(TEST_MINFFT
  PROPERTIES 
    ENVIRONMENT "RVCC=${RVCC}"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv64"
)

add_test(NAME TEST_DOT_SINGLE COMMAND run_dot_single.sh WORKING_DIRECTORY ../test/dot_single ) # dot_single
set_tests_properties(TEST_DOT_SINGLE
  PROPERTIES 
    ENVIRONMENT "RVCC=${RVCC}"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv64;blas-required"
)


add_test(NAME TEST_DOT_DOUBLE COMMAND run_dot_double.sh WORKING_DIRECTORY ../test/dot_double ) # dot_double
set_tests_properties(TEST_DOT_DOUBLE
  PROPERTIES 
    ENVIRONMENT "RVCC=${RVCC}"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv64;blas-required"
)


add_test(NAME TEST_DEP_CHECK COMMAND run_dep_check.sh WORKING_DIRECTORY ../test/dep_check ) # dep_check
set_tests_properties(TEST_DEP_CHECK
  PROPERTIES 
    ENVIRONMENT "RVCC=${RVCC}"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv32"
)


# -- PROCESS CTest Config Variables
if(NOT CTEST_BLAS_REQUIRED_TESTS)
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

