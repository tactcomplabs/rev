//
// _RevRand_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_REVRAND_H_
#define _SST_REVCPU_REVRAND_H_

#include <cstdint>
#include <random>

namespace SST{
namespace RevCPU{

/// Random Number Generator
inline uint32_t RevRand(uint32_t low, uint32_t high){
  static std::mt19937 r(std::random_device{}());
  return std::uniform_int_distribution<uint32_t>(low, high)(r);
}

} // namespace RevCPU
} // namespace SST

#endif
