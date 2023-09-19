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


#ifndef _REV_INVALID_HART_ID_
#define _REV_INVALID_HART_ID_ (uint16_t)~(uint16_t(0))
#endif

#define _INVALID_ADDR_ 0xFFFFFFFFFFFFFFFF

namespace SST{
  namespace RevCPU{

    enum RevRegClass : int { ///< Rev CPU Register Classes
      RegUNKNOWN    = 0,     ///< RevRegClass: Unknown register file
      RegIMM        = 1,     ///< RevRegClass: Treat the reg class like an immediate: S-Format
      RegGPR        = 2,     ///< RevRegClass: GPR reg file
      RegCSR        = 3,     ///< RevRegClass: CSR reg file
      RegFLOAT      = 4,     ///< RevRegClass: Float register file
    };

    enum MemOp{
      MemOpREAD           = 0,
      MemOpWRITE          = 1,
      MemOpFLUSH          = 2,
      MemOpREADLOCK       = 3,
      MemOpWRITEUNLOCK    = 4,
      MemOpLOADLINK       = 5,
      MemOpSTORECOND      = 6,
      MemOpCUSTOM         = 7,
      MemOpFENCE          = 8,
      MemOpAMO            = 9
    };

    auto make_lsq_hash = [] (uint8_t destReg, RevRegClass regType, uint16_t HartID){ return uint64_t((regType << (16 + 8)) | (destReg << 16) | HartID);};


    class MemReq {
      public:
      MemReq(uint64_t addr, uint16_t dest, RevRegClass regclass, 
             uint16_t hart, MemOp req, bool outstanding, std::function<void(MemReq)> func) :
        Addr(addr), DestReg(dest), RegType(regclass), Hart(hart), ReqType(req), isOutstanding(outstanding), MarkLoadComplete(func)
      {};
      MemReq():
        Addr(_INVALID_ADDR_), DestReg(0), RegType(RegUNKNOWN), Hart(_REV_INVALID_HART_ID_), 
        ReqType(MemOpCUSTOM), isOutstanding(false), MarkLoadComplete(nullptr)
      {};

      void Set(uint64_t addr, uint16_t dest, RevRegClass regclass, uint16_t hart, MemOp req, bool outstanding,
      std::function<void(MemReq)> func)
      {
        Addr = addr; DestReg = dest; RegType = regclass; Hart = hart; 
        ReqType = req; isOutstanding = outstanding;
        MarkLoadComplete = func;
      }; 
      uint64_t    Addr;
      uint16_t    DestReg;
      RevRegClass RegType;
      uint16_t    Hart;
      MemOp       ReqType;
      bool        isOutstanding;
      //std::shared_ptr<std::unordered_map<uint64_t, MemReq>> LSQueue;
      std::function<void(MemReq)> MarkLoadComplete;
    //  std::function<void((MemOp req)>) SetComplete;
      // add lambda that clears the dependency bit directly so we don't need to search the hash
    };

    //auto mark_mem_op_complete = [] (MemReq req)

  }//namespace RevCPU
}//namespace SST

#endif