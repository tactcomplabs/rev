//
// _RevInstTable_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_REVINSTTABLE_H_
#define _SST_REVCPU_REVINSTTABLE_H_

#include <bitset>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <map>
#include <memory>
#include <string>
#include <type_traits>

#include "RevRegFile.h"

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

#define FRM_RNE   0b000                     // Rounding mode: Round to Nearest, ties to Even
#define FRM_RTZ   0b001                     // Rounding mode: Round towards Zero
#define FRM_RDN   0b010                     // Rounding mode: Round Down (towards -INF)
#define FRM_RUP   0b011                     // Rounding mode: Round Up (towards +INF)
#define FRM_RMM   0b100                     // Rounding mode: Round to Nearest, ties to Max Magnitude

namespace SST::RevCPU{

/* Ref: RISC-V Priviledged Spec (pg. 39) */
enum EXCEPTION_CAUSE : uint32_t {
  MISALIGNED_INST_ADDR      = 0,
  INST_ACCESS_FAULT         = 1,
  ILLEGAL_INST              = 2,
  BREAKPOINT                = 3,
  LOAD_ADDR_MISALIGNED      = 4,
  LOAD_ACCESS_FAULT         = 5,
  STORE_AMO_ADDR_MISALIGNED = 6,
  STORE_AMO_ACCESS_FAULT    = 7,
  ECALL_USER_MODE           = 8,
  ECALL_SUPERVISOR_MODE     = 9,
  ECALL_MACHINE_MODE        = 11,
  INST_PAGE_FAULT           = 12,
  LOAD_PAGE_FAULT           = 13,
  STORE_AMO_PAGE_FAULT      = 15,
};

/// Floating-Point Rounding Mode
enum class RndMode : uint8_t {
  RNE = 0,   // Round to Nearest, ties to Even
  RTZ = 1,   // Round towards Zero
  RDN = 2,   // Round Down (towards -Inf)
  RUP = 3,   // Round Up (towards +Inf)
  RMM = 4,   // Round to Nearest, ties to Max Magnitude
  DYN = 7,   // In instruction's rm field, selects dynamic rounding mode; invalid in FCSR
};


enum RevInstF : int {    ///< Rev CPU Instruction Formats
  RVTypeUNKNOWN = 0,     ///< RevInstf: Unknown format
  RVTypeR       = 1,     ///< RevInstF: R-Type
  RVTypeI       = 2,     ///< RevInstF: I-Type
  RVTypeS       = 3,     ///< RevInstF: S-Type
  RVTypeU       = 4,     ///< RevInstF: U-Type
  RVTypeB       = 5,     ///< RevInstF: B-Type
  RVTypeJ       = 6,     ///< RevInstF: J-Type
  RVTypeR4      = 7,     ///< RevInstF: R4-Type for AMOs
  // -- Compressed Formats
  RVCTypeCR     = 10,    ///< RevInstF: Compressed CR-Type
  RVCTypeCI     = 11,    ///< RevInstF: Compressed CI-Type
  RVCTypeCSS    = 12,    ///< RevInstF: Compressed CSS-Type
  RVCTypeCIW    = 13,    ///< RevInstF: Compressed CIW-Type
  RVCTypeCL     = 14,    ///< RevInstF: Compressed CL-Type
  RVCTypeCS     = 15,    ///< RevInstF: Compressed CS-Type
  RVCTypeCA     = 16,    ///< RevInstF: Compressed CA-Type
  RVCTypeCB     = 17,    ///< RevInstF: Compressed CB-Type
  RVCTypeCJ     = 18,    ///< RevInstF: Compressed CJ-Type
};

enum RevImmFunc : int {  ///< Rev Immediate Values
  FUnk          = 0,     ///< RevRegClass: Imm12 is not used
  FImm          = 1,     ///< RevRegClass: Imm12 is an immediate
  FEnc          = 2,     ///< RevRegClass: Imm12 is an encoding value
  FVal          = 3,     ///< RevRegClass: Imm12 is an incoming register value
};

/*! \struct RevInst
 *  \brief Rev decoded instruction
 *
 * Contains all the details required to execute
 * following a successful crack + decode
 *
 */
struct RevInst {
  uint8_t opcode      = 0; ///< RevInst: opcode
  uint8_t funct2      = 0; ///< RevInst: compressed funct2 value
  uint8_t funct3      = 0; ///< RevInst: funct3 value
  uint8_t funct4      = 0; ///< RevInst: compressed funct4 value
  uint8_t funct6      = 0; ///< RevInst: compressed funct6 value
  uint8_t funct7      = 0; ///< RevInst: funct7 value
  uint64_t rd         =~0; ///< RevInst: rd value
  uint64_t rs1        =~0; ///< RevInst: rs1 value
  uint64_t rs2        =~0; ///< RevInst: rs2 value
  uint64_t rs3        =~0; ///< RevInst: rs3 value
  uint64_t imm        = 0; ///< RevInst: immediate value
  uint8_t fmt         = 0; ///< RevInst: floating point format
  uint8_t rm          = 0; ///< RevInst: floating point rounding mode
  uint8_t aq          = 0; ///< RevInst: aq field for atomic instructions
  uint8_t rl          = 0; ///< RevInst: rl field for atomic instructions
  uint16_t offset     = 0; ///< RevInst: compressed offset
  uint16_t jumpTarget = 0; ///< RevInst: compressed jumpTarget
  uint8_t instSize    = 0; ///< RevInst: size of the instruction in bytes
  bool compressed     = 0; ///< RevInst: determines if the instruction is compressed
  uint32_t cost       = 0; ///< RevInst: the cost to execute this instruction, in clock cycles
  unsigned entry      = 0; ///< RevInst: Where to find this instruction in the InstTables
  uint16_t hart       = 0;  ///< RevInst: What hart is this inst being executed on

