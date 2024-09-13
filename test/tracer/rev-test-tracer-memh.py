#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#
# rev-test-tracer.py
#

import os
import sst
import sys

# Environment settings
sim_nodes = int(os.getenv('SIM_NODES', 1))
rev_exe = os.getenv("REV_EXE", "")
arch = os.getenv("ARCH", "RV32I").upper()
if (rev_exe == ""):
    print("ERROR: REV_EXE is not set", file=sys.stderr)
    exit(1)

print(f"SIM_NODES={sim_nodes}")
print(f"ARCH={arch}")
print(f"REV_EXE={rev_exe}")

# memh settings
DEBUG_MEM = 0
DEBUG_LEVEL = 10
VERBOSE = 2
memSize = 1024*1024*1024-1

# Define SST core options
sst.setProgramOption("timebase", "1ps")

# Tell SST what statistics handling we want
sst.setStatisticLoadLevel(4)

# Define the simulation components
# Instantiate all the CPUs
for i in range(0, sim_nodes):
    print("Building " + str(i))
    comp_cpu = sst.Component("cpu" + str(i), "revcpu.RevCPU")
    comp_cpu.addParams({
          "verbose": 5,                       # Verbosity
          "numCores": 1,                      # Number of cores
          "clock": "1.0GHz",                  # Clock
          "memSize": 1024*1024*1024,          # Memory size in bytes
          "machine": f"[CORES:{arch}]",       # Core:Config; RV32I for all
          "startAddr": "[CORES:0x00000000]",  # Starting address for core 0
          "memCost": "[0:1:10]",              # Memory loads required 1-10 cycles
          "program": rev_exe,                 # Target executable
          "enableMemH": 1,                    # Enable memHierarchy support
          "splash": 1,                        # Display the splash message
          # "trcOp": "slli",                  # base command for tracing [default: slli]
          # "trcLimit": 0,                    # Maximum number of trace lines [default: 0]
          # "trcStartCycle": 0                # Starting trace cycle [default: 0]
          })
#  comp_cpu.enableAllStatistics()

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
    "addr_range_end": memSize,
    "backing": "malloc"
})

memory = memctrl.setSubComponent("backend", "memHierarchy.simpleMem")
memory.addParams({
    "access_time": "100ns",
    "mem_size": "8GB"
})

# sst.setStatisticOutput("sst.statOutputCSV")
# sst.enableAllStatisticsForAllComponents()
# sst.enableAllStatisticsForAllComponents()

link_iface_mem = sst.Link("link_iface_mem")
link_iface_mem.connect((iface, "port", "50ps"), (memctrl, "direct_link", "50ps"))

# EOF
