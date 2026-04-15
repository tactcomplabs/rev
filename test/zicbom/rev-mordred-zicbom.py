#
# Copyright (C) 2017-2026 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#
# rev-test-zicbom.py
#

import os
import sst
from sst import UnitAlgebra

DEBUG_L1 = 1
DEBUG_MEM = 10
DEBUG_LEVEL = 10
VERBOSE = 10
memSize = 1024*1024*1024-1

# Parameters for Mordred
# Set up some parameters via UnitAlgebra
clk = UnitAlgebra("2GHz")
clk_pd = clk.invert()
link_latency = UnitAlgebra(0.8) * clk_pd
flit_size = UnitAlgebra("16b")

FixedRtrParams = {
    "verbose": 0,
    "clock": clk,
    "num_vcs": "1",
    "num_vns": "1",
    "flit_size": flit_size,
    "input_buf_size": UnitAlgebra(16)*flit_size,
    "output_buf_size": UnitAlgebra(1)*flit_size
}

MordredNICParams = {
    "verbose": 0,
    "clock": clk,
    "input_buf_size": "1kiB",
    "output_buf_size": "1kiB",
}


# Define the simulation components
comp_cpu = sst.Component("cpu", "revcpu.RevCPU")
comp_cpu.addParams({
        "verbose": 10,                                  # Verbosity
        "numCores": 1,                                  # Number of cores
        "clock": "2.0GHz",                              # Clock
        "memSize": memSize,                             # Memory size in bytes
        "machine": "[0:RV64GC_Zicbom]",                 # Core:Config; RV64G for core 0
        "startAddr": "[0:0x00000000]",                  # Starting address for core 0
        "memCost": "[0:1:10]",                          # Memory loads required 1-10 cycles
        "program": os.getenv("REV_EXE", "zicbom.exe"),  # Target executable
        "enableMemH": 1,                                # Enable memHierarchy support
        "splash": 1                                     # Display the splash message
})
comp_cpu.enableAllStatistics()

# Create the RevMemCtrl subcomponent
comp_lsq = comp_cpu.setSubComponent("memory", "revcpu.RevBasicMemCtrl")
comp_lsq.addParams({
      "verbose": "10",
      "clock": "2.0Ghz",
      "max_loads": 64,
      "max_stores": 64,
      "max_flush": 64,
      "max_llsc": 64,
      "max_readlock": 64,
      "max_writeunlock": 64,
      "max_custom": 64,
      "ops_per_cycle": 64
})
comp_lsq.enableAllStatistics({"type": "sst.AccumulatorStatistic"})

iface = comp_lsq.setSubComponent("memIface", "memHierarchy.standardInterface")
iface.addParams({
      "verbose": VERBOSE
})

l1cache = sst.Component("l1cache", "memHierarchy.Cache")
l1cache.addParams({
    "access_latency_cycles": "4",
    "cache_frequency": "2 Ghz",
    "replacement_policy": "lru",
    "coherence_protocol": "MESI",
    "associativity": "4",
    "cache_line_size": "64",
    "debug": 1,
    "debug_level": DEBUG_LEVEL,
    "verbose": VERBOSE,
    "L1": "1",
    "cache_size": "16KiB"
})

# Put the mordred network between the cache and the memory controller

# Set up the components from the cache to the router
# Fill Cache lowlink subcomponent with memH.memNIC
cache_nic = l1cache.setSubComponent("lowlink", "memHierarchy.MemNIC")
cache_nic.addParams({
    "group": 1,  # Here's the miracle setting to make this work.
    # "debug": 1,
    # "debug_level": 10,
})

# Fill memH.memNIC.linkcontrol slot with mordred.mordredNIC
cache_nic_iface = cache_nic.setSubComponent("linkcontrol", "mordred.mordredNIC")
cache_nic_iface.addParams(MordredNICParams)

# Add the mordred.router
rtr = sst.Component("rtr0", "mordred.simple_rtr")
rtr.addParams({
    "id": 0,
    "num_ports": 6,
    "num_local_ports": 2
})
rtr.addParams(FixedRtrParams)
rtr_topo = rtr.setSubComponent("topology", "mordred.MeshTopology")
rtr_topo.addParams({
    "verbose": 1,
    "xDim": 1,
    "yDim": 1
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

# Now we add the subcomponents to get the MemController to talk to the router
# Fill highlink slot of MemController
memcpulink = memctrl.setSubComponent("highlink", "memHierarchy.MemNIC")
memcpulink.addParam("group", 2)
# Add params if necessary

# Fill linkcontrol slot of MemNIC
mem_mordredlink = memcpulink.setSubComponent("linkcontrol", "mordred.mordredNIC")
mem_mordredlink.addParams(MordredNICParams)

# sst.setStatisticLoadLevel(7)
# sst.setStatisticOutput("sst.statOutputConsole")
# sst.enableAllStatisticsForAllComponents()

# This is the CPU to L1 cache link
link0 = sst.Link("link0")
link0.connect((iface, "lowlink", "1ns"), (l1cache, "highlink", "1ns"))

# This is the L1 cache to router link
link1 = sst.Link("link1")
link1.connect((cache_nic_iface, "port", "50ps"), (rtr, "port4", "50ps"))

# This is the router to MemController link
link2 = sst.Link("link2")
link2.connect((rtr, "port5", "1ns"), (mem_mordredlink, "port", "50ps"))

# EOF
