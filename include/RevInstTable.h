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

#include "RevMem.h"
#include "RevFeature.h"
#include "../common/include/RevCommon.h"

#ifndef _REV_NUM_REGS_
#define _REV_NUM_REGS_ 32
#endif

#ifndef _REV_MAX_FORMAT_
#define _REV_MAX_FORMAT_ 7
#endif

#ifndef _REV_MAX_REGCLASS_
#define _REV_MAX_REGCLASS_ 3
#endif

#ifndef _REV_HART_COUNT_
#define _REV_HART_COUNT_ 1
#endif



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

//< Zero-extend value of bits size
template<typename T>
constexpr auto ZeroExt(T val, size_t bits){
  return static_cast<std::make_unsigned_t<T>>(val) & ~(~std::make_unsigned_t<T>{0} << bits);
}

//< Sign-extend value of bits size
template<typename T>
constexpr auto SignExt(T val, size_t bits){
  auto signbit = std::make_unsigned_t<T>{1} << (bits-1);
  return static_cast<std::make_signed_t<T>>((ZeroExt(val, bits) ^ signbit) - signbit);
}

/// BoxNaN: Store a boxed float inside a double
inline void BoxNaN(double* dest, const void* value){
  uint32_t i32;
  memcpy(&i32, value, sizeof(float));                // The FP32 value
  uint64_t i64 = uint64_t{i32} | ~uint64_t{0} << 32; // Boxed NaN value
  memcpy(dest, &i64, sizeof(double));                // Store in FP64 register
}

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

struct RevRegFile {
  uint32_t RV32[_REV_NUM_REGS_]{};    ///< RevRegFile: RV32I register file
  uint64_t RV64[_REV_NUM_REGS_]{};    ///< RevRegFile: RV64I register file
  float SPF[_REV_NUM_REGS_]{};        ///< RevRegFile: RVxxF register file
  double DPF[_REV_NUM_REGS_]{};       ///< RevRegFile: RVxxD register file

  /* Supervisor Mode CSRs */
  uint64_t RV64_SSTATUS{}; // During ecall, previous priviledge mode is saved in this register (Incomplete)
  uint64_t RV64_SEPC{};    // Holds address of instruction that caused the exception (ie. ECALL)
  uint64_t RV64_SCAUSE{};  // Used to store cause of exception (ie. ECALL_USER_EXCEPTION)
  uint64_t RV64_STVAL{};   // Used to store additional info about exception (ECALL does not use this and sets value to 0)
  uint64_t RV64_STVEC{};   // Holds the base address of the exception handling routine (trap handler) that the processor jumps to when and exception occurs

  uint32_t RV32_SSTATUS{};
  uint32_t RV32_SEPC{};
  uint32_t RV32_SCAUSE{};
  uint32_t RV32_STVAL{};
  uint32_t RV32_STVEC{};

  bool RV32_Scoreboard[_REV_NUM_REGS_]{}; ///< RevRegFile: Scoreboard for RV32I RF to manage pipeline hazard
  bool RV64_Scoreboard[_REV_NUM_REGS_]{}; ///< RevRegFile: Scoreboard for RV64I RF to manage pipeline hazard
  bool SPF_Scoreboard[_REV_NUM_REGS_]{};  ///< RevRegFile: Scoreboard for SPF RF to manage pipeline hazard
  bool DPF_Scoreboard[_REV_NUM_REGS_]{};  ///< RevRegFile: Scoreboard for DPF RF to manage pipeline hazard

  std::shared_ptr<std::unordered_map<uint64_t, MemReq>> LSQueue{};
  std::function<void(MemReq)> MarkLoadComplete{};

  uint32_t RV32_PC{};                 ///< RevRegFile: RV32 PC
  uint64_t RV64_PC{};                 ///< RevRegFile: RV64 PC
  uint64_t FCSR{};                    ///< RevRegFile: FCSR

  uint32_t cost{};                    ///< RevRegFile: Cost of the instruction
  bool trigger{};                     ///< RevRegFile: Has the instruction been triggered?
  unsigned Entry{};                   ///< RevRegFile: Instruction entry

