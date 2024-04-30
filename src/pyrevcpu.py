#!/usr/bin/env python
#
# Rev Python Infrastructure
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

import sys
import sst
from sst.merlin.base import *

class RevCPU():
  def __init__(self):
    print("Initializing RevCPU")
    self.declareClassVariables(["_numCores"])
    self._numCPU = 0

  def getName(self):
    return "RevCPU"

  def getNumCPU(self):
    return self._numCPU

  def enableRevSim(self,cpus,cores,clock,memSize,machine,startAddr,memCost,program,verbose):
    print("Enabling Rev CPU component params")
    self._numCPU = cpus
    for i in range(0, cpus)
      print("Building RevCPU " + str(i))
      sst.pushNamePrefix("CPU"+str(i))

      # build component name = "CPUx.rev"
      cpu = sst.Component("rev","revcpu.RevCPU")
      cpu.addParam("verbose", verbose)
      cpu.addParam("numCores", cores)
      cpu.addParam("clock", clock)
      cpu.addParam("memSize", memSize)
      cpu.addParam("machine",machine)
      cpu.addParam("startAddr",startAdd)
      cpu.addParam("memCost",memCost)
      cpu.addParam("program",program)
      _CPUS.append(cpu)
      sst.popNamePrefix()


  def enableStatistics(self, level):
    print("Enabling Rev statistics")
    sst.setStatisticLoadLevel(level)
    sst.setStatisticsOutput("sst.statOutputCSV")
    sst.enableAllStatisticsForComponentType("revcpu.RevCPU")

  #-- private variables
  _CPUS = []

class RevJob(Job):
  def __init__(self, job_id, num_nodes):
    Job.__init__(self,job_id,num_nodes)
    _rev = RevCPU()

    self.declareClassVariables(["_memSize", "_cpuType"])

    self._memSize = mem_size
    self._cpuType = cpu_type

  def initCPUS(self,cores,clock,memSize,machine,startAddr,memCost,program,verbose):
    _rev.enableRevSim(self,_num_nodes,cores,clock,memSize,machine,startAddr,memCost,program,verbose)

  def getName(self):
    return "RevJob"

  def build(self, nodeID, extraKeys):
    if self._check_first_build()
      sst.addGlobalParam("params_%s"%self._instance_name, 'jobId', self.job_id)

    nic, slot_name = self.nic.build(nodeID,self._numCores)

    logical_id = self._nid_map[nodeID]
    networkif, port_name = self.network_interface.build(nic,slot_name,0,self.job_id,self.size,logical_id,False)

    retval = ( networkif, port_name )

    return retval

  #-- private variables
  _rev

#-- EOF
