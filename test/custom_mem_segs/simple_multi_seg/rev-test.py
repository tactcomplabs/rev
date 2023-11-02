#
# Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#
# rev-test-ex1.py
#

import os
import sst

# Define SST core options
sst.setProgramOption("timebase", "1ps")

# Tell SST what statistics handling we want
sst.setStatisticLoadLevel(4)

max_addr_gb = 1

# Define the simulation components
comp_cpu = sst.Component("cpu", "revcpu.RevCPU")
comp_cpu.addParams({
        "verbose" : 6,                                # Verbosity
        "numCores" : 1,                               # Number of cores
        "clock" : "1.0GHz",                           # Clock
        "memSize" : 1024*1024*1024,                   # Memory size in bytes
        "machine" : "[0:RV64G]",                      # Core:Config; RV64I for core 0
        "customMemSegs": "[scratchpad, mem1, mem2]", # Custom memory segments
        "scratchpad.size" : 1024*1024*1024,                    # Size of the scratchpad
        "scratchpad.baseAddr" : "0x80000000",            # Starting address of the scratchpad
        "mem1.size" : 1024*1024*1024,                    # Size of the scratchpad
        "mem1.baseAddr" : "0x80000000",            # Starting address of the scratchpad
        "mem2.size" : 1024*1024*1024,                    # Size of the scratchpad
        "mem2.baseAddr" : "0x80000000",            # Starting address of the scratchpad
        "startAddr" : "[0:0x00000000]",               # Starting address for core 0
        "memCost" : "[0:1:10]",                       # Memory loads required 1-10 cycles
        "program" : os.getenv("REV_EXE", "basic-scratchpad.exe"),  # Target executable
        "splash" : 1                                  # Display the splash message
})

sst.setStatisticOutput("sst.statOutputCSV")
sst.enableAllStatisticsForAllComponents()

# EOF
