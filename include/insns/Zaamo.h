//
// _Zaamo_h_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_Zaamo_H_
#define _SST_REVCPU_Zaamo_H_

#include "../RevExt.h"
#include "../RevInstHelpers.h"

namespace SST::RevCPU {

class Zaamo : public RevExt {
  template<typename XLEN, RevFlag F_AMO>
  static bool amooper( const RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
    static_assert( std::is_unsigned_v<XLEN>, "XLEN must be unsigned integral type" );

    RevFlag flags{ F_AMO };

    if( Inst.aq ) {
      flags = RevFlag{ uint32_t( flags ) | uint32_t( RevFlag::F_AQ ) };
    }
    if( Inst.rl ) {
      flags = RevFlag{ uint32_t( flags ) | uint32_t( RevFlag::F_RL ) };
    }

    if( !R->IsRV64 ) {
      MemReq req(
        R->RV32[Inst.rs1], Inst.rd, RevRegClass::RegGPR, F->GetHartToExecID(), MemOp::MemOpAMO, true, R->GetMarkLoadComplete()
      );
      R->LSQueue->insert( req.LSQHashPair() );
      M->AMOVal( F->GetHartToExecID(), R->RV32[Inst.rs1], &R->RV32[Inst.rs2], &R->RV32[Inst.rd], req, flags );
    } else {
      flags = RevFlag{ uint32_t( flags ) | uint32_t( RevFlag::F_SEXT64 ) };
      MemReq req(
        R->RV64[Inst.rs1], Inst.rd, RevRegClass::RegGPR, F->GetHartToExecID(), MemOp::MemOpAMO, true, R->GetMarkLoadComplete()
      );
      R->LSQueue->insert( req.LSQHashPair() );
      M->AMOVal(
        F->GetHartToExecID(),
        R->RV64[Inst.rs1],
        reinterpret_cast<std::make_signed_t<XLEN>*>( &R->RV64[Inst.rs2] ),
        reinterpret_cast<std::make_signed_t<XLEN>*>( &R->RV64[Inst.rd] ),
        req,
        flags
      );
    }
    // update the cost
    R->cost += M->RandCost( F->GetMinCost(), F->GetMaxCost() );
    R->AdvancePC( Inst );
    return true;
  }

  static constexpr auto& amoaddw  = amooper<uint32_t, RevFlag::F_AMOADD>;
  static constexpr auto& amoandw  = amooper<uint32_t, RevFlag::F_AMOAND>;
  static constexpr auto& amomaxuw = amooper<uint32_t, RevFlag::F_AMOMAXU>;
  static constexpr auto& amomaxw  = amooper<uint32_t, RevFlag::F_AMOMAX>;
  static constexpr auto& amominuw = amooper<uint32_t, RevFlag::F_AMOMINU>;
  static constexpr auto& amominw  = amooper<uint32_t, RevFlag::F_AMOMIN>;
  static constexpr auto& amoorw   = amooper<uint32_t, RevFlag::F_AMOOR>;
  static constexpr auto& amoswapw = amooper<uint32_t, RevFlag::F_AMOSWAP>;
  static constexpr auto& amoxorw  = amooper<uint32_t, RevFlag::F_AMOXOR>;

  static constexpr auto& amoaddd  = amooper<uint64_t, RevFlag::F_AMOADD>;
  static constexpr auto& amoandd  = amooper<uint64_t, RevFlag::F_AMOAND>;
  static constexpr auto& amomaxd  = amooper<uint64_t, RevFlag::F_AMOMAX>;
  static constexpr auto& amomaxud = amooper<uint64_t, RevFlag::F_AMOMAXU>;
  static constexpr auto& amomind  = amooper<uint64_t, RevFlag::F_AMOMIN>;
  static constexpr auto& amominud = amooper<uint64_t, RevFlag::F_AMOMINU>;
  static constexpr auto& amoord   = amooper<uint64_t, RevFlag::F_AMOOR>;
  static constexpr auto& amoswapd = amooper<uint64_t, RevFlag::F_AMOSWAP>;
  static constexpr auto& amoxord  = amooper<uint64_t, RevFlag::F_AMOXOR>;

  // ----------------------------------------------------------------------
  //
  // RISC-V Zaamo Instructions
  //
  // ----------------------------------------------------------------------
  struct ZaamoInstDefaults : RevInstDefaults {
    ZaamoInstDefaults() { SetOpcode( 0b0101111 ); }
  };

