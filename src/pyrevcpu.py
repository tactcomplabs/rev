#!/usr/bin/env python
#
# Rev Python Infrastructure
#
# Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
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

  def getName(self):
    return "RevCPU"

  def enableStatistics(self, level):
    print("Enabling Rev statistics")
    sst.setStatisticLoadLevel(level)
    sst.setStatisticsOutput("sst.statOutputCSV")
    sst.enableAllStatisticsForComponentType("revcpu.RevCPU")

class RevJob(Job):
  def __init__(self, job_id, num_nodes, mem_size, num_cores = 1, cpu_type="RV64IMAFDC"):
    Job.__init__(self,job_id,num_nodes)
    self.declareClassVariables(["_numCores", "_memSize", "_cpuType"])

    self._numCores = num_cores
    self._memSize = mem_size
    self._cpuType = cpu_type

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

#-- EOF
