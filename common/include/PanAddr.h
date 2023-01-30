//
// _PanAddr_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _PANADDR_H_
#define _PANADDR_H_

#define _PAN_COMPLETION_ADDR_       0x30000000
#define _PAN_RDMA_MAILBOX_          0x30000008
#define _PAN_RDMA_MAX_ENTRIES_      255
#define _PAN_PE_TABLE_ADDR_         0x30001F48
#define _PAN_PE_TABLE_MAX_ENTRIES_  1025
#define _PAN_XFER_BUF_              1024
#define _PAN_XFER_BUF_ADDR_         0x30005F68
#define _PAN_FWARE_JUMP_            0x0000000000010000

#define _PAN_ENTRY_INVALID_       0
#define _PAN_ENTRY_INJECTED_      1
#define _PAN_ENTRY_VALID_         2
#define _PAN_ENTRY_DONE_SUCCESS_  3
#define _PAN_ENTRY_DONE_FAILED_   4

typedef struct{
  uint64_t Valid;   ///< MBoxEntry: Valid field
  uint64_t Dest;    ///< MBoxEntry: Destination field
  uint64_t Addr;    ///< MBoxEntry: Address field
}MBoxEntry;

typedef struct{
  int64_t ID;       ///< PEMap: Endpoint ID
  uint64_t host;    ///< PEMap: 1 = host; 0 = PAN
}PEMap;

typedef struct{
  uint8_t Valid;                      ///< PRTIME_XFER: Valid bit
  char Buffer[_PAN_XFER_BUF_];        ///< PRTIME_XFER: Bounce buffers for base packets
}PRTIME_XFER;

#endif

// EOF
