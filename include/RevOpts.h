//
// _RevOpts_h_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
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
#include "RevCommon.h"
#include <cinttypes>
#include <map>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace SST::RevCPU {

class RevOpts {

  // Queries whether type is a std::vector type
  template<typename T>
  struct is_vector : std::false_type {};

  template<typename T>
  struct is_vector<std::vector<T>> : std::true_type {};

  // Return a property by looking up its table
  // VAL is a universal reference so that either an lvalue reference or an assignable rvalue reference such as std::tie can be used
  template<typename MAP, typename VAL>
  bool GetProperty( uint32_t Core, const MAP& Map, VAL&& Val ) const {
    if constexpr( is_vector<MAP>::value ) {
      // If MAP is a vector
      return Core < numCores ? Val = Map[Core], true : false;
    } else {
      // If MAP is a map/unordered_map
      auto it                      = Map.find( Core );
      return it != Map.end() ? Val = it->second, true : false;
    }
  }

  template<typename VEC>
  bool InitPropertyMap( const std::vector<std::string>& Opts, VEC& map );

  template<typename VEC>
  bool InitPropertyMapCores( const std::vector<std::string>& Opts, VEC& map );

public:
  /// RevOpts: options constructor
  RevOpts( uint32_t NumCores, uint32_t NumHarts, int Verbosity )
    : numCores( NumCores ), numHarts( NumHarts ), verbosity( Verbosity ) {}

  /// RevOpts: Disallow copying and assignment
  RevOpts( const RevOpts& )            = delete;
  RevOpts( RevOpts&& )                 = delete;
  RevOpts& operator=( const RevOpts& ) = delete;
  RevOpts& operator=( RevOpts&& )      = delete;

  /// RevOpts: options destructor
  ~RevOpts()                           = default;

  /// RevOpts: retrieve the number of configured cores
  uint32_t GetNumCores() const { return numCores; }

  /// RevOpts: retrieve the number of configured harts per core
  uint32_t GetNumHarts() const { return numHarts; }

  /// RevOpts: retrieve the verbosity level
  int GetVerbosity() const { return verbosity; }

  /// RevOpts: initialize the set of starting addresses
  bool InitStartAddrs( const std::vector<std::string>& StartAddrs );

  /// RevOpts: initialize the set of potential starting symbols
  bool InitStartSymbols( const std::vector<std::string>& StartSymbols );

  /// RevOpts: initialize the set of machine models
  bool InitMachineModels( const std::vector<std::string>& Machines );

  /// RevOpts: initalize the set of instruction tables
  bool InitInstTables( const std::vector<std::string>& InstTables );

  /// RevOpts: initialize the memory latency cost tables
  bool InitMemCosts( const std::vector<std::string>& MemCosts );

  /// RevOpts: initialize the prefetch depths
  bool InitPrefetchDepth( const std::vector<std::string>& Depths );

  /// RevOpts: retrieve the start address for the target core
  bool GetStartAddr( uint32_t Core, uint64_t& StartAddr ) const { return GetProperty( Core, startAddr, StartAddr ); }

  /// RevOpts: retrieve the start symbol for the target core
  bool GetStartSymbol( uint32_t Core, std::string& Symbol ) const { return GetProperty( Core, startSym, Symbol ); }

  /// RevOpts: retrieve the machine model string for the target core
  bool GetMachineModel( uint32_t Core, std::string& MachModel ) const { return GetProperty( Core, machine, MachModel ); }

  /// RevOpts: retrieve instruction table for the target core
  bool GetInstTable( uint32_t Core, std::string& Table ) const { return GetProperty( Core, table, Table ); }

  /// RevOpts: retrieve the memory cost range for the target core
  bool GetMemCost( uint32_t Core, uint32_t& Min, uint32_t& Max ) const {
    return GetProperty( Core, memCosts, std::tie( Min, Max ) );
  }

  /// RevOpts: retrieve the prefetch depth for the target core
  bool GetPrefetchDepth( uint32_t Core, uint32_t& Depth ) const { return GetProperty( Core, prefetchDepth, Depth ); }

  /// RevOpts: set the argv array
  void SetArgs( const SST::Params& params );

  /// RevOpts: retrieve the argv array
  const std::vector<std::string>& GetArgv() const { return Argv; }

  /// RevOpts: splits a string into tokens
  static void splitStr( std::string s, const char* delim, std::vector<std::string>& v ) {
    char* ptr     = s.data();
    char* saveptr = nullptr;
    for( v.clear(); char* token = strtok_r( ptr, delim, &saveptr ); ptr = nullptr )
      v.push_back( token );
  }

private:
  uint32_t const numCores;   ///< RevOpts: number of initialized cores
  uint32_t const numHarts;   ///< RevOpts: number of harts per core
  int const      verbosity;  ///< RevOpts: verbosity level

  // init all the standard options
  // -- startAddr = 0x00000000
  // -- machine = "G" aka, "IMAFD"
  // -- table = internal
  // -- memCosts[core] = 0:10
  // -- prefetch depth = 16
  // -- pipeLine = 5 ???

  // clang-format off
  std::vector<uint64_t>                     startAddr{decltype( startAddr     )( numCores, 0 )};                 ///< RevOpts: starting address
  std::vector<std::string>                    machine{decltype( machine       )( numCores, "G" )};               ///< RevOpts: machine model
  std::vector<std::string>                      table{decltype( table         )( numCores, "_REV_INTERNAL_" )};  ///< RevOpts: inst table
  std::vector<std::pair<uint32_t, uint32_t>> memCosts{decltype( memCosts      )( numCores, {0, 10} )};           ///< RevOpts: memory cost range
  std::vector<uint32_t>                 prefetchDepth{decltype( prefetchDepth )( numCores, 16 )};                ///< RevOpts: prefretch depth
  // clang-format on

  std::unordered_map<uint32_t, std::string> startSym;       ///< RevOpts: starting symbol
  std::vector<std::string>                  Argv;           ///< RevOpts: vector of function arguments
  std::vector<std::string>                  MemDumpRanges;  ///< RevOpts: vector of function arguments

};  // class RevOpts

}  // namespace SST::RevCPU

#endif

// EOF
