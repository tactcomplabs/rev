//
// _Zifencei_h_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_ZIFENCEI_H_
#define _SST_REVCPU_ZIFENCEI_H_

#include "../RevExt.h"
#include "../RevInstHelpers.h"

namespace SST::RevCPU {

class Zifencei : public RevExt {

  static bool fencei( const RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
    M->FenceMem( F->GetHartToExecID() );
    R->AdvancePC( Inst );
    return true;
  }

  // clang-format off
  std::vector<RevInstEntry> ZifenceiTable = {
    RevInstDefaults().SetMnemonic("fence.i").SetFormat(RVTypeI).SetOpcode(0b0001111).SetFunct3(0b001).SetrdClass(RevRegClass::RegUNKNOWN).Setrs1Class(RevRegClass::RegUNKNOWN).Setrs2Class(RevRegClass::RegUNKNOWN).SetImplFunc(fencei),
  };
  // clang-format on

public:
  Zifencei( const RevFeature* Feature, RevMem* RevMem, SST::Output* Output ) : RevExt( "Zifencei", Feature, RevMem, Output ) {
    SetTable( std::move( ZifenceiTable ) );
  }

};  // end class Zifencei

}  // namespace SST::RevCPU

#endif
