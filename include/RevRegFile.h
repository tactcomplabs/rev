//
// _RevRegFile_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_REVREGFILE_H_
#define _SST_REVCPU_REVREGFILE_H_

#include <bitset>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <memory>
#include <ostream>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include "RevFeature.h"
#include "RevMem.h"
#include "../common/include/RevCommon.h"

namespace SST::RevCPU{

struct RevInst;

/// BoxNaN: Store a boxed float inside a double
inline void BoxNaN(double* dest, const void* value){
  uint32_t i32;
  memcpy(&i32, value, sizeof(float));                // The FP32 value
  uint64_t i64 = uint64_t{i32} | ~uint64_t{0} << 32; // Boxed NaN value
  memcpy(dest, &i64, sizeof(double));                // Store in FP64 register
}

/// RISC-V Register Mneumonics
enum class RevReg {
  zero =  0, ra  =  1, sp   =  2, gp   =  3, tp  =  4, t0  =  5, t1   =  6, t2   =  7,
  s0   =  8, s1  =  9, a0   = 10, a1   = 11, a2  = 12, a3  = 13, a4   = 14, a5   = 15,
  a6   = 16, a7  = 17, s2   = 18, s3   = 19, s4  = 20, s5  = 21, s6   = 22, s7   = 23,
  s8   = 24, s9  = 25, s10  = 26, s11  = 27, t3  = 28, t4  = 29, t5   = 30, t6   = 31,
  ft0  =  0, ft1 =  1, ft2  =  2, ft3  =  3, ft4 =  4, ft5 =  5, ft6  =  6, ft7  =  7,
  fs0  =  8, fs1 =  9, fa0  = 10, fa1  = 11, fa2 = 12, fa3 = 13, fa4  = 14, fa5  = 15,
  fa6  = 16, fa7 = 17, fs2  = 18, fs3  = 19, fs4 = 20, fs5 = 21, fs6  = 22, fs7  = 23,
  fs8  = 24, fs9 = 25, fs10 = 26, fs11 = 27, ft8 = 28, ft9 = 29, ft10 = 30, ft11 = 31,
};

/// Floating-point control register
// fcsr.NX, fcsr.UF, fcsr.OF, fcsr.DZ, fcsr.NV, fcsr.frm
struct FCSR{
  uint32_t NX  : 1;
  uint32_t UF  : 1;
  uint32_t OF  : 1;
  uint32_t DZ  : 1;
  uint32_t NV  : 1;
  uint32_t frm : 3;
  uint32_t     : 24;
};

class RevRegFile {
public:
  const bool IsRV32;                  ///< RevRegFile: Cached copy of Features->IsRV32()
  const bool HasD;                    ///< RevRegFile: Cached copy of Features->HasD()

private:
  bool trigger{};                     ///< RevRegFile: Has the instruction been triggered?
  unsigned Entry{};                   ///< RevRegFile: Instruction entry
  uint32_t cost{};                    ///< RevRegFile: Cost of the instruction
  RevTracer *Tracer = nullptr;                  ///< RegRegFile: Tracer object

  union{  // Anonymous union. We zero-initialize the largest member
    uint32_t RV32_PC;                   ///< RevRegFile: RV32 PC
    uint64_t RV64_PC{};                 ///< RevRegFile: RV64 PC
  };

  FCSR fcsr{}; ///< RevRegFile: FCSR

  std::shared_ptr<std::unordered_map<uint64_t, MemReq>> LSQueue{};
  std::function<void(const MemReq&)> MarkLoadCompleteFunc{};

  union{  // Anonymous union. We zero-initialize the largest member
    uint32_t RV32[_REV_NUM_REGS_];      ///< RevRegFile: RV32I register file
    uint64_t RV64[_REV_NUM_REGS_]{};    ///< RevRegFile: RV64I register file
  };

  union{  // Anonymous union. We zero-initialize the largest member
    float SPF[_REV_NUM_REGS_];          ///< RevRegFile: RVxxF register file
    double DPF[_REV_NUM_REGS_]{};       ///< RevRegFile: RVxxD register file
  };

