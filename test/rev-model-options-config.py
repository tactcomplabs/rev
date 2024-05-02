#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#
# rev-basic-config.py
#

import os
import argparse
import sst

DEBUG_L1 = 0
DEBUG_MEM = 0
DEBUG_LEVEL = 10
VERBOSE = 2
MEM_SIZE = 1024*1024*1024-1

# Setup argument parser
parser = argparse.ArgumentParser(description="Run Rev SST Simulation")
parser.add_argument("--numCores", type=int, help="Number of Rev Cores per RevCPU", default=1)
parser.add_argument("--numHarts", type=int, help="Number of HARTs per Rev Core", default=1)
parser.add_argument("--program", help="The program executable to run in the simulation", default="a.out")
parser.add_argument("--enableMemH", type=int, choices=[0, 1], help="Enable (1) or disable (0) memHierarchy backend", default=0)
parser.add_argument("--verbose", type=int, help="Verbosity level", default=2)
parser.add_argument("--machine", help="Machine type/configuration", default="[CORES:RV64GC]")
parser.add_argument("--args", help="Command line arguments to pass to the target executable", default="")
parser.add_argument("--startSymbol", help="ELF Symbol Rev should begin execution at", default="[0:main]")

# Parse arguments
args = parser.parse_args()

# Print arguments nicely
print("Rev SST Simulation Configuration:")
for arg in vars(args):
    print("\t", arg, " = ", getattr(args, arg))

# SST core options and parameters
mem_size = 1024*1024*1024-1
clock = "2.0GHz"

# Define the simulation components
comp_cpu = sst.Component("cpu", "revcpu.RevCPU")
comp_cpu.addParams({
    "verbose" : args.verbose,
    "numCores" : args.numCores,
    "numHarts" : args.numHarts,
    "clock" : clock,
    "memSize" : mem_size,
    "machine" : args.machine,
    "memCost" : "[0:1:10]",
    "program" : args.program,
    "startAddr" : "[0:0x00000000]",
    "startSymbol" : args.startSymbol,
    "enable_memH" : args.enableMemH,
    "args": args.args,
    "splash" : 1
})

sst.setStatisticOutput("sst.statOutputCSV")
sst.setStatisticLoadLevel(4)
sst.enableAllStatisticsForComponentType("revcpu.RevCPU")

# Conditional setup for memory hierarchy
if args.enableMemH:
    # Create the RevMemCtrl subcomponent
    comp_lsq = comp_cpu.setSubComponent("memory", "revcpu.RevBasicMemCtrl")
    comp_lsq.addParams({
        "verbose"         : "5",
        "clock"           : "2.0Ghz",
        "max_loads"       : 16,
        "max_stores"      : 16,
        "max_flush"       : 16,
        "max_llsc"        : 16,
        "max_readlock"    : 16,
        "max_writeunlock" : 16,
        "max_custom"      : 16,
        "ops_per_cycle"   : 16
    })
    comp_lsq.enableAllStatistics({"type":"sst.AccumulatorStatistic"})

    iface = comp_lsq.setSubComponent("memIface", "memHierarchy.standardInterface")
    iface.addParams({
        "verbose" : VERBOSE
    })
    memctrl = sst.Component("memory", "memHierarchy.MemController")
    memctrl.addParams({
        "debug" : DEBUG_MEM,
        "debug_level" : DEBUG_LEVEL,
        "clock" : "2GHz",
        "verbose" : VERBOSE,
        "addr_range_start" : 0,
        "addr_range_end" : MEM_SIZE,
        "backing" : "malloc"
    })

    memory = memctrl.setSubComponent("backend", "memHierarchy.simpleMem")
    memory.addParams({
        "access_time" : "100ns",
        "mem_size" : "8GB"
    })

    link_iface_mem = sst.Link("link_iface_mem")
    link_iface_mem.connect( (iface, "port", "50ps"), (memctrl, "direct_link", "50ps") )



    # Setup for memHierarchy backend
    # ... (Include your memHierarchy setup here)
    # TODO:
# else:
