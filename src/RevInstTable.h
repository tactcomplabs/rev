//
// _RevInstTable_h_
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_REVINSTTABLE_H_
#define _SST_REVCPU_REVINSTTABLE_H_

#include <bitset>
#include "RevMem.h"
#include "RevFeature.h"

#ifndef _REV_NUM_REGS_
#define _REV_NUM_REGS_ 32
#endif

#ifndef _REV_MAX_FORMAT_
#define _REV_MAX_FORMAT_ 7
#endif

#ifndef _REV_MAX_REGCLASS_
#define _REV_MAX_REGCLASS_ 3
#endif

// Masks
#define MASK8   0b11111111                          // 8bit mask
#define MASK16  0b1111111111111111                  // 16bit mask
#define MASK32  0b11111111111111111111111111111111  // 32bit mask

// Register Decoding Macros
#define DECODE_RD(x)    (((x)>>(7))&(0b11111))
#define DECODE_RS1(x)   (((x)>>(15))&(0b11111))
#define DECODE_RS2(x)   (((x)>>(20))&(0b11111))
#define DECODE_RS3(x)   (((x)>>(27))&(0b11111))
#define DECODE_IMM12(x) (((x)>>(20))&(0b111111111111))
#define DECODE_IMM20(x) (((x)>>(12))&(0b11111111111111111111))

#define DECODE_FUNCT7(x)  (((x)>>(25))&(0b1111111))
#define DECODE_FUNCT2(x)  (((x)>>(25))&(0b11))
#define DECODE_FUNCT3(x)  (((x)>>(12))&(0b111))

#define DECODE_RL(x)    (((x)>>(25))&(0b1))
#define DECODE_AQ(x)    (((x)>>(26))&(0b1))

// RV{32,64}{F,D} macros
#define FCSR_NX(x)  ((x)&(0b1))             // FCSR: NX field
#define FCSR_UF(x)  (((x)&(0b10))>>1)       // FCSR: UF field
#define FCSR_OF(x)  (((x)&(0b100))>>2)      // FCSR: OF field
#define FCSR_DZ(x)  (((x)&(0b1000))>>3)     // FCSR: DZ field
#define FCSR_NV(x)  (((x)&(0b10000))>>4)    // FCSR: NV field
#define FCSR_FRM(x) (((x)&(0b11100000))>>5) // FCSR: FRM field

#define FRM_RNE   0b000                     // Rounding mode: Round to Nearest, ties to Even
#define FRM_RTZ   0b001                     // Rounding mode: Round towards Zero
#define FRM_RDN   0b010                     // Rounding mode: Round Down (towards -INF)
#define FRM_RUP   0b011                     // Rounding mode: Round Up (towards +INF)
#define FRM_RMM   0b100                     // Rounding mode: Round to Nearest, ties to Max Magnitude

// RV{32,64} Register Operation Macros
#define SEXT(r,x,b) do {\
                    (r) = ((r) & (~r));\
                    (r) = ( (x) ^ ((1UL) << ((b) - 1)) ) - ((1UL) << ((b) - 1));\
                    }while(0)                // Sign extend the target register
#define ZEXT(r,x,b) do {\
                    (r) = ((r) & (~r));\
                    (r) = (x) | (((1UL) << (b)) - 1);\
                    }while(0)                // Zero extend the target register

#define SEXTI(r,b)  do {\
                    (r) = ( (r) ^ ((1UL) << ((b) - 1)) ) - ((1UL) << ((b) - 1));\
                    }while(0)                // Sign extend the target register inline
#define ZEXTI(r,b)  do {\
                    (r) = (r) | (((1UL) << (b)) - 1);\
                    }while(0)                // Zero extend the target register inline


