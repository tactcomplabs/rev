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

#include <cstddef>
#include <cstdint>
#include <functional>
#include <type_traits>

#ifndef _REV_NUM_REGS_
#define _REV_NUM_REGS_ 32
#endif

#ifndef _REV_INVALID_HART_ID_
#define _REV_INVALID_HART_ID_ (unsigned(~0))
#endif

#define _INVALID_ADDR_ (~uint64_t{0})

#define _INVALID_TID_ (uint32_t{0})

#define _MAX_HARTS_ 4096

namespace SST::RevCPU{

/// Zero-extend value of bits size
template<typename T>
constexpr auto ZeroExt(T val, size_t bits){
  return static_cast<std::make_unsigned_t<T>>(val) & ~(~std::make_unsigned_t<T>{0} << bits);
}

/// Sign-extend value of bits size
template<typename T>
constexpr auto SignExt(T val, size_t bits){
  auto signbit = std::make_unsigned_t<T>{1} << (bits-1);
  return static_cast<std::make_signed_t<T>>((ZeroExt(val, bits) ^ signbit) - signbit);
}

/// Base-2 logarithm of integers
template<typename T>
constexpr int lg(T x){
  static_assert(std::is_integral_v<T>);

  // We select the __builtin_clz which takes integers no smaller than x
  if constexpr(sizeof(x) <= sizeof(int)){
    return x ? 8*sizeof(int)-1 - __builtin_clz(x) : -1;
  }else if constexpr(sizeof(x) <= sizeof(long)){
    return x ? 8*sizeof(long)-1 - __builtin_clzl(x) : -1;
  }else{
    return x ? 8*sizeof(long long)-1 - __builtin_clzll(x) : -1;
  }
}

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

inline uint64_t make_lsq_hash(uint16_t destReg, RevRegClass regType, unsigned HartID){
  return static_cast<uint64_t>(regType) << (16 + 8) | static_cast<uint64_t>(destReg) << 16 | HartID;
};

struct MemReq{
  MemReq() = default;

  MemReq(uint64_t addr, uint16_t dest, RevRegClass regclass,
         unsigned hart, MemOp req, bool outstanding, std::function<void(MemReq)> func) :
    Addr(addr), DestReg(dest), RegType(regclass), Hart(hart),
    ReqType(req), isOutstanding(outstanding), MarkLoadComplete(func)
  {
  }

  void Set(uint64_t addr, uint16_t dest, RevRegClass regclass, unsigned hart, MemOp req, bool outstanding,
           std::function<void(MemReq)> func)
  {
    Addr = addr; DestReg = dest; RegType = regclass; Hart = hart;
    ReqType = req; isOutstanding = outstanding;
    MarkLoadComplete = func;
  }

  uint64_t    Addr          = _INVALID_ADDR_;
  uint16_t    DestReg       = 0;
  RevRegClass RegType       = RevRegClass::RegUNKNOWN;
  unsigned    Hart          = _REV_INVALID_HART_ID_;
  MemOp       ReqType       = MemOp::MemOpCUSTOM;
  bool        isOutstanding = false;

  std::function<void(MemReq)> MarkLoadComplete = nullptr;

  //std::shared_ptr<std::unordered_map<uint64_t, MemReq>> LSQueue;
  //  std::function<void((MemOp req)>) SetComplete;
  // add lambda that clears the dependency bit directly so we don't need to search the hash
};//struct MemReq

}//namespace SST::RevCPU

#endif
