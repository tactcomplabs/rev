//
// _Xcoproc_h_
//
// Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Xcoproc: the four custom opcodes (custom-0 0x0b, custom-1 0x2b, custom-2 0x5b, custom-3 0x7b) forwarded to the co-processor
// attached to the core, in the encoding RoCC uses: R-type, funct7 the command, and funct3 = { xd, xs1, xs2 } for whether the
// co-processor answers (the core writes rd) and which source registers it uses (the core reads them). What a command means is up
// to the co-processor (RevCoProc::Command); this file knows nothing about it.
//
// Each opcode has one table entry; the decoder keeps funct3 and funct7 of the instruction in the decoded RevInst.
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _SST_REVCPU_XCOPROC_H_
#define _SST_REVCPU_XCOPROC_H_

#include "../RevCoProcCmd.h"
#include "../RevExt.h"
#include "../RevInstHelpers.h"

namespace SST::RevCPU {

/// Hand the command to the co-processor of the core that owns the register file (RevCore.cc)
bool XcoprocForward( RevRegFile* R, const RevCoProcCmd& cmd, uint64_t& result );

class Xcoproc : public RevExt {

  template<unsigned N>
  static bool cmd( const RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
    RevCoProcCmd c;
    c.custom        = N;
    c.funct7        = Inst.funct2or7;
    c.xd            = Inst.funct3 & 0b100;
    c.xs1           = Inst.funct3 & 0b010;
    c.xs2           = Inst.funct3 & 0b001;
    c.rs1           = c.xs1 ? R->GetX<uint64_t>( Inst.rs1 ) : 0;
    c.rs2           = c.xs2 ? R->GetX<uint64_t>( Inst.rs2 ) : 0;
    c.hart          = F->GetHartToExecID();
    c.mem           = M;

    uint64_t result = 0;
    if( !XcoprocForward( R, c, result ) )
      return false;
    if( c.xd )
      R->SetX( Inst.rd, result );
    R->AdvancePC( Inst );
    return true;
  }

  struct RevXcoprocInstDefaults : RevInstDefaults {
    RevXcoprocInstDefaults() {
      SetFormat( RVTypeR );
      SetFunct3( 0 );
      SetFunct2or7( 0 );
    }
  };

  // clang-format off
  std::vector<RevInstEntry> XcoprocTable = {
    RevXcoprocInstDefaults().SetMnemonic( "cop0.cmd" ).SetOpcode( 0b0001011 ).SetImplFunc( cmd<0> ),
    RevXcoprocInstDefaults().SetMnemonic( "cop1.cmd" ).SetOpcode( 0b0101011 ).SetImplFunc( cmd<1> ),
    RevXcoprocInstDefaults().SetMnemonic( "cop2.cmd" ).SetOpcode( 0b1011011 ).SetImplFunc( cmd<2> ),
    RevXcoprocInstDefaults().SetMnemonic( "cop3.cmd" ).SetOpcode( 0b1111011 ).SetImplFunc( cmd<3> ),
  };
  // clang-format on

public:
  Xcoproc( const RevFeature* Feature, RevMem* RevMem, SST::Output* Output ) : RevExt( "Xcoproc", Feature, RevMem, Output ) {
    SetTable( std::move( XcoprocTable ) );
  }
};  // end class Xcoproc

}  // namespace SST::RevCPU

#endif
