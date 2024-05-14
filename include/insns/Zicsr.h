//
// _ZiCSR_h_
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

class ZiCSR : public RevExt {

  enum CSROper { Write, Set, Clear };

  /// Modify a CSR Register according to CSRRW, CSRRS, or CSRRC
  // Because CSR has a 32/64-bit width, this function is templatized
  template<typename T, CSROper OPER, OpKind OPKIND>
  static bool ModCSRImpl( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
    T old = 0;

    // CSRRW with rd == zero does not read CSR
    if( OPER != Write || Inst.rd != 0 ) {
      old = R->GetCSR<T>( Inst.imm );
    }

    // Operand is an zero-extended 5-bit immediate or register
    T val = OPKIND == OpKind::Imm ? ZeroExt( Inst.rs1, 5 ) : R->GetX<T>( Inst.rs1 );

    // Store the old CSR value in rd
    if( Inst.rd != 0 )
      R->SetX( Inst.rd, old );

    // Handle CSRRS/CSRRC
    if( OPER != Write ) {
      if( Inst.rs1 == 0 ) {
        return true;  // If CSRRS/CSRRC rs1 == 0, do not modify CSR
      } else if( OPER == Set ) {
        val = old | val;
      } else {
        val = old & ~val;
      }
    }

    // Write the new CSR value
    R->SetCSR( Inst.imm, val );
    return true;
  }

  /// Modify a CSR Register according to CSRRW, CSRRS, or CSRRC
  // This calls the 32/64-bit ModCSR depending on the current XLEN
  template<OpKind OPKIND, CSROper OPER>
  static bool ModCSR( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
    bool ret;
    if( R->IsRV32 ) {
      ret = ModCSRImpl<uint32_t, OPER, OPKIND>( F, R, M, Inst );
    } else {
      ret = ModCSRImpl<uint64_t, OPER, OPKIND>( F, R, M, Inst );
    }
    R->AdvancePC( Inst );
    return ret;
  }

  static constexpr auto& csrrw  = ModCSR<OpKind::Reg, Write>;
  static constexpr auto& csrrs  = ModCSR<OpKind::Reg, Set>;
  static constexpr auto& csrrc  = ModCSR<OpKind::Reg, Clear>;
  static constexpr auto& csrrwi = ModCSR<OpKind::Imm, Write>;
  static constexpr auto& csrrsi = ModCSR<OpKind::Imm, Set>;
  static constexpr auto& csrrci = ModCSR<OpKind::Imm, Clear>;

