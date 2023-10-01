#
# Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#
# rev-pan-test.py
#
# ---------------------------------------------------------------
#
#    host0                  pan1
#      |                     |
#     nic0                  nic1
#      |                     |
#    iface0 <-> router <-> iface1
#                  |
#               topology
#
#   <-> is a link
#   | is a subcomponent relationship
#

import os
import sst

# Define SST core options
sst.setProgramOption("timebase", "1ps")

# Tell SST what statistics handling we want
sst.setStatisticLoadLevel(4)

max_addr_gb = 1

# Define the simulation components
#-- HOST CPU
host_cpu0 = sst.Component("cpu0", "revcpu.RevCPU")
host_cpu0.addParams({
        "verbose" : 5,                                # Verbosity
        "numCores" : 1,                               # Number of cores
        "clock" : "1.0GHz",                           # Clock
        "memSize" : 1024*1024*1024,                   # Memory size in bytes
        "machine" : "[0:RV64GC]",                     # Core:Config; RV64G for core 0
        "startAddr" : "[0:0x00000000]",               # Starting address for core 0
        "memCost" : "[0:1:10]",                       # Memory loads required 1-10 cycles
        "program" : os.getenv("REV_EXE", "pan_test.exe"),  # Target executable
        "pan_nic" : "revcpu.PanNet",                  # Use the PAN NIC
        "enable_pan" : 1,                             # Enable the internal RevNIC
        "enable_test" : 0,                            # Disable the PAN test harness
        "enable_pan_stats" : 1,                       # Enable the PAN statistics
        "testIters" : 10,                             # Number of command packets for each test
        "msgPerCycle" : 5,                            # Number of messages per cycle
        "RDMAPerCycle" : 1,                           # Number of RDMA messages to flush to network per cycle
        "splash" : 1                                  # Display the splash message
})

#-- PAN CPU
pan_cpu1 = sst.Component("cpu1", "revcpu.RevCPU")
pan_cpu1.addParams({
        "verbose" : 5,                                # Verbosity
        "numCores" : 1,                               # Number of cores
        "clock" : "1.0GHz",                           # Clock
        "memSize" : 1024*1024*1024,                   # Memory size in bytes
        "machine" : "[0:RV64GP]",                     # Core:Config; RV64GP for core 0
        "startAddr" : "[0:0x00000000]",               # Starting address for core 0
        "memCost" : "[0:1:10]",                       # Memory loads required 1-10 cycles
        "program" : os.getenv("REV_EXE", "pan_spin.exe"),  # Target executable
        "pan_nic" : "revcpu.PanNet",                  # Use the PAN NIC
        "enable_pan" : 1,                             # Enable the internal RevNIC
        "enable_test" : 0,                            # Disable the PAN test harness
        "enable_pan_stats" : 1,                       # Enable the PAN statistics
        "msgPerCycle" : 5,                            # Number of messages per cycle
        "RDMAPerCycle" : 1,                           # Number of RDMA messages to flush to network per cycle
        "splash" : 1                                  # Display the splash message
})

# setup the NICs
nic0 = host_cpu0.setSubComponent("pan_nic", "revcpu.PanNet")  #-- host
nic1 = pan_cpu1.setSubComponent("pan_nic", "revcpu.PanNet")  #-- pan

iface0 = nic0.setSubComponent("iface", "merlin.linkcontrol")  #-- host
iface1 = nic1.setSubComponent("iface", "merlin.linkcontrol")  #-- pan

# setup the router
router = sst.Component("router", "merlin.hr_router")
router.setSubComponent("topology", "merlin.singlerouter")

# set the network params
#-- host
host_verb_params = {
  "verbose" : 6,
  "host_device" : 1
}

#-- pan
pan_verb_params = {
  "verbose" : 6,
  "host_device" : 0
}

#-- host network config
host_net_params = {
  "input_buf_size" : "512B",
  "output_buf_size" : "512B",
  "link_bw" : "1GB/s"
}

#-- pan network config
pan_net_params = {
  "input_buf_size" : "512B",
  "output_buf_size" : "512B",
  "link_bw" : "10GB/s"
}

nic0.addParams(host_verb_params) #-- host
nic1.addParams(pan_verb_params) #-- pan

iface0.addParams(host_net_params) #-- host
iface1.addParams(pan_net_params)  #-- pan
router.addParams(pan_net_params)  #-- pan router

router.addParams({
    "xbar_bw" : "10GB/s",
    "flit_size" : "32B",
    "num_ports" : "2",
    "id" : 0
})

# setup the links
link0 = sst.Link("link0")
link0.connect( (iface0, "rtr_port", "1ms"), (router, "port0", "1ms") )

link1 = sst.Link("link1")
link1.connect( (iface1, "rtr_port", "1ms"), (router, "port1", "1ms") )


sst.setStatisticOutput("sst.statOutputCSV")
sst.enableAllStatisticsForAllComponents()

# EOF
