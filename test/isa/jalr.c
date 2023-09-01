/*
 * jalr.c
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

#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "isa_test_macros.h"

#define FAIL do { asm(".byte 0x00"); asm(".byte 0x00"); asm(".byte 0x00"); asm(".byte 0x00"); } while(0)

int main(int argc, char **argv){
    void* ret1 = 0, *retaddr1 = 0;

    // Different input and output registers for JALR
    asm volatile(
        " lla %1, retaddr1\n"
        " jalr %0, 0(%2)\n"
        "retaddr1:\n"
        : "=r&"(ret1), "=&r"(retaddr1) : "r"(&&target1));
    FAIL;
 target1:
    if (ret1 != retaddr1)
        FAIL;

    // Same input and output registers for JALR
    void* ret2 = 0, *retaddr2 = 0;
    asm volatile(
        " lla %1, retaddr2\n"
        " jalr %0, 0(%0)\n"
        "retaddr2:\n"
        : "=r"(ret2), "=&r"(retaddr2) : "0"(&&target2));
    FAIL;
 target2:
    if (ret2 != retaddr2)
        FAIL;

    return 0;
}