  // Ensure that all default/copy/move constructors and copy/move assignment are available
  RevRegFile() = default;
  RevRegFile(const RevRegFile&) = default;
  RevRegFile(RevRegFile&&) = default;
  RevRegFile& operator=(const RevRegFile&) = default;
  RevRegFile& operator=(RevRegFile&&) = default;

  /// GetX: Get the specifed X register cast to a specific integral type
  template<typename T>
  T GetX(const RevFeature* F, size_t rs) const {
    if( F->IsRV32() ){
      return rs ? T(RV32[rs]) : T(0);
    }else{
      return rs ? T(RV64[rs]) : T(0);
    }
  }

  /// SetX: Set the specifed X register to a specific value
  template<typename T>
  void SetX(const RevFeature* F, size_t rd, T val) {
    if( F->IsRV32() ){
      RV32[rd] = rd ? uint32_t(val) : uint32_t{0};
    }else{
      RV64[rd] = rd ? uint64_t(val) : uint64_t{0};
    }
  }

  /// GetPC: Get the Program Counter
  uint64_t GetPC(const RevFeature* F) const {
    if( F->IsRV32() ){
      return RV32_PC;
    }else{
      return RV64_PC;
    }
  }

  /// SetPC: Set the Program Counter to a specific value
  template<typename T>
  void SetPC(const RevFeature* F, T val) {
    if( F->IsRV32() ){
      RV32_PC = static_cast<uint32_t>(val);
    }else{
      RV64_PC = static_cast<uint64_t>(val);
    }
  }

  /// AdvancePC: Advance the program counter a certain number of bytes
  template<typename T>
  void AdvancePC(const RevFeature* F, T bytes) {
    if ( F->IsRV32() ) {
      RV32_PC += bytes;
    }else{
      RV64_PC += bytes;
    }
  }

  /// GetFP32: Get the 32-bit float value of a specific FP register
  float GetFP32(const RevFeature* F, size_t rs) const {
    if( F->HasD() ){
      uint64_t i64;
      memcpy(&i64, &DPF[rs], sizeof(i64));   // The FP64 register's value
      if (~i64 >> 32)                        // Check for boxed NaN
        return NAN;                          // Return NaN if it's not boxed
      auto i32 = static_cast<uint32_t>(i64); // For endian independence
      float fp32;
      memcpy(&fp32, &i32, sizeof(fp32));     // The bottom half of FP64
      return fp32;                           // Reinterpreted as FP32
    } else {
      return SPF[rs];                        // The FP32 register's value
    }
  }

  /// SetFP32: Set a specific FP register to a 32-bit float value
  void SetFP32(const RevFeature* F, size_t rd, float value)
  {
    if( F->HasD() ){
      BoxNaN(&DPF[rd], &value);  // Store NaN-boxed in FP64 register
    } else {
      SPF[rd] = value;           // Store in FP32 register
    }
  }
}; // RevRegFile

inline std::bitset<_REV_HART_COUNT_> HART_CTS; ///< RevProc: Thread is clear to start (proceed with decode)
inline std::bitset<_REV_HART_COUNT_> HART_CTE; ///< RevProc: Thread is clear to execute (no register dependencides)

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

/// General template for converting between Floating Point and Integer.
/// FP values outside the range of the target integer type are clipped
/// at the integer type's numerical limits, whether signed or unsigned.
template<typename FP, typename INT>
bool CvtFpToInt(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
  FP fp;
  if constexpr(std::is_same_v<FP, double>){
    fp = R->DPF[Inst.rs1];         // Read the double FP register directly
  }else{
    fp = R->GetFP32(F, Inst.rs1);  // Read the F or D register, unboxing if D
  }
  constexpr INT max = std::numeric_limits<INT>::max();
  constexpr INT min = std::numeric_limits<INT>::min();
  INT res = std::isnan(fp) || fp > FP(max) ? max : fp < FP(min) ? min : static_cast<INT>(fp);

  // Make final result signed so sign extension occurs when sizeof(INT) < XLEN
  R->SetX(F, Inst.rd, static_cast<std::make_signed_t<INT>>(res));

  R->AdvancePC(F, Inst.instSize);
  return true;
}