  // Performance counters
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
  template<Half half, uint64_t counter( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst )>
  static bool perfCounter( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
    if( R->IsRV32 )
      if constexpr( half == Lo )
        R->SetX( Inst.rd, static_cast<uint32_t>( counter( F, R, M, Inst ) & 0xffffffff ) );
      else
        R->SetX( Inst.rd, static_cast<uint32_t>( counter( F, R, M, Inst ) >> 32 ) );
    else if constexpr( half == Lo )
      R->SetX( Inst.rd, counter( F, R, M, Inst ) );
    else
      return false;  // Hi half is not available on RV64
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
  struct RevZiCSRInstDefaults : RevInstDefaults {
    RevZiCSRInstDefaults() {
      SetOpcode( 0b1110011 );
      SetRaiseFPE( true );
      SetFunct2or7( 0 );
      SetrdClass( RevRegClass::RegGPR );
      Setrs2Class( RevRegClass::RegUNKNOWN );
      Setimm( FVal );
      SetFormat( RVTypeI );
    }
  };

  // clang-format off
  std::vector<RevInstEntry> ZiCSRTable = {


    { RevZiCSRInstDefaults().SetMnemonic( "csrrw %csr, %rd, %rs1"  ).SetFunct3( 0b001 ).SetImplFunc( csrrw      ).SetPredicate( []( uint32_t Inst ){ return DECODE_RD( Inst ) != 0 && DECODE_IMM12( Inst ) != 0x2; } ) },
    { RevZiCSRInstDefaults().SetMnemonic( "csrw %csr, %rd, %rs1"   ).SetFunct3( 0b001 ).SetImplFunc( csrrw      ).SetPredicate( []( uint32_t Inst ){ return DECODE_RD( Inst ) == 0; } ) },
    { RevZiCSRInstDefaults().SetMnemonic( "fsrm %rd, %rs"          ).SetFunct3( 0b001 ).SetImplFunc( csrrw      ).SetPredicate( []( uint32_t Inst ){ return DECODE_RD( Inst ) != 0 && DECODE_IMM12( Inst ) == 0x2; } ) },

    { RevZiCSRInstDefaults().SetMnemonic( "csrrs %rd, %csr, %rs1"  ).SetFunct3( 0b010 ).SetImplFunc( csrrs      ).SetPredicate( []( uint32_t Inst ){ return DECODE_RS1( Inst ) != 0; } ) },
    { RevZiCSRInstDefaults().SetMnemonic( "csrr %rd, %csr"         ).SetFunct3( 0b010 ).SetImplFunc( csrrs      ).SetPredicate( []( uint32_t Inst ){ return DECODE_RS1( Inst ) == 0 && [](auto imm){ return imm < 0xc80 || imm > 0xc82; }(DECODE_IMM12( Inst ) | 0x80); } ) },
    { RevZiCSRInstDefaults().SetMnemonic( "rdcycle %rd, %csr"      ).SetFunct3( 0b010 ).SetImplFunc( rdcycle    ).SetPredicate( []( uint32_t Inst ){ return DECODE_RS1( Inst ) == 0 && DECODE_IMM12( Inst ) == 0xc00; } ) },
    { RevZiCSRInstDefaults().SetMnemonic( "rdcycleh %rd, %csr"     ).SetFunct3( 0b010 ).SetImplFunc( rdcycleh   ).SetPredicate( []( uint32_t Inst ){ return DECODE_RS1( Inst ) == 0 && DECODE_IMM12( Inst ) == 0xc80; } ) },
    { RevZiCSRInstDefaults().SetMnemonic( "rdtime %rd, %csr"       ).SetFunct3( 0b010 ).SetImplFunc( rdtime     ).SetPredicate( []( uint32_t Inst ){ return DECODE_RS1( Inst ) == 0 && DECODE_IMM12( Inst ) == 0xc01; } ) },
    { RevZiCSRInstDefaults().SetMnemonic( "rdtimeh %rd, %csr"      ).SetFunct3( 0b010 ).SetImplFunc( rdtimeh    ).SetPredicate( []( uint32_t Inst ){ return DECODE_RS1( Inst ) == 0 && DECODE_IMM12( Inst ) == 0xc81; } ) },
    { RevZiCSRInstDefaults().SetMnemonic( "rdinstret %rd, %csr"    ).SetFunct3( 0b010 ).SetImplFunc( rdinstret  ).SetPredicate( []( uint32_t Inst ){ return DECODE_RS1( Inst ) == 0 && DECODE_IMM12( Inst ) == 0xc02; } ) },
    { RevZiCSRInstDefaults().SetMnemonic( "rdinstreth %rd, %csr"   ).SetFunct3( 0b010 ).SetImplFunc( rdinstreth ).SetPredicate( []( uint32_t Inst ){ return DECODE_RS1( Inst ) == 0 && DECODE_IMM12( Inst ) == 0xc82; } ) },

    { RevZiCSRInstDefaults().SetMnemonic( "csrrc %rd, %csr, %rs1"  ).SetFunct3( 0b011 ).SetImplFunc( csrrc      ).SetPredicate( []( uint32_t Inst ){ return DECODE_RD( Inst ) != 0; } ) },
    { RevZiCSRInstDefaults().SetMnemonic( "csrc %csr,%rs1"         ).SetFunct3( 0b011 ).SetImplFunc( csrrc      ).SetPredicate( []( uint32_t Inst ){ return DECODE_RD( Inst ) == 0; } ) },

    { RevZiCSRInstDefaults().SetMnemonic( "csrrwi %rd, %csr, $imm" ).SetFunct3( 0b101 ).SetImplFunc( csrrwi     ).SetPredicate( []( uint32_t Inst ){ return DECODE_RD( Inst ) != 0; } ) },
    { RevZiCSRInstDefaults().SetMnemonic( "csrwi %csr, $imm"       ).SetFunct3( 0b101 ).SetImplFunc( csrrwi     ).SetPredicate( []( uint32_t Inst ){ return DECODE_RD( Inst ) == 0; } ) },

    { RevZiCSRInstDefaults().SetMnemonic( "csrrsi %rd, %csr, $imm" ).SetFunct3( 0b110 ).SetImplFunc( csrrsi     ).SetPredicate( []( uint32_t Inst ){ return DECODE_RD( Inst ) != 0; } ) },
    { RevZiCSRInstDefaults().SetMnemonic( "csrsi %csr, $imm"       ).SetFunct3( 0b110 ).SetImplFunc( csrrsi     ).SetPredicate( []( uint32_t Inst ){ return DECODE_RD( Inst ) == 0; } ) },

    { RevZiCSRInstDefaults().SetMnemonic( "csrrci %rd, %csr, $imm" ).SetFunct3( 0b111 ).SetImplFunc( csrrci     ).SetPredicate( []( uint32_t Inst ){ return DECODE_RD( Inst ) != 0; } ) },
    { RevZiCSRInstDefaults().SetMnemonic( "csrci %csr, $imm"       ).SetFunct3( 0b111 ).SetImplFunc( csrrci     ).SetPredicate( []( uint32_t Inst ){ return DECODE_RD( Inst ) == 0; } ) },
  };
  // clang-format on

public:
  /// ZiCSR: standard constructor
  ZiCSR( RevFeature* Feature, RevMem* RevMem, SST::Output* Output ) : RevExt( "ZiCSR", Feature, RevMem, Output ) {
    SetTable( std::move( ZiCSRTable ) );
  }

};  // end class ZiCSR

}  // namespace SST::RevCPU

#endif
