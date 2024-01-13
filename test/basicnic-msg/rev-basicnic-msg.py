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
        "verbose" : 9,                                # Verbosity
        "numCores" : 1,                               # Number of cores
        "clock" : "1.0GHz",                           # Clock
        "memSize" : 1024*1024*1024,                   # Memory size in bytes
        "machine" : "[0:RV64GC]",                     # Core:Config; RV64I for core 0
        "startAddr" : "[0:0x00000000]",               # Starting address for core 0
        "memCost" : "[0:1:10]",                       # Memory loads required 1-10 cycles
        "program" : "basicnic-msg.exe",  # Target executable
        "enable_nic" : "1",                           # enable RevNIC
        "networkID" : "0",                            # logical network ID
        "splash" : 0                                  # Display the splash message
})

host_cpu1 = sst.Component("host1", "revcpu.RevCPU")
host_cpu1.addParams({
        "verbose" : 9,                                # Verbosity
        "numCores" : 1,                               # Number of cores
        "clock" : "1.0GHz",                           # Clock
        "memSize" : 1024*1024*1024,                   # Memory size in bytes
        "machine" : "[0:RV64GC]",                     # Core:Config; RV64I for core 0
        "startAddr" : "[0:0x00000000]",               # Starting address for core 0
        "memCost" : "[0:1:10]",                       # Memory loads required 1-10 cycles
        "program" : "basicnic-msg.exe",  # Target executable
        "enable_nic" : "1",                           # enable RevNIC
        "networkID" : "1",                            # logical network ID
        "splash" : 0                                  # Display the splash message
})

# --------------------------
# SETUP THE NETWORK
# --------------------------
nic_params = {
  "verbose" : 9,
  "clock" : "1GHz",
  "req_per_cycle" : 1
}
net_params = {
  "input_buf_size" : "2048B",
  "output_buf_size" : "2048B",
  "link_bw" : "100GB/s"
}
rtr_params = {
  "xbar_bw" : "100GB/s",
  "flit_size" : "8B",
  "num_ports" : "2",
  "id" : 0
}
host_nic0 = host_cpu0.setSubComponent("nic", "revcpu.RevNIC")
host_iface0 = host_nic0.setSubComponent("iface", "merlin.linkcontrol")

host_nic1 = host_cpu1.setSubComponent("nic", "revcpu.RevNIC")
host_iface1 = host_nic1.setSubComponent("iface", "merlin.linkcontrol")

router = sst.Component("router", "merlin.hr_router")
router.setSubComponent("topology", "merlin.singlerouter")

host_nic0.addParams(nic_params)
host_iface0.addParams(net_params)
host_nic1.addParams(nic_params)
host_iface1.addParams(net_params)
router.addParams(net_params)
router.addParams(rtr_params)

# --------------------------
# LINK THE VARIOUS COMPONENTS
# --------------------------
host_link0 = sst.Link("host0_link")
host_link0.connect( (host_iface0, "rtr_port", "1us"), (router, "port0", "1us") )

host_link1 = sst.Link("host1_link")
host_link1.connect( (host_iface1, "rtr_port", "1us"), (router, "port1", "1us") )

# EOF
