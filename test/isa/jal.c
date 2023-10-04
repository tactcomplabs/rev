/*
 * jal.c
 *
 * RISC-V ISA: RV32I
 *
 * Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
 * All Rights Reserved
 * contact@tactcomplabs.com
 *
 * See LICENSE in the top level directory for licensing details
 *
 */

int main(){
  asm(
    " mv t1, ra\n"
    " j 3f\n"

    "1:\n"
    "  ret\n"
    " .zero 0x80000-0x8\n"
    "2:\n"
    " ret\n"
    " .zero 0x80000-0x8\n"
    "3:\n"
    " jal 1b\n"
    " jal 2b\n"
    " jal 4f\n"
    " jal 5f\n"
    " j 6f\n"
    " ret\n"
    " .zero 0x80000-0x4\n"
    "4:\n"
    " ret\n"
    " .zero 0x80000-0x10\n"
    "5:\n"
    " ret\n"

    "6:\n"
    " mv ra, t1\n"
    : : : "t1");

    return 0;
}
