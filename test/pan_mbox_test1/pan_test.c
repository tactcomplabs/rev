/*
 * pan_test.c
 *
 * RISC-V ISA: RV64I
 *
 * Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
 * All Rights Reserved
 * contact@tactcomplabs.com
 *
 * See LICENSE in the top level directory for licensing details
 *
 */

#include "../../common/include/PanAddr.h"
#include <stdint.h>
#include <stdlib.h>

typedef struct {
  uint64_t Base;
  uint64_t Addr;
  uint64_t Data;
} Command;

Command    get_cmd;
Command    reserve_cmd;
Command    revoke_cmd;
Command    complete_cmd;
MBoxEntry* Mailbox = (MBoxEntry*) ( _PAN_RDMA_MAILBOX_ );

uint32_t Token     = 0xfeedbeef;
uint32_t Tag       = 0x1;
uint64_t Input     = 0x1;

void cmd_wait() {
  // this function blocks until the command is cleared from the mailbox
  volatile uint64_t value = Mailbox[0].Valid;
  while( value != _PAN_ENTRY_INJECTED_ ) {
    value = Mailbox[0].Valid;
  }
  Mailbox[0].Valid = _PAN_ENTRY_INVALID_;
}

void send_completion() {
  complete_cmd.Base = 0x80403feedbeef;
  complete_cmd.Addr = _PAN_COMPLETION_ADDR_;
  //complete_cmd.Data  = 0xdeadbeef;
  complete_cmd.Data = (uint64_t) ( &Input );  // updated to include buffer
  Mailbox[0].Addr   = (uint64_t) ( &complete_cmd );
  Mailbox[0].Dest   = 1;
  Mailbox[0].Valid  = _PAN_ENTRY_VALID_;
  cmd_wait();
  uint64_t* ptr = (uint64_t*) ( _PAN_COMPLETION_ADDR_ );
  *ptr          = 0x001ull;
  Tag++;
}

void send_put_cmd() {
  // payload = 0x402feedbeef
  Tag++;
}

void send_reserve_cmd() {
  reserve_cmd.Base = 0x4001feedbeef;
  Mailbox[0].Addr  = (uint64_t) ( &reserve_cmd.Base );
  Mailbox[0].Dest  = 1;
  Mailbox[0].Valid = _PAN_ENTRY_VALID_;
  cmd_wait();
  Tag++;
}

void send_revoke_cmd() {
  revoke_cmd.Base  = 0x5004feedbeef;
  Mailbox[0].Addr  = (uint64_t) ( &revoke_cmd );
  Mailbox[0].Dest  = 1;
  Mailbox[0].Valid = _PAN_ENTRY_VALID_;
  cmd_wait();
}

int main( int argc, char** argv ) {
  uint64_t*         ptr = (uint64_t*) ( _PAN_COMPLETION_ADDR_ );
  volatile uint64_t value;

  send_reserve_cmd();

  //send_put_cmd();

  send_completion();

  send_revoke_cmd();

  value = *ptr;
  while( value == 0x00ull ) {
    value = *ptr;
  }

  return 0;
}
