//
// _Rev_Common_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef __REV_COMMON__
#define __REV_COMMON__

#include <functional>
#include <cstdint>

#ifndef _REV_NUM_REGS_
#define _REV_NUM_REGS_ 32
#endif

#ifndef _REV_HART_COUNT_
#define _REV_HART_COUNT_ 1
#endif

#ifndef _REV_INVALID_HART_ID_
#define _REV_INVALID_HART_ID_ (uint16_t(~0))
#endif

#define _INVALID_ADDR_ (~uint64_t{0})

namespace SST::RevCPU{

enum class RevRegClass : uint8_t { ///< Rev CPU Register Classes
  RegUNKNOWN  = 0,           ///< RevRegClass: Unknown register file
  RegIMM      = 1,           ///< RevRegClass: Treat the reg class like an immediate: S-Format
  RegGPR      = 2,           ///< RevRegClass: GPR reg file
  RegCSR      = 3,           ///< RevRegClass: CSR reg file
  RegFLOAT    = 4,           ///< RevRegClass: Float register file
};

enum class MemOp : uint8_t {
  MemOpREAD        = 0,
  MemOpWRITE       = 1,
  MemOpFLUSH       = 2,
  MemOpREADLOCK    = 3,
  MemOpWRITEUNLOCK = 4,
  MemOpLOADLINK    = 5,
  MemOpSTORECOND   = 6,
  MemOpCUSTOM      = 7,
  MemOpFENCE       = 8,
  MemOpAMO         = 9,
};

inline uint64_t make_lsq_hash(uint8_t destReg, RevRegClass regType, uint16_t HartID){
  return static_cast<uint64_t>(regType) << (16 + 8) | static_cast<uint64_t>(destReg) << 16 | HartID;
};

struct MemReq{
  MemReq() = default;

  MemReq(uint64_t addr, uint16_t dest, RevRegClass regclass,
         uint16_t hart, MemOp req, bool outstanding, std::function<void(MemReq)> func) :
    Addr(addr), DestReg(dest), RegType(regclass), Hart(hart),
    ReqType(req), isOutstanding(outstanding), MarkLoadComplete(func)
  {
  }

  void Set(uint64_t addr, uint16_t dest, RevRegClass regclass, uint16_t hart, MemOp req, bool outstanding,
           std::function<void(MemReq)> func)
  {
    Addr = addr; DestReg = dest; RegType = regclass; Hart = hart;
    ReqType = req; isOutstanding = outstanding;
    MarkLoadComplete = func;
  }

  uint64_t    Addr          = _INVALID_ADDR_;
  uint16_t    DestReg       = 0;
  RevRegClass RegType       = RevRegClass::RegUNKNOWN;
  uint16_t    Hart          = _REV_INVALID_HART_ID_;
  MemOp       ReqType       = MemOp::MemOpCUSTOM;
  bool        isOutstanding = false;

  std::function<void(MemReq)> MarkLoadComplete = nullptr;

  //std::shared_ptr<std::unordered_map<uint64_t, MemReq>> LSQueue;
  //  std::function<void((MemOp req)>) SetComplete;
  // add lambda that clears the dependency bit directly so we don't need to search the hash
};//struct MemReq

}//namespace SST::RevCPU

#endif
