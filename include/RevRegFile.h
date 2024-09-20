//
// _RevRegFile_h_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_REVREGFILE_H_
#define _SST_REVCPU_REVREGFILE_H_

#include <bitset>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <memory>
#include <ostream>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include "../common/include/RevCommon.h"
#include "RevCSR.h"
#include "RevTracer.h"

namespace SST::RevCPU {

// Mappings from floating point to same-sized integer types
template<typename T>
struct uint_type {};

template<>
struct uint_type<double> {
  using type = uint64_t;
  static_assert( sizeof( type ) == sizeof( double ) );
};

template<>
struct uint_type<float> {
  using type = uint32_t;
  static_assert( sizeof( type ) == sizeof( float ) );
};

#if 0
template<>
struct uint_type<float16> {
  using type = uint16_t;
  static_assert( sizeof( type ) == sizeof( float16 ) );
};
#endif

template<typename T>
using uint_type_t = typename uint_type<T>::type;

/// BoxNaN: Store a boxed floating point value inside a possibly larger one
template<typename T, typename U, typename = std::enable_if_t<sizeof( T ) >= sizeof( U )>>
inline void BoxNaN( T* dest, const U* value ) {
  if constexpr( sizeof( T ) == sizeof( U ) ) {
    *dest = *value;
  } else {
    uint_type_t<U> i;
    memcpy( &i, value, sizeof( i ) );                                                    // The value
    uint_type_t<T> box = uint_type_t<T>{ i } | ~uint_type_t<T>{ 0 } << sizeof( U ) * 8;  // Boxed NaN value
    memcpy( dest, &box, sizeof( box ) );                                                 // Store in larger register
    static_assert( sizeof( i ) == sizeof( U ) && sizeof( box ) == sizeof( T ) );
  }
}

/// UnBoxNaN: Unbox a floating point value into a possibly smaller one
// The second argument indicates whether it is a FMV/FS move/store
// instruction which just transfers bits and not care about NaN-Boxing.
template<typename T, bool FMV_FS = false, typename U, typename = std::enable_if_t<sizeof( T ) <= sizeof( U )>>
inline T UnBoxNaN( const U* val ) {
  if constexpr( sizeof( T ) == sizeof( U ) ) {
    return *val;
  } else {
    uint_type_t<U> i;
    memcpy( &i, val, sizeof( i ) );
    static_assert( sizeof( i ) == sizeof( val ) );
    T fp;
    if( !FMV_FS && ~i >> sizeof( T ) * 8 ) {
      fp = std::numeric_limits<T>::quiet_NaN();
    } else {
      auto ifp = static_cast<uint_type_t<T>>( i );
      memcpy( &fp, &ifp, sizeof( fp ) );
      static_assert( sizeof( ifp ) == sizeof( fp ) );
    }
    return fp;
  }
}

/// RISC-V Register Mneumonics
// clang-format off
enum class RevReg : uint16_t {
  zero =  0, ra  =  1, sp   =  2, gp   =  3, tp  =  4, t0  =  5, t1   =  6, t2   =  7,
  s0   =  8, s1  =  9, a0   = 10, a1   = 11, a2  = 12, a3  = 13, a4   = 14, a5   = 15,
  fp   =  8,
  a6   = 16, a7  = 17, s2   = 18, s3   = 19, s4  = 20, s5  = 21, s6   = 22, s7   = 23,
  s8   = 24, s9  = 25, s10  = 26, s11  = 27, t3  = 28, t4  = 29, t5   = 30, t6   = 31,
  ft0  =  0, ft1 =  1, ft2  =  2, ft3  =  3, ft4 =  4, ft5 =  5, ft6  =  6, ft7  =  7,
  fs0  =  8, fs1 =  9, fa0  = 10, fa1  = 11, fa2 = 12, fa3 = 13, fa4  = 14, fa5  = 15,
  fa6  = 16, fa7 = 17, fs2  = 18, fs3  = 19, fs4 = 20, fs5 = 21, fs6  = 22, fs7  = 23,
  fs8  = 24, fs9 = 25, fs10 = 26, fs11 = 27, ft8 = 28, ft9 = 29, ft10 = 30, ft11 = 31,
};
// clang-format on

// Ref: RISC-V Privileged Spec (pg. 39)
enum class RevExceptionCause : int32_t {
  NONE                      = -1,
  MISALIGNED_INST_ADDR      = 0,
  INST_ACCESS_FAULT         = 1,
  ILLEGAL_INST              = 2,
  BREAKPOINT                = 3,
  LOAD_ADDR_MISALIGNED      = 4,
  LOAD_ACCESS_FAULT         = 5,
  STORE_AMO_ADDR_MISALIGNED = 6,
  STORE_AMO_ACCESS_FAULT    = 7,
  ECALL_USER_MODE           = 8,
  ECALL_SUPERVISOR_MODE     = 9,
  ECALL_MACHINE_MODE        = 11,
  INST_PAGE_FAULT           = 12,
  LOAD_PAGE_FAULT           = 13,
  STORE_AMO_PAGE_FAULT      = 15,
};

class RevCore;
class RevTracer;

class RevRegFile : public RevCSR {
public:
  RevCore* const Core;      ///< RevRegFile: Owning core of this register file's hart
  const bool     IsRV64_v;  ///< RevRegFile: Cached copy of Features->IsRV64()
  const bool     HasD_v;    ///< RevRegFile: Cached copy of Features->HasD()

