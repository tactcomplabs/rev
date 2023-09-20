#
# Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#
# rev-test-memh1.py
#

import os
import sst

DEBUG_L1 = 0
DEBUG_MEM = 0
DEBUG_LEVEL = 10
VERBOSE = 2
MEM_SIZE = 1024*1024*1024-1

# Define the simulation components
comp_cpu = sst.Component("cpu", "revcpu.RevCPU")
comp_cpu.addParams({
	"verbose" : 6,                                # Verbosity
        "numCores" : 1,                               # Number of cores
	"clock" : "2.0GHz",                           # Clock
        "memSize" : MEM_SIZE,                         # Memory size in bytes
        "machine" : "[0:RV64IMAFDC]",                      # Core:Config; RV32I for core 0
        "startAddr" : "[0:0x00000000]",               # Starting address for core 0
        "memCost" : "[0:1:10]",                       # Memory loads required 1-10 cycles
        "program" : os.getenv("REV_EXE", "file-io.exe"),  # Target executable
        "enable_memH" : 1,                            # Enable memHierarchy support
        "splash" : 1                                  # Display the splash message
})
comp_cpu.enableAllStatistics()

# Create the RevMemCtrl subcomponent
comp_lsq = comp_cpu.setSubComponent("memory", "revcpu.RevBasicMemCtrl");
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


#l1cache = sst.Component("l1cache", "memHierarchy.Cache")
#l1cache.addParams({
#    "access_latency_cycles" : "4",
#    "cache_frequency" : "2 Ghz",
#    "replacement_policy" : "lru",
#    "coherence_protocol" : "MSI",
#    "associativity" : "4",
#    "cache_line_size" : "64",
#    "debug" : DEBUG_L1,
#    "debug_level" : DEBUG_LEVEL,
#    "verbose" : VERBOSE,
#    "L1" : "1",
#    "cache_size" : "16KiB"
#})

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

#sst.setStatisticLoadLevel(7)
#sst.setStatisticOutput("sst.statOutputConsole")
#sst.enableAllStatisticsForAllComponents()

#link_cpu_cache_link = sst.Link("link_cpu_cache_link")
#link_cpu_cache_link.connect( (iface, "port", "1000ps"), (l1cache, "high_network_0", "1000ps") )
#link_mem_bus_link = sst.Link("link_mem_bus_link")
#link_mem_bus_link.connect( (l1cache, "low_network_0", "50ps"), (memctrl, "direct_link", "50ps") )

link_iface_mem = sst.Link("link_iface_mem")
link_iface_mem.connect( (iface, "port", "50ps"), (memctrl, "direct_link", "50ps") )

# EOF
