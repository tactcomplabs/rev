#!/usr/bin/env python
#
# Rev PyBind Infrastructure
#
# Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

import sys
import sst

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


#-- EOF