  bool       trigger{};  ///< RevRegFile: Has the instruction been triggered?
  unsigned   Entry{};    ///< RevRegFile: Instruction entry
  uint32_t   cost{};     ///< RevRegFile: Cost of the instruction
  RevTracer* Tracer{};   ///< RegRegFile: Tracer object

  union {                // Anonymous union. We zero-initialize the largest member
    uint32_t RV32_PC;    ///< RevRegFile: RV32 PC
    uint64_t RV64_PC{};  ///< RevRegFile: RV64 PC
  };

  std::shared_ptr<std::unordered_multimap<uint64_t, MemReq>> LSQueue{};
  std::function<void( const MemReq& )>                       MarkLoadCompleteFunc{};

  union {                             // Anonymous union. We zero-initialize the largest member
    uint32_t RV32[_REV_NUM_REGS_];    ///< RevRegFile: RV32I register file
    uint64_t RV64[_REV_NUM_REGS_]{};  ///< RevRegFile: RV64I register file
  };

  union {                          // Anonymous union. We zero-initialize the largest member
    float  SPF[_REV_NUM_REGS_];    ///< RevRegFile: RVxxF register file
    double DPF[_REV_NUM_REGS_]{};  ///< RevRegFile: RVxxD register file
  };

  std::bitset<_REV_NUM_REGS_> RV_Scoreboard{};  ///< RevRegFile: Scoreboard for RV32/RV64 RF to manage pipeline hazard
  std::bitset<_REV_NUM_REGS_> FP_Scoreboard{};  ///< RevRegFile: Scoreboard for SPF/DPF RF to manage pipeline hazard

  union {                  // Anonymous union. We zero-initialize the largest member
    uint64_t RV64_SEPC{};  // Holds address of instruction that caused the exception (ie. ECALL)
    uint32_t RV32_SEPC;
  };

  RevExceptionCause SCAUSE = RevExceptionCause::NONE;  // Used to store cause of exception (ie. ECALL_USER_EXCEPTION)

