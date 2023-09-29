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

#include "SST.h"

#include <cstdint>
#include <functional>
#include <limits>
#include <random>
#include <thread>
#include <type_traits>

namespace SST::RevCPU{

/// Wrapper class to SST::RNG::MersenneRNG
class RevRNG{
  SST::RNG::MersenneRNG SSTRNG;

  // Hardware random seed is different from run to run but fixed during run
  // so that distribution of HWSeed ^ ThreadSeed is uniform
  static uint32_t HWSeed(){
    static const uint32_t RevHWRNG = std::random_device{}();
    return RevHWRNG;
  }

  // Thread seed is based on thread ID of host
  static uint32_t ThreadSeed(){
    return uint32_t(std::hash<std::thread::id>{}(std::this_thread::get_id()));
  }

public:
  using result_type = uint64_t;
  static constexpr result_type min() { return std::numeric_limits<result_type>::min(); }
  static constexpr result_type max() { return std::numeric_limits<result_type>::max(); }
  result_type operator()() { return SSTRNG.generateNextUInt64(); }
  void seed(result_type seed) { *this = RevRNG(seed); }

  // Default seed is combination of HWSeed and ThreadSeed
  explicit RevRNG(result_type seed = HWSeed() ^ ThreadSeed()) : SSTRNG(uint32_t(seed)) {}

  // Not implemented:
  // discard()
  // operator==()
  // operator!=()
  // operator<<()
  // operator>>()
};

/// Random Number Generator
// Returns a value in [min, max] (integer) or [min, max) (floating-point)
template<typename T, typename U>
inline auto RevRand(T min, U max){
  // Thread-local RNG which is seeded differently for each thread
  thread_local RevRNG RNG;
  using TU = std::common_type_t<T, U>;
  if constexpr(std::is_floating_point_v<TU>){
    return std::uniform_real_distribution<TU>(min, max)(RNG);
  }else{
    return std::uniform_int_distribution<TU>(min, max)(RNG);
  }
}

} // namespace SST::RevCPU

#endif
