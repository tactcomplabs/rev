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
#include <map>
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

#ifndef _REV_THREAD_COUNT
#define _REV_THREAD_COUNT_ 1
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

#define DECODE_LOWER_CRS2(x) (((x)>>(2))&(0b11111))

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
                    //(r) = ((r) & (~r));
#define SEXT(r,x,b) do {\
                    (r) = ( (x) ^ ((1UL) << ((b) - 1)) ) - ((1UL) << ((b) - 1));\
                    }while(0)                // Sign extend the target register
#define ZEXT(r,x,b) do {\
                    (r) = (x) | (((1UL) << (b)) - 1);\
                    }while(0)                // Zero extend the target register

#define SEXTI(r,b)  do {\
                    (r) = ( (r) ^ ((1UL) << ((b) - 1)) ) - ((1UL) << ((b) - 1));\
                    }while(0)                // Sign extend the target register inline
#define ZEXTI(r,b)  do {\
                    (r) = (r) | (((1UL) << (b)) - 1);\
                    }while(0)                // Zero extend the target register inline

// Swizzle Macro
#define SWIZZLE(q,in,start,dest ) q |= ((in >> start) & 1) << dest;

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

    static std::bitset<_REV_THREAD_COUNT_>  THREAD_CTS;

    typedef enum{
      RVTypeUNKNOWN = 0,  ///< RevInstf: Unknown format
      RVTypeR       = 1,  ///< RevInstF: R-Type
      RVTypeI       = 2,  ///< RevInstF: I-Type
      RVTypeS       = 3,  ///< RevInstF: S-Type
      RVTypeU       = 4,  ///< RevInstF: U-Type
      RVTypeB       = 5,  ///< RevInstF: B-Type
      RVTypeJ       = 6,  ///< RevInstF: J-Type
      RVTypeR4      = 7,  ///< RevInstF: R4-Type for AMOs
      // -- Compressed Formats
      RVCTypeCR     = 10, ///< RevInstF: Compressed CR-Type
      RVCTypeCI     = 11, ///< RevInstF: Compressed CI-Type
      RVCTypeCSS    = 12, ///< RevInstF: Compressed CSS-Type
      RVCTypeCIW    = 13, ///< RevInstF: Compressed CIW-Type
      RVCTypeCL     = 14, ///< RevInstF: Compressed CL-Type
      RVCTypeCS     = 15, ///< RevInstF: Compressed CS-Type
      RVCTypeCA     = 16, ///< RevInstF: Compressed CA-Type
      RVCTypeCB     = 17, ///< RevInstF: Compressed CB-Type
      RVCTypeCJ     = 18  ///< RevInstF: Compressed CJ-Type
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
      uint8_t funct2;       ///< RevInst: compressed funct2 value
      uint8_t funct3;       ///< RevInst: funct3 value
      uint8_t funct4;       ///< RevInst: compressed funct4 value
      uint8_t funct6;       ///< RevInst: compressed funct6 value
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
      uint16_t offset;      ///< RevInst: compressed offset
      uint16_t jumpTarget;  ///< RevInst: compressed jumpTarget
      size_t instSize;      ///< RevInst: size of the instruction in bytes
      bool compressed;      ///< RevInst: determines if the instruction is compressed
    }RevInst;

    /// RevInstEntry: Holds the compressed index to normal index mapping
    static std::map<uint8_t,uint8_t> CRegMap =
    {
      {0b000,8},
      {0b001,9},
      {0b010,10},
      {0b011,11},
      {0b100,12},
      {0b101,13},
      {0b110,14},
      {0b111,15}
    };

    /*! \struct RevInstEntry
     *  \brief Rev instruction entry
     *
     * Contains all the details required to decode and execute
     * a target instruction as well as its cost function
     *
     */
    class RevInstDefaults {
      public:
      uint8_t     opcode;
      uint32_t    cost;
      uint8_t     funct2;     // compressed only
      uint8_t     funct3;
      uint8_t     funct4;     // compressed only
      uint8_t     funct6;     // compressed only
      uint8_t     funct7;
      uint16_t    offset;     // compressed only
      uint16_t    jumpTarget; // compressed only
      RevRegClass rdClass;
      RevRegClass rs1Class;
      RevRegClass rs2Class;
      RevRegClass rs3Class;
      uint16_t    imm12;
      RevImmFunc  imm;
      RevInstF    format;
      bool        compressed;

      RevInstDefaults(){
        opcode    = 0b00000000;
        cost      = 1;
        funct2    = 0b000;      // compressed only
        funct3    = 0b000;
        funct4    = 0b000;      // compressed only
        funct6    = 0b000;      // compressed only
        funct7    = 0b0000000;
        offset    = 0b0000000;  // compressed only
        jumpTarget= 0b0000000;  // compressed only
        rdClass   = RegGPR;
        rs1Class  = RegGPR;
        rs2Class  = RegGPR;
        rs3Class  = RegUNKNOWN;
        imm12     = 0b000000000000;
        imm       = FUnk;
        format    = RVTypeR;
        compressed = false;
      }
    };

    typedef struct {
        // disassembly
        std::string mnemonic; ///< RevInstEntry: instruction mnemonic
        uint32_t cost;        ///< RevInstEntry: instruction code in cycles

        // storage
        uint8_t opcode;       ///< RevInstEntry: opcode
        uint8_t funct2;       ///< RevInstentry: compressed funct2 value
        uint8_t funct3;       ///< RevInstEntry: funct3 value
        uint8_t funct4;       ///< RevInstentry: compressed funct4 value
        uint8_t funct6;       ///< RevInstentry: compressed funct6 value
        uint8_t funct7;       ///< RevInstEntry: funct7 value
        uint16_t offset;      ///< RevInstEntry: compressed offset value
        uint16_t jumpTarget;  ///< RevInstEntry: compressed jump target value

        // register encodings
        RevRegClass rdClass;  ///< RevInstEntry: Rd register class
        RevRegClass rs1Class; ///< RevInstEntry: Rs1 register class
        RevRegClass rs2Class; ///< RevInstEntry: Rs2 register class
        RevRegClass rs3Class; ///< RevInstEntry: Rs3 register class

        uint16_t imm12;       ///< RevInstEntry: imm12 value

        RevImmFunc imm;       ///< RevInstEntry: does the imm12 exist?

        // formatting
        RevInstF format;      ///< RevInstEntry: instruction format

        /// RevInstEntry: Instruction implementation function
        bool (*func)(RevFeature *, RevRegFile *, RevMem *, RevInst);

        bool compressed;      ///< RevInstEntry: compressed instruction
      } RevInstEntry;


    template <typename RevInstDefaultsPolicy>
    class RevInstEntryBuilder : public RevInstDefaultsPolicy{
      public:

      RevInstEntry InstEntry;

      RevInstEntryBuilder() : RevInstDefaultsPolicy() {
        //Set default values
        InstEntry.mnemonic  = std::string("nop");
        InstEntry.func      = NULL;
        InstEntry.opcode    = RevInstDefaultsPolicy::opcode;
        InstEntry.cost      = RevInstDefaultsPolicy::cost;
        InstEntry.funct2    = RevInstDefaultsPolicy::funct2;
        InstEntry.funct3    = RevInstDefaultsPolicy::funct3;
        InstEntry.funct4    = RevInstDefaultsPolicy::funct4;
        InstEntry.funct6    = RevInstDefaultsPolicy::funct6;
        InstEntry.funct7    = RevInstDefaultsPolicy::funct7;
        InstEntry.offset    = RevInstDefaultsPolicy::offset;
        InstEntry.jumpTarget= RevInstDefaultsPolicy::jumpTarget;
        InstEntry.rdClass   = RevInstDefaultsPolicy::rdClass;
        InstEntry.rs1Class  = RevInstDefaultsPolicy::rs1Class;
        InstEntry.rs2Class  = RevInstDefaultsPolicy::rs2Class;
        InstEntry.rs3Class  = RevInstDefaultsPolicy::rs3Class;
        InstEntry.imm12     = RevInstDefaultsPolicy::imm12;
        InstEntry.imm       = RevInstDefaultsPolicy::imm;
        InstEntry.format    = RevInstDefaultsPolicy::format;
        InstEntry.compressed= false;
      }

      // Begin Set() functions to allow call chaining - all Set() must return *this
      RevInstEntryBuilder& SetMnemonic(std::string m)   { InstEntry.mnemonic = m;   return *this;};
      RevInstEntryBuilder& SetCost(uint32_t c)          { InstEntry.cost = c;       return *this;};
      RevInstEntryBuilder& SetOpcode(uint8_t op)        { InstEntry.opcode = op;    return *this;};
      RevInstEntryBuilder& SetFunct2(uint8_t f2)        { InstEntry.funct2 = f2;    return *this;};
      RevInstEntryBuilder& SetFunct3(uint8_t f3)        { InstEntry.funct3 = f3;    return *this;};
      RevInstEntryBuilder& SetFunct4(uint8_t f4)        { InstEntry.funct4 = f4;    return *this;};
      RevInstEntryBuilder& SetFunct6(uint8_t f6)        { InstEntry.funct6 = f6;    return *this;};
      RevInstEntryBuilder& SetFunct7(uint8_t f7)        { InstEntry.funct7 = f7;    return *this;};
      RevInstEntryBuilder& SetOffset(uint16_t off)      { InstEntry.offset = off;   return *this;};
      RevInstEntryBuilder& SetJumpTarget(uint16_t jt)   { InstEntry.jumpTarget = jt;return *this;};
      RevInstEntryBuilder& SetrdClass(RevRegClass rd)   { InstEntry.rdClass = rd;   return *this;};
      RevInstEntryBuilder& Setrs1Class(RevRegClass rs1) {InstEntry.rs1Class = rs1;  return *this;};
      RevInstEntryBuilder& Setrs2Class(RevRegClass rs2) {InstEntry.rs2Class = rs2;  return *this;};
      RevInstEntryBuilder& Setrs3Class(RevRegClass rs3) {InstEntry.rs3Class = rs3;  return *this;};
      RevInstEntryBuilder& Setimm12(uint16_t imm12)     {InstEntry.imm12 = imm12;   return *this;};
      RevInstEntryBuilder& Setimm(RevImmFunc imm)       {InstEntry.imm = imm;       return *this;};
      RevInstEntryBuilder& SetFormat(RevInstF format)   {InstEntry.format = format; return *this;};
      RevInstEntryBuilder& SetCompressed(bool c)        {InstEntry.compressed = c; return *this;};

      RevInstEntryBuilder& SetImplFunc(bool (*func)(RevFeature *,
                                                    RevRegFile *,
                                                    RevMem *,
                                                    RevInst)){
        InstEntry.func = func; return *this;};

    };// class RevInstEntryBuilder;

  } // namespace RevCPU
} // namespace SST

#endif
