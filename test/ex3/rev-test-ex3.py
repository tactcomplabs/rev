#
# Copyright (C) 2017-2020 Tactical Computing Laboratories, LLC
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
sst.setProgramOption("stopAtCycle", "0s")

# Tell SST what statistics handling we want
sst.setStatisticLoadLevel(4)

max_addr_gb = 1

# Define the network
router = sst.Component("router", "merlin.hr_router")
router.addParams({
        "id" : 0,                                     # Router id
        "output_latency" : "25ps",                    # Router output latency
        "xbar_bw" : "96GB/s",                         # Router crossbar bandwidth
        "input_buf_size" : "2KB",                     # Input buffer size
        "input_latency" : "25ps",                     # Input latency
        "num_ports" : 1,                              # Number of ports
        "flit_size" : "8B",                           # FLIT size
        "output_buf_size" : "2KB",                    # Output buffer size
        "link_bw" : "96GB/s"                         # Link bandwidth
})
router.setSubComponent("topology", "merlin.singlerouter")

# Define the simulation components
comp_cpu = sst.Component("cpu", "revcpu.RevCPU")
comp_cpu.addParams({
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

comp_nic = comp_cpu.setSubComponent("nic","revcpu.RevNIC")
comp_nic.addParams({
        "id" : 0,                                     # NIC interface id
        "num_peers" : 0,                              # No other peers
        #"link_bw" : "96GB/s",                         # Match the router bandwidth
        "message_size": "8B"                          # Standard message size
})


linkif = comp_nic.setSubComponent("networkIF","merlin.linkcontrol")
linkif.addParam("link_bw","96GB/s")


link_cpu0 = sst.Link("rev_cpu_nic_0")
link_cpu0.connect( (linkif, "rtr_port", "1000ps"),
                   (router, "port0", "1000ps") )
link_cpu0.setNoCut()

sst.setStatisticOutput("sst.statOutputCSV")
sst.enableAllStatisticsForAllComponents()

# EOF
