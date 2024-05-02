//
// _RevInstTable_h_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
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
#define DECODE_RD( x )         ( ( ( x ) >> ( 7 ) ) & ( 0b11111 ) )
#define DECODE_RS1( x )        ( ( ( x ) >> ( 15 ) ) & ( 0b11111 ) )
#define DECODE_RS2( x )        ( ( ( x ) >> ( 20 ) ) & ( 0b11111 ) )
#define DECODE_RS3( x )        ( ( ( x ) >> ( 27 ) ) & ( 0b11111 ) )
#define DECODE_IMM12( x )      ( ( ( x ) >> ( 20 ) ) & ( 0b111111111111 ) )
#define DECODE_IMM20( x )      ( ( ( x ) >> ( 12 ) ) & ( 0b11111111111111111111 ) )

#define DECODE_LOWER_CRS2( x ) ( ( ( x ) >> ( 2 ) ) & ( 0b11111 ) )

#define DECODE_FUNCT7( x )     ( ( ( x ) >> ( 25 ) ) & ( 0b1111111 ) )
#define DECODE_FUNCT2( x )     ( ( ( x ) >> ( 25 ) ) & ( 0b11 ) )
#define DECODE_FUNCT3( x )     ( ( ( x ) >> ( 12 ) ) & ( 0b111 ) )

#define DECODE_RM( x )         static_cast<FRMode>( DECODE_FUNCT3( x ) )
#define DECODE_RL( x )         ( ( ( x ) >> ( 25 ) ) & ( 0b1 ) )
#define DECODE_AQ( x )         ( ( ( x ) >> ( 26 ) ) & ( 0b1 ) )

namespace SST::RevCPU {

enum RevInstF : int {  ///< Rev CPU Instruction Formats
  RVTypeUNKNOWN = 0,   ///< RevInstf: Unknown format
  RVTypeR       = 1,   ///< RevInstF: R-Type
  RVTypeI       = 2,   ///< RevInstF: I-Type
  RVTypeS       = 3,   ///< RevInstF: S-Type
  RVTypeU       = 4,   ///< RevInstF: U-Type
  RVTypeB       = 5,   ///< RevInstF: B-Type
  RVTypeJ       = 6,   ///< RevInstF: J-Type
  RVTypeR4      = 7,   ///< RevInstF: R4-Type for FMAs
  // -- Compressed Formats
  RVCTypeCR     = 10,  ///< RevInstF: Compressed CR-Type
  RVCTypeCI     = 11,  ///< RevInstF: Compressed CI-Type
  RVCTypeCSS    = 12,  ///< RevInstF: Compressed CSS-Type
  RVCTypeCIW    = 13,  ///< RevInstF: Compressed CIW-Type
  RVCTypeCL     = 14,  ///< RevInstF: Compressed CL-Type
  RVCTypeCS     = 15,  ///< RevInstF: Compressed CS-Type
  RVCTypeCA     = 16,  ///< RevInstF: Compressed CA-Type
  RVCTypeCB     = 17,  ///< RevInstF: Compressed CB-Type
  RVCTypeCJ     = 18,  ///< RevInstF: Compressed CJ-Type
};

enum RevImmFunc : int {  ///< Rev Immediate Values
  FUnk = 0,              ///< RevRegClass: Imm12 is not used
  FImm = 1,              ///< RevRegClass: Imm12 is an immediate
  FEnc = 2,              ///< RevRegClass: Imm12 is an encoding value
  FVal = 3,              ///< RevRegClass: Imm12 is an incoming register value
};

/*! \struct RevInst
 *  \brief Rev decoded instruction
 *
 * Contains all the details required to execute
 * following a successful crack + decode
 *
 */
struct RevInst {
  uint8_t  opcode    = 0;       ///< RevInst: opcode
  uint8_t  funct2    = 0;       ///< RevInst: compressed funct2 value
  uint8_t  funct3    = 0;       ///< RevInst: funct3 value
  uint8_t  funct4    = 0;       ///< RevInst: compressed funct4 value
  uint8_t  funct6    = 0;       ///< RevInst: compressed funct6 value
  uint8_t  funct2or7 = 0;       ///< RevInst: uncompressed funct2 or funct7 value
  uint64_t rd        = ~0;      ///< RevInst: rd value
  uint64_t rs1       = ~0;      ///< RevInst: rs1 value
  uint64_t rs2       = ~0;      ///< RevInst: rs2 value
  uint64_t rs3       = ~0;      ///< RevInst: rs3 value
  uint64_t imm       = 0;       ///< RevInst: immediate value
  bool     raisefpe  = 0;       ///< RevInst: raises FP exceptions
  FRMode   rm{ FRMode::None };  ///< RevInst: floating point rounding mode
  uint8_t  aq           = 0;    ///< RevInst: aq field for atomic instructions
  uint8_t  rl           = 0;    ///< RevInst: rl field for atomic instructions
  uint16_t offset       = 0;    ///< RevInst: compressed offset
  uint16_t jumpTarget   = 0;    ///< RevInst: compressed jumpTarget
  uint8_t  instSize     = 0;    ///< RevInst: size of the instruction in bytes
  bool     compressed   = 0;    ///< RevInst: determines if the instruction is compressed
  uint32_t cost         = 0;    ///< RevInst: the cost to execute this instruction, in clock cycles
  unsigned entry        = 0;    ///< RevInst: Where to find this instruction in the InstTables
  uint16_t hart         = 0;    ///< RevInst: What hart is this inst being executed on
  bool     isCoProcInst = 0;    ///< RevInst: whether instruction is coprocessor instruction