  explicit RevInst() = default; // prevent aggregate initialization

  ///< RevInst: Sign-extended immediate value
  constexpr int32_t ImmSignExt(size_t bits) const {
    return SignExt(imm, bits);
  }
}; // RevInst

#if 0

/// CRegMap: Holds the compressed index to normal index mapping
// TODO: Replace with macro below if mappings are trivial
inline const std::map<uint8_t, uint8_t> CRegMap =
{
  {0b000,  8},
  {0b001,  9},
  {0b010, 10},
  {0b011, 11},
  {0b100, 12},
  {0b101, 13},
  {0b110, 14},
  {0b111, 15},
};

/// CRegIdx: Maps the compressed index to normal index
#define CRegIdx(x) (CRegMap.at(x))

#else

/// CRegIdx: Maps the compressed index to normal index
#define CRegIdx(x) ((x) + 8)

#endif

struct RevInstDefaults {
  static constexpr uint8_t     opcode      = 0b00000000;
  static constexpr uint32_t    cost        = 1;
  static constexpr uint8_t     funct2      = 0b000;      // compressed only
  static constexpr uint8_t     funct3      = 0b000;
  static constexpr uint8_t     funct4      = 0b000;      // compressed only
  static constexpr uint8_t     funct6      = 0b000;      // compressed only
  static constexpr uint8_t     funct7      = 0b0000000;
  static constexpr uint16_t    offset      = 0b0000000;  // compressed only
  static constexpr uint16_t    jumpTarget  = 0b0000000;  // compressed only
  static constexpr RevRegClass rdClass     = RevRegClass::RegGPR;
  static constexpr RevRegClass rs1Class    = RevRegClass::RegGPR;
  static constexpr RevRegClass rs2Class    = RevRegClass::RegGPR;
  static constexpr RevRegClass rs3Class    = RevRegClass::RegUNKNOWN;
  static constexpr uint16_t    imm12       = 0b000000000000;
  static constexpr RevImmFunc  imm         = FUnk;
  static constexpr RevInstF    format      = RVTypeR;
  static constexpr bool        compressed  = false;
  static constexpr uint8_t     fpcvtOp     = 0b00000;    // overloaded rs2 field for R-type FP instructions
}; // RevInstDefaults

/*! \struct RevInstEntry
 *  \brief Rev instruction entry
 *
 * Contains all the details required to decode and execute
 * a target instruction as well as its cost function
 *
 */
struct RevInstEntry{
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

  uint8_t fpcvtOp;   ///<RenInstEntry: Stores the overloaded rs2 field in R-type instructions
}; // RevInstEntry

template <typename RevInstDefaultsPolicy>
struct RevInstEntryBuilder : RevInstDefaultsPolicy{
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
    InstEntry.fpcvtOp   = RevInstDefaultsPolicy::fpcvtOp;
  }

  // Begin Set() functions to allow call chaining - all Set() must return *this
  auto& SetMnemonic(std::string m)   { InstEntry.mnemonic = m;   return *this;}
  auto& SetCost(uint32_t c)          { InstEntry.cost = c;       return *this;}
  auto& SetOpcode(uint8_t op)        { InstEntry.opcode = op;    return *this;}
  auto& SetFunct2(uint8_t f2)        { InstEntry.funct2 = f2;    return *this;}
  auto& SetFunct3(uint8_t f3)        { InstEntry.funct3 = f3;    return *this;}
  auto& SetFunct4(uint8_t f4)        { InstEntry.funct4 = f4;    return *this;}
  auto& SetFunct6(uint8_t f6)        { InstEntry.funct6 = f6;    return *this;}
  auto& SetFunct7(uint8_t f7)        { InstEntry.funct7 = f7;    return *this;}
  auto& SetOffset(uint16_t off)      { InstEntry.offset = off;   return *this;}
  auto& SetJumpTarget(uint16_t jt)   { InstEntry.jumpTarget = jt;return *this;}
  auto& SetrdClass(RevRegClass rd)   { InstEntry.rdClass = rd;   return *this;}
  auto& Setrs1Class(RevRegClass rs1) { InstEntry.rs1Class = rs1; return *this;}
  auto& Setrs2Class(RevRegClass rs2) { InstEntry.rs2Class = rs2; return *this;}
  auto& Setrs3Class(RevRegClass rs3) { InstEntry.rs3Class = rs3; return *this;}
  auto& Setimm12(uint16_t imm12)     { InstEntry.imm12 = imm12;  return *this;}
  auto& Setimm(RevImmFunc imm)       { InstEntry.imm = imm;      return *this;}
  auto& SetFormat(RevInstF format)   { InstEntry.format = format;return *this;}
  auto& SetCompressed(bool c)        { InstEntry.compressed = c; return *this;}
  auto& SetfpcvtOp(uint8_t op)       { InstEntry.fpcvtOp = op;   return *this;}

  auto& SetImplFunc(bool func(RevFeature *, RevRegFile *, RevMem *, RevInst)){
    InstEntry.func = func;
    return *this;
  }

}; // class RevInstEntryBuilder;

} // namespace SST::RevCPU

#endif
