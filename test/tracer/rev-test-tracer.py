#
# Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#
# rev-test-tracer.py
#

import os
import sst

# Environment settings
sim_nodes = int(os.getenv('SIM_NODES', 1))
rev_exe = os.getenv("REV_EXE", "")
arch = os.getenv("ARCH","RV32I").upper()
if ( rev_exe == "" ):
  print("ERROR: REV_EXE is not set",file=sys.stderr);
  exit(1);

print(f"SIM_NODES={sim_nodes}")
print(f"ARCH={arch}")
print(f"REV_EXE={rev_exe}")

# Define SST core options
sst.setProgramOption("timebase", "1ps")

# Tell SST what statistics handling we want
sst.setStatisticLoadLevel(4)

# Define the simulation components
# Instantiate all the CPUs
for i in range(0, sim_nodes):
  print("Building "+ str(i))
  comp_cpu = sst.Component("cpu" + str(i), "revcpu.RevCPU")
  comp_cpu.addParams({
	"verbose" : 5,                                # Verbosity
        "numCores" : 1,                               # Number of cores
	"clock" : "1.0GHz",                           # Clock
        "memSize" : 1024*1024*1024,                   # Memory size in bytes
        "machine" : f"[CORES:{arch}]",                  # Core:Config; RV32I for all
        "startAddr" : "[CORES:0x00000000]",           # Starting address for core 0
        "memCost" : "[0:1:10]",                       # Memory loads required 1-10 cycles
        "program" : rev_exe,                          # Target executable
        "splash" : 1,                                 # Display the splash message
        # "trcOp": "slli",                            # base command for tracing [default: slli]
        # "trcLimit": 0,                              # Maximum number of trace lines [default: 0]
        # "trcStartCycle" : 0                         # Starting trace cycle [default: 0]
        })
#  comp_cpu.enableAllStatistics()

# sst.setStatisticOutput("sst.statOutputCSV")
# sst.enableAllStatisticsForAllComponents()

# EOF