/// fclass: Return FP classification like the RISC-V fclass instruction
// See: https://github.com/riscv/riscv-isa-sim/blob/master/softfloat/f32_classify.c
// Because quiet and signaling NaNs are not distinguished by the C++ standard,
// an additional argument has been added to disambiguate between quiet and
// signaling NaNs.
template<typename T>
unsigned fclass(T val, bool quietNaN = true) {
  switch(std::fpclassify(val)){
  case FP_INFINITE:
    return std::signbit(val) ? 1 : 1 << 7;
  case FP_NAN:
    return quietNaN ? 1 << 9 : 1 << 8;
  case FP_NORMAL:
    return std::signbit(val) ? 1 << 1 : 1 << 6;
  case FP_SUBNORMAL:
    return std::signbit(val) ? 1 << 2 : 1 << 5;
  case FP_ZERO:
    return std::signbit(val) ? 1 << 3 : 1 << 4;
  default:
    return 0;
  }
}

/// Load template
template<typename T>
bool load(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
  MemReq req{};
  if( sizeof(T) < sizeof(int64_t) && F->IsRV32() ){
    static constexpr auto flags = sizeof(T) < sizeof(int32_t) ?
      REVMEM_FLAGS(std::is_signed_v<T> ? RevCPU::RevFlag::F_SEXT32 :
                   RevCPU::RevFlag::F_ZEXT32) : REVMEM_FLAGS(0);
    req.Set(R->GetX<uint64_t>(F, Inst.rs1) + Inst.ImmSignExt(12), Inst.rd, RevRegClass::RegGPR, F->GetHartToExec(), MemOp::MemOpREAD, true, R->MarkLoadComplete);
    R->LSQueue->insert({make_lsq_hash(Inst.rd, RevRegClass::RegGPR, F->GetHartToExec()), req});
    M->ReadVal(F->GetHartToExec(),
               R->GetX<uint64_t>(F, Inst.rs1) + Inst.ImmSignExt(12),
               reinterpret_cast<std::make_unsigned_t<T>*>(&R->RV32[Inst.rd]),
               req,
               flags);
    R->SetX(F, Inst.rd, static_cast<T>(R->RV32[Inst.rd]));
  }else{
    static constexpr auto flags = sizeof(T) < sizeof(int64_t) ?
      REVMEM_FLAGS(std::is_signed_v<T> ? RevCPU::RevFlag::F_SEXT64 :
                   RevCPU::RevFlag::F_ZEXT64) : REVMEM_FLAGS(0);
    req.Set(R->GetX<uint64_t>(F, Inst.rs1) + Inst.ImmSignExt(12), Inst.rd, RevRegClass::RegGPR, F->GetHartToExec(), MemOp::MemOpREAD, true, R->MarkLoadComplete);
    R->LSQueue->insert({make_lsq_hash(Inst.rd, RevRegClass::RegGPR, F->GetHartToExec()), req});
    M->ReadVal(F->GetHartToExec(),
               R->GetX<uint64_t>(F, Inst.rs1) + Inst.ImmSignExt(12),
               reinterpret_cast<std::make_unsigned_t<T>*>(&R->RV64[Inst.rd]),
               req,
               flags);
    R->SetX(F, Inst.rd, static_cast<T>(R->RV64[Inst.rd]));
  }
  // update the cost
  R->cost += M->RandCost(F->GetMinCost(), F->GetMaxCost());
  R->AdvancePC(F, Inst.instSize);
  return true;
}

/// Store template
template<typename T>
bool store(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
  M->Write(F->GetHartToExec(),
           R->GetX<uint64_t>(F, Inst.rs1) + Inst.ImmSignExt(12),
           R->GetX<T>(F, Inst.rs2));
  R->AdvancePC(F, Inst.instSize);
  return true;
}

