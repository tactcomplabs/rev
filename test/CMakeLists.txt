set( RISCV_ENV "$ENV{RISCV}" )
if(RISCV_ENV)
  message(STATUS "RISCV environment set to ${RISCV_ENV}")
else()
  message(FATAL_ERROR "RISCV environment variable is not set.")
endif()

# Check for RISCV Compiler
find_program(RVTOOLS riscv64-unknown-elf-gcc)
find_program(RVGNULINUX riscv64-unknown-linux-gnu-gcc)
find_program(RVMUSLLINUX riscv64-unknown-linux-musl-gcc)

if(NOT (RVTOOLS OR RVGNULINUX OR RVMUSLLINUX))
    message(FATAL_ERROR "No riscv64 compatible gcc (riscv64-unknown-elf-gcc || riscv64-unknown-linux-gnu-gcc || riscv64-unknown-linux-musl-gcc) binary found in path")
endif()

# find_program(RVLINUX riscv64-unknown-linux-gnu-gcc)
set (passRegex "Simulation is complete")
#Program execution complete

#------- TEST EX 1 ---------------
add_test(NAME TEST_EX1 COMMAND run_ex1.sh WORKING_DIRECTORY ../test/ex1)
set_property (TEST TEST_EX1 PROPERTY PASS_REGULAR_EXPRESSION "${passRegex}")
set_property (TEST TEST_EX1 PROPERTY TIMEOUT 30)

#------- TEST EX 2 ---------------
add_test(NAME TEST_EX2 COMMAND run_ex2.sh WORKING_DIRECTORY ../test/ex2)
set_property (TEST TEST_EX2 PROPERTY TIMEOUT 30)
set_property (TEST TEST_EX2 PROPERTY PASS_REGULAR_EXPRESSION "${passRegex}")

#------- TEST EX 3 ---------------
add_test(NAME TEST_EX3 COMMAND run_ex3.sh WORKING_DIRECTORY ../test/ex3)
set_property (TEST TEST_EX3 PROPERTY TIMEOUT 30)
set_property (TEST TEST_EX3 PROPERTY PASS_REGULAR_EXPRESSION "${passRegex}")

#------- TEST EX 4 ---------------
add_test(NAME TEST_EX4 COMMAND run_ex4.sh WORKING_DIRECTORY ../test/ex4)
set_property (TEST TEST_EX4 PROPERTY TIMEOUT 30)
set_property (TEST TEST_EX4 PROPERTY PASS_REGULAR_EXPRESSION "${passRegex}")

#------- TEST EX 5 ---------------
add_test(NAME TEST_EX5 COMMAND run_ex5.sh WORKING_DIRECTORY ../test/ex5)
set_property (TEST TEST_EX5 PROPERTY TIMEOUT 30)
set_property (TEST TEST_EX5 PROPERTY PASS_REGULAR_EXPRESSION "${passRegex}")

#------- TEST EX 6 ---------------
add_test(NAME TEST_EX6 COMMAND run_ex6.sh WORKING_DIRECTORY ../test/ex6)
set_property (TEST TEST_EX6 PROPERTY TIMEOUT 30)
set_property (TEST TEST_EX6 PROPERTY PASS_REGULAR_EXPRESSION "${passRegex}")

#------- TEST BIG_LOOP ---------------
add_test(NAME TEST_BIG_LOOP COMMAND run_big_loop.sh WORKING_DIRECTORY ../test/big_loop)
set_property (TEST TEST_BIG_LOOP PROPERTY TIMEOUT 60)
set_property (TEST TEST_BIG_LOOP PROPERTY PASS_REGULAR_EXPRESSION "${passRegex}")

#------- TEST DOT_DOUBLE ---------------
  add_test(NAME TEST_DOT_DOUBLE COMMAND run_dot_double.sh WORKING_DIRECTORY ../test/dot_double)
if(RVLINUX)
  set_property (TEST TEST_DOT_DOUBLE PROPERTY TIMEOUT 30)
  set_property (TEST TEST_DOT_DOUBLE PROPERTY PASS_REGULAR_EXPRESSION "${passRegex}")
else()
  set_property (TEST TEST_DOT_DOUBLE PROPERTY DISABLED True)
  message(NOTICE "TEST_DOT_DOUBLE Skipped: RV Linux Compiler not found") 
endif()

#------- TEST DOT_SINGLE ---------------
  add_test(NAME TEST_DOT_SINGLE COMMAND run_dot_single.sh WORKING_DIRECTORY ../test/dot_single)
if(RVLINUX)
  set_property (TEST TEST_DOT_SINGLE PROPERTY TIMEOUT 30)
  set_property (TEST TEST_DOT_SINGLE PROPERTY PASS_REGULAR_EXPRESSION "${passRegex}")
else()
  set_property (TEST TEST_DOT_SINGLE PROPERTY DISABLED True)
  message(NOTICE "TEST_DOT_SINGLE Skipped: RV Linux Compiler not found") 
endif()

#------- TEST MINFFT ---------------
add_test(NAME TEST_MINFFT COMMAND run_minfft.sh WORKING_DIRECTORY ../test/minfft)
set_property (TEST TEST_MINFFT PROPERTY TIMEOUT 30)
set_property (TEST TEST_MINFFT PROPERTY DISABLED True)
set_property (TEST TEST_MINFFT PROPERTY PASS_REGULAR_EXPRESSION "${passRegex}")

