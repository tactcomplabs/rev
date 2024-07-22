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
#include "RevFeature.h"
#include "RevMem.h"

namespace SST::RevCPU {

struct RevInst;

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

template<>
struct uint_type<float16> {
  using type = uint16_t;
  static_assert( sizeof( type ) == sizeof( float16 ) );
};

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

/// Floating-Point Rounding Mode
enum class FRMode : uint32_t {
  None = 0xff,
  RNE = 0,   // Round to Nearest, ties to Even
  RTZ = 1,   // Round towards Zero
  RDN = 2,   // Round Down (towards -Inf)
  RUP = 3,   // Round Up (towards +Inf)
  RMM = 4,   // Round to Nearest, ties to Max Magnitude
  DYN = 7,   // In instruction's rm field, selects dynamic rounding mode; invalid in FCSR
};

/// Floating-point control register
enum class FCSR : uint32_t {
  NX = 1,
  UF = 2,
  OF = 4,
  DZ = 8,
  NV = 16,
};

#define CSR_LIMIT 0x1000

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

// clang-format on

class RevCore;

class RevRegFile {
public:
  RevCore* const Core;    ///< RevRegFile: Owning core of this register file's hart
  const bool     IsRV32;  ///< RevRegFile: Cached copy of Features->IsRV32()
  const bool     HasD;    ///< RevRegFile: Cached copy of Features->HasD()

private:
  bool       trigger{};         ///< RevRegFile: Has the instruction been triggered?
  unsigned   Entry{};           ///< RevRegFile: Instruction entry
  uint32_t   cost{};            ///< RevRegFile: Cost of the instruction
  RevTracer* Tracer = nullptr;  ///< RegRegFile: Tracer object

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

  // Supervisor Mode CSRs
  uint64_t CSR[CSR_LIMIT]{};

  // Floating-point CSR
  FCSR fcsr{};

  // Number of instructions retired
  uint64_t InstRet{};

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
  // Template is to prevent circular dependencies by not requiring RevCore to be a complete type now
  template<typename T, typename = std::enable_if_t<std::is_same_v<T, RevCore>>>
  explicit RevRegFile( T* core ) : Core( core ), IsRV32( core->GetRevFeature()->IsRV32() ), HasD( core->GetRevFeature()->HasD() ) {}

  /// RevRegFile: disallow copying and assignment
  RevRegFile( const RevRegFile& )            = delete;
  RevRegFile& operator=( const RevRegFile& ) = delete;

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
    if( IsRV32 ) {
      RV32_SEPC = RV32_PC;
    } else {
      RV64_SEPC = RV64_PC;
    }
  }