/// Floating-point load template
template<typename T>
bool fload(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
  MemReq req{};
  if(std::is_same_v<T, double> || F->HasD()){
    static constexpr auto flags = sizeof(T) < sizeof(double) ?
      REVMEM_FLAGS(RevCPU::RevFlag::F_BOXNAN) : REVMEM_FLAGS(0);

    req.Set(R->GetX<uint64_t>(F, Inst.rs1) + Inst.ImmSignExt(12), Inst.rd, RevRegClass::RegFLOAT, F->GetHartToExec(), MemOp::MemOpREAD, true, R->MarkLoadComplete);
    R->LSQueue->insert({make_lsq_hash(Inst.rd, RevRegClass::RegFLOAT, F->GetHartToExec()), req});
    M->ReadVal(F->GetHartToExec(),
               R->GetX<uint64_t>(F, Inst.rs1) + Inst.ImmSignExt(12),
               reinterpret_cast<T*>(&R->DPF[Inst.rd]),
               req,
               flags);

    // Box float value into 64-bit FP register
    if(std::is_same_v<T, float>){
      BoxNaN(&R->DPF[Inst.rd], &R->DPF[Inst.rd]);
    }
  }else{
    req.Set(R->GetX<uint64_t>(F, Inst.rs1) + Inst.ImmSignExt(12), Inst.rd, RevRegClass::RegFLOAT, F->GetHartToExec(), MemOp::MemOpREAD, true, R->MarkLoadComplete);
    R->LSQueue->insert({make_lsq_hash(Inst.rd, RevRegClass::RegFLOAT, F->GetHartToExec()), req});
    M->ReadVal(F->GetHartToExec(),
               R->GetX<uint64_t>(F, Inst.rs1) + Inst.ImmSignExt(12),
               &R->SPF[Inst.rd],
               req,
               REVMEM_FLAGS(0));
  }
  // update the cost
  R->cost += M->RandCost(F->GetMinCost(), F->GetMaxCost());
  R->AdvancePC(F, Inst.instSize);
  return true;
}

/// Floating-point store template
template<typename T>
bool fstore(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
  T val;
  if constexpr(std::is_same_v<T, double>){
    val = R->DPF[Inst.rs2];
  }else{
    val = R->GetFP32(F, Inst.rs2);
  }
  M->Write(F->GetHartToExec(), R->GetX<uint64_t>(F, Inst.rs1) + Inst.ImmSignExt(12), val);
  R->AdvancePC(F, Inst.instSize);
  return true;
}

/// Floating-point operation template
template<typename T, template<class> class OP>
bool foper(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
  if constexpr(std::is_same_v<T, double>){
    R->DPF[Inst.rd] = OP()(R->DPF[Inst.rs1], R->DPF[Inst.rs2]);
  }else{
    R->SetFP32(F, Inst.rd, OP()(R->GetFP32(F, Inst.rs1), R->GetFP32(F, Inst.rs2)));
  }
  R->AdvancePC(F, Inst.instSize);
  return true;
}

/// Floating-point minimum functor
template<typename = void>
struct FMin{
  template<typename T>
  auto operator()(T x, T y) const { return std::fmin(x, y); }
};

/// Floating-point maximum functor
template<typename = void>
struct FMax{
  template<typename T>
  auto operator()(T x, T y) const { return std::fmax(x, y); }
};

/// Floating-point conditional operation template
template<typename T, template<class> class OP>
bool fcondop(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
  bool res;
  if constexpr(std::is_same_v<T, double>){
    res = OP()(R->DPF[Inst.rs1], R->DPF[Inst.rs2]);
  }else{
    res = OP()(R->GetFP32(F, Inst.rs1), R->GetFP32(F, Inst.rs2));
  }
  R->SetX(F, Inst.rd, res);
  R->AdvancePC(F, Inst.instSize);
  return true;
}

/// Operand Kind (immediate or register)
enum class OpKind { Imm, Reg };

