//
// _RevMemCoProc_cc_
//
// Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "RevMemCoProc.h"

namespace SST::RevCPU {

RevMemCoProc::RevMemCoProc( ComponentId_t id, Params& params, RevCore* parent, const std::string& clock, const std::string& memorySlot )
  : RevCoProc( id, params, parent ) {

  auto tc = registerClock( clock, new Clock::Handler<RevMemCoProc, &RevMemCoProc::ClockHandler>( this ) );

  memory = loadUserSubComponent<StdMem>(
    memorySlot, ComponentInfo::SHARE_NONE, tc, new StdMem::Handler<RevMemCoProc, &RevMemCoProc::MemResponse>( this )
  );
  if( !memory )
    output->fatal( CALL_INFO, -1, "Error: no memory interface in the '%s' subcomponent slot\n", memorySlot.c_str() );
}

}  // namespace SST::RevCPU