  ///Set the value for extra information about exception
  /// (ECALL doesn't use it and sets it to 0)
  template<typename T>
  void SetSTVAL( T val ) {
    if( IsRV32 ) {
      RV32_STVAL = val;
    } else {
      RV64_STVAL = val;
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
    if( IsRV32 ) {
      res = RevReg( rs ) != RevReg::zero ? T( RV32[size_t( rs )] ) : 0;
      TRACE_REG_READ( size_t( rs ), uint32_t( res ) );
    } else {
      res = RevReg( rs ) != RevReg::zero ? T( RV64[size_t( rs )] ) : 0;
      TRACE_REG_READ( size_t( rs ), uint64_t( res ) );
    }
    return res;
  }

  /// SetX: Set the specifed X register to a specific value
  template<typename T, typename U>
  void SetX( U rd, T val ) {
    T res;
    if( IsRV32 ) {
      res                = RevReg( rd ) != RevReg::zero ? uint32_t( val ) : 0;
      RV32[size_t( rd )] = res;
      TRACE_REG_WRITE( size_t( rd ), uint32_t( res ) );
    } else {
      res                = RevReg( rd ) != RevReg::zero ? uint64_t( val ) : 0;
      RV64[size_t( rd )] = res;
      TRACE_REG_WRITE( size_t( rd ), uint64_t( res ) );
    }
  }

  /// GetPC: Get the Program Counter
  uint64_t GetPC() const {
    if( IsRV32 ) {
      return RV32_PC;
    } else {
      return RV64_PC;
    }
  }

  /// SetPC: Set the Program Counter to a specific value
  template<typename T>
  void SetPC( T val ) {
    if( IsRV32 ) {
      RV32_PC = static_cast<uint32_t>( val );
      TRACE_PC_WRITE( RV32_PC );
    } else {
      RV64_PC = static_cast<uint64_t>( val );
      TRACE_PC_WRITE( RV64_PC );
    }
  }

  /// AdvancePC: Advance the program counter to the next instruction
  // Note: This does not create tracer events like SetPC() does
  template<typename T>  // Used to allow RevInst to be incomplete type right now
  void AdvancePC( const T& Inst ) {
    if( IsRV32 ) {
      RV32_PC += Inst.instSize;
    } else {
      RV64_PC += Inst.instSize;
    }
  }

  /// GetFP: Get the specified FP register cast to a specific FP type
  // The second argument indicates whether it is a FMV/FS move/store
  // instruction which just transfers bits and not care about NaN-Boxing.
  template<typename T, bool FMV_FS = false, typename U>
  T GetFP( U rs ) const {
    if constexpr( std::is_same_v<T, double> ) {
      return DPF[size_t( rs )];
    } else if( HasD ) {
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
    } else if( HasD ) {
      BoxNaN( &DPF[size_t( rd )], &value );
    } else {
      BoxNaN( &SPF[size_t( rd )], &value );
    }
  }

private:
  // Performance counters

  // Template is used to break circular dependencies between RevCore and RevRegFile
  template<typename CORE, typename = std::enable_if_t<std::is_same_v<CORE, RevCore>>>
  uint64_t rdcycle( CORE* core ) const {
    return core->GetCycles();
  }

  template<typename CORE, typename = std::enable_if_t<std::is_same_v<CORE, RevCore>>>
  uint64_t rdtime( CORE* core ) const {
    return core->GetCurrentSimCycle();
  }

  template<typename CORE, typename = std::enable_if_t<std::is_same_v<CORE, RevCore>>>
  uint64_t rdinstret( CORE* ) const {
    return InstRet;
  }

  enum class Half { Lo, Hi };

  /// Performance Counter template
  // Passed a function which gets the 64-bit value of a performance counter
  template<typename T, Half HALF, uint64_t ( RevRegFile::*COUNTER )( RevCore* ) const>
  T GetPerfCounter() const {
    if constexpr( sizeof( T ) == sizeof( uint32_t ) ) {
      // clang-format off
      if constexpr( HALF == Half::Lo ) {
        return static_cast<T>( ( this->*COUNTER )( Core ) & 0xffffffff );
      } else {
        return static_cast<T>( ( this->*COUNTER )( Core ) >> 32 );
      }
      // clang-format on
    } else {
      if constexpr( HALF == Half::Lo ) {
        return ( this->*COUNTER )( Core );
      } else {
        return 0;  // Hi half is not available on RV64
      }
    }
  }

public:
  /// Get a CSR register
  template<typename T>
  T GetCSR( size_t csr ) const {
    // clang-format off
    switch( csr ) {
    default: return static_cast<T>( CSR[csr] );

    // We store fcsr separately from the global CSR
    case 1: return static_cast<uint32_t>( fcsr ) >> 0 & 0b00011111u;
    case 2: return static_cast<uint32_t>( fcsr ) >> 5 & 0b00000111u;
    case 3: return static_cast<uint32_t>( fcsr ) >> 0 & 0b11111111u;

      // Performance Counters
    case 0xc00: return GetPerfCounter<T, Half::Lo, &RevRegFile::rdcycle>();
    case 0xc80: return GetPerfCounter<T, Half::Hi, &RevRegFile::rdcycle>();
    case 0xc01: return GetPerfCounter<T, Half::Lo, &RevRegFile::rdtime>();
    case 0xc81: return GetPerfCounter<T, Half::Hi, &RevRegFile::rdtime>();
    case 0xc02: return GetPerfCounter<T, Half::Lo, &RevRegFile::rdinstret>();
    case 0xc82: return GetPerfCounter<T, Half::Hi, &RevRegFile::rdinstret>();
    }
    // clang-format on
  }

  /// Set a CSR register
  template<typename T>
  void SetCSR( size_t csr, T val ) {
    // We store fcsr separately from the global CSR
    switch( csr ) {
    case 1:
      fcsr = FCSR{ ( static_cast<uint32_t>( fcsr ) & ~uint32_t{ 0b00011111u } ) | static_cast<uint32_t>( val & 0b00011111u ) };
      break;
    case 2:
      fcsr = FCSR{ ( static_cast<uint32_t>( fcsr ) & ~uint32_t{ 0b11100000u } ) | static_cast<uint32_t>( val & 0b00000111u ) << 5 };
      break;
    case 3:
      fcsr = FCSR{ ( static_cast<uint32_t>( fcsr ) & ~uint32_t{ 0b11111111u } ) | static_cast<uint32_t>( val & 0b11111111u ) };
      break;
    default: CSR[csr] = val;
    }
  }

  /// Get the Floating-Point Rounding Mode
  FRMode GetFRM() const { return FRMode{ ( static_cast<uint32_t>( fcsr ) >> 5 ) & 0x3u }; }

  /// Return the Floating-Point Status Register
  FCSR& GetFCSR() { return fcsr; }

  // Friend functions and classes to access internal register state
  template<typename INT, typename FP>
  friend bool fcvtif( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst );

  template<typename T>
  friend bool load( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst );

  template<typename T>
  friend bool store( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst );

  template<typename T>
  friend bool fload( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst );

  template<typename T>
  friend bool fstore( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst );

  template<typename T, template<class> class OP>
  friend bool foper( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst );

  template<typename T, template<class> class OP>
  friend bool fcondop( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst );

  friend std::ostream& operator<<( std::ostream& os, const RevRegFile& regFile );

  friend class RevCore;
  friend class RV32A;
  friend class RV64A;
};  // class RevRegFile

}  // namespace SST::RevCPU

#endif
