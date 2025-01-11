//
// _Zalrsc_h_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_ZALRSC_H_
#define _SST_REVCPU_ZALRSC_H_

#include "../RevExt.h"
#include "../RevInstHelpers.h"

namespace SST::RevCPU {

class Zalrsc : public RevExt {

  // Load Reserved instruction
  template<typename TYPE>
  static bool lr( const RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
    static_assert( std::is_unsigned_v<TYPE>, "TYPE must be unsigned integral type" );
    auto addr = R->GetX<uint64_t>( Inst.rs1 );

    // Create the load request
    MemReq req{ addr, Inst.rd, RevRegClass::RegGPR, F->GetHartToExecID(), MemOp::MemOpAMO, true, R->GetMarkLoadComplete() };
    R->LSQueue->insert( req.LSQHashPair() );

    // Flags for LR memory load
    RevFlag flags = RevFlag::F_NONE;
    if( sizeof( TYPE ) < sizeof( int64_t ) && F->IsRV64() )
      RevFlagSet( flags, RevFlag::F_SEXT64 );
    if( Inst.aq )
      RevFlagSet( flags, RevFlag::F_AQ );
    if( Inst.rl )
      RevFlagSet( flags, RevFlag::F_RL );

    // Where the data will eventually end up
    void* target;
    if( sizeof( TYPE ) >= sizeof( int64_t ) || F->IsRV64() ) {
      target = &R->RV64[Inst.rd];
    } else {
      target = &R->RV32[Inst.rd];
    }

    // Create a reservation and load the data
    M->LR( F->GetHartToExecID(), addr, sizeof( TYPE ), target, req, flags );
    R->cost += M->RandCost( F->GetMinCost(), F->GetMaxCost() );
    R->AdvancePC( Inst );
    return true;
  }

  // Store Conditional instruction
  template<typename TYPE>
  static bool sc( const RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
    static_assert( std::is_unsigned_v<TYPE>, "TYPE must be unsigned integral type" );
    auto addr     = R->GetX<uint64_t>( Inst.rs1 );
    auto val      = R->GetX<TYPE>( Inst.rs2 );

    // Flags for SC Store Conditional
    RevFlag flags = RevFlag::F_NONE;
    if( Inst.aq )
      RevFlagSet( flags, RevFlag::F_AQ );
    if( Inst.rl )
      RevFlagSet( flags, RevFlag::F_RL );

    // Conditionally store the data, unconditionally removing any reservation
    bool sc = M->SC( F->GetHartToExecID(), addr, sizeof( TYPE ), &val, flags );

    // If the SC succeeded, store 0 in the destination register; otherwise store 1
    R->SetX( Inst.rd, !sc );
    R->AdvancePC( Inst );
    return true;
  }

  static constexpr auto& lrw = lr<uint32_t>;
  static constexpr auto& scw = sc<uint32_t>;
  static constexpr auto& lrd = lr<uint64_t>;
  static constexpr auto& scd = sc<uint64_t>;

  // ----------------------------------------------------------------------
  //
  // RISC-V Zalrsc Instructions
  //
  // ----------------------------------------------------------------------
  struct ZalrscInstDefaults : RevInstDefaults {
    ZalrscInstDefaults() { SetOpcode( 0b0101111 ); }
  };

  // clang-format off
  std::vector<RevInstEntry> ZalrscTable = {
    ZalrscInstDefaults().SetMnemonic( "lr.w %rd, (%rs1)"     ).SetFunct3( 0b010 ).SetFunct2or7( 0b0000010 ).SetImplFunc( lrw ).Setrs2Class( RevRegClass::RegUNKNOWN ),
    ZalrscInstDefaults().SetMnemonic( "sc.w %rd, %rs1, %rs2" ).SetFunct3( 0b010 ).SetFunct2or7( 0b0000011 ).SetImplFunc( scw ),
  };

  std::vector<RevInstEntry> RV64ZalrscTable() { return {
    ZalrscInstDefaults().SetMnemonic( "lr.d %rd, (%rs1)"     ).SetFunct3( 0b011 ).SetFunct2or7( 0b0000010 ).SetImplFunc( lrd ).Setrs2Class( RevRegClass::RegUNKNOWN ),
    ZalrscInstDefaults().SetMnemonic( "sc.d %rd, %rs1, %rs2" ).SetFunct3( 0b011 ).SetFunct2or7( 0b0000011 ).SetImplFunc( scd ),
    };
  }

  // clang-format on

public:
  /// Zalrsc: standard constructor
  Zalrsc( const RevFeature* Feature, RevMem* RevMem, SST::Output* Output ) : RevExt( "Zalrsc", Feature, RevMem, Output ) {
    if( Feature->IsRV64() ) {
      auto Table{ RV64ZalrscTable() };
      ZalrscTable.insert( ZalrscTable.end(), std::move_iterator( Table.begin() ), std::move_iterator( Table.end() ) );
    }
    SetTable( std::move( ZalrscTable ) );
  }

};  // end class Zalrsc

}  // namespace SST::RevCPU

#endif
