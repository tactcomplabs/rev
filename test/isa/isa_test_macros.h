

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
#define ASM_TEST_NUM_GEN(n) asm ("test_" #n "_data:")

#define ASM_GEN_MASK(a, v) asm volatile("li " #a ", " XSTR(MASK_XLEN(v)))
#define ASM_GEN_SEXT(inst, d, a, v) asm volatile(#inst " " #d ", " #a ", " XSTR(SEXT_IMM(v)))

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
      ASM_GEN_MASK(x5, val1); \
      ASM_GEN_MASK(x6, val2); \
      ASM_GEN(inst x14, x5, x6); \
    )
#define TEST_RR_SRC1_EQ_DEST( testnum, inst, result, val1, val2 ) \
    TEST_CASE( testnum, x5, result, \
      ASM_GEN_MASK(x5, val1); \
      ASM_GEN_MASK(x6, val2); \
      ASM_GEN(inst x5, x5, x6); \
    )

#define TEST_RR_SRC2_EQ_DEST( testnum, inst, result, val1, val2 ) \
    TEST_CASE( testnum, x6, result, \
      ASM_GEN_MASK(x5, val1); \
      ASM_GEN_MASK(x6, val2); \
      ASM_GEN(inst x6, x5, x6); \
    )

#define TEST_RR_SRC12_EQ_DEST( testnum, inst, result, val1 ) \
   TEST_CASE( testnum, x5, result, \
      ASM_GEN_MASK(x5, val1); \
      ASM_GEN(inst x5, x5, x5); \
    )

#define TEST_IMM_OP( testnum, inst, result, val1, imm ) \
    TEST_CASE( testnum, x14, result, \
      ASM_GEN_MASK(x5, val1); \
      ASM_GEN_SEXT(inst, x14, x5, imm); \
    )

#define TEST_IMM_SRC1_EQ_DEST( testnum, inst, result, val1, imm ) \
    TEST_CASE( testnum, x5, result, \
      ASM_GEN_MASK(x5, val1); \
      ASM_GEN_SEXT(inst, x5, x5, imm); \
    )

#define TEST_FP_OP_1S_INTERNAL( testnum, flags, result, val1, code... ) \
  asm volatile("test_%0:" : :"I"(testnum)); \
  asm volatile("li  gp, %0;" : : "I"(testnum)); \
  asm volatile("la  a0, test_%0_data;" : : "I"(testnum)) ;\
  ASM_GEN(flw f0, 0(a0)); \
  ASM_GEN(lw a3, 4(a0)); \
  code; \
  ASM_GEN(bne a0, a3, fail);

#define TEST_FP_OP_2S_INTERNAL( testnum, flags, result, val1, val2, code... ) \
  asm volatile("test_%0:" : :"I"(testnum)); \
  asm volatile("li  gp, %0;" : : "I"(testnum)); \
  asm volatile("la  a0, test_%0_data;" : : "I"(testnum)) ;\
  ASM_GEN(flw f0, 0(a0)); \
  ASM_GEN(flw f1, 4(a0)); \
  ASM_GEN(lw a3, 8(a0)); \
  code; \
  ASM_GEN(bne a0, a3, fail);

#define TEST_FP_OP_3S_INTERNAL( testnum, flags, result, val1, val2, val3, code... ) \
  asm volatile("test_%0:" : :"I"(testnum)); \
  asm volatile("li  gp, %0;" : : "I"(testnum)); \
  asm volatile("la  a0, test_%0_data;" : : "I"(testnum)) ; \
  ASM_GEN(flw f0, 0(a0)); \
  ASM_GEN(flw f1, 4(a0)); \
  ASM_GEN(flw f2, 8(a0)); \
  ASM_GEN(lw a3, 12(a0)); \
  code; \
  ASM_GEN(bne a0, a3, fail);

  /*#define TEST_FP_OP_DATA2(testnum, result, val1, val2) \
  ASM_GEN_NV(.pushsection .data); \
  ASM_GEN_NV(.align 2;); \
  ASM_TEST_NUM_GEN(testnum); \
  ASM_GEN_NV(.float val1); \
  ASM_GEN_NV(.float val2); \
  ASM_GEN_NV(.float result); \
  ASM_GEN_NV(.popsection);*/

#define TEST_FP_OP_DATA_BEGIN  \
  ASM_GEN_NV(.pushsection .fp_data, "aw", @progbits); \
  ASM_GEN_NV(.align 2;);

