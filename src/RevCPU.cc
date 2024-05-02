//
// _RevCPU_cc_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "RevCPU.h"
#include "RevMem.h"
#include "RevThread.h"
#include <cmath>
#include <fstream>
#include <memory>

namespace SST::RevCPU {

using MemSegment        = RevMem::MemSegment;

const char splash_msg[] = "\
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

RevCPU::RevCPU( SST::ComponentId_t id, const SST::Params& params ) :
  SST::Component( id ), testStage( 0 ), PrivTag( 0 ), address( -1 ),
  EnableMemH( false ), DisableCoprocClock( false ), Nic( nullptr ),
  Ctrl( nullptr ), ClockHandler( nullptr ) {

  const int Verbosity = params.find< int >( "verbose", 0 );

  // Initialize the output handler
  output.init(
    "RevCPU[" + getName() + ":@p:@t]: ", Verbosity, 0, SST::Output::STDOUT );

  // Register a new clock handler
  const std::string cpuClock = params.find< std::string >( "clock", "1GHz" );
  ClockHandler  = new SST::Clock::Handler< RevCPU >( this, &RevCPU::clockTick );
  timeConverter = registerClock( cpuClock, ClockHandler );

  // Inform SST to wait until we authorize it to exit
  registerAsPrimaryComponent();
  primaryComponentDoNotEndSim();

  // Derive the simulation parameters
  // We must always derive the number of cores before initializing the options
  numCores = params.find< unsigned >( "numCores", "1" );
  numHarts = params.find< uint16_t >( "numHarts", "1" );

  // Make sure someone isn't trying to have more than 65536 harts per core
  if( numHarts > _MAX_HARTS_ ) {
    output.fatal( CALL_INFO,
                  -1,
                  "Error: number of harts must be <= %" PRIu32 "\n",
                  _MAX_HARTS_ );
  }
  output.verbose( CALL_INFO,
                  1,
                  0,
                  "Building Rev with %" PRIu32 " cores and %" PRIu32
                  " hart(s) on each core \n",
                  numCores,
                  numHarts );

  // read the binary executable name
  Exe  = params.find< std::string >( "program", "a.out" );

  // read the program arguments
  Args = params.find< std::string >( "args", "" );

  // Create the options object
  Opts = new( std::nothrow ) RevOpts( numCores, numHarts, Verbosity );
  if( !Opts )
    output.fatal(
      CALL_INFO, -1, "Error: failed to initialize the RevOpts object\n" );

  // Initialize the remaining options
  for( auto [ParamName, InitFunc] : {
         std::pair( "startAddr", &RevOpts::InitStartAddrs ),
         std::pair( "startSymbol", &RevOpts::InitStartSymbols ),
         std::pair( "machine", &RevOpts::InitMachineModels ),
         std::pair( "table", &RevOpts::InitInstTables ),
         std::pair( "memCost", &RevOpts::InitMemCosts ),
         std::pair( "prefetchDepth", &RevOpts::InitPrefetchDepth ),
       } ) {
    std::vector< std::string > optList;
    params.find_array( ParamName, optList );
    if( !( Opts->*InitFunc )( optList ) )
      output.fatal(
        CALL_INFO, -1, "Error: failed to initialize %s\n", ParamName );
  }

  // See if we should load the network interface controller
  EnableNIC = params.find< bool >( "enable_nic", 0 );

  if( EnableNIC ) {
    // Look up the network component

    Nic = loadUserSubComponent< nicAPI >( "nic" );

    // check to see if the nic was loaded.  if not, DO NOT load an anonymous endpoint
    if( !Nic )
      output.fatal(
        CALL_INFO, -1, "Error: no NIC object loaded into RevCPU\n" );

    Nic->setMsgHandler(
      new Event::Handler< RevCPU >( this, &RevCPU::handleMessage ) );

    // record the number of injected messages per cycle
    msgPerCycle = params.find< unsigned >( "msgPerCycle", 1 );
  }

  // Look for the fault injection logic
  EnableFaults = params.find< bool >( "enable_faults", 0 );
  if( EnableFaults ) {
    std::vector< std::string > faults;
    params.find_array< std::string >( "faults", faults );
    DecodeFaultCodes( faults );

    std::string width = params.find< std::string >( "fault_width", "1" );
    DecodeFaultWidth( width );

    fault_width = params.find< int64_t >( "fault_range", "65536" );
    FaultCntr   = fault_width;
  }

  // Create the memory object
  const uint64_t memSize =
    params.find< unsigned long >( "memSize", 1073741824 );
  EnableMemH = params.find< bool >( "enable_memH", 0 );
  if( !EnableMemH ) {
    // TODO: Use std::nothrow to return null instead of throwing std::bad_alloc
    Mem = new RevMem( memSize, Opts, &output );
    if( !Mem )
      output.fatal(
        CALL_INFO, -1, "Error: failed to initialize the memory object\n" );
  } else {
    Ctrl = loadUserSubComponent< RevMemCtrl >( "memory" );
    if( !Ctrl )
      output.fatal(
        CALL_INFO,
        -1,
        "Error : failed to inintialize the memory controller subcomponent\n" );

    // TODO: Use std::nothrow to return null instead of throwing std::bad_alloc
    Mem = new RevMem( memSize, Opts, Ctrl, &output );
    if( !Mem )
      output.fatal(
        CALL_INFO, -1, "Error : failed to initialize the memory object\n" );

    if( EnableFaults )
      output.verbose( CALL_INFO,
                      1,
                      0,
                      "Warning: memory faults cannot be enabled with "
                      "memHierarchy support\n" );
  }

  // Set TLB Size
  const uint64_t tlbSize = params.find< unsigned long >( "tlbSize", 512 );
  Mem->SetTLBSize( tlbSize );

  // Set max heap size
  const uint64_t maxHeapSize =
    params.find< unsigned long >( "maxHeapSize", memSize / 4 );
  Mem->SetMaxHeapSize( maxHeapSize );

  // Load the binary into memory
  // TODO: Use std::nothrow to return null instead of throwing std::bad_alloc
  Loader = new RevLoader( Exe, Args, Mem, &output );
  if( !Loader ) {
    output.fatal(
      CALL_INFO, -1, "Error: failed to initialize the RISC-V loader\n" );
  }

  Opts->SetArgs( Loader->GetArgv() );

  EnableCoProc = params.find< bool >( "enableCoProc", 0 );
  if( EnableCoProc ) {

    // Create the processor objects
    Procs.reserve( Procs.size() + numCores );
    for( unsigned i = 0; i < numCores; i++ ) {
      Procs.push_back( new RevCore(
        i, Opts, numHarts, Mem, Loader, this->GetNewTID(), &output ) );
    }
    // Create the co-processor objects
    for( unsigned i = 0; i < numCores; i++ ) {
      RevCoProc* CoProc = loadUserSubComponent< RevCoProc >(
        "co_proc", SST::ComponentInfo::SHARE_NONE, Procs[i] );
      if( !CoProc ) {
        output.fatal(
          CALL_INFO,
          -1,
          "Error : failed to inintialize the co-processor subcomponent\n" );
      }
      CoProcs.push_back( CoProc );
      Procs[i]->SetCoProc( CoProc );
    }
  } else {
    // Create the processor objects
    Procs.reserve( Procs.size() + numCores );
    for( unsigned i = 0; i < numCores; i++ ) {
      Procs.push_back( new RevCore(
        i, Opts, numHarts, Mem, Loader, this->GetNewTID(), &output ) );
    }
  }

  // Memory dumping option(s)
  std::vector< std::string > memDumpRanges;
  params.find_array( "memDumpRanges", memDumpRanges );
  if( !memDumpRanges.empty() ) {
    for( auto& segName : memDumpRanges ) {
      // FIXME: Figure out how to parse units (GB, MB, KB, etc.)
      // segName is a string... Look for scoped params for this segment
      const auto& scopedParams = params.get_scoped_params( segName );
      // Check scopedParams for the following:
      // - startAddr (hex)
      // - size (bytes)
      if( !scopedParams.contains( "startAddr" ) ) {
        output.fatal(
          CALL_INFO,
          -1,
          "Error: memDumpRanges requires startAddr. Please specify this scoped "
          "param as %s.startAddr in the configuration file\n",
          segName.c_str() );
      } else if( !scopedParams.contains( "size" ) ) {
        output.fatal( CALL_INFO,
                      -1,
                      "Error: memDumpRanges requires size. Please specify this "
                      "scoped param as %s.size in the configuration file\n",
                      segName.c_str() );
      }
      const uint64_t startAddr = scopedParams.find< uint64_t >( "startAddr" );
      const uint64_t size      = scopedParams.find< uint64_t >( "size" );
      // FIXME: @Ken I don't think we need these
      Mem->AddDumpRange( segName, startAddr, size );
    }
  }

#ifndef NO_REV_TRACER
  // Configure tracer and assign to each core
  if( output.getVerboseLevel() >= 5 ) {
    for( unsigned i = 0; i < numCores; i++ ) {
      // Each core gets its very own tracer
      RevTracer*  trc = new RevTracer( getName(), &output );
      std::string diasmType;
      Opts->GetMachineModel( 0, diasmType );  // TODO first param is core
      if( trc->SetDisassembler( diasmType ) )
        output.verbose(
          CALL_INFO,
          1,
          0,
          "Warning: tracer could not find disassembler. Using REV default\n" );

      trc->SetTraceSymbols( Loader->GetTraceSymbols() );

      // tracer user controls - cycle on and off. Ignored unless > 0
      trc->SetStartCycle( params.find< uint64_t >( "trcStartCycle", 0 ) );
      trc->SetCycleLimit( params.find< uint64_t >( "trcLimit", 0 ) );
      trc->SetCmdTemplate(
        params.find< std::string >( "trcOp", TRC_OP_DEFAULT ).c_str() );

      // clear trace states
      trc->Reset();

      // Assign to components
      Procs[i]->SetTracer( trc );
      if( Ctrl )
        Ctrl->setTracer( trc );
    }
  }
#endif
  // Setup timeConverter
  for( size_t i = 0; i < Procs.size(); i++ ) {
    Procs[i]->SetTimeConverter( timeConverter );
  }

  // Initial thread setup
  uint32_t MainThreadID =
    id + 1;  // Prevents having MainThreadID == 0 which is reserved for INVALID

  uint64_t    StartAddr = 0x00ull;
  std::string StartSymbol;

  bool IsStartSymbolProvided = Opts->GetStartSymbol( id, StartSymbol );
  bool IsStartAddrProvided =
    Opts->GetStartAddr( id, StartAddr ) && StartAddr != 0x00ull;
  uint64_t ResolvedStartSymbolAddr =
    ( IsStartSymbolProvided ) ? Loader->GetSymbolAddr( StartSymbol ) : 0x00ull;

  // If no start address has been provided ...
  if( !IsStartAddrProvided ) {
    // ... check if symbol was provided ...
    if( !IsStartSymbolProvided ) {
      // ... no, try to default to 'main' ...
      StartAddr = Loader->GetSymbolAddr( "main" );
      if( StartAddr == 0x00ull ) {
        // ... no hope left!
        output.fatal( CALL_INFO,
                      -1,
                      "Error: failed to auto discover address for <main> for "
                      "main thread\n" );
      }
    } else {
      // ... if symbol was provided, check whether it is valid or not ...
      if( !ResolvedStartSymbolAddr ) {
        // ... not valid, error out
        output.fatal( CALL_INFO,
                      -1,
                      "Error: failed to resolve address for symbol <%s>\n",
                      StartSymbol.c_str() );
      }
      // ... valid use the resolved symbol
      StartAddr = ResolvedStartSymbolAddr;
    }
  } else {  // A start address was provided ...
    // ... check if a symbol was provided and is compatible with the start address ...
    if( ( IsStartSymbolProvided ) &&
        ( ResolvedStartSymbolAddr != StartAddr ) ) {
      // ... they are different, don't know the user intent so error out now
      output.fatal(
        CALL_INFO,
        -1,
        "Error: start address and start symbol differ startAddr=0x%" PRIx64
        " StartSymbol=%s ResolvedStartSymbolAddr=0x%" PRIx64 "\n",
        StartAddr,
        StartSymbol.c_str(),
        ResolvedStartSymbolAddr );
    }  // ... else no symbol provided, continue on with StartAddr as the target
  }

  output.verbose(
    CALL_INFO, 11, 0, "Start address is 0x%" PRIx64 "\n", StartAddr );

  InitMainThread( MainThreadID, StartAddr );

  // setup the per-proc statistics
  TotalCycles.reserve( numCores );
  CyclesWithIssue.reserve( numCores );
  FloatsRead.reserve( numCores );
  FloatsWritten.reserve( numCores );
  DoublesRead.reserve( numCores );
  DoublesWritten.reserve( numCores );
  BytesRead.reserve( numCores );
  BytesWritten.reserve( numCores );
  FloatsExec.reserve( numCores );
  TLBHitsPerCore.reserve( numCores );
  TLBMissesPerCore.reserve( numCores );

  for( unsigned s = 0; s < numCores; s++ ) {
    auto core = "core_" + std::to_string( s );
    TotalCycles.push_back(
      registerStatistic< uint64_t >( "TotalCycles", core ) );
    CyclesWithIssue.push_back(
      registerStatistic< uint64_t >( "CyclesWithIssue", core ) );
    FloatsRead.push_back( registerStatistic< uint64_t >( "FloatsRead", core ) );
    FloatsWritten.push_back(
      registerStatistic< uint64_t >( "FloatsWritten", core ) );
    DoublesRead.push_back(
      registerStatistic< uint64_t >( "DoublesRead", core ) );
    DoublesWritten.push_back(
      registerStatistic< uint64_t >( "DoublesWritten", core ) );
    BytesRead.push_back( registerStatistic< uint64_t >( "BytesRead", core ) );
    BytesWritten.push_back(
      registerStatistic< uint64_t >( "BytesWritten", core ) );
    FloatsExec.push_back( registerStatistic< uint64_t >( "FloatsExec", core ) );
    TLBHitsPerCore.push_back(
      registerStatistic< uint64_t >( "TLBHitsPerCore", core ) );
    TLBMissesPerCore.push_back(
      registerStatistic< uint64_t >( "TLBMissesPerCore", core ) );
  }

  // determine whether we need to enable/disable manual coproc clocking
  DisableCoprocClock    = params.find< bool >( "independentCoprocClock", 0 );

  // Create the completion array
  Enabled               = new bool[numCores]{ false };

  const unsigned Splash = params.find< bool >( "splash", 0 );

  if( Splash > 0 )
    output.verbose( CALL_INFO, 1, 0, splash_msg );

  // Done with initialization
  output.verbose( CALL_INFO, 1, 0, "Initialization of RevCPUs complete.\n" );

  for( const auto& [Name, Seg] : Mem->GetDumpRanges() ) {
    // Open a file called '{Name}.init.dump'
    std::ofstream dumpFile( Name + ".dump.init", std::ios::binary );
    Mem->DumpMemSeg( Seg, 16, dumpFile );
  }
}

RevCPU::~RevCPU() {
  // delete the competion array
  delete[] Enabled;

  // delete the processors objects
  for( size_t i = 0; i < Procs.size(); i++ ) {
    delete Procs[i];
  }

  for( size_t i = 0; i < CoProcs.size(); i++ ) {
    delete CoProcs[i];
  }

  // delete the memory controller if present
  delete Ctrl;

  // delete the memory object
  delete Mem;

  // delete the loader object
  delete Loader;

  // delete the options object
  delete Opts;
}

void RevCPU::DecodeFaultWidth( const std::string& width ) {
  fault_width = 1;  // default to single bit failures

  if( width == "single" ) {
    fault_width = 1;
  } else if( width == "word" ) {
    fault_width = 8;
  } else {
    fault_width = std::stoi( width );
  }

  if( fault_width > 64 ) {
    output.fatal( CALL_INFO, -1, "Fault width must be <= 64 bits" );
  }
}

void RevCPU::DecodeFaultCodes( const std::vector< std::string >& faults ) {
  if( faults.empty() ) {
    output.fatal( CALL_INFO, -1, "No fault codes defined" );
  }

  EnableCrackFaults = EnableMemFaults = EnableRegFaults = EnableALUFaults =
    false;

  for( auto& fault : faults ) {
    if( fault == "decode" ) {
      EnableCrackFaults = true;
    } else if( fault == "mem" ) {
      EnableMemFaults = true;
    } else if( fault == "reg" ) {
      EnableRegFaults = true;
    } else if( fault == "alu" ) {
      EnableALUFaults = true;
    } else if( fault == "all" ) {
      EnableCrackFaults = EnableMemFaults = EnableRegFaults = EnableALUFaults =
        true;
    } else {
      output.fatal( CALL_INFO, -1, "Undefined fault code: %s", fault.c_str() );
    }
  }
}

void RevCPU::registerStatistics() {
  SyncGetSend        = registerStatistic< uint64_t >( "SyncGetSend" );
  SyncPutSend        = registerStatistic< uint64_t >( "SyncPutSend" );
  AsyncGetSend       = registerStatistic< uint64_t >( "AsyncGetSend" );
  AsyncPutSend       = registerStatistic< uint64_t >( "AsyncPutSend" );
  SyncStreamGetSend  = registerStatistic< uint64_t >( "SyncStreamGetSend" );
  SyncStreamPutSend  = registerStatistic< uint64_t >( "SyncStreamPutSend" );
  AsyncStreamGetSend = registerStatistic< uint64_t >( "AsyncStreamGetSend" );
  AsyncStreamPutSend = registerStatistic< uint64_t >( "AsyncStreamPutSend" );
  ExecSend           = registerStatistic< uint64_t >( "ExecSend" );
  StatusSend         = registerStatistic< uint64_t >( "StatusSend" );
  CancelSend         = registerStatistic< uint64_t >( "CancelSend" );
  ReserveSend        = registerStatistic< uint64_t >( "ReserveSend" );
  RevokeSend         = registerStatistic< uint64_t >( "RevokeSend" );
  HaltSend           = registerStatistic< uint64_t >( "HaltSend" );
  ResumeSend         = registerStatistic< uint64_t >( "ResumeSend" );
  ReadRegSend        = registerStatistic< uint64_t >( "ReadRegSend" );
  WriteRegSend       = registerStatistic< uint64_t >( "WriteRegSend" );
  SingleStepSend     = registerStatistic< uint64_t >( "SingleStepSend" );
  SetFutureSend      = registerStatistic< uint64_t >( "SetFutureSend" );
  RevokeFutureSend   = registerStatistic< uint64_t >( "RevokeFutureSend" );
  StatusFutureSend   = registerStatistic< uint64_t >( "StatusFutureSend" );
  SuccessSend        = registerStatistic< uint64_t >( "SuccessSend" );
  FailedSend         = registerStatistic< uint64_t >( "FailedSend" );
  BOTWSend           = registerStatistic< uint64_t >( "BOTWSend" );
  SyncGetRecv        = registerStatistic< uint64_t >( "SyncGetRecv" );
  SyncPutRecv        = registerStatistic< uint64_t >( "SyncPutRecv" );
  AsyncGetRecv       = registerStatistic< uint64_t >( "AsyncGetRecv" );
  AsyncPutRecv       = registerStatistic< uint64_t >( "AsyncPutRecv" );
  SyncStreamGetRecv  = registerStatistic< uint64_t >( "SyncStreamGetRecv" );
  SyncStreamPutRecv  = registerStatistic< uint64_t >( "SyncStreamPutRecv" );
  AsyncStreamGetRecv = registerStatistic< uint64_t >( "AsyncStreamGetRecv" );
  AsyncStreamPutRecv = registerStatistic< uint64_t >( "AsyncStreamPutRecv" );
  ExecRecv           = registerStatistic< uint64_t >( "ExecRecv" );
  StatusRecv         = registerStatistic< uint64_t >( "StatusRecv" );
  CancelRecv         = registerStatistic< uint64_t >( "CancelRecv" );
  ReserveRecv        = registerStatistic< uint64_t >( "ReserveRecv" );
  RevokeRecv         = registerStatistic< uint64_t >( "RevokeRecv" );
  HaltRecv           = registerStatistic< uint64_t >( "HaltRecv" );
  ResumeRecv         = registerStatistic< uint64_t >( "ResumeRecv" );
  ReadRegRecv        = registerStatistic< uint64_t >( "ReadRegRecv" );
  WriteRegRecv       = registerStatistic< uint64_t >( "WriteRegRecv" );
  SingleStepRecv     = registerStatistic< uint64_t >( "SingleStepRecv" );
  SetFutureRecv      = registerStatistic< uint64_t >( "SetFutureRecv" );
  RevokeFutureRecv   = registerStatistic< uint64_t >( "RevokeFutureRecv" );
  StatusFutureRecv   = registerStatistic< uint64_t >( "StatusFutureRecv" );
  SuccessRecv        = registerStatistic< uint64_t >( "SuccessRecv" );
  FailedRecv         = registerStatistic< uint64_t >( "FailedRecv" );
  BOTWRecv           = registerStatistic< uint64_t >( "BOTWRecv" );
}

void RevCPU::setup() {
  if( EnableNIC ) {
    Nic->setup();
    address = Nic->getAddress();
  }
  if( EnableMemH ) {
    Ctrl->setup();
  }
}

void RevCPU::finish() {
}

void RevCPU::init( unsigned int phase ) {
  if( EnableNIC )
    Nic->init( phase );
  if( EnableMemH )
    Ctrl->init( phase );
}

void RevCPU::handleMessage( Event* ev ) {
  nicEvent* event = static_cast< nicEvent* >( ev );
  // -- RevNIC: This is where you can unpack and handle the data payload
  delete event;
}

uint8_t RevCPU::createTag() {
  uint8_t rtn = 0;
  if( PrivTag == 0b11111111 ) {
    rtn     = 0b00000000;
    PrivTag = 0b00000001;
    return 0b00000000;
  } else {
    rtn = PrivTag;
    PrivTag += 1;
  }
  return rtn;
}

void RevCPU::HandleCrackFault( SST::Cycle_t currentCycle ) {
  output.verbose( CALL_INFO,
                  4,
                  0,
                  "FAULT: Crack fault injected at cycle: %" PRIu64 "\n",
                  currentCycle );

  // select a random processor core
  unsigned Core = 0;
  if( numCores > 1 ) {
    Core = RevRand( 0, numCores - 1 );
  }

  Procs[Core]->HandleCrackFault( fault_width );
}

void RevCPU::HandleMemFault( SST::Cycle_t currentCycle ) {
  output.verbose( CALL_INFO,
                  4,
                  0,
                  "FAULT: Memory fault injected at cycle: %" PRIu64 "\n",
                  currentCycle );
  Mem->HandleMemFault( fault_width );
}

void RevCPU::HandleRegFault( SST::Cycle_t currentCycle ) {
  output.verbose( CALL_INFO,
                  4,
                  0,
                  "FAULT: Register fault injected at cycle: %" PRIu64 "\n",
                  currentCycle );

  // select a random processor core
  unsigned Core = 0;
  if( numCores > 1 ) {
    Core = RevRand( 0, numCores - 1 );
  }

  Procs[Core]->HandleRegFault( fault_width );
}

void RevCPU::HandleALUFault( SST::Cycle_t currentCycle ) {
  output.verbose( CALL_INFO,
                  4,
                  0,
                  "FAULT: ALU fault injected at cycle: %" PRIu64 "\n",
                  currentCycle );

  // select a random processor core
  unsigned Core = 0;
  if( numCores > 1 ) {
    Core = RevRand( 0, numCores - 1 );
  }

  Procs[Core]->HandleALUFault( fault_width );
}

void RevCPU::HandleFaultInjection( SST::Cycle_t currentCycle ) {
  // build up a vector of faults
  // then randomly determine which one to inject
  std::vector< std::string > myfaults;
  if( EnableCrackFaults ) {
    myfaults.push_back( "crack" );
  }
  if( EnableMemFaults ) {
    myfaults.push_back( "mem" );
  }
  if( EnableRegFaults ) {
    myfaults.push_back( "reg" );
  }
  if( EnableALUFaults ) {
    myfaults.push_back( "alu" );
  }

  if( myfaults.size() == 0 ) {
    output.fatal(
      CALL_INFO,
      -1,
      "Error: no faults enabled; add a fault vector in the 'faults' param\n" );
  }

  unsigned selector = 0;
  if( myfaults.size() != 1 ) {
    selector = RevRand( 0, int( myfaults.size() ) - 1 );
  }

  // handle the selected fault
  if( myfaults[selector] == "crack" ) {
    HandleCrackFault( currentCycle );
  } else if( myfaults[selector] == "mem" ) {
    HandleMemFault( currentCycle );
  } else if( myfaults[selector] == "reg" ) {
    HandleRegFault( currentCycle );
  } else if( myfaults[selector] == "alu" ) {
    HandleALUFault( currentCycle );
  } else {
    output.fatal( CALL_INFO, -1, "Error: erroneous fault selection\n" );
  }
}

void RevCPU::UpdateCoreStatistics( unsigned coreNum ) {
  auto [stats, memStats] = Procs[coreNum]->GetAndClearStats();
  TotalCycles[coreNum]->addData( stats.totalCycles );
  CyclesWithIssue[coreNum]->addData( stats.cyclesBusy );
  FloatsRead[coreNum]->addData( memStats.floatsRead );
  FloatsWritten[coreNum]->addData( memStats.floatsWritten );
  DoublesRead[coreNum]->addData( memStats.doublesRead );
  DoublesWritten[coreNum]->addData( memStats.doublesWritten );
  BytesRead[coreNum]->addData( memStats.bytesRead );
  BytesWritten[coreNum]->addData( memStats.bytesWritten );
  FloatsExec[coreNum]->addData( stats.floatsExec );
  TLBHitsPerCore[coreNum]->addData( memStats.TLBHits );
  TLBMissesPerCore[coreNum]->addData( memStats.TLBMisses );
}

bool RevCPU::clockTick( SST::Cycle_t currentCycle ) {
  bool rtn = true;

  output.verbose( CALL_INFO, 8, 0, "Cycle: %" PRIu64 "\n", currentCycle );

  // Execute each enabled core
  for( size_t i = 0; i < Procs.size(); i++ ) {
    // Check if we have more work to assign and places to put it
    UpdateThreadAssignments( i );
    if( Enabled[i] ) {
      if( !Procs[i]->ClockTick( currentCycle ) ) {
        if( EnableCoProc && !CoProcs.empty() ) {
          CoProcs[i]->Teardown();
        }
        UpdateCoreStatistics( i );
        Enabled[i] = false;
        output.verbose( CALL_INFO,
                        5,
                        0,
                        "Closing Processor %zu at Cycle: %" PRIu64 "\n",
                        i,
                        currentCycle );
      }
      if( EnableCoProc && !CoProcs[i]->ClockTick( currentCycle ) &&
          !DisableCoprocClock ) {
        output.verbose( CALL_INFO,
                        5,
                        0,
                        "Closing Co-Processor %zu at Cycle: %" PRIu64 "\n",
                        i,
                        currentCycle );
      }
    }

    // See if any of the threads on this proc changes state
    HandleThreadStateChangesForProc( i );

    if( Procs[i]->HasNoBusyHarts() ) {
      Enabled[i] = false;
    }
  }

  // check to see if we need to inject a fault
  if( EnableFaults ) {
    if( FaultCntr == 0 ) {
      // inject a fault
      HandleFaultInjection( currentCycle );

      // reset the fault counter
      FaultCntr = fault_width;
    } else {
      FaultCntr--;
    }
  }

  // check to see if all the processors are completed
  for( size_t i = 0; i < Procs.size(); i++ ) {
    if( Enabled[i] ) {
      rtn = false;
    }
  }

  // If all Procs are disabled (ie. rtn == false at this point)
  // check to see if there are threads to assign
  if( ReadyThreads.size() ) {
    rtn = false;
  } else if( BlockedThreads.size() ) {
    CheckBlockedThreads();
    rtn = false;
  }

  // check to see if the network has any outstanding messages: fixme
  if( !TrackTags.empty() || !ZeroRqst.empty() ) {
    rtn = false;
  }

  if( rtn && CompletedThreads.size() ) {
    for( unsigned i = 0; i < numCores; i++ ) {
      UpdateCoreStatistics( i );
      Procs[i]->PrintStatSummary();
    }

    for( const auto& [Name, Seg] : Mem->GetDumpRanges() ) {
      // Open a file called '{Name}.init.dump'
      std::ofstream dumpFile( Name + ".dump.final", std::ios::binary );
      Mem->DumpMemSeg( Seg, 16, dumpFile );
    }
    primaryComponentOKToEndSim();
    output.verbose( CALL_INFO,
                    5,
                    0,
                    "OK to end sim at cycle: %" PRIu64 "\n",
                    static_cast< uint64_t >( currentCycle ) );
  } else {
    rtn = false;
  }

  return rtn;
}

// Initializes a RevThread object.
// - Moves it to the 'Threads' map
// - Adds it's ThreadID to the ReadyThreads to be scheduled
void RevCPU::InitThread( std::unique_ptr< RevThread >&& ThreadToInit ) {
  // Get a pointer to the register state for this thread
  std::unique_ptr< RevRegFile > RegState = ThreadToInit->TransferVirtRegState();
  // Set the global pointer register and the frame pointer register
  auto gp = Loader->GetSymbolAddr( "__global_pointer$" );
  RegState->SetX( RevReg::gp, gp );
  RegState->SetX( RevReg::s0, gp );  // s0 = x8 = frame pointer register
  // Set state to Ready
  ThreadToInit->SetState( ThreadState::READY );
  output.verbose( CALL_INFO,
                  4,
                  0,
                  "Initializing Thread %" PRIu32 "\n",
                  ThreadToInit->GetID() );
  output.verbose( CALL_INFO,
                  11,
                  0,
                  "Thread Information: %s",
                  ThreadToInit->to_string().c_str() );
  ReadyThreads.emplace_back( std::move( ThreadToInit ) );
}

// Assigns a RevThred to a specific Proc which then loads it into a RevHart
// This should not be called without first checking if the Proc has an IdleHart
void RevCPU::AssignThread( std::unique_ptr< RevThread >&& ThreadToAssign,
                           unsigned                       ProcID ) {
  output.verbose( CALL_INFO,
                  4,
                  0,
                  "Assigning Thread %" PRIu32 " to Processor %" PRIu32 "\n",
                  ThreadToAssign->GetID(),
                  ProcID );
  Procs[ProcID]->AssignThread( std::move( ThreadToAssign ) );
  return;
}

// Checks if a thread with a given Thread ID can proceed (used for pthread_join).
// it does this by seeing if a given thread's WaitingOnTID has completed
bool RevCPU::ThreadCanProceed( const std::unique_ptr< RevThread >& Thread ) {
  bool rtn              = false;

  // Get the thread's waiting to join TID
  uint32_t WaitingOnTID = Thread->GetWaitingToJoinTID();

  // If the thread is waiting on another thread, check if that thread has completed
  if( WaitingOnTID != _INVALID_TID_ ) {
    // Check if WaitingOnTID has completed... if so, return = true, else return false
    output.verbose( CALL_INFO,
                    6,
                    0,
                    "Thread %" PRIu32 " is waiting on Thread %u\n",
                    Thread->GetID(),
                    WaitingOnTID );

    // Check if the WaitingOnTID has completed, if not, thread cannot proceed
    rtn = ( CompletedThreads.find( WaitingOnTID ) != CompletedThreads.end() ) ?
            true :
            false;
  }
  // If the thread is not waiting on another thread, it can proceed
  else {
    // Thread is waiting on INVALID_TID (ie. No thread)
    // so it can proceed
    rtn = true;
  }

  return rtn;
}

// Checks if any of the blocked threads can proceed based on if the thread
// they were waiting on has completed
void RevCPU::CheckBlockedThreads() {
  // Iterate over all block threads
  for( auto it = BlockedThreads.begin(); it != BlockedThreads.end(); ) {
    if( ThreadCanProceed( *it ) ) {
      ReadyThreads.emplace_back( std::move( *it ) );
      it = BlockedThreads.erase( it );
    } else {
      ++it;
    }
  }
  return;
}

// ----------------------------------
// We need to initialize the x10 register to include the value of ARGC
// This is >= 1 (the executable name is always included)
// We also need to initialize the ARGV pointer to the value
// of the ARGV base pointer in memory which is currently set to the
// program header region.  When we come out of reset, this is StackTop+60 bytes
// ----------------------------------
void RevCPU::SetupArgs( const std::unique_ptr< RevRegFile >& RegFile ) {
  auto Argv = Opts->GetArgv();
  // setup argc
  RegFile->SetX( RevReg::a0, Argv.size() );
  RegFile->SetX( RevReg::a1, Mem->GetStackTop() + 60 );
  return;
}

// Checks core 'i' to see if it has any available harts to assign work to
// if it does and there is work to assign (ie. ReadyThreads is not empty)
// assign it and enable the processor if not already enabled.
void RevCPU::UpdateThreadAssignments( uint32_t ProcID ) {
  // Check if we have anything to assign
  if( ReadyThreads.empty() ) {
    return;
  }

  // There is work to assign, check if this proc has room
  if( Procs[ProcID]->HasIdleHart() ) {
    // There is room, assign a thread
    // Get the next thread to assign
    Procs[ProcID]->AssignThread( std::move( ReadyThreads.front() ) );

    // Remove thread from ready threads vector
    ReadyThreads.erase( ReadyThreads.begin() );

    // Proc has a thread assigned to it, enable it
    Enabled[ProcID] = true;
  }
  return;
}

// Checks for state changes in the threads of a given processor index 'i'
// and handle appropriately
void RevCPU::HandleThreadStateChangesForProc( uint32_t ProcID ) {
  // Handle any thread state changes for this core
  // NOTE: At this point we handle EVERY thread that changed state every cycle
  for( auto& Thread : Procs[ProcID]->TransferThreadsThatChangedState() ) {
    uint32_t ThreadID = Thread->GetID();
    // Handle the thread that changed state based on the new state
    switch( Thread->GetState() ) {
    case ThreadState::DONE:
      // This thread has completed execution
      output.verbose( CALL_INFO,
                      8,
                      0,
                      "Thread %" PRIu32 " on Core %" PRIu32 " is DONE\n",
                      ThreadID,
                      ProcID );
      CompletedThreads.emplace( ThreadID, std::move( Thread ) );
      break;

    case ThreadState::BLOCKED:
      // This thread is blocked (currently only caused by a rev_pthread_join)
      output.verbose( CALL_INFO,
                      8,
                      0,
                      "Thread %" PRIu32 "on Core %" PRIu32 " is BLOCKED\n",
                      ThreadID,
                      ProcID );

      // Set its state to BLOCKED
      Thread->SetState( ThreadState::BLOCKED );

      // Add it to BlockedThreads
      BlockedThreads.emplace_back( std::move( Thread ) );
      break;
    case ThreadState::START:
      // A new thread was created
      output.verbose( CALL_INFO,
                      99,
                      1,
                      "A new thread with ID = %" PRIu32
                      " was found on Core %" PRIu32,
                      Thread->GetID(),
                      ProcID );

      // Mark it ready for execution
      Thread->SetState( ThreadState::READY );

      // Add it to the ReadyThreads so it is scheduled
      ReadyThreads.emplace_back( std::move( Thread ) );

      break;

    case ThreadState::RUNNING:
      output.verbose( CALL_INFO,
                      11,
                      0,
                      "Thread %" PRIu32 " on Core %" PRIu32 " is RUNNING\n",
                      ThreadID,
                      ProcID );
      break;

    case ThreadState::READY:
      // If this happens we are not setting state when assigning thread somewhere
      output.fatal( CALL_INFO,
                    99,
                    "Error: Thread %" PRIu32 " on Core %" PRIu32
                    " is assigned but is in READY state... This is a bug\n",
                    ThreadID,
                    ProcID );
      break;
    default:  // Should DEFINITELY never happen
      output.fatal( CALL_INFO,
                    99,
                    "Error: Thread %" PRIu32 " on Core %" PRIu32
                    " is in an unknown state... This is a bug.\n%s\n",
                    ThreadID,
                    ProcID,
                    Thread->to_string().c_str() );
      break;
    }
    // If the Proc does not have any threads assigned to it, there is no work to do, it is disabled
    if( Procs[ProcID]->HasNoWork() ) {
      Enabled[ProcID] = false;
    }
  }
}

void RevCPU::InitMainThread( uint32_t MainThreadID, const uint64_t StartAddr ) {
  // @Lee: Is there a better way to get the feature info?
  std::unique_ptr< RevRegFile > MainThreadRegState =
    std::make_unique< RevRegFile >( Procs[0]->GetRevFeature() );
  MainThreadRegState->SetPC( StartAddr );
  MainThreadRegState->SetX( RevReg::tp,
                            Mem->GetThreadMemSegs().front()->getTopAddr() );
  MainThreadRegState->SetX( RevReg::sp,
                            Mem->GetThreadMemSegs().front()->getTopAddr() -
                              Mem->GetTLSSize() );
  MainThreadRegState->SetX( RevReg::gp,
                            Loader->GetSymbolAddr( "__global_pointer$" ) );
  MainThreadRegState->SetX( 8, Loader->GetSymbolAddr( "__global_pointer$" ) );
  SetupArgs( MainThreadRegState );
  std::unique_ptr< RevThread > MainThread =
    std::make_unique< RevThread >( MainThreadID,
                                   _INVALID_TID_,  // No Parent Thread ID
                                   Mem->GetThreadMemSegs().front(),
                                   std::move( MainThreadRegState ) );
  MainThread->SetState( ThreadState::READY );

  output.verbose( CALL_INFO,
                  11,
                  0,
                  "Main thread initialized %s\n",
                  MainThread->to_string().c_str() );

  // Add to ReadyThreads so it gets scheduled
  ReadyThreads.emplace_back( std::move( MainThread ) );
}

}  // namespace SST::RevCPU

// EOF
