//
// _RevCPU_cc_
//
// Copyright (C) 2017-2020 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "RevCPU.h"

const char *splash_msg = "\
\n\
*******                   \n\
/**////**                  \n\
/**   /**   *****  **    **\n\
/*******   **///**/**   /**\n\
/**///**  /*******//** /** \n\
/**  //** /**////  //****  \n\
/**   //**//******  //**   \n\
//     //  //////    //    \n\
\n\
";

RevCPU::RevCPU( SST::ComponentId_t id, SST::Params& params )
  : SST::Component(id) {

  const int Verbosity = params.find<int>("verbose", 0);

  // Initialize the output handler
  output.init("RevCPU[" + getName() + ":@p:@t]: ", Verbosity, 0, SST::Output::STDOUT);

  // Register a new clock handler
  std::string cpuClock = params.find<std::string>("clock", "1GHz");
  timeConverter  = registerClock(cpuClock, new SST::Clock::Handler<RevCPU>(this,&RevCPU::clockTick));

  // Inform SST to wait unti we authorize it to exit
  registerAsPrimaryComponent();
  primaryComponentDoNotEndSim();

  // Derive the simulation parameters
  // We must always derive the number of cores before initializing the options
  numCores = params.find<unsigned>("numCores", "0");

  // read the binary executable name
  Exe = params.find<std::string>("program", "a.out");

  // read the program arguments
  Args = params.find<std::string>("args", "");

  // Create the options object
  Opts = new RevOpts(numCores,Verbosity);
  if( !Opts )
    output.fatal(CALL_INFO, -1, "Error: failed to initialize the RevOpts object\n" );

  // Initialize the remaining options
  std::vector<std::string> startAddrs;
  params.find_array<std::string>("startAddr", startAddrs);
  if( !Opts->InitStartAddrs( startAddrs ) )
    output.fatal(CALL_INFO, -1, "Error: failed to initialize the starting addresses\n" );

  std::vector<std::string> machModels;
  params.find_array<std::string>("machine", machModels);
  if( !Opts->InitMachineModels( machModels ) )
    output.fatal(CALL_INFO, -1, "Error: failed to initialize the machine models\n" );

  std::vector<std::string> instTables;
  params.find_array<std::string>("table",instTables);
  if( !Opts->InitInstTables( instTables ) )
    output.fatal(CALL_INFO, -1, "Error: failed to initialize the instruction tables\n" );

  std::vector<std::string> memCosts;
  params.find_array<std::string>("memCost",memCosts);
  if( !Opts->InitMemCosts( memCosts ) )
    output.fatal(CALL_INFO, -1, "Error: failed to initialize the memory latency range\n" );

  // Create the memory object
  unsigned long memSize = params.find<unsigned long>("memSize", 1073741824);
  Mem = new RevMem( memSize, Opts,  &output );
  if( !Mem )
    output.fatal(CALL_INFO, -1, "Error: failed to initialize the memory object\n" );

  // Look up the network component
  Nic = loadUserSubComponent<SST::Interfaces::SimpleNetwork>("networkIF",
                                                        ComponentInfo::SHARE_NONE,
                                                        1);
  if(!Nic){
    output.verbose(CALL_INFO, 1, 0, "No network interface controller loaded. Loading merlin.test_nic\n");
    // load the anonymous sub component
    Params if_params;
    if_params.insert("id","0");
    if_params.insert("num_peers", "1");
    if_params.insert("num_vns", "1");
    if_params.insert("link_bw", "96GB/s");
    if_params.insert("message_size","8B");
    if_params.insert("port","nic_link");

    Nic = loadAnonymousSubComponent<SST::Interfaces::SimpleNetwork>
      ("merlin.test_nic", "networkIF", 0,
       ComponentInfo::SHARE_PORTS | ComponentInfo::INSERT_STATS, if_params, 1 );

    if( Nic == nullptr )
      output.fatal(CALL_INFO, -1, "Error: unable to load network interface.\n");
  }

  // Load the binary into memory
  Loader = new RevLoader( Exe, Args, Mem, &output );
  if( !Loader )
    output.fatal(CALL_INFO, -1, "Error: failed to initialize the RISC-V loader\n" );

  // Create the processor objects
  for( unsigned i=0; i<numCores; i++ ){
    Procs.push_back( new RevProc( i, Opts, Mem, Loader, &output ) );
  }

  // Create the completion array
  Enabled = new bool [numCores];
  for( unsigned i=0; i<numCores; i++ ){
    Enabled[i] = true;
  }

  unsigned Splash = params.find<bool>("splash",0);

  if( Splash > 0 ){
    output.verbose(CALL_INFO,1,0,splash_msg);
  }

  // Done with initialization
  output.verbose(CALL_INFO, 1, 0, "Initialization of RevCPUs complete.\n");
}

RevCPU::~RevCPU(){

  // delete the competion array
  delete[] Enabled;

  // delete the processors objects
  for( unsigned i=0; i<Procs.size(); i++ ){
    delete Procs[i];
  }

  // delete the memory object
  delete Mem;

  // delete the loader object
  delete Loader;

  // delete the options object
  delete Opts;
}

void RevCPU::setup(){
}

void RevCPU::finish(){
}

void RevCPU::init( unsigned int phase ){
}

bool RevCPU::clockTick( SST::Cycle_t currentCycle ){
  bool rtn = true;

  output.verbose(CALL_INFO, 8, 0, "Cycle: %" PRIu64 "\n", static_cast<uint64_t>(currentCycle));

  // Execute each enabled core
  for( unsigned i=0; i<Procs.size(); i++ ){
    if( Enabled[i] ){
      if( !Procs[i]->ClockTick(currentCycle) )
        Enabled[i] = false;
    }
  }

  for( unsigned i=0; i<Procs.size(); i++ ){
    if( Enabled[i] )
      rtn = false;
  }

  return rtn;
}

// EOF
