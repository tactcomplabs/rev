//
// _RevFlag_h_
//
// Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVFLAG_H_
#define _SST_REVFLAG_H_

#include <cstdint>
#include <type_traits>

#include "RevCommon.h"
#include "SST.h"

namespace SST::RevCPU {

using namespace SST::Interfaces;

// ----------------------------------------
// Extended StandardMem::Request::Flag enums
// ----------------------------------------
enum class RevFlag : uint32_t {
  F_NONE               = 0,        /// no special operation
  F_NONCACHEABLE       = 1u << 1,  /// non cacheable

  F_BOXNAN             = 1u << 16,  /// NaN-box the 32-bit float
  F_SEXT32             = 1u << 17,  /// sign extend the 32bit result
  F_SEXT64             = 1u << 18,  /// sign extend the 64bit result
  F_ZEXT32             = 1u << 19,  /// zero extend the 32bit result
  F_ZEXT64             = 1u << 20,  /// zero extend the 64bit result
  F_RESP               = F_BOXNAN | F_SEXT32 | F_SEXT64 | F_ZEXT32 | F_ZEXT64,

  F_AQ                 = 1u << 21,  /// AMO AQ Flag
  F_RL                 = 1u << 22,  /// AMO RL Flag

  F_AMOADD             = 1u << 23,   /// AMO Add
  F_AMOXOR             = 2u << 23,   /// AMO Xor
  F_AMOAND             = 3u << 23,   /// AMO And
  F_AMOOR              = 4u << 23,   /// AMO Or
  F_AMOMIN             = 5u << 23,   /// AMO Min
  F_AMOMAX             = 6u << 23,   /// AMO Max
  F_AMOMINU            = 7u << 23,   /// AMO Minu
  F_AMOMAXU            = 8u << 23,   /// AMO Maxu
  F_AMOSWAP            = 9u << 23,   /// AMO Swap
  F_FORZASUB           = 10u << 23,  /// XForza: AMO SUB
  F_FORZATHRS          = 11u << 23,  /// XForza: AMO THRS

  F_FORZAFADD          = 1u << 27,  /// XForza: AMO FADD
  F_FORZAFSUB          = 2u << 27,  /// XForza: AMO FSUB
  F_FORZAFSUBR         = 3u << 27,  /// XForza: AMO FSUBR
  F_FORZAFMIN          = 3u << 27,  /// XForza: AMO FSUBR
  F_FORZAFMAX          = 4u << 27,  /// XForza: AMO FSUBR
  F_FORZA_ATOMIC_FLOAT = F_FORZAFADD | F_FORZAFSUB | F_FORZAFSUBR | F_FORZAFMIN | F_FORZAFMAX,

  F_ATOMIC = F_AMOADD | F_AMOXOR | F_AMOAND | F_AMOOR | F_AMOMIN | F_AMOMAX | F_AMOMINU | F_AMOMAXU | F_AMOSWAP | F_FORZASUB |
             F_FORZATHRS | F_FORZA_ATOMIC_FLOAT,

  F_FORZANN = 1u << 30,  /// XForza: AMO RETURN NN
  F_FORZAON = 2u << 30,  /// XForza: AMO RETURN ON
  F_FORZANO = 3u << 30,  /// XForza: AMO RETURN NO
  F_RETURN  = F_FORZANN | F_FORZAON | F_FORZANO,
};

// Ensure RevFlag is same underlying type as StandardMem::Request::flags_t
static_assert( std::is_same_v<StandardMem::Request::flags_t, std::underlying_type_t<RevFlag>> );

/// RevFlag: determine if the request has certain flags set
constexpr bool RevFlagHas( RevFlag flag, RevFlag has ) {
  return ( safe_static_cast<uint32_t>( flag ) & safe_static_cast<uint32_t>( has ) ) == safe_static_cast<uint32_t>( has );
}

/// RevFlag: set certain flags
constexpr void RevFlagSet( RevFlag& flag, RevFlag set ) {
  flag = RevFlag{ safe_static_cast<uint32_t>( flag ) | safe_static_cast<uint32_t>( set ) };
}

/// RevFlag: determine if the request is an AMO, and if so, return the operation; otherwise return 0
constexpr RevFlag RevFlagAtomic( RevFlag flag ) {
  return RevFlag{ safe_static_cast<uint32_t>( flag ) & safe_static_cast<uint32_t>( RevFlag::F_ATOMIC ) };
}

/// RevFlag: determine if the request is a float AMO, and if so, return the operation; otherwise return 0
constexpr RevFlag RevFlagAtomicFloat( RevFlag flag ) {
  return RevFlag{ safe_static_cast<uint32_t>( flag ) & safe_static_cast<uint32_t>( RevFlag::F_FORZA_ATOMIC_FLOAT ) };
}

/// RevFlag: determine the return flags
constexpr RevFlag RevFlagReturn( RevFlag flag ) {
  return RevFlag{ safe_static_cast<uint32_t>( flag ) & safe_static_cast<uint32_t>( RevFlag::F_RETURN ) };
}

/// RevFlag: determine which response flags are present
constexpr RevFlag RevFlagResp( RevFlag flag ) {
  return RevFlag{ safe_static_cast<uint32_t>( flag ) & safe_static_cast<uint32_t>( RevFlag::F_RESP ) };
}

// RevFlag: determine if the request is cache-able
constexpr bool isCacheable( RevFlag flag ) {
  return ( safe_static_cast<uint32_t>( flag ) & safe_static_cast<uint32_t>( RevFlag::F_NONCACHEABLE ) ) == 0;
}

}  // namespace SST::RevCPU

#endif  // _SST_REVFLAG_H_
