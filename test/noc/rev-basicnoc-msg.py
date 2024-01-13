#
# Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#
# rev-basicnic.py
#

import os
import sst

# Define SST core options
sst.setProgramOption("timebase", "1ps")

DEBUG_L1 = 0
DEBUG_MEM = 0
DEBUG_LEVEL = 10
VERBOSE = 2
MEM_SIZE = 1024*1024*1024-1

# --------------------------
# SETUP THE ZAP
# --------------------------
host_cpu0 = sst.Component("host0", "revcpu.RevCPU")
host_cpu0.addParams({
    "verbose" : 6,                                # Verbosity
    "numCores" : 4,                               # Number of cores
    "clock" : "1.0GHz",                           # Clock
    "memSize" : 1024*1024*1024,                   # Memory size in bytes
    "machine" : "[CORES:RV64GC]",                 # Core:Config; RV64I for core 0
    "startAddr" : "[0:0x00000000]",               # Starting address for core 0
    "memCost" : "[0:1:10]",                       # Memory loads required 1-10 cycles
    "program" : "basicnoc_msg.exe",               # Target executable
    "enable_noc": 1,                              # Enable the noc interfaces for each core
    "splash" : 0                                  # Display the splash message
})

host_cpu1 = sst.Component("host1", "revcpu.RevCPU")
host_cpu1.addParams({
    "verbose" : 6,                                # Verbosity
    "numCores" : 1,                               # Number of cores
    "clock" : "1.0GHz",                           # Clock
    "memSize" : 1024*1024*1024,                   # Memory size in bytes
    "machine" : "[0:RV64GC]",                     # Core:Config; RV64I for core 0
    "startAddr" : "[0:0x00000000]",               # Starting address for core 0
    "memCost" : "[0:1:10]",                       # Memory loads required 1-10 cycles
    "program" : "basicnoc_msg.exe",               # Target executable
    "enable_noc": 1,                              # enable the noc interfaces for each core
    "splash" : 0                                  # Display the splash message
})

# --------------------------
# SETUP THE NETWORK
# --------------------------
noc_nic_params = {
  "verbose" : 9,
  "clock" : "1GHz",
  "req_per_cycle" : 1
}
noc_params = {
  "input_buf_size" : "2048B",
  "output_buf_size" : "2048B",
  "link_bw" : "100GB/s"
}
noc_rtr_params = {
  "xbar_bw" : "100GB/s",
  "flit_size" : "8B",
  "num_ports" : "5",
  "id" : 0,
  "debug": 1
}

host0_core0_noc = host_cpu0.setSubComponent("noc0", "revcpu.RevNOC")
host_core0_iface = host0_core0_noc.setSubComponent("iface", "merlin.linkcontrol")
host0_core1_noc = host_cpu0.setSubComponent("noc1", "revcpu.RevNOC")
host_core1_iface = host0_core1_noc.setSubComponent("iface", "merlin.linkcontrol")
host0_core2_noc = host_cpu0.setSubComponent("noc2", "revcpu.RevNOC")
host_core2_iface = host0_core2_noc.setSubComponent("iface", "merlin.linkcontrol")
host0_core3_noc = host_cpu0.setSubComponent("noc3", "revcpu.RevNOC")
host_core3_iface = host0_core3_noc.setSubComponent("iface", "merlin.linkcontrol")

host_noc1 = host_cpu1.setSubComponent("noc0", "revcpu.RevNOC")
host_iface1 = host_noc1.setSubComponent("iface", "merlin.linkcontrol")

router = sst.Component("router", "merlin.hr_router")
router.setSubComponent("topology", "merlin.singlerouter")

host0_core0_noc.addParams(noc_nic_params)
host_core0_iface.addParams(noc_params)
host0_core1_noc.addParams(noc_nic_params)
host_core1_iface.addParams(noc_params)
host0_core2_noc.addParams(noc_nic_params)
host_core2_iface.addParams(noc_params)
host0_core3_noc.addParams(noc_nic_params)
host_core3_iface.addParams(noc_params)
host_noc1.addParams(noc_nic_params)
host_iface1.addParams(noc_params)
router.addParams(noc_params)
router.addParams(noc_rtr_params)

# --------------------------
# LINK THE VARIOUS COMPONENTS
# --------------------------
host_core0_link = sst.Link("host_core0_link")
host_core0_link.connect( (host_core0_iface, "rtr_port", "1us"), (router, "port0", "1us"))
host_core1_link = sst.Link("host_core1_link")
host_core1_link.connect( (host_core1_iface, "rtr_port", "1us"), (router, "port1", "1us"))
host_core2_link = sst.Link("host_core2_link")
host_core2_link.connect( (host_core2_iface, "rtr_port", "1us"), (router, "port2", "1us"))
host_core3_link = sst.Link("host_core3_link")
host_core3_link.connect( (host_core3_iface, "rtr_port", "1us"), (router, "port3", "1us"))

host_link1 = sst.Link("host1_link")
host_link1.connect( (host_iface1, "rtr_port", "1us"), (router, "port4", "1us"))

# EOF