/// td_u32: convert u32 in two's complement to decimal
static uint32_t td_u32(uint32_t binary, unsigned bits){
  uint32_t tmp = binary;
  uint32_t i = 0;
  if( (binary & (1<<(bits-1))) > 0 ){
    // sign extend to 32 bits
    for( i=bits; i<32; i++ ){
      tmp |= (1<<i);
    }

    // invert all the bits
    tmp = ~tmp;

    // add 1
    tmp += 1;

    // set the sign bit
    tmp = tmp*-1;
  }
  return tmp;
}

/// td_u64: convert u64 in two's complement to decimal
static uint64_t td_u64(uint64_t binary, unsigned bits){
  uint64_t tmp = binary;
  uint64_t sext = 0x00ull;
  uint64_t i = 0;

  if( (binary & (1<<(bits-1))) > 0 ){
    // sign extend to 64 bits
    for( i=0; i<bits; i++ ){
      sext |= (1<<i);
    }
    sext = ~sext;

    tmp = tmp | sext;

    // invert all the bits
    tmp = ~tmp;

    // add 1
    tmp += 1;

    // set the sign bit
    tmp = tmp*-1;
  }
  return tmp;
}

/// dt_u32: convert u32 in decimal to two's complement
static uint32_t dt_u32(int32_t binary, unsigned bits){
  uint32_t tmp = binary;
  uint32_t i = 0;
  if( (binary & (1<<(bits-1))) > 0 ){
    // sign extend to 32 bits
    for( i=bits; i<32; i++ ){
      tmp |= (1<<i);
    }

    // invert all the bits
    tmp = ~tmp;

    // add 1
    tmp += 1;

    // set the sign bit
    tmp = tmp*-1;
  }
  return tmp;
}

/// td_u64: convert u64 in decimal to two's complement
static uint64_t dt_u64(int64_t binary, unsigned bits){
  uint64_t tmp = binary;
  uint64_t i = 0;
  if( (binary & (1<<(bits-1))) > 0 ){
    // sign extend to 32 bits
    for( i=bits; i<64; i++ ){
      tmp |= (1<<i);
    }

    // invert all the bits
    tmp = ~tmp;

    // add 1
    tmp += 1;

    // set the sign bit
    tmp = tmp*-1;
  }
  return tmp;
}

static uint32_t twos_compl(uint32_t binary, int bits){
  uint32_t tmp = binary;
  uint32_t i = 0;
  //std::cout << "Binary =  " << std::bitset<32>(tmp) << std::endl;
  if( (binary & (1<<(bits-1))) > 0 ){
    // sign extend to 32 bits
    for( i=bits; i<32; i++ ){
      tmp |= (1<<i);
    }

    // invert all the bits
    tmp = ~tmp;
    //std::cout << "Flipped = " << std::bitset<32>(tmp) << std::endl;

    // add 1
    tmp += 1;
    //std::cout << "Added =   " << std::bitset<32>(tmp) << std::endl;

    // set the sign bit
    //tmp |= (1<<31);
    tmp = tmp*-1;
    //std::cout << "Signed =  " << std::bitset<32>(tmp) << std::endl;
  }
  return tmp;
}

namespace SST{
  namespace RevCPU {

    typedef struct{
      uint32_t RV32[_REV_NUM_REGS_];    ///< RevRegFile: RV32I register file
      uint64_t RV64[_REV_NUM_REGS_];    ///< RevRegFile: RV64I register file
      float SPF[_REV_NUM_REGS_];        ///< RevRegFile: RVxxF register file
      double DPF[_REV_NUM_REGS_];       ///< RevRegFile: RVxxD register file

      uint32_t RV32_PC;                 ///< RevRegFile: RV32 PC
      uint64_t RV64_PC;                 ///< RevRegFile: RV64 PC
      uint64_t FCSR;                    ///< RevRegFile: FCSR

      uint32_t cost;                    ///< RevRegFile: Cost of the instruction
      bool trigger;                     ///< RevRegFile: Has the instruction been triggered?
      unsigned Entry;                   ///< RevRegFile: Instruction entry
    }RevRegFile;                        ///< RevProc: register file construct

