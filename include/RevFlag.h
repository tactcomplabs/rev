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

#include "RevCommon.h"
#include "SST.h"

namespace SST::RevCPU {

// ----------------------------------------
// Extended StandardMem::Request::Flag enums
// ----------------------------------------
using flags_t = StandardMem::Request::flags_t;

enum class RevFlag : flags_t {
  F_NONE         = 0,          /// no special operation
  F_NONCACHEABLE = 1u << 1,    /// non cacheable
  F_BOXNAN       = 1u << 16,   /// NaN-box the 32-bit float
  F_SEXT32       = 1u << 17,   /// sign extend the 32bit result
  F_SEXT64       = 1u << 18,   /// sign extend the 64bit result
  F_ZEXT32       = 1u << 19,   /// zero extend the 32bit result
  F_ZEXT64       = 1u << 20,   /// zero extend the 64bit result
  F_AQ           = 1u << 21,   /// AMO AQ Flag
  F_RL           = 1u << 22,   /// AMO RL Flag
  F_AMOADD       = 1u << 23,   /// AMO Add
  F_AMOXOR       = 2u << 23,   /// AMO Xor
  F_AMOAND       = 3u << 23,   /// AMO And
  F_AMOOR        = 4u << 23,   /// AMO Or
  F_AMOMIN       = 5u << 23,   /// AMO Min
  F_AMOMAX       = 6u << 23,   /// AMO Max
  F_AMOMINU      = 7u << 23,   /// AMO Minu
  F_AMOMAXU      = 8u << 23,   /// AMO Maxu
  F_AMOSWAP      = 9u << 23,   /// AMO Swap
  F_AMOSUB       = 10u << 23,  /// AMO Sub
  F_AMOTHRES     = 11u << 23,  /// AMO Threshold
  F_AMOCAS       = 12u << 23,  /// AMO Compare and swap
  F_AMOFADD      = 1u << 27,   /// AMO Fadd
  F_AMOFSUB      = 2u << 27,   /// AMO Fsub
  F_AMOFSUBR     = 3u << 27,   /// AMO Fsubr
  F_AMONN        = 1u << 29,   /// RevFlag: AMO RETURN NN
  F_AMOON        = 2u << 29,   /// RevFlag: AMO RETURN ON
  F_AMONO        = 3u << 29,   /// RevFlag: AMO RETURN NO
  F_ATOMIC_RESP  = F_BOXNAN | F_SEXT32 | F_SEXT64 | F_ZEXT32 | F_ZEXT64,
  F_ATOMIC_FLOAT = F_AMOFADD | F_AMOFSUB | F_AMOFSUBR,
  F_ATOMIC       = F_AMOADD | F_AMOXOR | F_AMOAND | F_AMOOR | F_AMOMIN | F_AMOMAX | F_AMOMINU | F_AMOMAXU | F_AMOSWAP | F_AMOSUB |
             F_AMOTHRES | F_AMOCAS | F_ATOMIC_FLOAT,
  F_ATOMIC_RETURN = F_AMONN | F_AMOON | F_AMONO,
};

/// RevFlag: determine if the request has certain flags set
constexpr bool RevFlagHas( RevFlag flag, RevFlag has ) {
  return ( safe_static_cast<flags_t>( flag ) & safe_static_cast<flags_t>( has ) ) == safe_static_cast<flags_t>( has );
}

/// RevFlag: set certain flags
constexpr void RevFlagSet( RevFlag& flag, RevFlag set ) {
  flag = RevFlag{ safe_static_cast<flags_t>( flag ) | safe_static_cast<flags_t>( set ) };
}

/// RevFlag: determine if the request is an AMO, and if so, return the operation; otherwise return 0
constexpr RevFlag RevFlagAtomic( RevFlag flag ) {
  return RevFlag{ safe_static_cast<flags_t>( flag ) & safe_static_cast<flags_t>( RevFlag::F_ATOMIC ) };
}

/// RevFlag: determine if the request is a float AMO, and if so, return the operation; otherwise return 0
constexpr RevFlag RevFlagAtomicFloat( RevFlag flag ) {
  return RevFlag{ safe_static_cast<flags_t>( flag ) & safe_static_cast<flags_t>( RevFlag::F_ATOMIC_FLOAT ) };
}

/// RevFlag: determine the return flags
constexpr RevFlag RevFlagReturn( RevFlag flag ) {
  return RevFlag{ safe_static_cast<flags_t>( flag ) & safe_static_cast<flags_t>( RevFlag::F_ATOMIC_RETURN ) };
}

/// RevFlag: determine which response flags are present
constexpr RevFlag RevFlagResp( RevFlag flag ) {
  return RevFlag{ safe_static_cast<flags_t>( flag ) & safe_static_cast<flags_t>( RevFlag::F_ATOMIC_RESP ) };
}

// RevFlag: determine if the request is cache-able
constexpr bool isCacheable( RevFlag flag ) {
  return ( safe_static_cast<flags_t>( flag ) & safe_static_cast<flags_t>( RevFlag::F_NONCACHEABLE ) ) == 0;
}

}  // namespace SST::RevCPU

#endif  // _SST_REVFLAG_H_
