#
# Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#
# rev-test-cache2.py
#

import os
import sst

DEBUG_L1 = 1
DEBUG_MEM = 10
DEBUG_LEVEL = 20
VERBOSE = 20
MEM_SIZE = 1024*1024*1024-1

# Define the simulation components
comp_cpu = sst.Component("cpu", "revcpu.RevCPU")
comp_cpu.addParams({
        "debug" : 1,
        "debug_level" : DEBUG_LEVEL,
        "verbose" : VERBOSE,
        "numCores" : 2,                               # Number of cores
        "numHarts" : 2,
        "clock" : "2.0GHz",                           # Clock
        "memSize" : MEM_SIZE,                         # Memory size in bytes
	"machine": "[CORES:RV64IMAFD]",
        "startAddr" : "[0:0x000100b0]",               # Starting address for core 0
        "memCost" : "[0:1:10,1:1:10]",                       # Memory loads required 1-10 cycles
        "program" : os.getenv("REV_EXE", "pthread-basic.exe"),  # Target executable
        "enable_memH" : 1,                            # Enable memHierarchy support
        "splash" : 1                                  # Display the splash message
})
comp_cpu.enableAllStatistics()

# Create the RevMemCtrl subcomponent
comp_lsq = comp_cpu.setSubComponent("memory", "revcpu.RevBasicMemCtrl");
comp_lsq.addParams({
      "debug_level" : DEBUG_LEVEL,
      "debug" : 1,
      "verbose" : VERBOSE,
      "clock"           : "2.0Ghz",
      "max_loads"       : 64,
      "max_stores"      : 64,
      "max_flush"       : 64,
      "max_llsc"        : 64,
      "max_readlock"    : 64,
      "max_writeunlock" : 64,
      "max_custom"      : 64,
      "ops_per_cycle"   : 64,
      "iface_ports"     : 2,      #-- number of ifaces
      "harts_per_port"  : 1       #-- number of harts per port
})
comp_lsq.enableAllStatistics({"type":"sst.AccumulatorStatistic"})

iface0 = comp_lsq.setSubComponent("memIface0", "memHierarchy.standardInterface")
iface0.addParams({
      "verbose" : VERBOSE,
      "debug" : 1,
      "debug_level" : DEBUG_LEVEL,
})
iface1 = comp_lsq.setSubComponent("memIface1", "memHierarchy.standardInterface")
iface1.addParams({
      "verbose" : VERBOSE,
      "debug" : 1,
      "debug_level" : DEBUG_LEVEL,
})



l1cache0 = sst.Component("l1cache0", "memHierarchy.Cache")
l1cache0.addParams({
    "access_latency_cycles" : "4",
    "cache_frequency" : "2 Ghz",
    "replacement_policy" : "lru",
    "coherence_protocol" : "MESI",
    "associativity" : "4",
    "cache_line_size" : "64",
    "debug" : 1,
    "debug_level" : DEBUG_LEVEL,
    "verbose" : VERBOSE,
    "L1" : "1",
    "cache_size" : "16KiB"
})

l1cache1 = sst.Component("l1cache1", "memHierarchy.Cache")
l1cache1.addParams({
    "access_latency_cycles" : "4",
    "cache_frequency" : "2 Ghz",
    "replacement_policy" : "lru",
    "coherence_protocol" : "MESI",
    "associativity" : "4",
    "cache_line_size" : "64",
    "debug" : 1,
    "debug_level" : DEBUG_LEVEL,
    "verbose" : VERBOSE,
    "L1" : "1",
    "cache_size" : "16KiB"
})

bus = sst.Component("bus0", "memHierarchy.Bus")
bus.addParams({
  "bus_frequency" : "2 Ghz"
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

#sst.setStatisticLoadLevel(7)
#sst.setStatisticOutput("sst.statOutputConsole")
#sst.enableAllStatisticsForAllComponents()

#-- connect each memory port to a unique L1 cache
link0 = sst.Link("memport0")
link0.connect( (iface0, "port", "1ns"), (l1cache0, "high_network_0", "1ns") )
link1 = sst.Link("memport1")
link1.connect( (iface1, "port", "1ns"), (l1cache1, "high_network_0", "1ns") )

#-- connect the L1 caches to a bus high network
link2 = sst.Link("buslink0")
link2.connect( (l1cache0, "low_network_0", "1ns"), (bus, "high_network_0", "1ns") )
link3 = sst.Link("buslink1")
link3.connect( (l1cache1, "low_network_0", "1ns"), (bus, "high_network_1", "1ns") )

#-- conenct the bus to memory
link4 = sst.Link("busmem0")
link4.connect( (bus, "low_network_0", "1ns"), (memctrl, "direct_link", "1ns") )

#link2.connect( (l1cache, "low_network_0", "1ns"), (memctrl, "direct_link", "1ns") )

# EOF