    typedef enum{
      RVTypeUNKNOWN = 0,  ///< RevInstf: Unknown format
      RVTypeR       = 1,  ///< RevInstF: R-Type
      RVTypeI       = 2,  ///< RevInstF: I-Type
      RVTypeS       = 3,  ///< RevInstF: S-Type
      RVTypeU       = 4,  ///< RevInstF: U-Type
      RVTypeB       = 5,  ///< RevInstF: B-Type
      RVTypeJ       = 6,  ///< RevInstF: J-Type
      RVTypeR4      = 7,  ///< RevInstF: R4-Type for AMOs
    }RevInstF;            ///< Rev CPU Instruction Formats

    typedef enum{
      RegUNKNOWN    = 0,  ///< RevRegClass: Unknown register file
      RegIMM        = 1,  ///< RevRegClass: Treat the reg class like an immediate: S-Format
      RegGPR        = 2,  ///< RevRegClass: GPR reg file
      RegCSR        = 3,  ///< RevRegClass: CSR reg file
      RegFLOAT      = 4   ///< RevRegClass: Float register file
    }RevRegClass;         ///< Rev CPU Register Classes

    typedef enum{
      FUnk          = 0,  ///< RevRegClass: Imm12 is not used
      FImm          = 1,  ///< RevRegClass: Imm12 is an immediate
      FEnc          = 2,  ///< RevRegClass: Imm12 is an encoding value
      FVal          = 3   ///< RevRegClass: Imm12 is an incoming register value
    }RevImmFunc;          ///< Rev Immediate Values

    /*! \struct RevInst
     *  \brief Rev decoded instruction
     *
     * Contains all the details required to execute
     * following a successful crack + decode
     *
     */
    typedef struct{
      uint8_t opcode;       ///< RevInst: opcode
      uint8_t funct3;       ///< RevInst: funct3 value
      uint8_t funct2;       ///< RevInst: funct2 value
      uint8_t funct7;       ///< RevInst: funct7 value
      uint8_t rd;           ///< RevInst: rd value
      uint8_t rs1;          ///< RevInst: rs1 value
      uint8_t rs2;          ///< RevInst: rs2 value
      uint8_t rs3;          ///< RevInst: rs3 value
      uint32_t imm;         ///< RevInst: immediate value
      uint8_t fmt;          ///< RevInst: floating point format
      uint8_t rm;           ///< RevInst: floating point rounding mode
      uint8_t aq;           ///< RevInst: aq field for atomic instructions
      uint8_t rl;           ///< RevInst: rl field for atomic instructions
      size_t instSize;      ///< RevInst: size of the instruction in bytes
    }RevInst;

    /*! \struct RevInstEntry
     *  \brief Rev instruction entry
     *
     * Contains all the details required to decode and execute
     * a target instruction as well as its cost function
     *
     */
    typedef struct{
      // disassembly
      std::string mnemonic; ///< RevInstEntry: instruction mnemonic
      uint32_t cost;        ///< RevInstEntry: instruction code in cycles

      // storage
      uint8_t opcode;       ///< RevInstEntry: opcode
      uint8_t funct3;       ///< RevInstEntry: funct3 value
      uint8_t funct7;       ///< RevInstEntry: funct7 value

      // register encodings
      RevRegClass rdClass;  ///< RevInstEntry: Rd register class
      RevRegClass rs1Class; ///< RevInstEntry: Rs1 register class
      RevRegClass rs2Class; ///< RevInstEntry: Rs2 register class
      RevRegClass rs3Class; ///< RevInstEntry: Rs3 register class

      uint16_t imm12;       ///< RevInstEntry: imm12 value

      RevImmFunc imm;       ///< RevInstEntry: does the imm12 exist?

      // formatting
      RevInstF format;      ///< RevInstEntry: instruction formatA

      /// RevInstEntry: Instruction implementation function
      bool (*func)(RevFeature *, RevRegFile *, RevMem *, RevInst);

    }RevInstEntry;

  } // namespace RevCPU
} // namespace SST

#endif
