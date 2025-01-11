/*
 * pan_spin.c
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

int main( int argc, char** argv ) {
  uint64_t*         ptr   = (uint64_t*) ( _PAN_COMPLETION_ADDR_ );
  volatile uint64_t value = *ptr;

  while( value == 0x00ull ) {
    value = *ptr;
  }

  return 0;
}
