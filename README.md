# rev : RISC-V Native CPU Model for SST

![rev](documentation/imgs/rev_logo.png)

## Getting Started

The Rev SST component is designed to provide cycle-based simulation
capabilities of an arbitrary RISC-V core or cores. Rev utilizes the Sandia
[Structural Simulation Toolkit](http://sst-simulator.org/) as the core parallel
discrete event simulation framework. We utilize the standard SST "core"
component libraries to build the Rev component. As a result, Rev can be
attached to any other existing SST component for full system and network
simulations.

The Rev model is unique in the scope of other SST CPU models in that is
provides users the ability to load compiled binaries (ELF binaries). Rather
than requiring input in the form of textual assembly or hex dumps, SST contains
a RISC-V compatible loader function that generates all the necessary symbol
tables and addressing modes for the target RISC-V CPU. Further, Rev permits
users to generate simulation configurations that contain heterogeneous RISC-V
designs with support for disparate extensions.

The Rev component infrastructure can also be extended to include custom
instruction extensions. This provides users the ability to design new
instruction templates without requiring modifications to the core crack+decode
infrastructure. For more information on creating custom templates, see the
developer documentation.

## Prerequisites

Given that this is an SST external component, the primary prerequisite is a
current installation of the SST Core. The Rev building infrastructure assumes
that the `sst-config` tool is installed and can be found in the current PATH
environment.

Rev relies on CMake for building the component from source. The minimum required
version for this is `3.19`.

## Building

Building the Rev SST component from source using CMake (>= 3.19) can be performed as follows:

    git clone
    cd rev/build
    cmake -DRVCC=/path/to/riscv/compiler/exe ..
    make -j
    make install

Additional build options include:
    * -DBUILD_ASM_TESTING=ON

After a successful build you can test your install with:

    make test

You can also run a single test with:

    ctest -R <test_name>

where you can substitute `test_name` with the name of the test, for example:

    ctest -R TEST_EX1

will run the test found in test/ex1. See the full list of tests in
`test/CMakeLists.txt`.

## Building Compatible Compilers

As mentioned above, the Rev SST model supports standard ELF binary payloads as
input to the model. As a result, we need a cross compilation framework to build
source code into suitable binaries. We highly recommend building a suitable
RISC-V compiler from source. This will permit you to tune the necessary options
in order to support a multitude of different standard, optional and custom
extensions.

We recommend compiling the [riscv-gnu-toolchain](https://github.com/riscv/riscv-gnu-toolchain) 
using the `multilib` option. This is analogous to the following:

    git clone https://github.com/riscv/riscv-gnu-toolchain
    cd riscv-gnu-toolchain
    git submodule update --init --recursive
    ./configure --prefix=/opt/riscv --enable-multilib
    make -j

## Example Execution

### Component Options

The Rev SST component contains the following options:

| Parameter           | Required? | Type               | Description |
|---------------------|-----------|--------------------|-------------|
| verbose             |           | unsigned integer   | Values of 0-8. Increasing values increase the verbosity of output |
| numCores            |    X      | unsigned integer   | Values of 1-N. Sets the number of cores in the simulation |
| clock               |    X      | Hertz              | "xGHz", "xKHz". Sets the clock frequency of the device. |
| memSize             |    X      | unsigned integer   | Sets the size of physical memory in bytes  |
| machine             |    X      | "[Core:Arch]"      | "[0:RV32I],[1:RV64G]". Sets the RISC-V architecture for the target core |
| startAddr           |    X      | "[Core:StartAddr]" | "[0:0x00010144],[1:0x123456]". Sets the entry point for each core  |
| memCost             |           | "[Core:Min:Max]"   | "[0:1:10],[1:50:100]", Sets the minimum and maximum latency (in cycles) for each core's memory load  |
| program             |    X      | string             | "example.exe". Sets the target ELF executable  |
| table               |           | string             | "/path/to/table.txt". Sets the path the instruction cost table |
| splash              |           | 0/1                | Default=0. Setting to 1 displays the Rev bootsplash  |
| enable\_nic         |           | 0/1                | Default=0. Setting to 1 enables a standard NIC |
| enable\_pan         |           | 0/1                | Default=0. Setting to 1 enables a PAN NIC |
| enable\_test        |           | 0/1                | Default=0. Setting to 1 enables the internal PAN test harness |
| enable\_pan\_stats  |           | 0/1                | Default=0. Setting to 1 enables internal statistics for PAN commands |
| enableRDMAMbox      |           | 0/1                | Default=1. Setting to 1 enables the internal RDMA Mailbox for applications to initiate messages |
| msgPerCycle         |           | unsigned integer   | Default=1. Sets the number of messages to inject per cycle |
| testIters           |           | unsigned integer   | Default=255. Sets the number of iterations for each PAN test loop |

### Deriving the ELF Entry Point

The latest version of Rev no longer requires the user to manually derive the
starting address for binaries that contain a `main()` function. If the user
specifies the starting address as `0x00`, then the Rev loader will
automatically derive the `main()` symbol address and use it as the starting
address. From here, the Rev model will perform an initial setup and reset of
the target core or cores in the same manner as prescribed by the RISC-V ABI.
Most users will expect to execute their application starting at the `main()`
function. If the user requires a different starting address or the target
payload does not contain a `main()` function, then the user must manually
derive the address. Given an executable that has been compiled
(`example.exe`), we may derive the entry point address using the tool chain's
`objdump` tool. An example of doing so is as follows:

    riscv64-unknown-elf-objdump -dC example.exe | grep "<main>"

This will give us output similar to the following:

    00010144 <main>:

Using this, the address `0x00010144` becomes our entry point address.

The Rev component model has the ability to start execution at valid address in
the RISC-V text space. However, keep in mind, that Rev assumes no prior state
when starting execution (start from reset). As a result, the user cannot assume
that the Rev model will prepopulate any memory or register state outside of
what is provided when executing from `main()`.

### Multicore Execution
As mentioned above, Rev has the ability to execute multiple, heterogeneous
cores in the same simulation. However, if users seek to execute multiple,
homogeneous cores, there is an additional configuration option for doing so.
For example, if you seek to simulate 8 homogeneous cores, set `numCores` to 8
and use the following configuration parameter for the `machine` option:

    "machine" : "[CORES:RV64G]"

This `CORES` option sets all 8 cores to `RV64G`. Similarly, if you seek to start
all the cores at the same `startAddr`, you can use the same option as follows:

    "startAddr : "[CORES:0x00000000]"

### Sample Execution

Executing one of the included sample tests can be performed as follows:

    export REV_EXE=ex1.exe
    sst rev-test-ex1.py

## Adding Tests to the test suite

To add tests to the Rev test suite, edit `test/CMakeLists.txt`. By default, the
tests look for the SST output string "Program Execution Complete" and have a
max runtime of 30 seconds. Both of these values are user defined with the
`test/CMakeLists.txt` file.

All tests should follow the existing directory structure and be added to
`test/<your_new_test_name>/`

CTest will look in your newly created folder for a shell script, this is the
script that will build the RISC-VV executable using the RISC-V compiler. See
`test/ext/run_ex1.sh` for an example.

## Contributing

We welcome outside contributions from corporate, academic and individual
developers. However, there are a number of fundamental ground rules that you
must adhere to in order to participate. These rules are outlined as follows:

* By contributing to this code, one must agree to the licensing described in
the top-level [LICENSE](LICENSE) file.
* All code must adhere to the existing C++ coding style. While we are somewhat
flexible in basic style, you will adhere to what is currently in place. This
includes camel case C++ methods and inline comments. Uncommented, complicated
algorithmic constructs will be rejected.
* We support compilaton and adherence to C++ standard methods. All new methods
and variables contained within public, private and protected class methods must
be commented using the existing Doxygen-style formatting. All new classes must
also include Doxygen blocks in the new header files. Any pull requests that
lack these features will be rejected.
* All changes to functionality and the API infrastructure must be accompanied
by complementary tests All external pull requests **must** target the `devel`
branch. No external pull requests will be accepted to the master branch.
* All external pull requests must contain sufficient documentation in the pull
request comments in order to be accepted.

## Extension Development

See the [developer documentation](documentation/ExtensionDevelopment.md).

## License

See the [LICENSE](./LICENSE) file

## Authors
* *John Leidel* - *Chief Scientist* - [Tactical Computing Labs](http://www.tactcomplabs.com)
* *David Donofrio* - *Chief Hardware Architect* - [Tactical Computing Labs](http://www.tactcomplabs.com)
* *Chris Taylor* - *Sr. Principal Research Engineer* - [Tactical Computing Labs](http://www.tactcomplabs.com)
* *Ryan Kabrick* - *Sr. Research Engineer* - [Tactical Computing Labs](http://www.tactcomplabs.com)
* *Lee Killough* - *Sr. Principal Research Engineer* - [Tactical Computing Labs](http://www.tactcomplabs.com)

## Acknowledgements
* TBD
