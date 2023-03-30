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

#include <vector>

#include "../include/RevMem.h"
#include "../include/RevFeature.h"

namespace SST{
namespace RevCPU {

#define REVPREF_INIT_ADDR ((uint32_t)(0xdeadbeef))

class RevPrefetcher{
public:
  /// RevPrefetcher: default constructor
  RevPrefetcher(RevMem *Mem, unsigned Depth)
    : mem(Mem), depth(Depth){}

  /// RevPrefetcher: default destructor
  ~RevPrefetcher(){
    // delete all the existing streams
    baseAddr.clear();
    for( unsigned i=0; i<iStack.size(); i++ ){
      delete [] iStack[i];
    }
  }

  /// RevPrefetcher: fetch the next instruction
  bool InstFetch(uint64_t Addr, bool &Fetched, uint32_t &Inst);

  /// RevPrefetcher: determines in the target instruction is already cached in a stream
  bool IsAvail(uint64_t Addr);

private:
  RevMem *mem;                                ///< RevMem object
  unsigned depth;                             ///< Depth of each prefetcher stream
  std::vector<uint64_t> baseAddr;             ///< Vector of base addresses for each stream
  std::vector<uint32_t*> iStack;              ///< Vector of instruction vectors

  /// fills a missed stream cache instruction
  void Fill(uint64_t Addr);

  /// deletes the target stream buffer
  void DeleteStream(unsigned i);

  /// attempts to fetch the upper half of a 32bit word of an unaligned base address
  bool FetchUpper(uint64_t Addr, bool &Fetched, uint32_t &UInst);
};

} // namespace RevCPU
} // namespace SST

#endif // _SST_REVCPU_REVPREFETCHER_H_
