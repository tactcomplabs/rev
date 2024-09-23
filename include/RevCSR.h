//
// _RevCSR_h_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#ifndef _SST_REVCSR_H_
#define _SST_REVCSR_H_

#include "RevFCSR.h"
#include "RevFeature.h"
#include "RevZicntr.h"
#include "SST.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace SST::RevCPU {

////////////////////////////////////////////////////////////////////////////////
//
// A note about the separation of scopes in include/insns/Zicsr.h and
// include/RevCSR.h:
//
// RevCSR.h: GetCSR() and SetCSR() are used to get and set specific CSR
// registers in the register file, regardless of how we arrive here. If a
// particular CSR register is disabled because of CPU extensions present, or if
// a particular CSR register does not apply to it (such as RDTIMEH on RV64),
// then raise an invalid instruction or other exception here.
//
// Zicsr.h: Decode and execute one of only 6 CSR instructions (csrrw, csrrs,
// csrrc, csrrwi, csrrsi, csrrci). Do not enable or disable certain CSR
// registers, or implement the semantics of particular CSR registers here.
// All CSR instructions with a valid encoding are valid as far as Zicsr.h is
// concerned. The particular CSR register accessed in a CSR instruction is
// secondary to the scope of Zicsr.h. Certain pseudoinstructions like RDTIME or
// FRFLAGS are listed separately in Zicsr.h only for user-friendly disassembly,
// not for enabling, disabling or implementing them.
//
// To ease maintainability and prevent large code size, it is recommended that
// functions related to specific CSR registers be made base classes of RevCSR
// in separate header files (e.g. RevZicntr). RevCSR can then dispatch the
// GetCSR()/SetCSR() functions of these CSR registers to the base class.
//
// To access a RevCSR or RevRegFile member function in one of its base classes,
// it is recommended that the function be made a pure virtual function in the
// base class so that RevCSR or RevRegFile must override it, similar to how
// GetCore() is a pure virtual function in RevZicntr which RevRegFile
// overrides. RevZicntr needs GetCore(), but rather than have to store a
// pointer in RevZicntr's constructor, it is much simpler to simply declare
// GetCore() as a pure virtual function in RevZicntr which must be overriden,
// which makes RevZicntr and RevCSR abstract classes which cannot be
// instantiated except as a base class of RevRegFile, which defines GetCore().
//
////////////////////////////////////////////////////////////////////////////////

class RevCore;

class RevCSR : public RevZicntr {
  static constexpr size_t         CSR_LIMIT = 0x1000;
  std::array<uint64_t, CSR_LIMIT> CSR{};  ///< RegCSR: CSR registers

public:
  // CSR Registers
  enum : uint16_t {
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

  /// Get the Floating-Point Rounding Mode
  FRMode GetFRM() const { return static_cast<FRMode>( CSR[fcsr] >> 5 & 0b111 ); }

  /// Set Floating-Point flags
  void SetFFlags( FCSR flags ) { CSR[fcsr] |= static_cast<uint32_t>( flags ) & 0b11111; }

  /// Get a CSR register
  template<typename XLEN>
  XLEN GetCSR( uint16_t csr ) const {
    // clang-format off
    switch( csr ) {
      case fflags:   return static_cast<XLEN>( CSR[fcsr] >> 0 & 0b00011111 );
      case frm:      return static_cast<XLEN>( CSR[fcsr] >> 5 & 0b00000111 );
      case fcsr:     return static_cast<XLEN>( CSR[fcsr] >> 0 & 0b11111111 );

      // Performance Counters
      case cycle:    return GetPerfCounter<XLEN, Half::Lo, rdcycle  >();
      case cycleh:   return GetPerfCounter<XLEN, Half::Hi, rdcycle  >();
      case time:     return GetPerfCounter<XLEN, Half::Lo, rdtime   >();
      case timeh:    return GetPerfCounter<XLEN, Half::Hi, rdtime   >();
      case instret:  return GetPerfCounter<XLEN, Half::Lo, rdinstret>();
      case instreth: return GetPerfCounter<XLEN, Half::Hi, rdinstret>();

      default:       return static_cast<XLEN>( CSR.at( csr ) );
    }
    // clang-format on
  }

  /// Set a CSR register
  template<typename XLEN>
  bool SetCSR( uint16_t csr, XLEN val ) {
    // Read-only CSRs cannot be written to
    if( csr >= 0xc00 && csr < 0xe00 )
      return false;

    switch( csr ) {
    case fflags: CSR[fcsr] = ( CSR[fcsr] & ~uint64_t{ 0b00011111 } ) | ( val & 0b00011111 ); break;
    case frm: CSR[fcsr] = ( CSR[fcsr] & ~uint64_t{ 0b11100000 } ) | ( val & 0b00000111 ) << 5; break;
    case fcsr: CSR[fcsr] = ( CSR[fcsr] & ~uint64_t{ 0b11111111 } ) | ( val & 0b11111111 ); break;
    default: CSR.at( csr ) = val;
    }
    return true;
  }
};  // class RevCSR

}  // namespace SST::RevCPU

#endif
