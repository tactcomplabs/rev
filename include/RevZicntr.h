//
// _RevZicntr_h_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVZICNTR_H_
#define _SST_REVZICNTR_H_

#include <cstdint>
#include <type_traits>

// Zicntr performance counters

namespace SST::RevCPU {

class RevCore;

class RevZicntr {
  uint64_t InstRet{};  ///< RevZicntr: Number of instructions retired

  /// RevZicntr: Get the core owning this hart
  virtual RevCore* GetCore() const = 0;

  /// RevZicntr: Get the Program Counter
  virtual uint64_t GetPC() const   = 0;

  /// RevZicntr: Is this RV64?
  virtual bool IsRV64() const      = 0;

  // Performance counters
  // Template allows RevCore to be an incomplete type now
  // std::enable_if_t<...> makes the functions only match ZICNTR == RevZicntr

  template<typename ZICNTR, typename = std::enable_if_t<std::is_same_v<ZICNTR, RevZicntr>>>
  static void fatal( const ZICNTR* Zicntr, const char* msg ) {
    return Zicntr->GetCore()->output->fatal( CALL_INFO, -1, msg, Zicntr->GetPC() );
  }

  template<typename ZICNTR, typename = std::enable_if_t<std::is_same_v<ZICNTR, RevZicntr>>>
  static bool isZicntr( const ZICNTR* Zicntr ) {
    return Zicntr->GetCore()->GetRevFeature()->IsModeEnabled( RV_ZICNTR );
  }

protected:
  template<typename ZICNTR, typename = std::enable_if_t<std::is_same_v<ZICNTR, RevZicntr>>>
  static uint64_t rdcycle( const ZICNTR* Zicntr ) {
    return Zicntr->GetCore()->GetCycles();
  }

  template<typename ZICNTR, typename = std::enable_if_t<std::is_same_v<ZICNTR, RevZicntr>>>
  static uint64_t rdtime( const ZICNTR* Zicntr ) {
    return Zicntr->GetCore()->GetCurrentSimCycle();
  }

  template<typename ZICNTR, typename = std::enable_if_t<std::is_same_v<ZICNTR, RevZicntr>>>
  static uint64_t rdinstret( const ZICNTR* Zicntr ) {
    return Zicntr->InstRet;
  }

  enum class Half { Lo, Hi };

  /// Performance Counter template
  // Passed a COUNTER function which gets the 64-bit value of a performance counter
  template<typename XLEN, Half HALF, uint64_t COUNTER( const RevZicntr* )>
  XLEN GetPerfCounter() const {
    if( !isZicntr( this ) ) {
      fatal( this, "Illegal instruction at PC = 0x%" PRIx64 ": Zicntr extension not available\n" );
      return 0;
    } else if( IsRV64() ) {
      if constexpr( HALF == Half::Hi ) {
        fatal( this, "Illegal instruction at PC = 0x%" PRIx64 ": High half of Zicntr register not available on RV64\n" );
        return 0;
      } else {
        return COUNTER( this );
      }
    } else {
      if constexpr( HALF == Half::Hi ) {
        return COUNTER( this ) >> 32;
      } else {
        return COUNTER( this ) & 0xffffffff;
      }
    }
  }

public:
  /// RevZicntr: Increment the number of retired instructions
  void IncrementInstRet() { ++InstRet; }
};  // class RevZicntr

}  // namespace SST::RevCPU

#endif
