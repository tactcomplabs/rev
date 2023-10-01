/*
 * dep_check.c
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

int main(int argc, char **argv){
  int o;


/* The assembly sequence below should produce a
 * cascade of dependent instructions
 * The cost of "add" in the RV32I instruction
 * table MUST be set to a value larger than 1
 * otherwise you will see no change w/ or w/o a
 * Dependency Check - in other words, if cost == 1
 * then the reported eff of the core will remain
 * at 100%, at cost > 1 eff will drop below 100%
 * */

  //ADDI a0, zero, 42
  //ADD a0, a0, a0
  //ADD a0, a0, a0
  //ADD a0, a0, a0

  asm volatile("ADDI a0, zero, %1" : "=r"(o) :  "I"(42));
  asm volatile("ADD a0, a0, a0");
  asm volatile("ADD a0, a0, a0");
  asm volatile("ADD a0, a0, a0");
  asm volatile("ADD a0, a0, a0");
  asm volatile("ADD a0, a0, a0");
  asm volatile("ADD a0, a0, a0");
  asm volatile("ADD a0, a0, a0");
  asm volatile("ADD a0, a0, a0");
  asm volatile("ADD a0, a0, a0");
  asm volatile("ADD a0, a0, a0");
  asm volatile("ADD a0, a0, a0");
  asm volatile("ADD a0, a0, a0");
  asm volatile("ADD a0, a0, a0");
  asm volatile("ADD a0, a0, a0");
  asm volatile("ADD a0, a0, a0");
  asm volatile("ADD a0, a0, a0");
  asm volatile("ADD a0, a0, a0");
  asm volatile("ADD a0, a0, a0");
  asm volatile("ADD a0, a0, a0");
  asm volatile("ADD a0, a0, a0");
  asm volatile("ADD a0, a0, a0");
  asm volatile("ADD a0, a0, a0" : "=r"(o));

  return o;
}
