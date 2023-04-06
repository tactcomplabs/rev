

#ifndef __ISA_TEST_MACROS_H
#define __ISA_TEST_MACROS_H

#define MASK_XLEN(x) ((x) & ((1 << (__riscv_xlen - 1) << 1) - 1))
#define SEXT_IMM(x) ((x) | (-(((x) >> 11) & 1) << 11))

#define TESTNUM gp
#define STR(s) #s
#define XSTR(s) STR(s)

#define ASM_GEN(...) asm volatile(#__VA_ARGS__);

#define ASM_GEN_MASK(a, v) asm volatile("li " #a ", " XSTR(MASK_XLEN(v))) 
#define ASM_GEN_SEXT(inst, d, a, v) asm volatile(#inst " " #d ", " #a "," XSTR(SEXT_IMM(v))) 

#define TEST_INSERT_NOPS_0
#define TEST_INSERT_NOPS_1  ASM_GEN(nop); TEST_INSERT_NOPS_0
#define TEST_INSERT_NOPS_2  ASM_GEN(nop); TEST_INSERT_NOPS_1
#define TEST_INSERT_NOPS_3  ASM_GEN(nop); TEST_INSERT_NOPS_2
#define TEST_INSERT_NOPS_4  ASM_GEN(nop); TEST_INSERT_NOPS_3
#define TEST_INSERT_NOPS_5  ASM_GEN(nop); TEST_INSERT_NOPS_4
#define TEST_INSERT_NOPS_6  ASM_GEN(nop); TEST_INSERT_NOPS_5
#define TEST_INSERT_NOPS_7  ASM_GEN(nop); TEST_INSERT_NOPS_6
#define TEST_INSERT_NOPS_8  ASM_GEN(nop); TEST_INSERT_NOPS_7
#define TEST_INSERT_NOPS_9  ASM_GEN(nop); TEST_INSERT_NOPS_8
#define TEST_INSERT_NOPS_10 ASM_GEN(nop); TEST_INSERT_NOPS_9

#define TEST_CASE( testnum, testreg, correctval, code... ) \
asm volatile("test_%0:" : :"I"(testnum)); \
    asm volatile("li  gp, %0;" : : "I"(testnum)); \
    code; \
    ASM_GEN_MASK(x7, correctval); \
    ASM_GEN(bne testreg, x7, fail); 

#define TEST_RR_OP( testnum, inst, result, val1, val2 ) \
    TEST_CASE( testnum, x14, result, \
      ASM_GEN_MASK(x1, val1); \
      ASM_GEN_MASK(x2, val2); \
      ASM_GEN(inst x14, x1,x2); \
    )
#define TEST_RR_SRC1_EQ_DEST( testnum, inst, result, val1, val2 ) \
    TEST_CASE( testnum, x1, result, \
      ASM_GEN_MASK(x1, val1); \
      ASM_GEN_MASK(x2, val2); \
      ASM_GEN(inst x1, x1, x2); \
    )

#define TEST_RR_SRC2_EQ_DEST( testnum, inst, result, val1, val2 ) \
    TEST_CASE( testnum, x2, result, \
      ASM_GEN_MASK(x1, val1); \
      ASM_GEN_MASK(x2, val2); \
      ASM_GEN(inst x2, x1, x2); \
    )

#define TEST_RR_SRC12_EQ_DEST( testnum, inst, result, val1 ) \
   TEST_CASE( testnum, x1, result, \
      ASM_GEN_MASK(x1, val1); \
      ASM_GEN(inst x1, x1, x1); \
    )

#define TEST_IMM_OP( testnum, inst, result, val1, imm ) \
    TEST_CASE( testnum, x14, result, \
      ASM_GEN_MASK(x1, val1); \
      ASM_GEN_SEXT(inst, x14, x1, imm); \
    )

#endif