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
  static bool ModCSRImpl( RevRegFile* R, const RevInst& Inst ) {
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
        goto end;  // If CSRRS/CSRRC rs1 == 0, do not modify CSR
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

  end:
    // Advance PC
    R->AdvancePC( Inst );

    return true;
  }

  /// Modify a CSR Register according to CSRRW, CSRRS, or CSRRC
  // This calls the 32/64-bit ModCSR depending on the current XLEN
  template<OpKind OPKIND, CSROp OP>
  static bool ModCSR( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
    return R->IsRV32 ? ModCSRImpl<uint32_t, OPKIND, OP>( R, Inst ) : ModCSRImpl<uint64_t, OPKIND, OP>( R, Inst );
  }

  static constexpr auto& csrrw  = ModCSR<OpKind::Reg, CSROp::Write>;
  static constexpr auto& csrrs  = ModCSR<OpKind::Reg, CSROp::Set>;
  static constexpr auto& csrrc  = ModCSR<OpKind::Reg, CSROp::Clear>;
  static constexpr auto& csrrwi = ModCSR<OpKind::Imm, CSROp::Write>;
  static constexpr auto& csrrsi = ModCSR<OpKind::Imm, CSROp::Set>;
  static constexpr auto& csrrci = ModCSR<OpKind::Imm, CSROp::Clear>;

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
    RevZicsrInstDefaults().SetMnemonic( "csrw %csr, %rs1"        ).SetFunct3( 0b001 ).SetImplFunc( csrrw      ).SetPredicate( []( uint32_t Inst ){ return DECODE_RD( Inst ) == 0; } ),
    RevZicsrInstDefaults().SetMnemonic( "csrrw %csr, %rd, %rs1"  ).SetFunct3( 0b001 ).SetImplFunc( csrrw      ).SetPredicate( []( uint32_t Inst ){ return DECODE_RD( Inst ) != 0 && DECODE_IMM12( Inst ) != 0x1 && DECODE_IMM12( Inst ) != 0x2; } ),
    RevZicsrInstDefaults().SetMnemonic( "fsflags %rs"            ).SetFunct3( 0b001 ).SetImplFunc( csrrw      ).SetPredicate( []( uint32_t Inst ){ return DECODE_RD( Inst ) != 0 && DECODE_IMM12( Inst ) == 0x1; } ),
    RevZicsrInstDefaults().SetMnemonic( "fsrm %rs"               ).SetFunct3( 0b001 ).SetImplFunc( csrrw      ).SetPredicate( []( uint32_t Inst ){ return DECODE_RD( Inst ) != 0 && DECODE_IMM12( Inst ) == 0x2; } ),

    RevZicsrInstDefaults().SetMnemonic( "csrrs %rd, %csr, %rs1"  ).SetFunct3( 0b010 ).SetImplFunc( csrrs      ).SetPredicate( []( uint32_t Inst ){ return DECODE_RS1( Inst ) != 0 && DECODE_RD( Inst ) != 0; } ),
    RevZicsrInstDefaults().SetMnemonic( "csrs %csr, %rs1"        ).SetFunct3( 0b010 ).SetImplFunc( csrrs      ).SetPredicate( []( uint32_t Inst ){ return DECODE_RS1( Inst ) != 0 && DECODE_RD( Inst ) == 0; } ),
    RevZicsrInstDefaults().SetMnemonic( "csrr %rd, %csr"         ).SetFunct3( 0b010 ).SetImplFunc( csrrs      ).SetPredicate( []( uint32_t Inst ){ return DECODE_RS1( Inst ) == 0 && DECODE_IMM12( Inst ) != 0x1 && DECODE_IMM12( Inst ) != 0x2 && [](auto imm){ return imm < 0xc80 || imm > 0xc82; }( DECODE_IMM12( Inst ) | 0x80 ); } ),
    RevZicsrInstDefaults().SetMnemonic( "frflags %rd"            ).SetFunct3( 0b010 ).SetImplFunc( csrrs      ).SetPredicate( []( uint32_t Inst ){ return DECODE_RS1( Inst ) == 0 && DECODE_IMM12( Inst ) == 0x1; } ),
    RevZicsrInstDefaults().SetMnemonic( "frrm %rd"               ).SetFunct3( 0b010 ).SetImplFunc( csrrs      ).SetPredicate( []( uint32_t Inst ){ return DECODE_RS1( Inst ) == 0 && DECODE_IMM12( Inst ) == 0x2; } ),

    RevZicsrInstDefaults().SetMnemonic( "csrrc %rd, %csr, %rs1"  ).SetFunct3( 0b011 ).SetImplFunc( csrrc      ).SetPredicate( []( uint32_t Inst ){ return DECODE_RD( Inst ) != 0; } ),
    RevZicsrInstDefaults().SetMnemonic( "csrc %csr,%rs1"         ).SetFunct3( 0b011 ).SetImplFunc( csrrc      ).SetPredicate( []( uint32_t Inst ){ return DECODE_RD( Inst ) == 0; } ),

    RevZicsrInstDefaults().SetMnemonic( "csrrwi %rd, %csr, $imm" ).SetFunct3( 0b101 ).SetImplFunc( csrrwi     ).SetPredicate( []( uint32_t Inst ){ return DECODE_RD( Inst ) != 0; } ),
    RevZicsrInstDefaults().SetMnemonic( "csrwi %csr, $imm"       ).SetFunct3( 0b101 ).SetImplFunc( csrrwi     ).SetPredicate( []( uint32_t Inst ){ return DECODE_RD( Inst ) == 0; } ),

    RevZicsrInstDefaults().SetMnemonic( "csrrsi %rd, %csr, $imm" ).SetFunct3( 0b110 ).SetImplFunc( csrrsi     ).SetPredicate( []( uint32_t Inst ){ return DECODE_RD( Inst ) != 0; } ),
    RevZicsrInstDefaults().SetMnemonic( "csrsi %csr, $imm"       ).SetFunct3( 0b110 ).SetImplFunc( csrrsi     ).SetPredicate( []( uint32_t Inst ){ return DECODE_RD( Inst ) == 0; } ),

    RevZicsrInstDefaults().SetMnemonic( "csrrci %rd, %csr, $imm" ).SetFunct3( 0b111 ).SetImplFunc( csrrci     ).SetPredicate( []( uint32_t Inst ){ return DECODE_RD( Inst ) != 0; } ),
    RevZicsrInstDefaults().SetMnemonic( "csrci %csr, $imm"       ).SetFunct3( 0b111 ).SetImplFunc( csrrci     ).SetPredicate( []( uint32_t Inst ){ return DECODE_RD( Inst ) == 0; } ),

    RevZicsrInstDefaults().SetMnemonic( "rdcycle %rd"            ).SetFunct3( 0b010 ).SetImplFunc( csrrs      ).SetPredicate( []( uint32_t Inst ){ return DECODE_RS1( Inst ) == 0 && DECODE_IMM12( Inst ) == 0xc00; } ),
    RevZicsrInstDefaults().SetMnemonic( "rdcycleh %rd"           ).SetFunct3( 0b010 ).SetImplFunc( csrrs      ).SetPredicate( []( uint32_t Inst ){ return DECODE_RS1( Inst ) == 0 && DECODE_IMM12( Inst ) == 0xc80; } ),
    RevZicsrInstDefaults().SetMnemonic( "rdtime %rd"             ).SetFunct3( 0b010 ).SetImplFunc( csrrs      ).SetPredicate( []( uint32_t Inst ){ return DECODE_RS1( Inst ) == 0 && DECODE_IMM12( Inst ) == 0xc01; } ),
    RevZicsrInstDefaults().SetMnemonic( "rdtimeh %rd"            ).SetFunct3( 0b010 ).SetImplFunc( csrrs      ).SetPredicate( []( uint32_t Inst ){ return DECODE_RS1( Inst ) == 0 && DECODE_IMM12( Inst ) == 0xc81; } ),
    RevZicsrInstDefaults().SetMnemonic( "rdinstret %rd"          ).SetFunct3( 0b010 ).SetImplFunc( csrrs      ).SetPredicate( []( uint32_t Inst ){ return DECODE_RS1( Inst ) == 0 && DECODE_IMM12( Inst ) == 0xc02; } ),
    RevZicsrInstDefaults().SetMnemonic( "rdinstreth %rd"         ).SetFunct3( 0b010 ).SetImplFunc( csrrs      ).SetPredicate( []( uint32_t Inst ){ return DECODE_RS1( Inst ) == 0 && DECODE_IMM12( Inst ) == 0xc82; } ),
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
