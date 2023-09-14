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
#include <functional>
#include <thread>
#include <type_traits>

namespace SST::RevCPU{

/// Singleton to return thread-local RNG, initializing it only on-demand
inline auto& RevRNG(){
  // Non-deterministic hardware random number generator
  static std::random_device RevHWRNG;

  // All threads share the same hardware seed which changes from run to run
  static uint64_t HWSeed = uint64_t{RevHWRNG()} << 32 | RevHWRNG();

  // Thread-local deterministic pseudo random number generator
  // The hardware random seed and a thread id hash are combined into a seed
  // See https://www.youtube.com/watch?v=LDPMpc-ENqY
  thread_local std::mt19937_64 RevRNG{HWSeed ^ std::hash<std::thread::id>{}(std::this_thread::get_id())};

  return RevRNG;
}

/// Random Number Generator
// Returns a value in [min, max] (integer) or [min, max) (floating-point)
template<typename T, typename U>
inline auto RevRand(T min, U max){
  using TU = std::common_type_t<T, U>;
  if constexpr(std::is_floating_point_v<TU>){
    return std::uniform_real_distribution<TU>(min, max)(RevRNG());
  }else{
    return std::uniform_int_distribution<TU>(min, max)(RevRNG());
  }
}

} // namespace SST::RevCPU

#endif