  // clang-format off
  std::vector<RevInstEntry> ZaamoTable = {
    ZaamoInstDefaults().SetMnemonic( "amoswap.w %rd, %rs1, %rs2" ).SetFunct3( 0b010 ).SetFunct2or7( 0b0000001 ).SetImplFunc( amoswapw ),
    ZaamoInstDefaults().SetMnemonic( "amoadd.w %rd, %rs1, %rs2"  ).SetFunct3( 0b010 ).SetFunct2or7( 0b0000000 ).SetImplFunc( amoaddw  ),
    ZaamoInstDefaults().SetMnemonic( "amoxor.w %rd, %rs1, %rs2"  ).SetFunct3( 0b010 ).SetFunct2or7( 0b0000100 ).SetImplFunc( amoxorw  ),
    ZaamoInstDefaults().SetMnemonic( "amoand.w %rd, %rs1, %rs2"  ).SetFunct3( 0b010 ).SetFunct2or7( 0b0001100 ).SetImplFunc( amoandw  ),
    ZaamoInstDefaults().SetMnemonic( "amoor.w %rd, %rs1, %rs2"   ).SetFunct3( 0b010 ).SetFunct2or7( 0b0001000 ).SetImplFunc( amoorw   ),
    ZaamoInstDefaults().SetMnemonic( "amomin.w %rd, %rs1, %rs2"  ).SetFunct3( 0b010 ).SetFunct2or7( 0b0010000 ).SetImplFunc( amominw  ),
    ZaamoInstDefaults().SetMnemonic( "amomax.w %rd, %rs1, %rs2"  ).SetFunct3( 0b010 ).SetFunct2or7( 0b0010100 ).SetImplFunc( amomaxw  ),
    ZaamoInstDefaults().SetMnemonic( "amominu.w %rd, %rs1, %rs2" ).SetFunct3( 0b010 ).SetFunct2or7( 0b0011000 ).SetImplFunc( amominuw ),
    ZaamoInstDefaults().SetMnemonic( "amomaxu.w %rd, %rs1, %rs2" ).SetFunct3( 0b010 ).SetFunct2or7( 0b0011100 ).SetImplFunc( amomaxuw ),
  };

  static std::vector<RevInstEntry> RV64Table() { return {
    ZaamoInstDefaults().SetMnemonic( "amoswap.d %rd, %rs1, %rs2" ).SetFunct3( 0b011 ).SetFunct2or7( 0b0000001 ).SetImplFunc( amoswapd ),
    ZaamoInstDefaults().SetMnemonic( "amoadd.d %rd, %rs1, %rs2"  ).SetFunct3( 0b011 ).SetFunct2or7( 0b0000000 ).SetImplFunc( amoaddd  ),
    ZaamoInstDefaults().SetMnemonic( "amoxor.d %rd, %rs1, %rs2"  ).SetFunct3( 0b011 ).SetFunct2or7( 0b0000100 ).SetImplFunc( amoxord  ),
    ZaamoInstDefaults().SetMnemonic( "amoand.d %rd, %rs1, %rs2"  ).SetFunct3( 0b011 ).SetFunct2or7( 0b0001100 ).SetImplFunc( amoandd  ),
    ZaamoInstDefaults().SetMnemonic( "amoor.d %rd, %rs1, %rs2"   ).SetFunct3( 0b011 ).SetFunct2or7( 0b0001000 ).SetImplFunc( amoord   ),
    ZaamoInstDefaults().SetMnemonic( "amomin.d %rd, %rs1, %rs2"  ).SetFunct3( 0b011 ).SetFunct2or7( 0b0010000 ).SetImplFunc( amomind  ),
    ZaamoInstDefaults().SetMnemonic( "amomax.d %rd, %rs1, %rs2"  ).SetFunct3( 0b011 ).SetFunct2or7( 0b0010100 ).SetImplFunc( amomaxd  ),
    ZaamoInstDefaults().SetMnemonic( "amominu.d %rd, %rs1, %rs2" ).SetFunct3( 0b011 ).SetFunct2or7( 0b0011000 ).SetImplFunc( amominud ),
    ZaamoInstDefaults().SetMnemonic( "amomaxu.d %rd, %rs1, %rs2" ).SetFunct3( 0b011 ).SetFunct2or7( 0b0011100 ).SetImplFunc( amomaxud ),
  }; }

  // clang-format on

public:
  /// Zaamo: standard constructor
  Zaamo( const RevFeature* Feature, RevMem* RevMem, SST::Output* Output ) : RevExt( "Zaamo", Feature, RevMem, Output ) {
    if( Feature->IsRV64() ) {
      auto Table{ RV64Table() };
      ZaamoTable.insert( ZaamoTable.end(), std::move_iterator( Table.begin() ), std::move_iterator( Table.end() ) );
    }
    SetTable( std::move( ZaamoTable ) );
  }

};  // end class Zaamo

}  // namespace SST::RevCPU

#endif
