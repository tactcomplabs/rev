//
// _RevCorePasskey_h_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//


#ifndef __REV_PROC_PASSKEY__
#define __REV_PROC_PASSKEY__

namespace SST::RevCPU {

template< typename T >
class RevCorePasskey {
private:
  friend T;
  RevCorePasskey(){};
  RevCorePasskey( const RevCorePasskey& ){};
  RevCorePasskey& operator=( const RevCorePasskey& ) = delete;
};
}  // namespace SST::RevCPU
#endif
