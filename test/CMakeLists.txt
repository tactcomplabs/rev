set( RISCV_ENV "$ENV{RISCV}" )
if(RISCV_ENV)
  message(STATUS "RISCV environment set to ${RISCV_ENV}")
else()
  message(FATAL_ERROR "RISCV environment variable is not set.")
endif()

# Check for RISCV Compiler
find_program(RVTOOLS riscv64-unknown-elf-gcc)
if(NOT RVTOOLS)
  message(FATAL_ERROR "No riscv64-unknown-elf-gcc binary found in path")
endif()

set (passRegex "Program execution complete")

#------- TEST EX 1 ---------------
add_executable(ex1 ./ex1/ex1.c)
add_custom_command(TARGET ex1 POST_BUILD COMMAND make WORKING_DIRECTORY ../test/ex1)
add_test(NAME TEST_EX1 COMMAND sst rev-test-ex1.py WORKING_DIRECTORY ../test/ex1)
set_property (TEST TEST_EX1 PROPERTY PASS_REGULAR_EXPRESSION "${passRegex}")

#------- TEST EX 2 ---------------
add_custom_command(OUTPUT ex2.exe COMMAND make WORKING_DIRECTORY ../test/ex2)
add_test(NAME TEST_EX2 COMMAND sst rev-test-ex2.py WORKING_DIRECTORY ../test/ex2)
set_property (TEST TEST_EX2 PROPERTY PASS_REGULAR_EXPRESSION "${passRegex}")
set_tests_properties(TEST_EX2 PROPERTIES TIMEOUT 10)

#------- TEST EX 3 ---------------
add_custom_command(OUTPUT ex3.exe COMMAND make WORKING_DIRECTORY ../test/ex3)
add_test(NAME TEST_EX3 COMMAND sst rev-test-ex3.py WORKING_DIRECTORY ../test/ex3)
set_property (TEST TEST_EX3 PROPERTY PASS_REGULAR_EXPRESSION "${passRegex}")

#------- TEST EX 4 ---------------
add_custom_command(OUTPUT ex4.exe COMMAND make WORKING_DIRECTORY ../test/ex4)
add_test(NAME TEST_EX4 COMMAND sst rev-test-ex4.py WORKING_DIRECTORY ../test/ex4)
set_property (TEST TEST_EX4 PROPERTY PASS_REGULAR_EXPRESSION "${passRegex}")

#------- TEST EX 5 ---------------
add_custom_command(OUTPUT ex5.exe COMMAND make WORKING_DIRECTORY ../test/ex5)
add_test(NAME TEST_EX5 COMMAND sst rev-test-ex5.py WORKING_DIRECTORY ../test/ex5)
set_property (TEST TEST_EX5 PROPERTY PASS_REGULAR_EXPRESSION "${passRegex}")

#------- TEST EX 6 ---------------
add_custom_command(OUTPUT ex6.exe COMMAND make WORKING_DIRECTORY ../test/ex6)
add_test(NAME TEST_EX6 COMMAND sst rev-test-ex6.py WORKING_DIRECTORY ../test/ex6)
set_property (TEST TEST_EX6 PROPERTY PASS_REGULAR_EXPRESSION "${passRegex}")

#------- TEST BIG_LOOP ---------------
add_custom_command(OUTPUT big_loop.exe COMMAND make WORKING_DIRECTORY ../test/big_loop)
add_test(NAME TEST_BIG_LOOP COMMAND sst big_loop.py WORKING_DIRECTORY ../test/big_loop)
set_property (TEST TEST_EX3 PROPERTY PASS_REGULAR_EXPRESSION "${passRegex}")

# add_test(dot_single)
# add_test(dot_double)
# add_test(dot_double)
# add_test(fault)
# add_test(minfft)
# add_test(pan_mbox_test1)
# add_test(pan_test1)
# add_test(pan_test2)
