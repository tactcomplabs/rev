//
// _Zicbom_h_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_ZICBOM_H_
#define _SST_REVCPU_ZICBOM_H_

#include "../RevExt.h"
#include "../RevInstHelpers.h"

#include <limits>
#include <vector>

namespace SST::RevCPU {

class Zicbom : public RevExt {
  enum CBO : uint16_t {
    INVAL = 0b000,
    CLEAN = 0b001,
    FLUSH = 0b010,
    ZERO  = 0b100,
  };

  static bool cmo( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
    switch( Inst.imm ) {
    case CBO::INVAL: M->InvLine( F->GetHartToExecID(), R->GetX<uint64_t>( Inst.rs1 ) ); break;
    case CBO::CLEAN: M->CleanLine( F->GetHartToExecID(), R->GetX<uint64_t>( Inst.rs1 ) ); break;
    case CBO::FLUSH: M->FlushLine( F->GetHartToExecID(), R->GetX<uint64_t>( Inst.rs1 ) ); break;
    default: return false;
    }
    R->AdvancePC( Inst );
    return true;
  }

  struct RevZicbomInstDefaults : RevInstDefaults {
    RevZicbomInstDefaults() {
      SetFormat( RVTypeI );
      SetOpcode( 0b0001111 );
      SetFunct3( 0b010 );
      Setrs2Class( RevRegClass::RegUNKNOWN );
      SetrdClass( RevRegClass::RegUNKNOWN );
      Setimm( FEnc );
      SetImplFunc( cmo );
    }
  };

  // clang-format off
  std::vector<RevInstEntry> ZicbomTable = {
    RevZicbomInstDefaults().SetMnemonic( "cbo.inval" ).Setimm12( CBO::INVAL ),
    RevZicbomInstDefaults().SetMnemonic( "cbo.clean" ).Setimm12( CBO::CLEAN ),
    RevZicbomInstDefaults().SetMnemonic( "cbo.flush" ).Setimm12( CBO::FLUSH ),
//  RevZicbomInstDefaults().SetMnemonic( "cbo.zero"  ).Setimm12( CBO::ZERO  ),
  };
  // clang-format on

public:
  Zicbom( RevFeature* Feature, RevMem* RevMem, SST::Output* Output ) : RevExt( "Zicbom", Feature, RevMem, Output ) {
    SetTable( std::move( ZicbomTable ) );
  }
};  // end class Zicbom
}  // namespace SST::RevCPU

#endif
