#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#
# rev-test-tracer.py
#

import argparse
import os
import sst
import sys

# Environment settings
sim_nodes = int(os.getenv('SIM_NODES', 1))
rev_exe = os.getenv("REV_EXE", "probe.exe")
arch = os.getenv("ARCH", "rv64g_zicntr").upper()
if (rev_exe == ""):
    print("ERROR: REV_EXE is not set", file=sys.stderr)
    exit(1)

print(f"SIM_NODES={sim_nodes}")
print(f"ARCH={arch}")
print(f"REV_EXE={rev_exe}")

# memh settings
DEBUG_MEM = 0
DEBUG_LEVEL = 0
VERBOSE = 2
MEM_SIZE = 1024*1024*1024-1

parser = argparse.ArgumentParser(description="debug probe demo")
parser.add_argument("--probeStartCycle", type=int, help="cycle to initiate debug probe. 0=Off", default=0)
parser.add_argument("--probeEndCycle", type=int, help="cycle to end debug probe. 0=Never", default=0)
parser.add_argument("--probeBufferSize", type=int, help="number of records in circular buffer", default=32)
parser.add_argument("--probePostDelay", type=int, help="number of events to capture after trigger event", default=32)
parser.add_argument("--probePort", type=int, help="sst probe starting socket. 0=None", default=0)
# parser.add_argument("--verbose", type=int, help="verbosity. 5=send/recv", default=1)
# 0b0100_0000 : 0x40 : 64 Every checkpoint
# 0b0010_0000 : 0x20 : 32 Every checkpoint when probe is active
# 0b0001_0000 : 0x10 : 16 Every checkpoint sync state change
# 0b0000_0100 : 0x04 : 04 Every probe sample
# 0b0000_0010 : 0x02 : 02 Every probe sample from trigger onward
# 0b0000_0001 : 0x01 : 01 Every probe state change,
parser.add_argument("--cliControl", type=int, help="event types on which to break into interactive mode"
                    " [64 Every checkpoint]"
                    " [32 Every checkpoint when probe is active]"
                    " [16 Every checkpoint sync state change]"
                    " [04 Every probe sample]"
                    " [02 Every probe sample from trigger onward]"
                    " [01 Every probe state change]",
                    default=0)
args = parser.parse_args()
print("debug probe demo configuration:")
for arg in vars(args):
    print("\t", arg, " = ", getattr(args, arg))

# Define SST core options
sst.setProgramOption("timebase", "1ps")

# Tell SST what statistics handling we want
sst.setStatisticLoadLevel(4)

# enable probe if start > 0
probeMode = 0
if args.probeStartCycle > 0:
    probeMode = 1
# Define the simulation components
# Instantiate all the CPUs
for i in range(0, sim_nodes):
    print("Building " + str(i))
    comp_cpu = sst.Component("cpu" + str(i), "revcpu.RevCPU")
    comp_cpu.addParams({
            "verbose": 5,                                # Verbosity
            "numCores": 2,                               # Number of cores
            "clock": "1.0GHz",                           # Clock
            "enableMemH": 1,
            "memSize": 1024*1024*1024,                   # Memory size in bytes
            "machine": f"[CORES:{arch}]",                # Core:Config; RV32I for all
            "startAddr": "[CORES:0x00000000]",           # Starting address for core 0
            "memCost": "[0:1:10]",                       # Memory loads required 1-10 cycles
            "program": rev_exe,                          # Target executable
            "splash": 1,                                 # Display the splash message
            # tracer controls
            # "trcOp": "slli",                           # base command for tracing [default: slli]
            # "trcLimit": 0,                             # Maximum number of trace lines [default: 0]
            "trcStartCycle": 1,                          # Starting trace cycle [default: 0]
            # debug probe controls
            "probeMode": probeMode,
            "probeStartCycle": args.probeStartCycle,
            "probeEndCycle": args.probeEndCycle,
            "probeBufferSize": args.probeBufferSize,
            "probePostDelay": args.probePostDelay,
            "probePort": args.probePort,
            "cliControl": args.cliControl,
            })
# comp_cpu.enableAllStatistics()

# Create the RevMemCtrl subcomponent
comp_lsq = comp_cpu.setSubComponent("memory", "revcpu.RevBasicMemCtrl")
comp_lsq.addParams({
      "verbose": "5",
      "clock": "2.0Ghz",
      "max_loads": 16,
      "max_stores": 16,
      "max_flush": 16,
      "max_llsc": 16,
      "max_readlock": 16,
      "max_writeunlock": 16,
      "max_custom": 16,
      "ops_per_cycle": 16
})
# comp_lsq.enableAllStatistics({"type":"sst.AccumulatorStatistic"})

iface = comp_lsq.setSubComponent("memIface", "memHierarchy.standardInterface")
iface.addParams({
      "verbose": VERBOSE
})


memctrl = sst.Component("memory", "memHierarchy.MemController")
memctrl.addParams({
    "debug": DEBUG_MEM,
    "debug_level": DEBUG_LEVEL,
    "clock": "2GHz",
    "verbose": VERBOSE,
    "addr_range_start": 0,
    "addr_range_end": MEM_SIZE,
    "backing": "malloc"
})

memory = memctrl.setSubComponent("backend", "memHierarchy.simpleMem")
memory.addParams({
    "access_time": "100ns",
    "mem_size": "8GB"
})

# sst.setStatisticOutput("sst.statOutputCSV")
# sst.enableAllStatisticsForAllComponents()

link_iface_mem = sst.Link("link_iface_mem")
link_iface_mem.connect((iface, "port", "50ps"), (memctrl, "direct_link", "50ps"))

# EOF
