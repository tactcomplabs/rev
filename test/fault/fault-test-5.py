#
# Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#
# fault-test-5.py
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
        "verbose" : 5,                                # Verbosity
        "numCores" : 1,                               # Number of cores
        "clock" : "1.0GHz",                           # Clock
        "memSize" : 1024*1024*1024,                   # Memory size in bytes
        "machine" : "[0:RV64G]",                      # Core:Config; RV64I for core 0
        "startAddr" : "[0:0x00000000]",               # Starting address for core 0
        "memCost" : "[0:1:10]",                       # Memory loads required 1-10 cycles
        "program" : "fault1.exe",                     # Target executable

        "enable_faults" : 1,                          # Enable the fault interfaces
        "faults" : "alu",                             # Enable the decode faults
        "fault_width" : "word",                       # word flips flips
        "fault_range" : 65536,                        # clocks between faults

        "splash" : 1                                  # Display the splash message
})

sst.setStatisticOutput("sst.statOutputCSV")
sst.enableAllStatisticsForAllComponents()

# EOF
