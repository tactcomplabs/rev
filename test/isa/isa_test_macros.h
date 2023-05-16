

#ifndef __ISA_TEST_MACROS_H
#define __ISA_TEST_MACROS_H

#define assert(x) if (!(x)) { asm(".byte 0x00"); asm(".byte 0x00"); asm(".byte 0x00"); asm(".byte 0x00"); }

#define MASK_XLEN(x) ((x) & ((1 << (__riscv_xlen - 1) << 1) - 1))
#define SEXT_IMM(x) ((x) | (-(((x) >> 11) & 1) << 11))

#define TESTNUM gp
#define STR(s) #s
#define XSTR(s) STR(s)

#define ASM_GEN(...) asm volatile(#__VA_ARGS__);
#define ASM_GEN_NV(...) asm (#__VA_ARGS__);

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

#define TEST_IMM_SRC1_EQ_DEST( testnum, inst, result, val1, imm ) \
    TEST_CASE( testnum, x1, result, \
      ASM_GEN_MASK(x1, val1); \
      ASM_GEN_SEXT(inst, x1, x1, imm); \
    )

#define TEST_LD_OP( testnum, inst, result, offset, base ) \
  TEST_CASE( testnum, x14, result, \
    ASM_GEN(li  x15, result);  \
    ASM_GEN(la  x1, base); \
    ASM_GEN(inst x14, offset(x1)); \
  )

#define TEST_ST_OP( testnum, load_inst, store_inst, result, offset, base ) \
    TEST_CASE( testnum, x14, result, \
      ASM_GEN(la  x1, base); \
      ASM_GEN(li  x2, result); \
      ASM_GEN(la  x15, 7f);  \
      ASM_GEN(store_inst x2, offset(x1)); \
      ASM_GEN(load_inst x14, offset(x1)); \
      ASM_GEN(j 8f); \
      asm volatile("7:");    \
      ASM_GEN(mv x14, x2); \
      asm volatile("8:");    \
  )

#define RVTEST_DATA_BEGIN                                               \
        ASM_GEN_NV(.pushsection .tohost,"aw",@progbits);                            \
        ASM_GEN_NV(.align 6; .global tohost; tohost: .dword 0; .size tohost, 8);    \
        ASM_GEN_NV(.align 6; .global fromhost; fromhost: .dword 0; .size fromhost, 8);\
        ASM_GEN_NV(.popsection);                                                    \
        ASM_GEN_NV(.align 4; .global begin_signature; begin_signature:);

#define RVTEST_DATA_END \
       ASM_GEN_NV(.align 4; .global end_signature; end_signature:);

#endif
