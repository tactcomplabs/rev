
/*
 * ex2.c
 *
 * RISC-V ISA: RV64I
 *
 * Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
 * All Rights Reserved
 * contact@tactcomplabs.com
 *
 * See LICENSE in the top level directory for licensing details
 *
 */

#include <stdlib.h>
#include "../../../common/syscalls/syscalls.h"
#define SCRATCHPAD_ADDR 0x80000000

int main(int argc, char **argv){
  
  rev_basic_scratchpad_access(SCRATCHPAD_ADDR);
  int i = 9;
  i = i + argc;
  return i;
}