#define TEST_FP_OP_DATA_END \
  ASM_GEN_NV(.popsection);

  #define TEST_FP_OP_DATA1(testnum, result, val1) \
  ASM_TEST_NUM_GEN(testnum); \
  ASM_GEN_NV(.float val1); \
  ASM_GEN_NV(.float result);

  #define TEST_FP_INT_OP_DATA1(testnum, result, val1) \
  ASM_TEST_NUM_GEN(testnum); \
  ASM_GEN_NV(.float result);

  #define TEST_INT_FP_OP_DATA1(testnum, result, val1) \
  ASM_TEST_NUM_GEN(testnum); \
  ASM_GEN_NV(.float val1); \
  ASM_GEN_NV(.dword result);

  #define TEST_FP_OP_DATA2(testnum, result, val1, val2) \
  ASM_TEST_NUM_GEN(testnum); \
  ASM_GEN_NV(.float val1); \
  ASM_GEN_NV(.float val2); \
  ASM_GEN_NV(.float result);

  #define TEST_FP_OP_DATA2_CMP(testnum, result, val1, val2) \
  ASM_TEST_NUM_GEN(testnum); \
  ASM_GEN_NV(.float val1); \
  ASM_GEN_NV(.float val2); \
  ASM_GEN_NV(.dword result);

  #define TEST_FP_OP_DATA3(testnum, result, val1, val2, val3) \
  ASM_TEST_NUM_GEN(testnum); \
  ASM_GEN_NV(.float val1); \
  ASM_GEN_NV(.float val2); \
  ASM_GEN_NV(.float val3); \
  ASM_GEN_NV(.float result);

#define TEST_FP_OP1_S( testnum, inst, flags, result, val1 ) \
  TEST_FP_OP_1S_INTERNAL( testnum, flags, float result, val1, \
                    ASM_GEN(inst f3, f0); \
                    ASM_GEN(fmv.x.s a0, f3));

#define TEST_FP_OP2_S( testnum, inst, flags, result, val1, val2 ) \
  TEST_FP_OP_2S_INTERNAL( testnum, flags, float result, val1, val2, \
                    ASM_GEN(inst f3, f0, f1); \
                    ASM_GEN(fmv.x.s a0, f3));

#define TEST_FP_OP3_S( testnum, inst, flags, result, val1, val2, val3 ) \
  TEST_FP_OP_3S_INTERNAL( testnum, flags, float result, val1, val2, val3, \
                    ASM_GEN(inst f3, f0, f1, f2); \
                    ASM_GEN(fmv.x.s a0, f3));

#define TEST_FP_CMP_OP_S( testnum, inst, flags, result, val1, val2 ) \
  TEST_FP_OP_2S_INTERNAL( testnum, flags, word result, val1, val2, \
                    ASM_GEN(inst a0, f0, f1));

#define TEST_INT_FP_OP_S( testnum, inst, result, val1 ) \
  asm volatile("test_%0:" : :"I"(testnum)); \
  asm volatile("li  gp, %0;" : : "I"(testnum)); \
  asm volatile("la  a0, test_%0_data;" : : "I"(testnum)) ;\
  ASM_GEN(lw  a3, 0(a0)); \
  ASM_GEN(addi  a0, zero, val1); \
  ASM_GEN(inst f0, a0); \
  ASM_GEN(fmv.x.s a0, f0); \
  ASM_GEN(bne a0, a3, fail);

#define TEST_FP_INT_OP_S( testnum, inst, flags, result, val1, rm ) \
  TEST_FP_OP_1S_INTERNAL( testnum, flags, word result, val1,  \
                    ASM_GEN(inst a0, f0, rm));

#define TEST_FCLASS_S(testnum, correct, input) \
  TEST_CASE(testnum, a0, correct, \
        ASM_GEN(li a0, input); \
        ASM_GEN(fmv.s.x fa0, a0); \
        ASM_GEN(fclass.s a0, fa0); )

#define TEST_LD_OP( testnum, inst, result, offset, base ) \
  TEST_CASE( testnum, x14, result, \
    ASM_GEN(li  x15, result);  \
    ASM_GEN(la  x5, base); \
    ASM_GEN(inst x14, offset(x5)); \
  )

#define TEST_ST_OP( testnum, load_inst, store_inst, result, offset, base ) \
    TEST_CASE( testnum, x14, result, \
      ASM_GEN(la  x5, base); \
      ASM_GEN(li  x6, result); \
      ASM_GEN(la  x15, 7f);  \
      ASM_GEN(store_inst x6, offset(x5)); \
      ASM_GEN(load_inst x14, offset(x5)); \
      ASM_GEN(j 8f); \
      asm volatile("7:");    \
      ASM_GEN(mv x14, x6); \
      asm volatile("8:");    \
  )

#define RVTEST_DATA_BEGIN                                               \
        ASM_GEN_NV(.pushsection .tohost, "aw", @progbits);                            \
        ASM_GEN_NV(.align 6; .global tohost; tohost: .dword 0; .size tohost, 8);    \
        ASM_GEN_NV(.align 6; .global fromhost; fromhost: .dword 0; .size fromhost, 8);\
        ASM_GEN_NV(.popsection);                                                    \
        ASM_GEN_NV(.align 4; .global begin_signature; begin_signature:);

#define RVTEST_DATA_END \
       ASM_GEN_NV(.align 4; .global end_signature; end_signature:);

#endif
