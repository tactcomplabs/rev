/*
 * large_bss.c
 *
 * RISC-V ISA: RV64G
 *
 * Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
 * All Rights Reserved
 * contact@tactcomplabs.com
 *
 * See LICENSE in the top level directory for licensing details
 *
 */

#include <stdlib.h>

char t[1024*1024*12]; int main() { return 0; }
