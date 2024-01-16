/*
 * ex7.c
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

__attribute__((naked)) int main() {
  asm(
    " add     sp, sp, -16\n"
    " sw      zero, 0(sp)\n"
    " lw      a0, 0(sp)\n"
    " fcvt.s.w fa0, a0\n"
    " fmv.w.x fa1, zero\n"
    " feq.s   a0, fa0, fa1\n"
    " bnez    a0, 1f\n"
    " .word   0\n"
    "1:\n"
    " add     sp, sp, 16\n"
    " li      a0, 0\n"
    " ret\n"
    );
}
