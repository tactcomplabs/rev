//
// _RevPrefetcher_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_REVPREFETCHER_H_
#define _SST_REVCPU_REVPREFETCHER_H_

#include <cstdint>
#include <memory>
#include <vector>
#include <memory>
#include <unordered_map>

#include "RevMem.h"
#include "RevFeature.h"

namespace SST::RevCPU{

#define REVPREF_INIT_ADDR 0xDEADBEEF

class RevPrefetcher{
public:
  /// RevPrefetcher: constructor
  RevPrefetcher(RevMem *Mem, RevFeature *Feature, unsigned Depth,
                std::shared_ptr<std::unordered_map<uint64_t, MemReq>> lsq,
                std::function<void(const MemReq&)> func)
    : mem(Mem), feature(Feature), depth(Depth), LSQueue(lsq), MarkLoadAsComplete(func){}

  /// RevPrefetcher: destructor
  ~RevPrefetcher() = default;

  /// RevPrefetcher: fetch the next instruction
  bool InstFetch(uint64_t Addr, bool &Fetched, uint32_t &Inst);

  /// RevPrefetcher: determines in the target instruction is already cached in a stream
  bool IsAvail(uint64_t Addr);

private:
  RevMem *mem;                                ///< RevMem object
  RevFeature *feature;                        ///< RevFeature object
  unsigned depth;                             ///< Depth of each prefetcher stream
  std::vector<uint64_t> baseAddr;             ///< Vector of base addresses for each stream
  std::vector<std::vector<uint32_t>> iStack; ///< Vector of instruction vectors
  std::shared_ptr<std::unordered_map<uint64_t, MemReq>> LSQueue;
  std::function<void(const MemReq&)> MarkLoadAsComplete;

  /// fills a missed stream cache instruction
  void Fill(uint64_t Addr);

  /// deletes the target stream buffer
  void DeleteStream(size_t i);

  /// attempts to fetch the upper half of a 32bit word of an unaligned base address
  bool FetchUpper(uint64_t Addr, bool &Fetched, uint32_t &UInst);
};

} // namespace SST::RevCPU

#endif // _SST_REVCPU_REVPREFETCHER_H_