/// Arithmetic operator template
// The First parameter is the operator functor (such as std::plus)
// The second parameter is the operand kind (OpKind::Imm or OpKind::Reg)
// The third parameter is std::make_unsigned_t or std::make_signed_t (default)
// The optional fourth parameter indicates W mode (32-bit on XLEN == 64)
template<template<class> class OP, OpKind KIND,
         template<class> class SIGN = std::make_signed_t, bool W_MODE = false>
bool oper(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
  if( !W_MODE && F->IsRV32() ){
    using T = SIGN<int32_t>;
    T rs1 = R->GetX<T>(F, Inst.rs1);
    T rs2 = KIND == OpKind::Imm ? T(Inst.ImmSignExt(12)) : R->GetX<T>(F, Inst.rs2);
    T res = OP()(rs1, rs2);
    R->SetX(F, Inst.rd, res);
  }else{
    using T = SIGN<std::conditional_t<W_MODE, int32_t, int64_t>>;
    T rs1 = R->GetX<T>(F, Inst.rs1);
    T rs2 = KIND == OpKind::Imm ? T(Inst.ImmSignExt(12)) : R->GetX<T>(F, Inst.rs2);
    T res = OP()(rs1, rs2);
    // In W_MODE, cast the result to int32_t so that it's sign-extended
    R->SetX(F, Inst.rd, std::conditional_t<W_MODE, int32_t, T>(res));
  }
  R->AdvancePC(F, Inst.instSize);
  return true;
}

/// Left shift functor
template<typename = void>
struct ShiftLeft{
  template<typename T>
  constexpr T operator()(T val, unsigned shift) const {
    return val << (sizeof(T) == 4 ? shift & 0x1f : shift & 0x3f);
  }
};

/// Right shift functor
template<typename = void>
struct ShiftRight{
  template<typename T>
  constexpr T operator()(T val, unsigned shift) const {
    return val >> (sizeof(T) == 4 ? shift & 0x1f : shift & 0x3f);
  }
};

enum class DivRem { Div, Rem };

/// Division/Remainder template
// The first parameter is DivRem::Div or DivRem::Rem
// The second parameter is std::make_signed_t or std::make_unsigned_t
// The optional third parameter indicates W mode (32-bit on XLEN == 64)
template<DivRem DIVREM, template<class> class SIGN, bool W_MODE = false>
bool divrem(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
  if( !W_MODE && F->IsRV32() ){
    using T = SIGN<int32_t>;
    T rs1 = R->GetX<T>(F, Inst.rs1);
    T rs2 = R->GetX<T>(F, Inst.rs2);
    T res;
    if constexpr(DIVREM == DivRem::Div){
      res = std::is_signed_v<T> && rs1 == std::numeric_limits<T>::min() &&
        rs2 == -T{1} ? rs1 : rs2 ? rs1 / rs2 : -T{1};
    }else{
      res = std::is_signed_v<T> && rs1 == std::numeric_limits<T>::min() &&
        rs2 == -T{1} ? 0 : rs2 ? rs1 % rs2 : rs1;
    }
    R->SetX(F, Inst.rd, res);
  } else {
    using T = SIGN<std::conditional_t<W_MODE, int32_t, int64_t>>;
    T rs1 = R->GetX<T>(F, Inst.rs1);
    T rs2 = R->GetX<T>(F, Inst.rs2);
    T res;
    if constexpr(DIVREM == DivRem::Div){
      res = std::is_signed_v<T> && rs1 == std::numeric_limits<T>::min() &&
        rs2 == -T{1} ? rs1 : rs2 ? rs1 / rs2 : -T{1};
    }else{
      res = std::is_signed_v<T> && rs1 == std::numeric_limits<T>::min() &&
        rs2 == -T{1} ? 0 : rs2 ? rs1 % rs2 : rs1;
    }
    // In W_MODE, cast the result to int32_t so that it's sign-extended
    R->SetX(F, Inst.rd, std::conditional_t<W_MODE, int32_t, T>(res));
  }
  R->AdvancePC(F, Inst.instSize);
  return true;
}

} // namespace SST::RevCPU

#endif