  union {                   // Anonymous union. We zero-initialize the largest member
    uint64_t RV64_STVAL{};  // Used to store additional info about exception (ECALL does not use this and sets value to 0)
    uint32_t RV32_STVAL;
  };

#if 0  // not used
  union{  // Anonymous union. We zero-initialize the largest member
    uint64_t RV64_STVEC{};   // Holds the base address of the exception handling routine (trap handler) that the processor jumps to when and exception occurs
    uint32_t RV32_STVEC;
  };
#endif

public:
  // Constructor which takes a RevCore to indicate its hart's parent core
  // Template allows RevCore to be an incomplete type now
  // std::enable_if_t<...> makes the constructor only match CORE == RevCore
  template<typename CORE, typename = std::enable_if_t<std::is_same_v<CORE, RevCore>>>
  explicit RevRegFile( CORE* Core )
    : RevCSR( Core ), Core( Core ), IsRV64_v( Core->GetRevFeature()->IsRV64() ), HasD_v( Core->GetRevFeature()->HasD() ) {}

  /// RevRegFile: disallow copying and assignment
  RevRegFile( const RevRegFile& )            = delete;
  RevRegFile& operator=( const RevRegFile& ) = delete;

  /// RevRegFile: standard destructor
  ~RevRegFile()                              = default;

  // Feature tests
  ///< RevRegFile: Whether it is RV64
  bool IsRV64() const { return IsRV64_v; }

  ///< RevRegFile: Whenter it is D
  bool HasD() const { return HasD_v; }

  // Getters/Setters
  /// Get cost of the instruction
  const uint32_t& GetCost() const { return cost; }

  uint32_t& GetCost() { return cost; }

  /// Set cost of the instruction
  void SetCost( uint32_t c ) { cost = c; }

  /// Get whether the instruction has been triggered
  bool GetTrigger() const { return trigger; }

  /// Set whether the instruction has been triggered
  void SetTrigger( bool t ) { trigger = t; }

  /// Get the instruction entry
  unsigned GetEntry() const { return Entry; }

  /// Set the instruction entry
  void SetEntry( unsigned e ) { Entry = e; }

  /// Get the Load/Store Queue
  const auto& GetLSQueue() const { return LSQueue; }

  /// Set the Load/Store Queue
  void SetLSQueue( std::shared_ptr<std::unordered_multimap<uint64_t, MemReq>> lsq ) { LSQueue = std::move( lsq ); }

  /// Set the current tracer
  void SetTracer( RevTracer* t ) { Tracer = t; }

  /// Get the MarkLoadComplete function
  const std::function<void( const MemReq& )>& GetMarkLoadComplete() const { return MarkLoadCompleteFunc; }

  /// Set the MarkLoadComplete function
  void SetMarkLoadComplete( std::function<void( const MemReq& )> func ) { MarkLoadCompleteFunc = std::move( func ); }

  /// Invoke the MarkLoadComplete function
  void MarkLoadComplete( const MemReq& req ) const { MarkLoadCompleteFunc( req ); }

  /// Capture the PC of current instruction which raised exception
  void SetSEPC() {
    if( IsRV64() ) {
      RV64_SEPC = RV64_PC;
    } else {
      RV32_SEPC = RV32_PC;
    }
  }

  ///Set the value for extra information about exception
  /// (ECALL doesn't use it and sets it to 0)
  template<typename T>
  void SetSTVAL( T val ) {
    if( IsRV64() ) {
      RV64_STVAL = val;
    } else {
      RV32_STVAL = val;
    }
  }

  /// Set the exception cause
  void SetSCAUSE( RevExceptionCause val ) { SCAUSE = val; }

  /// Get the exception cause
  RevExceptionCause GetSCAUSE() const { return SCAUSE; }

  /// GetX: Get the specifed X register cast to a specific integral type
  template<typename T, typename U>
  T GetX( U rs ) const {
    T res;
    if( IsRV64() ) {
      res = RevReg( rs ) != RevReg::zero ? T( RV64[size_t( rs )] ) : 0;
      TRACE_REG_READ( size_t( rs ), uint64_t( res ) );
    } else {
      res = RevReg( rs ) != RevReg::zero ? T( RV32[size_t( rs )] ) : 0;
      TRACE_REG_READ( size_t( rs ), uint32_t( res ) );
    }
    return res;
  }