  explicit RevInst()    = default;  // prevent aggregate initialization

  ///< RevInst: Sign-extended immediate value
  constexpr int32_t ImmSignExt( size_t bits ) const { return SignExt( imm, bits ); }
};  // RevInst

/// CRegIdx: Maps the compressed index to normal index
#define CRegIdx( x ) ( ( x ) + 8 )

/*! \struct RevInstEntry
 *  \brief Rev instruction entry
 *
 * Contains all the details required to decode and execute
 * a target instruction as well as its cost function
 *
 */
struct RevInstEntry {
  // disassembly
  std::string mnemonic = "nop";  ///< RevInstEntry: instruction mnemonic
  uint32_t    cost     = 1;      ///< RevInstEntry: instruction code in cycles

  // storage
  uint8_t  opcode      = 0;  ///< RevInstEntry: opcode
  uint8_t  funct2      = 0;  ///< RevInstentry: compressed funct2 value
  uint8_t  funct3      = 0;  ///< RevInstEntry: funct3 value
  uint8_t  funct4      = 0;  ///< RevInstentry: compressed funct4 value
  uint8_t  funct6      = 0;  ///< RevInstentry: compressed funct6 value
  uint8_t  funct2or7   = 0;  ///< RevInstEntry: uncompressed funct2 or funct7 value
  uint16_t offset      = 0;  ///< RevInstEntry: compressed offset value
  uint16_t jumpTarget  = 0;  ///< RevInstEntry: compressed jump target value

  // register encodings
  RevRegClass rdClass  = RevRegClass::RegGPR;      ///< RevInstEntry: Rd register class
  RevRegClass rs1Class = RevRegClass::RegGPR;      ///< RevInstEntry: Rs1 register class
  RevRegClass rs2Class = RevRegClass::RegGPR;      ///< RevInstEntry: Rs2 register class
  RevRegClass rs3Class = RevRegClass::RegUNKNOWN;  ///< RevInstEntry: Rs3 register class
  uint16_t    imm12    = 0;                        ///< RevInstEntry: imm12 value
  RevImmFunc  imm      = FUnk;                     ///< RevInstEntry: does the imm12 exist?

  // formatting
  RevInstF format      = RVTypeR;  ///< RevInstEntry: instruction format
  bool     compressed  = false;    ///< RevInstEntry: compressed instruction
  uint8_t  fpcvtOp     = 0;        ///<RevInstEntry: Stores the rs2 field in R-instructions
  bool     raisefpe    = false;    ///<RevInstEntry: Whether FP exceptions are raised

  /// Instruction implementation function
  bool ( *func )( RevFeature*, RevRegFile*, RevMem*, const RevInst& ) = nullptr;

  /// Predicate for enabling table entries for only certain encodings
  bool ( *predicate )( uint32_t Inst )                                = []( uint32_t ) { return true; };

  // Begin Set() functions to allow call chaining - all Set() must return *this
  // clang-format off
  auto& SetMnemonic(std::string m)   { this->mnemonic   = std::move(m); return *this; }
  auto& SetCost(uint32_t c)          { this->cost       = c;     return *this; }
  auto& SetOpcode(uint8_t op)        { this->opcode     = op;    return *this; }
  auto& SetFunct2(uint8_t f2)        { this->funct2     = f2;    return *this; }
  auto& SetFunct3(uint8_t f3)        { this->funct3     = f3;    return *this; }
  auto& SetFunct4(uint8_t f4)        { this->funct4     = f4;    return *this; }
  auto& SetFunct6(uint8_t f6)        { this->funct6     = f6;    return *this; }
  auto& SetFunct2or7(uint8_t f27)    { this->funct2or7  = f27;   return *this; }
  auto& SetOffset(uint16_t off)      { this->offset     = off;   return *this; }
  auto& SetJumpTarget(uint16_t jt)   { this->jumpTarget = jt;    return *this; }
  auto& SetrdClass(RevRegClass rd)   { this->rdClass    = rd;    return *this; }
  auto& Setrs1Class(RevRegClass rs1) { this->rs1Class   = rs1;   return *this; }
  auto& Setrs2Class(RevRegClass rs2) { this->rs2Class   = rs2;   return *this; }
  auto& Setrs3Class(RevRegClass rs3) { this->rs3Class   = rs3;   return *this; }
  auto& Setimm12(uint16_t imm12)     { this->imm12      = imm12; return *this; }
  auto& Setimm(RevImmFunc imm)       { this->imm        = imm;   return *this; }
  auto& SetFormat(RevInstF format)   { this->format     = format;return *this; }
  auto& SetCompressed(bool c)        { this->compressed = c;     return *this; }
  auto& SetfpcvtOp(uint8_t op)       { this->fpcvtOp    = op;    return *this; }
  auto& SetRaiseFPE(bool c)          { this->raisefpe   = c;     return *this; }
  auto& SetImplFunc( bool func( RevFeature *, RevRegFile *, RevMem *, const RevInst& ) )
                                     { this->func       = func;  return *this; }
  auto& SetPredicate( bool pred( uint32_t ) )
                                     { this->predicate  = pred;  return *this; }

  // clang-format on
};  // RevInstEntry

// The default initialization for RevInstDefaults is the same as RevInstEntry
using RevInstDefaults = RevInstEntry;

// Compressed instruction defaults
struct RevCInstDefaults : RevInstDefaults {
  RevCInstDefaults() { SetCompressed( true ); }
};

}  // namespace SST::RevCPU

#endif
