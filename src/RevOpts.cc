//
// _RevOpts_cc_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "RevOpts.h"

namespace SST::RevCPU {

void RevOpts::SetArgs( const SST::Params& params ) {
  static constexpr char delim[] = " \t\n";

  // If the "args" param does not start with a left bracket, split it up at whitespace
  // Otherwise interpet it as an array
  std::string args              = params.find<std::string>( "args" );
  auto        nonspace          = args.find_first_not_of( delim );
  if( nonspace == args.npos || args[nonspace] != '[' ) {
    RevOpts::splitStr( args, delim, Argv );
  } else {
    params.find_array( "args", Argv );
  }
}

template<typename MAP>
bool RevOpts::InitPropertyMap( const std::vector<std::string>& Opts, MAP& map ) {
  for( auto& s : Opts ) {
    std::vector<std::string> vstr;

    splitStr( s, ":", vstr );
    if( vstr.size() != 2 )
      return false;

    auto Core = std::stoull( vstr[0], nullptr, 0 );
    if( Core >= numCores )
      return false;

    // Store as cast integer if target is integer; otherwise store as string
    auto parse = [&]( auto val ) {
      if constexpr( std::is_integral_v<decltype( val )> ) {
        map[Core] = decltype( val )( std::stoull( vstr[1], nullptr, 0 ) );
      } else {
        map[Core] = vstr[1];
      }
    };

    if constexpr( is_vector<MAP>::value ) {
      parse( typename MAP::value_type{} );
    } else {
      parse( typename MAP::mapped_type{} );
    }
  }

  return true;
}

template<typename MAP>
std::pair<bool, bool> RevOpts::InitPropertyMapCores( const std::vector<std::string>& Opts, MAP& map ) {
  // check to see if we expand into multiple cores
  if( Opts.size() == 1 ) {
    auto&                    s = Opts[0];
    std::vector<std::string> vstr;

    splitStr( s, ":", vstr );
    if( vstr.size() != 2 )
      return { true, false };

    if( vstr[0] == "CORES" ) {

      // set all cores to the value, stored as cast integer or as string
      auto parse = [&]( auto val ) {
        if constexpr( std::is_integral_v<decltype( val )> ) {
          auto Val = decltype( val )( std::stoull( vstr[1], nullptr, 0 ) );
          for( uint32_t i = 0; i < numCores; i++ )
            map[i] = Val;
        } else {
          for( uint32_t i = 0; i < numCores; i++ )
            map[i] = vstr[1];
        }
      };

      if constexpr( is_vector<MAP>::value ) {
        parse( typename MAP::value_type{} );
      } else {
        parse( typename MAP::mapped_type{} );
      }

      return { true, true };
    }
  }
  return { false, false };
}

/// RevOpts: initialize the set of starting addresses
bool RevOpts::InitStartAddrs( const std::vector<std::string>& StartAddrs ) {
  auto [ret, retval] = InitPropertyMapCores( StartAddrs, startAddr );
  return ret ? retval : InitPropertyMap( StartAddrs, startAddr );
}

/// RevOpts: initialize the set of potential starting symbols
bool RevOpts::InitStartSymbols( const std::vector<std::string>& StartSymbols ) {
  return InitPropertyMap( StartSymbols, startSym );
}

/// RevOpts: initialize the set of machine models
bool RevOpts::InitMachineModels( const std::vector<std::string>& Machines ) {
  auto [ret, retval] = InitPropertyMapCores( Machines, machine );
  return ret ? retval : InitPropertyMap( Machines, machine );
}

/// RevOpts: initalize the set of instruction tables
bool RevOpts::InitInstTables( const std::vector<std::string>& InstTables ) {
  return InitPropertyMap( InstTables, table );
}

/// RevOpts: initialize the prefetch depths
bool RevOpts::InitPrefetchDepth( const std::vector<std::string>& Depths ) {
  return InitPropertyMap( Depths, prefetchDepth );
}

/// RevOpts: initialize the memory latency cost tables
bool RevOpts::InitMemCosts( const std::vector<std::string>& MemCosts ) {
  for( auto& s : MemCosts ) {
    std::vector<std::string> vstr;
    splitStr( s, ":", vstr );
    if( vstr.size() != 3 )
      return false;
    auto Core = std::stoull( vstr[0], nullptr, 0 );
    auto Min  = decltype( memCosts[Core].first )( std::stoull( vstr[1], nullptr, 0 ) );
    auto Max  = decltype( memCosts[Core].second )( std::stoull( vstr[2], nullptr, 0 ) );
    if( Core >= numCores || !Min || !Max )
      return false;
    memCosts[Core] = std::pair( Min, Max );
  }
  return true;
}

}  // namespace SST::RevCPU