  std::bitset<_REV_NUM_REGS_> RV_Scoreboard{}; ///< RevRegFile: Scoreboard for RV32/RV64 RF to manage pipeline hazard
  std::bitset<_REV_NUM_REGS_> FP_Scoreboard{}; ///< RevRegFile: Scoreboard for SPF/DPF RF to manage pipeline hazard

  // Supervisor Mode CSRs
#if 0 // not used
  union{  // Anonymous union. We zero-initialize the largest member
    uint64_t RV64_SSTATUS{}; // During ecall, previous priviledge mode is saved in this register (Incomplete)
    uint32_t RV32_SSTATUS;
  };
#endif

  union{  // Anonymous union. We zero-initialize the largest member
    uint64_t RV64_SEPC{};    // Holds address of instruction that caused the exception (ie. ECALL)
    uint32_t RV32_SEPC;
  };

  union{  // Anonymous union. We zero-initialize the largest member
    uint64_t RV64_SCAUSE{};  // Used to store cause of exception (ie. ECALL_USER_EXCEPTION)/
    uint32_t RV32_SCAUSE;
  };

  union{  // Anonymous union. We zero-initialize the largest member
    uint64_t RV64_STVAL{};   // Used to store additional info about exception (ECALL does not use this and sets value to 0)
    uint32_t RV32_STVAL;
  };

#if 0 // not used
  union{  // Anonymous union. We zero-initialize the largest member
    uint64_t RV64_STVEC{};   // Holds the base address of the exception handling routine (trap handler) that the processor jumps to when and exception occurs
    uint32_t RV32_STVEC;
  };
#endif

public:
  // Constructor which takes a RevFeature
  explicit RevRegFile(const RevFeature* feature)
    : IsRV32(feature->IsRV32()), HasD(feature->HasD()) {
  }

  // Getters/Setters

  /// Get cost of the instruction
  const uint32_t& GetCost() const { return cost; }
  uint32_t& GetCost() { return cost; }

  /// Set cost of the instruction
  void SetCost(uint32_t c) { cost = c; }

  /// Get whether the instruction has been triggered
  bool GetTrigger() const { return trigger; }

  /// Set whether the instruction has been triggered
  void SetTrigger(bool t) { trigger = t; }

  /// Get the instruction entry
  unsigned GetEntry() const { return Entry; }

  /// Set the instruction entry
  void SetEntry(unsigned e) { Entry = e; }

  /// Get the Load/Store Queue
  const auto& GetLSQueue() const { return LSQueue; }

  /// Set the Load/Store Queue
  void SetLSQueue(std::shared_ptr<std::unordered_map<uint64_t, MemReq>> lsq){
    LSQueue = std::move(lsq);
  }

  /// Set the current tracer
  void SetTracer(RevTracer *t) { Tracer = t; }

  /// Insert an item in the Load/Store Queue
  void LSQueueInsert(std::pair<uint64_t, MemReq> item){
    LSQueue->insert(std::move(item));
  }

  /// Get the MarkLoadComplete function
  const std::function<void(const MemReq&)>& GetMarkLoadComplete() const {
    return MarkLoadCompleteFunc;
  }

  /// Set the MarkLoadComplete function
  void SetMarkLoadComplete(std::function<void(const MemReq&)> func){
    MarkLoadCompleteFunc = std::move(func);
  }

  /// Invoke the MarkLoadComplete function
  void MarkLoadComplete(const MemReq& req) const {
    MarkLoadCompleteFunc(req);
  }

  /// Capture the PC of current instruction which raised exception
  void SetSEPC(){
    if( IsRV32 ){
      RV32_SEPC = RV32_PC;
    }else{
      RV64_SEPC = RV64_PC;
    }
  }

  ///Set the value for extra information about exception
  /// (ECALL doesn't use it and sets it to 0)
  template<typename T>
  void SetSTVAL(T val){
    if( IsRV32 ){
      RV32_STVAL = val;
    }else{
      RV64_STVAL = val;
    }
  }

