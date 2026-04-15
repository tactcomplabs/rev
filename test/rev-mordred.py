#
# Copyright (C) 2017-2026 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#
# rev-basic-config.py
#

import argparse
import sst
from sst import UnitAlgebra

DEBUG_L1 = 0
DEBUG_MEM = 0
DEBUG_LEVEL = 10
VERBOSE = 2
memSize = 1024*1024*1024-1

# SST core options and parameters
clock = "2.0GHz"

# Parameters for Mordred

# Set up some parameters via UnitAlgebra
clk = UnitAlgebra("2GHz")
clk_pd = clk.invert()
link_latency = UnitAlgebra(0.8) * clk_pd
flit_size = UnitAlgebra("16b")

FixedRtrParams = {
    "verbose": 5,
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

# Setup argument parser
parser = argparse.ArgumentParser(description="Run Rev SST Simulation")
parser.add_argument("--numCores", type=int, help="Number of Rev Cores per RevCPU", default=1)
parser.add_argument("--numHarts", type=int, help="Number of HARTs per Rev Core", default=1)
parser.add_argument("--program", help="The program executable to run in the simulation", default="a.out")
parser.add_argument("--enableMemH", type=int, choices=[0, 1], help="Enable (1) or disable (0) memHierarchy backend", default=0)
parser.add_argument("--verbose", type=int, help="Verbosity level", default=2)
parser.add_argument("--machine", help="Machine type/configuration", default="[CORES:RV64GC]")
parser.add_argument("--args", help="Command line arguments to pass to the target executable", default="")
parser.add_argument("--startSymbol", help="ELF Symbol Rev should begin execution at", default="[0:main]")

# Parse arguments
args = parser.parse_args()

# Print arguments nicely
print("Rev SST Simulation Configuration:")
for arg in vars(args):
    print("\t", arg, " = ", getattr(args, arg))


# Define the simulation components
comp_cpu = sst.Component("cpu", "revcpu.RevCPU")
comp_cpu.addParams({
    "verbose": args.verbose,
    "numCores": args.numCores,
    "numHarts": args.numHarts,
    "clock": clock,
    "memSize": memSize,
    "machine": args.machine,
    "memCost": "[0:1:10]",
    "program": args.program,
    "startAddr": "[0:0x00000000]",
    "startSymbol": args.startSymbol,
    "enableMemH": args.enableMemH,
    "args": args.args,
    "splash": 1
})

sst.setStatisticOutput("sst.statOutputCSV")
sst.setStatisticLoadLevel(4)
sst.enableAllStatisticsForComponentType("revcpu.RevCPU")

# Conditional setup for memory hierarchy
if args.enableMemH:
    # Create the RevMemCtrl subcomponent
    comp_lsq = comp_cpu.setSubComponent("memory", "revcpu.RevBasicMemCtrl")
    comp_lsq.addParams({
        "verbose": "5",
        "clock": "2.0Ghz",
        "max_loads": 16,
        "max_stores": 16,
        "max_flush": 16,
        "max_llsc": 16,
        "max_readlock": 16,
        "max_writeunlock": 16,
        "max_custom": 16,
        "ops_per_cycle": 16
    })
    comp_lsq.enableAllStatistics({"type": "sst.AccumulatorStatistic"})

    # Should be a SST::Interface::StandardMem

    iface = comp_lsq.setSubComponent("memIface", "memHierarchy.standardInterface")
    iface.addParams({
        "verbose": VERBOSE,
        # "debug": 1,
        # "debug_level": 10,
        # "noncacheable_regions": f"0,{memSize}",
    })

    # fill subcomponent in iface
    iface_nic = iface.setSubComponent("lowlink", "memHierarchy.MemNIC")
    iface_nic.addParams({
        "group": 2,  # Here's the miracle setting to make this work.
        # "debug": 1,
        # "debug_level": 10,
    })

    # Fill subcomponent slot in iface_nic
    iface_nic_mordred = iface_nic.setSubComponent("linkcontrol", "mordred.mordredNIC")
    iface_nic_mordred.addParams(MordredNICParams)

    # Router
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

    memcpulink = memctrl.setSubComponent("highlink", "memHierarchy.MemNIC")
    memcpulink.addParam("group", 3)
    # Add params if necessary

    mem_mordredlink = memcpulink.setSubComponent("linkcontrol", "mordred.mordredNIC")
    mem_mordredlink.addParams(MordredNICParams)

    # Link between the CPU side and the router
    link_cpuiface_rtr = sst.Link("link_cpuiface_rtr")
    link_cpuiface_rtr.connect((iface_nic_mordred, "port", "50ps"), (rtr, "port4", "50ps"))

    # Link between the memory side and the router
    link_memiface_rtr = sst.Link("link_memiface_rtr")
    link_memiface_rtr.connect((mem_mordredlink, "port", "50ps"), (rtr, "port5", "50ps"))

    # Setup for memHierarchy backend
    # ... (Include your memHierarchy setup here)
    # TODO:
# else:
