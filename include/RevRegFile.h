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

/// Floating-Point Rounding Mode
enum class FRMode : uint32_t {
  None = 0xff,
  RNE  = 0,  // Round to Nearest, ties to Even
  RTZ  = 1,  // Round towards Zero
  RDN  = 2,  // Round Down (towards -Inf)
  RUP  = 3,  // Round Up (towards +Inf)
  RMM  = 4,  // Round to Nearest, ties to Max Magnitude
  DYN  = 7,  // In instruction's rm field, selects dynamic rounding mode; invalid in FCSR
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

// CSR Registers
enum class RevCSR : uint32_t {

  // Unprivileged and User-level CSRs
  fflags         = 0x001,
  frm            = 0x002,
  fcsr           = 0x003,
  cycle          = 0xc00,
  time           = 0xc01,
  instret        = 0xc02,
  hpmcounter3    = 0xc03,
  hpmcounter4    = 0xc04,
  hpmcounter5    = 0xc05,
  hpmcounter6    = 0xc06,
  hpmcounter7    = 0xc07,
  hpmcounter8    = 0xc08,
  hpmcounter9    = 0xc09,
  hpmcounter10   = 0xc0a,
  hpmcounter11   = 0xc0b,
  hpmcounter12   = 0xc0c,
  hpmcounter13   = 0xc0d,
  hpmcounter14   = 0xc0e,
  hpmcounter15   = 0xc0f,
  hpmcounter16   = 0xc10,
  hpmcounter17   = 0xc11,
  hpmcounter18   = 0xc12,
  hpmcounter19   = 0xc13,
  hpmcounter20   = 0xc14,
  hpmcounter21   = 0xc15,
  hpmcounter22   = 0xc16,
  hpmcounter23   = 0xc17,
  hpmcounter24   = 0xc18,
  hpmcounter25   = 0xc19,
  hpmcounter26   = 0xc1a,
  hpmcounter27   = 0xc1b,
  hpmcounter28   = 0xc1c,
  hpmcounter29   = 0xc1d,
  hpmcounter30   = 0xc1e,
  hpmcounter31   = 0xc1f,
  cycleh         = 0xc80,
  timeh          = 0xc81,
  instreth       = 0xc82,
  hpmcounter3h   = 0xc83,
  hpmcounter4h   = 0xc84,
  hpmcounter5h   = 0xc85,
  hpmcounter6h   = 0xc86,
  hpmcounter7h   = 0xc87,
  hpmcounter8h   = 0xc88,
  hpmcounter9h   = 0xc89,
  hpmcounter10h  = 0xc8a,
  hpmcounter11h  = 0xc8b,
  hpmcounter12h  = 0xc8c,
  hpmcounter13h  = 0xc8d,
  hpmcounter14h  = 0xc8e,
  hpmcounter15h  = 0xc8f,
  hpmcounter16h  = 0xc90,
  hpmcounter17h  = 0xc91,
  hpmcounter18h  = 0xc92,
  hpmcounter19h  = 0xc93,
  hpmcounter20h  = 0xc94,
  hpmcounter21h  = 0xc95,
  hpmcounter22h  = 0xc96,
  hpmcounter23h  = 0xc97,
  hpmcounter24h  = 0xc98,
  hpmcounter25h  = 0xc99,
  hpmcounter26h  = 0xc9a,
  hpmcounter27h  = 0xc9b,
  hpmcounter28h  = 0xc9c,
  hpmcounter29h  = 0xc9d,
  hpmcounter30h  = 0xc9e,
  hpmcounter31h  = 0xc9f,

  // Supervisor-Level CSRs
  sstatus        = 0x100,
  sie            = 0x104,
  stvec          = 0x105,
  scounteren     = 0x106,
  senvcfg        = 0x10a,
  scountinhibit  = 0x120,
  sscratch       = 0x140,
  sepc           = 0x141,
  scause         = 0x142,
  stval          = 0x143,
  sip            = 0x144,
  scountovf      = 0xda0,
  satp           = 0x180,
  scontext       = 0x5a8,
  sstateen0      = 0x10c,
  sstateen1      = 0x10d,
  sstateen2      = 0x10e,
  sstateen3      = 0x10f,

  // Hypervisor and VS CSRs
  hstatus        = 0x600,
  hedeleg        = 0x602,
  hideleg        = 0x603,
  hie            = 0x604,
  hcounteren     = 0x606,
  hgeie          = 0x607,
  hedelegh       = 0x612,
  htval          = 0x643,
  hip            = 0x644,
  hvip           = 0x645,
  htinst         = 0x64a,
  hgeip          = 0xe12,
  henvcfg        = 0x60a,
  henvcfgh       = 0x61a,
  hgatp          = 0x680,
  hcontext       = 0x6a8,
  htimedelta     = 0x605,
  htimedeltah    = 0x615,
  hstateen0      = 0x60c,
  hstateen1      = 0x60d,
  hstateen2      = 0x60e,
  hstateen3      = 0x60f,
  hstateen0h     = 0x61c,
  hstateen1h     = 0x61d,
  hstateen2h     = 0x61e,
  hstateen3h     = 0x61f,
  vsstatus       = 0x200,
  vsie           = 0x204,
  vstvec         = 0x205,
  vsscratch      = 0x240,
  vsepc          = 0x241,
  vscause        = 0x242,
  vstval         = 0x243,
  vsip           = 0x244,
  vsatp          = 0x280,

  // Machine-Level CSRs
  mvendorid      = 0xf11,
  marchid        = 0xf12,
  mimpid         = 0xf13,
  mhartid        = 0xf14,
  mconfigptr     = 0xf15,
  mstatus        = 0x300,
  misa           = 0x301,
  medeleg        = 0x302,
  mideleg        = 0x303,
  mie            = 0x304,
  mtvec          = 0x305,
  mcounteren     = 0x306,
  mstatush       = 0x310,
  medelegh       = 0x312,
  mscratch       = 0x340,
  mepc           = 0x341,
  mcause         = 0x342,
  mtval          = 0x343,
  mip            = 0x344,
  mtinst         = 0x34a,
  mtval2         = 0x34b,
  menvcfg        = 0x30a,
  menvcfgh       = 0x31a,
  mseccfg        = 0x747,
  mseccfgh       = 0x757,
  pmpcfg0        = 0x3a0,
  pmpcfg1        = 0x3a1,
  pmpcfg2        = 0x3a2,
  pmpcfg3        = 0x3a3,
  pmpcfg4        = 0x3a4,
  pmpcfg5        = 0x3a5,
  pmpcfg6        = 0x3a6,
  pmpcfg7        = 0x3a7,
  pmpcfg8        = 0x3a8,
  pmpcfg9        = 0x3a9,
  pmpcfg10       = 0x3aa,
  pmpcfg11       = 0x3ab,
  pmpcfg12       = 0x3ac,
  pmpcfg13       = 0x3ad,
  pmpcfg14       = 0x3ae,
  pmpcfg15       = 0x3af,
  pmpaddr0       = 0x3b0,
  pmpaddr1       = 0x3b1,
  pmpaddr2       = 0x3b2,
  pmpaddr3       = 0x3b3,
  pmpaddr4       = 0x3b4,
  pmpaddr5       = 0x3b5,
  pmpaddr6       = 0x3b6,
  pmpaddr7       = 0x3b7,
  pmpaddr8       = 0x3b8,
  pmpaddr9       = 0x3b9,
  pmpaddr10      = 0x3ba,
  pmpaddr11      = 0x3bb,
  pmpaddr12      = 0x3bc,
  pmpaddr13      = 0x3bd,
  pmpaddr14      = 0x3be,
  pmpaddr15      = 0x3bf,
  pmpaddr16      = 0x3c0,
  pmpaddr17      = 0x3c1,
  pmpaddr18      = 0x3c2,
  pmpaddr19      = 0x3c3,
  pmpaddr20      = 0x3c4,
  pmpaddr21      = 0x3c5,
  pmpaddr22      = 0x3c6,
  pmpaddr23      = 0x3c7,
  pmpaddr24      = 0x3c8,
  pmpaddr25      = 0x3c9,
  pmpaddr26      = 0x3ca,
  pmpaddr27      = 0x3cb,
  pmpaddr28      = 0x3cc,
  pmpaddr29      = 0x3cd,
  pmpaddr30      = 0x3ce,
  pmpaddr31      = 0x3cf,
  pmpaddr32      = 0x3d0,
  pmpaddr33      = 0x3d1,
  pmpaddr34      = 0x3d2,
  pmpaddr35      = 0x3d3,
  pmpaddr36      = 0x3d4,
  pmpaddr37      = 0x3d5,
  pmpaddr38      = 0x3d6,
  pmpaddr39      = 0x3d7,
  pmpaddr40      = 0x3d8,
  pmpaddr41      = 0x3d9,
  pmpaddr42      = 0x3da,
  pmpaddr43      = 0x3db,
  pmpaddr44      = 0x3dc,
  pmpaddr45      = 0x3dd,
  pmpaddr46      = 0x3de,
  pmpaddr47      = 0x3df,
  pmpaddr48      = 0x3e0,
  pmpaddr49      = 0x3e1,
  pmpaddr50      = 0x3e2,
  pmpaddr51      = 0x3e3,
  pmpaddr52      = 0x3e4,
  pmpaddr53      = 0x3e5,
  pmpaddr54      = 0x3e6,
  pmpaddr55      = 0x3e7,
  pmpaddr56      = 0x3e8,
  pmpaddr57      = 0x3e9,
  pmpaddr58      = 0x3ea,
  pmpaddr59      = 0x3eb,
  pmpaddr60      = 0x3ec,
  pmpaddr61      = 0x3ed,
  pmpaddr62      = 0x3ee,
  pmpaddr63      = 0x3ef,
  mstateen0      = 0x30c,
  mstateen1      = 0x30d,
  mstateen2      = 0x30e,
  mstateen3      = 0x30f,
  mstateen0h     = 0x31c,
  mstateen1h     = 0x31d,
  mstateen2h     = 0x31e,
  mstateen3h     = 0x31f,
  mnscratch      = 0x740,
  mnepc          = 0x741,
  mncause        = 0x742,
  mnstatus       = 0x744,
  mcycle         = 0xb00,
  minstret       = 0xb02,
  mhpmcounter3   = 0xb03,
  mhpmcounter4   = 0xb04,
  mhpmcounter5   = 0xb05,
  mhpmcounter6   = 0xb06,
  mhpmcounter7   = 0xb07,
  mhpmcounter8   = 0xb08,
  mhpmcounter9   = 0xb09,
  mhpmcounter10  = 0xb0a,
  mhpmcounter11  = 0xb0b,
  mhpmcounter12  = 0xb0c,
  mhpmcounter13  = 0xb0d,
  mhpmcounter14  = 0xb0e,
  mhpmcounter15  = 0xb0f,
  mhpmcounter16  = 0xb10,
  mhpmcounter17  = 0xb11,
  mhpmcounter18  = 0xb12,
  mhpmcounter19  = 0xb13,
  mhpmcounter20  = 0xb14,
  mhpmcounter21  = 0xb15,
  mhpmcounter22  = 0xb16,
  mhpmcounter23  = 0xb17,
  mhpmcounter24  = 0xb18,
  mhpmcounter25  = 0xb19,
  mhpmcounter26  = 0xb1a,
  mhpmcounter27  = 0xb1b,
  mhpmcounter28  = 0xb1c,
  mhpmcounter29  = 0xb1d,
  mhpmcounter30  = 0xb1e,
  mhpmcounter31  = 0xb1f,
  mcycleh        = 0xb80,
  minstreth      = 0xb82,
  mhpmcounter3h  = 0xb83,
  mhpmcounter4h  = 0xb84,
  mhpmcounter5h  = 0xb85,
  mhpmcounter6h  = 0xb86,
  mhpmcounter7h  = 0xb87,
  mhpmcounter8h  = 0xb88,
  mhpmcounter9h  = 0xb89,
  mhpmcounter10h = 0xb8a,
  mhpmcounter11h = 0xb8b,
  mhpmcounter12h = 0xb8c,
  mhpmcounter13h = 0xb8d,
  mhpmcounter14h = 0xb8e,
  mhpmcounter15h = 0xb8f,
  mhpmcounter16h = 0xb90,
  mhpmcounter17h = 0xb91,
  mhpmcounter18h = 0xb92,
  mhpmcounter19h = 0xb93,
  mhpmcounter20h = 0xb94,
  mhpmcounter21h = 0xb95,
  mhpmcounter22h = 0xb96,
  mhpmcounter23h = 0xb97,
  mhpmcounter24h = 0xb98,
  mhpmcounter25h = 0xb99,
  mhpmcounter26h = 0xb9a,
  mhpmcounter27h = 0xb9b,
  mhpmcounter28h = 0xb9c,
  mhpmcounter29h = 0xb9d,
  mhpmcounter30h = 0xb9e,
  mhpmcounter31h = 0xb9f,
  mcountinhibit  = 0x320,
  mhpmevent3     = 0x323,
  mhpmevent4     = 0x324,
  mhpmevent5     = 0x325,
  mhpmevent6     = 0x326,
  mhpmevent7     = 0x327,
  mhpmevent8     = 0x328,
  mhpmevent9     = 0x329,
  mhpmevent10    = 0x32a,
  mhpmevent11    = 0x32b,
  mhpmevent12    = 0x32c,
  mhpmevent13    = 0x32d,
  mhpmevent14    = 0x32e,
  mhpmevent15    = 0x32f,
  mhpmevent16    = 0x330,
  mhpmevent17    = 0x331,
  mhpmevent18    = 0x332,
  mhpmevent19    = 0x333,
  mhpmevent20    = 0x334,
  mhpmevent21    = 0x335,
  mhpmevent22    = 0x336,
  mhpmevent23    = 0x337,
  mhpmevent24    = 0x338,
  mhpmevent25    = 0x339,
  mhpmevent26    = 0x33a,
  mhpmevent27    = 0x33b,
  mhpmevent28    = 0x33c,
  mhpmevent29    = 0x33d,
  mhpmevent30    = 0x33e,
  mhpmevent31    = 0x33f,
  mhpmevent3h    = 0x723,
  mhpmevent4h    = 0x724,
  mhpmevent5h    = 0x725,
  mhpmevent6h    = 0x726,
  mhpmevent7h    = 0x727,
  mhpmevent8h    = 0x728,
  mhpmevent9h    = 0x729,
  mhpmevent10h   = 0x72a,
  mhpmevent11h   = 0x72b,
  mhpmevent12h   = 0x72c,
  mhpmevent13h   = 0x72d,
  mhpmevent14h   = 0x72e,
  mhpmevent15h   = 0x72f,
  mhpmevent16h   = 0x730,
  mhpmevent17h   = 0x731,
  mhpmevent18h   = 0x732,
  mhpmevent19h   = 0x733,
  mhpmevent20h   = 0x734,
  mhpmevent21h   = 0x735,
  mhpmevent22h   = 0x736,
  mhpmevent23h   = 0x737,
  mhpmevent24h   = 0x738,
  mhpmevent25h   = 0x739,
  mhpmevent26h   = 0x73a,
  mhpmevent27h   = 0x73b,
  mhpmevent28h   = 0x73c,
  mhpmevent29h   = 0x73d,
  mhpmevent30h   = 0x73e,
  mhpmevent31h   = 0x73f,
  tselect        = 0x7a0,
  tdata1         = 0x7a1,
  tdata2         = 0x7a2,
  tdata3         = 0x7a3,
  mcontext       = 0x7a8,
  dcsr           = 0x7b0,
  dpc            = 0x7b1,
  dscratch0      = 0x7b2,
  dscratch1      = 0x7b3,
};

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

class RevRegFile {
public:
  RevCore* const Core;    ///< RevRegFile: Owning core of this register file's hart
  const bool     IsRV64;  ///< RevRegFile: Cached copy of Features->IsRV64()
  const bool     HasD;    ///< RevRegFile: Cached copy of Features->HasD()

private:
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
  explicit RevRegFile( T* core ) : Core( core ), IsRV64( core->GetRevFeature()->IsRV64() ), HasD( core->GetRevFeature()->HasD() ) {}

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
    if( IsRV64 ) {
      RV64_SEPC = RV64_PC;
    } else {
      RV32_SEPC = RV32_PC;
    }
  }

  ///Set the value for extra information about exception
  /// (ECALL doesn't use it and sets it to 0)
  template<typename T>
  void SetSTVAL( T val ) {
    if( IsRV64 ) {
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
    if( IsRV64 ) {
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
    if( IsRV64 ) {
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
    if( IsRV64 ) {
      return RV64_PC;
    } else {
      return RV32_PC;
    }
  }

  /// SetPC: Set the Program Counter to a specific value
  template<typename T>
  void SetPC( T val ) {
    if( IsRV64 ) {
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
    if( IsRV64 ) {
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
  T GetCSR( uint16_t csr ) const {
    // clang-format off
    switch( RevCSR{csr} ) {
    // We store fcsr separately from the global CSR
      case RevCSR::fflags: return static_cast<uint32_t>( fcsr ) >> 0 & 0b00011111u;
      case RevCSR::frm: return static_cast<uint32_t>( fcsr ) >> 5 & 0b00000111u;
      case RevCSR::fcsr: return static_cast<uint32_t>( fcsr ) >> 0 & 0b11111111u;

      // Performance Counters
      case RevCSR::cycle: return GetPerfCounter<T, Half::Lo, &RevRegFile::rdcycle>();
      case RevCSR::cycleh: return GetPerfCounter<T, Half::Hi, &RevRegFile::rdcycle>();
      case RevCSR::time: return GetPerfCounter<T, Half::Lo, &RevRegFile::rdtime>();
      case RevCSR::timeh: return GetPerfCounter<T, Half::Hi, &RevRegFile::rdtime>();
      case RevCSR::instret: return GetPerfCounter<T, Half::Lo, &RevRegFile::rdinstret>();
      case RevCSR::instreth: return GetPerfCounter<T, Half::Hi, &RevRegFile::rdinstret>();

      default: return static_cast<T>( CSR[csr] );
    }
    // clang-format on
  }

  /// Set a CSR register
  template<typename T>
  void SetCSR( uint16_t csr, T val ) {
    // We store fcsr separately from the global CSR
    switch( RevCSR{ csr } ) {
    case RevCSR::fflags:
      fcsr = FCSR{ ( static_cast<uint32_t>( fcsr ) & ~uint32_t{ 0b00011111u } ) | static_cast<uint32_t>( val & 0b00011111u ) };
      break;
    case RevCSR::frm:
      fcsr = FCSR{ ( static_cast<uint32_t>( fcsr ) & ~uint32_t{ 0b11100000u } ) | static_cast<uint32_t>( val & 0b00000111u ) << 5 };
      break;
    case RevCSR::fcsr:
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
  friend class Zaamo;
  friend class Zalrsc;
};  // class RevRegFile

}  // namespace SST::RevCPU

#endif
