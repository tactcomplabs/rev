//
// _RevInstTable_h_
//
// Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_REVINSTTABLE_H_
#define _SST_REVCPU_REVINSTTABLE_H_

#include "RevCommon.h"
#include "RevFCSR.h"
#include <bitset>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <map>
#include <memory>
#include <string>
#include <type_traits>

namespace SST::RevCPU {

// Register Decoding functions
// clang-format off
constexpr auto DECODE_RD        ( uint32_t Inst ) { return BitExtract< 7,  5>( Inst ); }
constexpr auto DECODE_RS1       ( uint32_t Inst ) { return BitExtract<15,  5>( Inst ); }
constexpr auto DECODE_RS2       ( uint32_t Inst ) { return BitExtract<20,  5>( Inst ); }
constexpr auto DECODE_RS3       ( uint32_t Inst ) { return BitExtract<27,  5>( Inst ); }
constexpr auto DECODE_IMM12     ( uint32_t Inst ) { return BitExtract<20, 12>( Inst ); }
constexpr auto DECODE_IMM20     ( uint32_t Inst ) { return BitExtract<12, 20>( Inst ); }
constexpr auto DECODE_LOWER_CRS2( uint32_t Inst ) { return BitExtract< 2,  5>( Inst ); }
constexpr auto DECODE_FUNCT7    ( uint32_t Inst ) { return BitExtract<25,  7>( Inst ); }
constexpr auto DECODE_FUNCT2    ( uint32_t Inst ) { return BitExtract<25,  2>( Inst ); }
constexpr auto DECODE_FUNCT3    ( uint32_t Inst ) { return BitExtract<12,  3>( Inst ); }
constexpr auto DECODE_RL        ( uint32_t Inst ) { return BitExtract<25,  1>( Inst ); }
constexpr auto DECODE_AQ        ( uint32_t Inst ) { return BitExtract<26,  1>( Inst ); }
constexpr auto DECODE_RM        ( uint32_t Inst ) { return FRMode{ BitExtract<12, 3>( Inst ) }; }

// clang-format on

enum RevInstF : int {  ///< Rev CPU Instruction Formats
  RVTypeUNKNOWN = 0,   ///< RevInstF: Unknown format
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
  // -- Vector Formats
  RVVTypeOpv    = 32,  ///< RevInstF: OPV
  RVVTypeLdSt   = 33,  ///< RevInstF: LOAD-FP/STORE-FP
};

enum class RevImmFunc {  ///< Rev Immediate Values
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
class RevInst {
public:
  uint8_t  opcode    = 0;            ///< RevInst: opcode
  uint8_t  funct2    = 0;            ///< RevInst: compressed funct2 value
  uint8_t  funct3    = 0;            ///< RevInst: funct3 value
  uint8_t  funct4    = 0;            ///< RevInst: compressed funct4 value
  uint8_t  funct6    = 0;            ///< RevInst: compressed funct6 value
  uint8_t  funct2or7 = 0;            ///< RevInst: uncompressed funct2 or funct7 value
  uint32_t rd        = ~uint32_t{};  ///< RevInst: rd value
  uint32_t rs1       = ~uint32_t{};  ///< RevInst: rs1 value
  uint32_t rs2       = ~uint32_t{};  ///< RevInst: rs2 value
  uint32_t rs3       = ~uint32_t{};  ///< RevInst: rs3 value
  uint32_t imm       = 0;            ///< RevInst: immediate value
  bool     raisefpe  = 0;            ///< RevInst: raises FP exceptions
  FRMode   rm{ FRMode::None };       ///< RevInst: floating point rounding mode
  bool     aq           = false;     ///< RevInst: aqr field for atomic instructions
  bool     rl           = false;     ///< RevInst: rel field for atomic instructions
  uint16_t offset       = 0;         ///< RevInst: compressed offset
  uint16_t jumpTarget   = 0;         ///< RevInst: compressed jumpTarget
  uint8_t  instSize     = 0;         ///< RevInst: size of the instruction in bytes
  bool     compressed   = 0;         ///< RevInst: determines if the instruction is compressed
  uint32_t cost         = 0;         ///< RevInst: the cost to execute this instruction, in clock cycles
  uint32_t entry        = 0;         ///< RevInst: Where to find this instruction in the InstTables
  uint16_t hart         = 0;         ///< RevInst: What hart is this inst being executed on
  bool     isCoProcInst = 0;         ///< RevInst: whether instruction is coprocessor instruction

  explicit RevInst()    = default;  // prevent aggregate initialization

  ///< RevInst: Sign-extended immediate value
  constexpr auto ImmSignExt( int bits ) const { return SignExt( imm, bits ); }
};  // RevInst

/// CRegIdx: Maps the compressed index to normal index
template<typename T>
constexpr auto CRegIdx( T x ) {
  return x + 8;
}

class RevFeature;
class RevRegFile;
class RevMem;

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
  RevImmFunc  imm      = RevImmFunc::FUnk;         ///< RevInstEntry: does the imm12 exist?

  // formatting
  RevInstF format      = RVTypeR;  ///< RevInstEntry: instruction format
  bool     compressed  = false;    ///< RevInstEntry: compressed instruction
  uint8_t  rs2fcvtOp   = 0;        ///<RevInstEntry: Stores the rs2 field in R-instructions
  bool     raisefpe    = false;    ///<RevInstEntry: Whether FP exceptions are raised

  /// Instruction implementation function
  bool ( *func )( const RevFeature*, RevRegFile*, RevMem*, const RevInst& ){};

  /// Predicate for enabling table entries for only certain encodings
  bool ( *predicate )( uint32_t Inst ) = []( uint32_t ) { return true; };

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
  auto& Setrs2fcvtOp(uint8_t op)     { this->rs2fcvtOp  = op;    return *this; }
  auto& SetRaiseFPE(bool c)          { this->raisefpe   = c;     return *this; }
  auto& SetImplFunc( bool func( const RevFeature *, RevRegFile *, RevMem *, const RevInst& ) )
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
