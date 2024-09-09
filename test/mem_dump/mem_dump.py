#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#
# mem_dump.py
#

import argparse
import sst

DEBUG_L1 = 0
DEBUG_MEM = 0
DEBUG_LEVEL = 10
VERBOSE = 2
memSize = 1024 * 1024 * 10 - 1

# Setup argument parser
parser = argparse.ArgumentParser(description="Run Rev SST Simulation")
parser.add_argument(
    "--numCores", type=int, help="Number of Rev Cores per RevCPU", default=1
)
parser.add_argument(
    "--numHarts", type=int, help="Number of HARTs per Rev Core", default=1
)
parser.add_argument(
    "--program", help="The program executable to run in the simulation", default="a.out"
)
parser.add_argument(
    "--enableMemH",
    type=int,
    choices=[0, 1],
    help="Enable (1) or disable (0) memHierarchy backend",
    default=0,
)
parser.add_argument("--verbose", type=int, help="Verbosity level", default=2)
parser.add_argument(
    "--machine", help="Machine type/configuration", default="[CORES:RV64GC]"
)
parser.add_argument(
    "--args", help="Command line arguments to pass to the target executable", default=""
)
parser.add_argument(
    "--startSymbol", help="ELF Symbol Rev should begin execution at", default="[0:main]"
)

# Parse arguments
args = parser.parse_args()

# Print arguments nicely
print("Rev SST Simulation Configuration:")
for arg in vars(args):
    print("\t", arg, " = ", getattr(args, arg))

# SST core options and parameters
clock = "2.0GHz"

# Define the simulation components
comp_cpu = sst.Component("cpu", "revcpu.RevCPU")
comp_cpu.addParams(
    {
        "verbose": args.verbose,
        "numCores": args.numCores,
        "numHarts": args.numHarts,
        "clock": clock,
        "memSize": memSize,
        "machine": args.machine,
        "memCost": "[0:1:10]",
        "program": "basic.exe",
        "startAddr": "[0:0x00000000]",
        "startSymbol": args.startSymbol,
        "args": args.args,
        "splash": 1,
        "memDumpRanges": ["range1", "range2"],
        "range1.startAddr": 0x010000,
        "range1.size": 0x100000,
        "range2.startAddr": 0x090000,
        "range2.size": 0x100,
    }
)

sst.setStatisticOutput("sst.statOutputCSV")
sst.setStatisticLoadLevel(4)
sst.enableAllStatisticsForComponentType("revcpu.RevCPU")
