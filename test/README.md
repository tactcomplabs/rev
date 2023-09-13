
REV tests infrastructure
========================

REV's tests are located under the 'test/' folder. Each test folder contains C/C++ code source, a makefile, a python file, a run script. Tests can be compiled and ran by changing directory, invoking make and the provided run bash script.

This setup is meant to enable custom compilation and execution of tests for targeted configurations. The makefiles and run scripts are following conventions to support common environment variables which allows an external test harness to unify configuration settings across tests, for example to change the compiler used or the SST python configuration file.

The following variables are standardized across makefiles and SST python scripts:
* RVCC: absolute path to the C compiler for makefiles to use
* ARCH: RISC-V architecture that makefiles can use as the compiler's -march argument
* ABI: RISC-V ABI that makefiles can use as the compiler's -mabi argument
* REV_SST_CONFIG: absolute path to the SST .py file to use
* REV_EXE_ARGS: program argument to be consumed by the SST .py file; the python script must use os.env to read the value.


Adding tests to REV
-------------------

We distinguish compilation and runtime arguments. The ground rule is that unless a test cannot pass unless specific options are set, the makefile and run script should conditionally define
the standardized variables described above.

The rules are as follow:
* Makefiles should conditionally define and rely on the RVCC, ARCH, and ABI variables
* Run scripts should conditionally define and rely on REV_EXE_ARGS (if appropriate) and REV_SST_CONFIG

Running individual REV tests
-----------------------------

Most tests provide a makefile and a run script that both use default settings and are invoked as follow:
```
cd test/<test_name>
make clean && make
./run_<test_name>.sh
````

CMake test-harness infrastructure
=================================

REV tests can be executed as a test-suites through Cmake and CTest. The 'CMakeLists.txt' file under the 'test/' folder declares which tests are part of the CTest test-suite.

Adding tests to CMake
---------------------

* Add a REV test as described in the previou section
* Add a new test entry to the 'CMakeLists.txt' file under the 'test/' folder.
* Make sure to follow the templates used by existing tests and that test-specific settings that should not be overriden are hardcoded in the 'ENVIRONMENT' property.

At the moment, the CMake test-suite must be manually kept in sync with the content under the 'test/' folder.

The following snippet shows a classical example, where the 'argc_revmem' REV test is invoked by calling the `run_argc.sh` script,
which itself compiles the program before executing it. The 'ENVIRONMENT' property transmits the standardized variables to the run
script. In this particular instance, the 'REV_EXE_ARGS' variable is defined to specify the arguments to be fed to the simulated program
through the SST configuration file.

```
add_test(NAME ARGC_REVMEM COMMAND run_argc.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/argc_revmem")
set_tests_properties(ARGC_REVMEM
  PROPERTIES
    ENVIRONMENT "REV_EXE_ARGS=one two three; RVCC=${RVCC}; REV_SST_CONFIG=${REV_SST_CONFIG}; ARCH=${ARCH}; ABI=${ABI};"
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "${passRegex}"
    LABELS "all;rv64"
)
```

Executing tests using CMake
---------------------------

Environment settings
--------------------

The following environment variables can be passed down to the run scripts:

Individual tests are labelled to easily group tests based on specific properties.
There's currently two labels actively supported:
* 'all' is applied to each and every test
* 'rv64' for tests that are intended for rv64 compilation only

CTest usage examples
--------------------

The following illustrate how to customize the test-suite runs. Note that some care must be taken so that makefiles, run scripts, and environment variables are used in a coherent way.
For example, changing the architecture implicates both the makefile (ARCH varible to set the correct compiler argument) and the SST configuration flag (DREV_SST_CONFIG variables point to a python file that has a machine definitionm matching the ARCH).

After building REV, change to the `build` directory
```
cd rev/build
```

**Default run** is equivalent to going into each individual test folder and invoking the run script there which relying on the local SST python configuration file.

```
cmake
```

**Only run tests with a specific label** is equivalent to going into a subset of individual test folder and invoking the run script there relying on the local SST python configuration file.

```
ctest -L rv64
```

**Override the default compiler and architecture** for all the tests.
```
RVCC=/path/to/compiler/cc ARCH=rv64imafd ctest
```

**Override the default compiler, architecture, ABI, and SST model** used for all the tests.
```
RVCC=/path/to/compiler/cc ARCH=rv64imafd ABI=lp64d REV_SST_CONFIG=/path/to/custom/sst/config.py ctest
```


