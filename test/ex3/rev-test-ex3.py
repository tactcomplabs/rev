#
# Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#
# rev-test-ex3.py
#

import os
import sst

# Define SST core options
sst.setProgramOption("timebase", "1ps")

# Tell SST what statistics handling we want
sst.setStatisticLoadLevel(4)

max_addr_gb = 1

# Define the simulation components
comp_cpu0 = sst.Component("cpu0", "revcpu.RevCPU")
comp_cpu0.addParams({
        "verbose" : 6,                                # Verbosity
        "numCores" : 1,                               # Number of cores
        "clock" : "1.0GHz",                           # Clock
        "memSize" : 1024*1024*1024,                   # Memory size in bytes
        "machine" : "[0:RV32I]",                      # Core:Config; RV32I for core 0
        "startAddr" : "[0:0x000]",               # Starting address for core 0
        "memCost" : "[0:1:10]",                       # Memory loads required 1-10 cycles
        "program" : os.getenv("REV_EXE", "ex3.exe"),  # Target executable
        "nic" : "revcpu.RevNIC",
        "enable_nic" : 1,                             # Enable the internal RevNIC
        "splash" : 1                                  # Display the splash message
})

comp_cpu1 = sst.Component("cpu1", "revcpu.RevCPU")
comp_cpu1.addParams({
        "verbose" : 6,                                # Verbosity
        "numCores" : 1,                               # Number of cores
        "clock" : "1.0GHz",                           # Clock
        "memSize" : 1024*1024*1024,                   # Memory size in bytes
        "machine" : "[0:RV32I]",                      # Core:Config; RV32I for core 0
        "startAddr" : "[0:0x00010144]",               # Starting address for core 0
        "memCost" : "[0:1:10]",                       # Memory loads required 1-10 cycles
        "program" : os.getenv("REV_EXE", "ex3.exe"),  # Target executable
        "nic" : "revcpu.RevNIC",
        "enable_nic" : 1,                             # Enable the internal RevNIC
        "splash" : 1                                  # Display the splash message
})

# setup the NICs
nic0 = comp_cpu0.setSubComponent("nic", "revcpu.RevNIC")
nic1 = comp_cpu1.setSubComponent("nic", "revcpu.RevNIC")

iface0 = nic0.setSubComponent("iface", "merlin.linkcontrol")
iface1 = nic1.setSubComponent("iface", "merlin.linkcontrol")

# setup the router
router = sst.Component("router", "merlin.hr_router")
router.setSubComponent("topology", "merlin.singlerouter")

# set the network params
verb_params = { "verbose" : 6 }
net_params = {
  "input_buf_size" : "512B",
  "output_buf_size" : "512B",
  "link_bw" : "1GB/s"
}

nic0.addParams(verb_params)
nic1.addParams(verb_params)
iface0.addParams(net_params)
iface1.addParams(net_params)
router.addParams(net_params)

router.addParams({
    "xbar_bw" : "1GB/s",
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
