//
// _PanAddr_h_
//
// Copyright (C) 2017-2020 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _PANADDR_H_
#define _PANADDR_H_

#define _PAN_COMPLETION_ADDR_ 0x30000000
#define _PAN_RDMA_MAILBOX_    0x30000008
#define _PAN_RDMA_MAX_ENTRIES 1024

typedef struct{
  uint64_t Valid;   ///< MBoxEntry: Valid field
  uint64_t Addr;    ///< MBoxEntry: Address field
}MBoxEntry;

#endif

// EOF