  /// SetX: Set the specifed X register to a specific value
  template<typename T, typename U>
  void SetX( U rd, T val ) {
    T res;
    if( IsRV64() ) {
      res                = RevReg( rd ) != RevReg::zero ? uint64_t( val ) : 0;
      RV64[size_t( rd )] = res;
      TRACE_REG_WRITE( size_t( rd ), uint64_t( res ) );
    } else {
      res                = RevReg( rd ) != RevReg::zero ? uint32_t( val ) : 0;
      RV32[size_t( rd )] = res;
      TRACE_REG_WRITE( size_t( rd ), uint32_t( res ) );
    }
  }

  /// GetPC: Get the Program Counter
  uint64_t GetPC() const {
    if( IsRV64() ) {
      return RV64_PC;
    } else {
      return RV32_PC;
    }
  }

  /// SetPC: Set the Program Counter to a specific value
  template<typename T>
  void SetPC( T val ) {
    if( IsRV64() ) {
      RV64_PC = static_cast<uint64_t>( val );
      TRACE_PC_WRITE( RV64_PC );
    } else {
      RV32_PC = static_cast<uint32_t>( val );
      TRACE_PC_WRITE( RV32_PC );
    }
  }

  /// AdvancePC: Advance the program counter to the next instruction
  // Note: This does not create tracer events like SetPC() does
  template<typename T>  // Used to allow RevInst to be incomplete type right now
  void AdvancePC( const T& Inst ) {
    if( IsRV64() ) {
      RV64_PC += Inst.instSize;
    } else {
      RV32_PC += Inst.instSize;
    }
  }

  /// GetFP: Get the specified FP register cast to a specific FP type
  // The second argument indicates whether it is a FMV/FS move/store
  // instruction which just transfers bits and not care about NaN-Boxing.
  template<typename T, bool FMV_FS = false, typename U>
  T GetFP( U rs ) const {
    if constexpr( std::is_same_v<T, double> ) {
      return DPF[size_t( rs )];
    } else if( HasD() ) {
      return UnBoxNaN<T, FMV_FS>( &DPF[size_t( rs )] );
    } else {
      return UnBoxNaN<T, FMV_FS>( &SPF[size_t( rs )] );
    }
  }

  /// SetFP: Set a specific FP register to a floating-point value
  template<typename T, typename U>
  void SetFP( U rd, T value ) {
    if constexpr( std::is_same_v<T, double> ) {
      DPF[size_t( rd )] = value;
    } else if( HasD() ) {
      BoxNaN( &DPF[size_t( rd )], &value );
    } else {
      BoxNaN( &SPF[size_t( rd )], &value );
    }
  }

  // Friend functions and classes to access internal register state
  template<typename INT, typename FP>
  friend bool fcvtif( const class RevFeature* F, RevRegFile* R, class RevMem* M, const class RevInst& Inst );

  template<typename T>
  friend bool load( const class RevFeature* F, RevRegFile* R, class RevMem* M, const class RevInst& Inst );

  template<typename T>
  friend bool store( const class RevFeature* F, RevRegFile* R, class RevMem* M, const class RevInst& Inst );

  template<typename T>
  friend bool fload( const class RevFeature* F, RevRegFile* R, class RevMem* M, const class RevInst& Inst );

  template<typename T>
  friend bool fstore( const class RevFeature* F, RevRegFile* R, class RevMem* M, const class RevInst& Inst );

  template<typename T, template<class> class OP>
  friend bool foper( const class RevFeature* F, RevRegFile* R, class RevMem* M, const class RevInst& Inst );

  template<typename T, template<class> class OP>
  friend bool fcondop( const class RevFeature* F, RevRegFile* R, class RevMem* M, const class RevInst& Inst );

  friend std::ostream& operator<<( std::ostream& os, const RevRegFile& regFile );

  friend class RevCore;
  friend class Zaamo;
  friend class Zalrsc;
};  // class RevRegFile

}  // namespace SST::RevCPU

#endif
