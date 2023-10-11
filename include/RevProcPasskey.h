//
// _RevProcPasskey_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//


#ifndef __REV_PROC_PASSKEY__
#define __REV_PROC_PASSKEY__
namespace SST::RevCPU{
  
template<typename T>
class RevProcPasskey{
  private:
  friend T;
  RevProcPasskey() {};
  RevProcPasskey(const RevProcPasskey&) {};
  RevProcPasskey& operator=(const RevProcPasskey&) = delete;
};
}
#endif