  /// Set the exception cause
  template<typename T>
  void SetSCAUSE(T val){
    if( IsRV32 ){
      RV32_SCAUSE = val;
    }else{
      RV64_SCAUSE = val;
    }
  }

  /// GetX: Get the specifed X register cast to a specific integral type
  template<typename T, typename U>
  T GetX(U rs) const {
    T res;
    if( IsRV32 ){
      res = RevReg(rs) != RevReg::zero ? T(RV32[size_t(rs)]) : 0;
    }else{
      res = RevReg(rs) != RevReg::zero ? T(RV64[size_t(rs)]) : 0;
    }
    TRACE_REG_READ(rs,res);
    return res;
  }

  /// SetX: Set the specifed X register to a specific value
  template<typename T, typename U>
  void SetX(U rd, T val) {
    T res;
    if( IsRV32 ){
      res = RevReg(rd) != RevReg::zero ? uint32_t(val) : 0;
      RV32[size_t(rd)] = res;
    }else{
      res = RevReg(rd) != RevReg::zero ? uint64_t(val) : 0;
      RV64[size_t(rd)] = res;
    }
    TRACE_REG_WRITE(rd, res);
  }

  /// GetPC: Get the Program Counter
  uint64_t GetPC() const {
    if( IsRV32 ){
      return RV32_PC;
    }else{
      return RV64_PC;
    }
  }

  /// SetPC: Set the Program Counter to a specific value
  template<typename T>
  void SetPC(T val) {
    if( IsRV32 ){
      RV32_PC = static_cast<uint32_t>(val);
      TRACE_PC_WRITE(RV32_PC);
    }else{
      RV64_PC = static_cast<uint64_t>(val);
      TRACE_PC_WRITE(RV64_PC);
    }
  }

  /// AdvancePC: Advance the program counter a certain number of bytes
  template<typename T>
  void AdvancePC(T bytes) {
    if ( IsRV32 ) {
      RV32_PC += bytes;
      TRACE_PC_WRITE(RV32_PC);
    }else{
      RV64_PC += bytes;
      TRACE_PC_WRITE(RV64_PC);
    }
  }

  /// GetFP32: Get the 32-bit float value of a specific FP register
  template<typename U>
  float GetFP32(U rs) const {
    if( HasD ){
      uint64_t i64;
      memcpy(&i64, &DPF[size_t(rs)], sizeof(i64));   // The FP64 register's value
      if (~i64 >> 32)                        // Check for boxed NaN
        return NAN;                          // Return NaN if it's not boxed
      auto i32 = static_cast<uint32_t>(i64); // For endian independence
      float fp32;
      memcpy(&fp32, &i32, sizeof(fp32));     // The bottom half of FP64
      return fp32;                           // Reinterpreted as FP32
    } else {
      return SPF[size_t(rs)];                // The FP32 register's value
    }
  }

  /// SetFP32: Set a specific FP register to a 32-bit float value
  template<typename U>
  void SetFP32(U rd, float value)
    {
      if( HasD ){
        BoxNaN(&DPF[size_t(rd)], &value);    // Store NaN-boxed in FP64 register
      } else {
        SPF[size_t(rd)] = value;             // Store in FP32 register
      }
    }

  // Friend functions and classes to access internal register state
  template<typename FP, typename INT>
  friend bool CvtFpToInt(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst);

  template<typename T>
  friend bool load(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst);

  template<typename T>
  friend bool store(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst);

  template<typename T>
  friend bool fload(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst);

  template<typename T>
  friend bool fstore(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst);

  template<typename T, template<class> class OP>
  friend bool foper(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst);

  template<typename T, template<class> class OP>
  friend bool fcondop(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst);

  friend std::ostream& operator<<(std::ostream& os, const RevRegFile& regFile);

  friend class RevProc;
  friend class RV32A;
  friend class RV64A;
  friend class RV32D;
  friend class RV64D;

}; // class RevRegFile

} // namespace SST::RevCPU

#endif
