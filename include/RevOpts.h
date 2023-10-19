//
// _RevOpts_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_REVOPTS_H_
#define _SST_REVCPU_REVOPTS_H_

// -- SST Headers
#include "SST.h"

// -- Standard Headers
#include <cinttypes>
#include <map>
#include <vector>
#include <string>
#include <utility>

namespace SST::RevCPU{

class RevOpts{
public:
  /// RevOpts: options constructor
  RevOpts( unsigned NumCores, unsigned NumHarts, const int Verbosity);

  /// RevOpts: options destructor
  ~RevOpts();

  /// RevOpts: retrieve the number of configured cores
  unsigned GetNumCores() { return numCores; }

  /// RevOpts: retrieve the number of configured harts per core
  unsigned GetNumHarts() { return numHarts; }

  /// RevOpts: retrieve the verbosity level
  int GetVerbosity() { return verbosity; }

  /// RevOpts: initialize the set of starting addresses
  bool InitStartAddrs( std::vector<std::string> StartAddrs );

  /// RevOpts: initialize the set of potential starting symbols
  bool InitStartSymbols( std::vector<std::string> StartSymbols );

  /// RevOpts: initialize the set of machine models
  bool InitMachineModels( std::vector<std::string> Machines );

  /// RevOpts: initalize the set of instruction tables
  bool InitInstTables( std::vector<std::string> InstTables );

  /// RevOpts: initialize the memory latency cost tables
  bool InitMemCosts( std::vector<std::string> MemCosts );

  /// RevOpts: initialize the prefetch depths
  bool InitPrefetchDepth( std::vector<std::string> Depths );

  /// RevOpts: retrieve the start address for the target core
  bool GetStartAddr( unsigned Core, uint64_t &StartAddr );

  /// RevOpts: retrieve the start symbol for the target core
  bool GetStartSymbol( unsigned Core, std::string &Symbol );

  /// RevOpts: retrieve the machine model string for the target core
  bool GetMachineModel( unsigned Core, std::string &MachModel );

  /// RevOpts: retrieve instruction table for the target core
  bool GetInstTable( unsigned Core, std::string &Table );

  /// RevOpts: retrieve the memory cost range for the target core
  bool GetMemCost( unsigned Core, unsigned &Min, unsigned &Max );

  /// RevOpts: retrieve the prefetch depth for the target core
  bool GetPrefetchDepth( unsigned Core, unsigned &Depth );

  /// RevOpts: set the argv arrary
  void SetArgs(std::vector<std::string> A){ Argv = A; }

  /// RevOpts: retrieve the argv array
  std::vector<std::string> GetArgv() { return Argv; }

private:
  unsigned numCores;                            ///< RevOpts: number of initialized cores
  unsigned numHarts;                            ///< RevOpts: number of harts per core
  int verbosity;                                ///< RevOpts: verbosity level

  std::map<unsigned, uint64_t> startAddr;        ///< RevOpts: map of core id to starting address
  std::map<unsigned, std::string> startSym;      ///< RevOpts: map of core id to starting symbol
  std::map<unsigned, std::string> machine;       ///< RevOpts: map of core id to machine model
  std::map<unsigned, std::string> table;         ///< RevOpts: map of core id to inst table
  std::map<unsigned, unsigned> prefetchDepth;    ///< RevOpts: map of core id to prefretch depth

  std::vector<std::pair<unsigned, unsigned>> memCosts; ///< RevOpts: vector of memory cost ranges

  std::vector<std::string> Argv;                ///< RevOpts: vector of function arguments

  /// RevOpts: splits a string into tokens
  void splitStr(const std::string& s, char c, std::vector<std::string>& v);

}; // class RevOpts

} // namespace SST::RevCPU

#endif

// EOF
