//
// _RevAccel_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//


#ifndef __REV_DMA_H__
#define __REV_DMA_H__

// -- C++ Headers
#include <ctime>
#include <vector>
#include <list>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <random>
#include <queue>
#include <tuple>

// -- SST Headers
#include "SST.h"

#include "RevMem.h"

namespace SST::RevCPU{

  // ----------------------------------------------------
  // RevAccel - Generic Acclerator component for RevCPU
  // ----------------------------------------------------
class RevAccel : SST::Component{
  public:
  SST_ELI_REGISTER_COMPONENT( RevAccel,
                              "RevAccelerator",
                              SST_ELI_ELEMENT_VERSION(1,0,0),
                              "Base Rev Accelerator Component",
                              COMPONENT_CATEGORY_UNCATEGORIZED
                              );

  SST_ELI_DOCUMENT_PARAMS({"verbose",        "Set the verbosity of output for the onchip router",     "0" },
                          { "clock",          "Sets the clock frequency of the onchip router",         "1Ghz" }
                          );
  
    // Register any subcomponents used by this element
  SST_ELI_DOCUMENT_SUBCOMPONENT_SLOTS()

      // Register any ports used with this element
  SST_ELI_DOCUMENT_PORTS(
        //Add Router and/or Memory ports
  )

  // Add statistics
  SST_ELI_DOCUMENT_STATISTICS(
      {"TotalInstructions",        "Counts the total number of instructions processed by the router",    "count", 1}
  )

  // Enum for referencing statistics
  typedef enum{
    TotalInstructions = 0
  }UpDownStats;

  ///< RevAccel Constructor
  RevAccel(SST::ComponentId_t id, SST::Params& params);

  virtual ~RevAccel();

  virtual bool ClockTick(SST::Cycle_t cycle) {std::cout << "Hello from the Accel component" << std::endl; return true;} ;
  virtual bool Reset() {}; 

  protected:
  SST::Output * output;

}//class RevAccel

}

