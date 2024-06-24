//
// _Zicsr_h_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_ZICSR_H_
#define _SST_REVCPU_ZICSR_H_

#include "../RevExt.h"
#include "../RevInstHelpers.h"

namespace SST::RevCPU {

class Zicsr : public RevExt {

  enum class CSROp { Write, Set, Clear };

  /// Modify a CSR Register according to CSRRW, CSRRS, or CSRRC
  // Because CSR has a 32/64-bit width, this function is templatized
  template<typename XLEN, OpKind OPKIND, CSROp OP>
  static bool ModCSRImpl( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {

    // Alternative forms of rdcycle[h], rdtime[h], rdinstret[h] which use an immediate 0 or csrrc
    // Canonical forms of rdcycle[h], rdtime[h], rdinstret[h] use cssrs with register x0
    if( Inst.rs1 == 0 && OP != CSROp::Write ) {
      // clang-format off
      switch( Inst.imm ) {
        case 0xc00: return rdcycle   ( F, R, M, Inst );
        case 0xc80: return rdcycleh  ( F, R, M, Inst );
        case 0xc01: return rdtime    ( F, R, M, Inst );
        case 0xc81: return rdtimeh   ( F, R, M, Inst );
        case 0xc02: return rdinstret ( F, R, M, Inst );
        case 0xc82: return rdinstreth( F, R, M, Inst );
      }
      // clang-format on
    }

    XLEN old = 0;

    // CSRRW with rd == zero does not read CSR
    if( OP != CSROp::Write || Inst.rd != 0 ) {
      old = R->GetCSR<XLEN>( Inst.imm );
    }

    // Operand is an zero-extended 5-bit immediate or register
    XLEN val = OPKIND == OpKind::Imm ? ZeroExt( Inst.rs1, 5 ) : R->GetX<XLEN>( Inst.rs1 );

    // Store the old CSR value in rd
    if( Inst.rd != 0 )
      R->SetX( Inst.rd, old );

    // Handle CSRRS/CSRRC
    if( OP != CSROp::Write ) {
      if( Inst.rs1 == 0 ) {
        return true;  // If CSRRS/CSRRC rs1 == 0, do not modify CSR
      } else if( OP == CSROp::Set ) {
        val = old | val;
      } else {
        val = old & ~val;
      }
    }

    // Read-only CSRs cannot be written to
    if( Inst.imm >= 0xc00 && Inst.imm < 0xe00 )
      return false;

    // Write the new CSR value
    R->SetCSR( Inst.imm, val );
    return true;
  }

  /// Modify a CSR Register according to CSRRW, CSRRS, or CSRRC
  // This calls the 32/64-bit ModCSR depending on the current XLEN
  template<OpKind OPKIND, CSROp OP>
  static bool ModCSR( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
    bool ret;
    if( R->IsRV32 ) {
      ret = ModCSRImpl<uint32_t, OPKIND, OP>( F, R, M, Inst );
    } else {
      ret = ModCSRImpl<uint64_t, OPKIND, OP>( F, R, M, Inst );
    }
    R->AdvancePC( Inst );
    return ret;
  }

  static constexpr auto& csrrw  = ModCSR<OpKind::Reg, CSROp::Write>;
  static constexpr auto& csrrs  = ModCSR<OpKind::Reg, CSROp::Set>;
  static constexpr auto& csrrc  = ModCSR<OpKind::Reg, CSROp::Clear>;
  static constexpr auto& csrrwi = ModCSR<OpKind::Imm, CSROp::Write>;
  static constexpr auto& csrrsi = ModCSR<OpKind::Imm, CSROp::Set>;
  static constexpr auto& csrrci = ModCSR<OpKind::Imm, CSROp::Clear>;

  // Performance counters
  // TODO: These should be moved to a separate Zicntr extension, but right now the
  // spec is unclear on what order Zicntr should appear in an architecture string

  // template is used to break circular dependencies and allow for an incomplete RevCore type now
  template<typename T, typename = std::enable_if_t<std::is_same_v<T, RevRegFile>>>
  static uint64_t rdcycleImpl( RevFeature* F, T* R, RevMem* M, const RevInst& Inst ) {
    return R->Core->GetCycles();
  }

  template<typename T, typename = std::enable_if_t<std::is_same_v<T, RevRegFile>>>
  static uint64_t rdtimeImpl( RevFeature* F, T* R, RevMem* M, const RevInst& Inst ) {
    return R->Core->GetCurrentSimCycle();
  }

  static uint64_t rdinstretImpl( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) { return R->GetInstRet(); }

  enum Half { Lo, Hi };

  /// Performance Counter template
  // Passed a function which gets the 64-bit value of a performance counter
  template<Half HALF, uint64_t COUNTER( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst )>
  static bool perfCounter( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
    if( R->IsRV32 ) {
      if constexpr( HALF == Lo ) {
        R->SetX( Inst.rd, static_cast<uint32_t>( COUNTER( F, R, M, Inst ) & 0xffffffff ) );
      } else {
        R->SetX( Inst.rd, static_cast<uint32_t>( COUNTER( F, R, M, Inst ) >> 32 ) );
      }
    } else {
      if constexpr( HALF == Lo ) {
        R->SetX( Inst.rd, COUNTER( F, R, M, Inst ) );
      } else {
        return false;  // Hi half is not available on RV64
      }
    }
    R->AdvancePC( Inst );
    return true;
  }

  static constexpr auto& rdcycle    = perfCounter<Lo, rdcycleImpl>;
  static constexpr auto& rdcycleh   = perfCounter<Hi, rdcycleImpl>;
  static constexpr auto& rdtime     = perfCounter<Lo, rdtimeImpl>;
  static constexpr auto& rdtimeh    = perfCounter<Hi, rdtimeImpl>;
  static constexpr auto& rdinstret  = perfCounter<Lo, rdinstretImpl>;
  static constexpr auto& rdinstreth = perfCounter<Hi, rdinstretImpl>;

  // ----------------------------------------------------------------------
  //
  // RISC-V CSR Instructions
  //
  // ----------------------------------------------------------------------
  struct RevZicsrInstDefaults : RevInstDefaults {
    RevZicsrInstDefaults() {
      SetOpcode( 0b1110011 );
      SetRaiseFPE( true );
      SetrdClass( RevRegClass::RegGPR );
      Setrs2Class( RevRegClass::RegUNKNOWN );
      Setimm( FImm );
      SetFormat( RVTypeI );
    }
  };

  // clang-format off
  std::vector<RevInstEntry> ZicsrTable = {
    { RevZicsrInstDefaults().SetMnemonic( "csrw %csr, %rs1"        ).SetFunct3( 0b001 ).SetImplFunc( csrrw      ).SetPredicate( []( uint32_t Inst ){ return DECODE_RD( Inst ) == 0; } ) },
    { RevZicsrInstDefaults().SetMnemonic( "csrrw %csr, %rd, %rs1"  ).SetFunct3( 0b001 ).SetImplFunc( csrrw      ).SetPredicate( []( uint32_t Inst ){ return DECODE_RD( Inst ) != 0 && DECODE_IMM12( Inst ) != 0x1 && DECODE_IMM12( Inst ) != 0x2; } ) },
    { RevZicsrInstDefaults().SetMnemonic( "fsflags %rs"            ).SetFunct3( 0b001 ).SetImplFunc( csrrw      ).SetPredicate( []( uint32_t Inst ){ return DECODE_RD( Inst ) != 0 && DECODE_IMM12( Inst ) == 0x1; } ) },
    { RevZicsrInstDefaults().SetMnemonic( "fsrm %rs"               ).SetFunct3( 0b001 ).SetImplFunc( csrrw      ).SetPredicate( []( uint32_t Inst ){ return DECODE_RD( Inst ) != 0 && DECODE_IMM12( Inst ) == 0x2; } ) },

    { RevZicsrInstDefaults().SetMnemonic( "csrrs %rd, %csr, %rs1"  ).SetFunct3( 0b010 ).SetImplFunc( csrrs      ).SetPredicate( []( uint32_t Inst ){ return DECODE_RS1( Inst ) != 0; } ) },
    { RevZicsrInstDefaults().SetMnemonic( "csrr %rd, %csr"         ).SetFunct3( 0b010 ).SetImplFunc( csrrs      ).SetPredicate( []( uint32_t Inst ){ return DECODE_RS1( Inst ) == 0 && DECODE_IMM12( Inst ) != 0x1 && DECODE_IMM12( Inst ) != 0x2 && [](auto imm){ return imm < 0xc80 || imm > 0xc82; }( DECODE_IMM12( Inst ) | 0x80 ); } ) },
    { RevZicsrInstDefaults().SetMnemonic( "frflags %rd"            ).SetFunct3( 0b010 ).SetImplFunc( csrrs      ).SetPredicate( []( uint32_t Inst ){ return DECODE_RS1( Inst ) == 0 && DECODE_IMM12( Inst ) == 0x1; } ) },
    { RevZicsrInstDefaults().SetMnemonic( "frrm %rd"               ).SetFunct3( 0b010 ).SetImplFunc( csrrs      ).SetPredicate( []( uint32_t Inst ){ return DECODE_RS1( Inst ) == 0 && DECODE_IMM12( Inst ) == 0x2; } ) },

    { RevZicsrInstDefaults().SetMnemonic( "csrrc %rd, %csr, %rs1"  ).SetFunct3( 0b011 ).SetImplFunc( csrrc      ).SetPredicate( []( uint32_t Inst ){ return DECODE_RD( Inst ) != 0; } ) },
    { RevZicsrInstDefaults().SetMnemonic( "csrc %csr,%rs1"         ).SetFunct3( 0b011 ).SetImplFunc( csrrc      ).SetPredicate( []( uint32_t Inst ){ return DECODE_RD( Inst ) == 0; } ) },

    { RevZicsrInstDefaults().SetMnemonic( "csrrwi %rd, %csr, $imm" ).SetFunct3( 0b101 ).SetImplFunc( csrrwi     ).SetPredicate( []( uint32_t Inst ){ return DECODE_RD( Inst ) != 0; } ) },
    { RevZicsrInstDefaults().SetMnemonic( "csrwi %csr, $imm"       ).SetFunct3( 0b101 ).SetImplFunc( csrrwi     ).SetPredicate( []( uint32_t Inst ){ return DECODE_RD( Inst ) == 0; } ) },

    { RevZicsrInstDefaults().SetMnemonic( "csrrsi %rd, %csr, $imm" ).SetFunct3( 0b110 ).SetImplFunc( csrrsi     ).SetPredicate( []( uint32_t Inst ){ return DECODE_RD( Inst ) != 0; } ) },
    { RevZicsrInstDefaults().SetMnemonic( "csrsi %csr, $imm"       ).SetFunct3( 0b110 ).SetImplFunc( csrrsi     ).SetPredicate( []( uint32_t Inst ){ return DECODE_RD( Inst ) == 0; } ) },

    { RevZicsrInstDefaults().SetMnemonic( "csrrci %rd, %csr, $imm" ).SetFunct3( 0b111 ).SetImplFunc( csrrci     ).SetPredicate( []( uint32_t Inst ){ return DECODE_RD( Inst ) != 0; } ) },
    { RevZicsrInstDefaults().SetMnemonic( "csrci %csr, $imm"       ).SetFunct3( 0b111 ).SetImplFunc( csrrci     ).SetPredicate( []( uint32_t Inst ){ return DECODE_RD( Inst ) == 0; } ) },

    { RevZicsrInstDefaults().SetMnemonic( "rdcycle %rd"            ).SetFunct3( 0b010 ).SetImplFunc( rdcycle    ).SetPredicate( []( uint32_t Inst ){ return DECODE_RS1( Inst ) == 0 && DECODE_IMM12( Inst ) == 0xc00; } ) },
    { RevZicsrInstDefaults().SetMnemonic( "rdcycleh %rd"           ).SetFunct3( 0b010 ).SetImplFunc( rdcycleh   ).SetPredicate( []( uint32_t Inst ){ return DECODE_RS1( Inst ) == 0 && DECODE_IMM12( Inst ) == 0xc80; } ) },
    { RevZicsrInstDefaults().SetMnemonic( "rdtime %rd"             ).SetFunct3( 0b010 ).SetImplFunc( rdtime     ).SetPredicate( []( uint32_t Inst ){ return DECODE_RS1( Inst ) == 0 && DECODE_IMM12( Inst ) == 0xc01; } ) },
    { RevZicsrInstDefaults().SetMnemonic( "rdtimeh %rd"            ).SetFunct3( 0b010 ).SetImplFunc( rdtimeh    ).SetPredicate( []( uint32_t Inst ){ return DECODE_RS1( Inst ) == 0 && DECODE_IMM12( Inst ) == 0xc81; } ) },
    { RevZicsrInstDefaults().SetMnemonic( "rdinstret %rd"          ).SetFunct3( 0b010 ).SetImplFunc( rdinstret  ).SetPredicate( []( uint32_t Inst ){ return DECODE_RS1( Inst ) == 0 && DECODE_IMM12( Inst ) == 0xc02; } ) },
    { RevZicsrInstDefaults().SetMnemonic( "rdinstreth %rd"         ).SetFunct3( 0b010 ).SetImplFunc( rdinstreth ).SetPredicate( []( uint32_t Inst ){ return DECODE_RS1( Inst ) == 0 && DECODE_IMM12( Inst ) == 0xc82; } ) },
  };
  // clang-format on

public:
  /// Zicsr: standard constructor
  Zicsr( RevFeature* Feature, RevMem* RevMem, SST::Output* Output ) : RevExt( "Zicsr", Feature, RevMem, Output ) {
    SetTable( std::move( ZicsrTable ) );
  }

};  // end class Zicsr

}  // namespace SST::RevCPU

#endif
