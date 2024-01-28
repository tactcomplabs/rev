//
// _RevHeap_h_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_REVHEAP_H_
#define _SST_REVCPU_REVHEAP_H_

// TODO: Add license
// Starts at size 0, and grows as needed.
// FreeList represents 64B chunks of memory.
// Perhaps we should reserve

#include <cstddef>
#include <stdlib.h>
#include <vector>
#include <cstdio>

#include "SST.h"

namespace SST::RevCPU {

class RevMem; ///< RevHeap: Forward declaration of RevMem

///< RevHeap: Allocation metadata (Written to the beginning of each allocation)
struct AllocMetadata {
  size_t Size;
};

class RevHeap {
  public:
    RevHeap(RevMem* Mem, const size_t MaxHeapSize, const uint64_t baseAddr, SST::Output* output);
    ~RevHeap() = default;
    void* malloc(size_t size);
    void free(void* ptr);

    void sbrk(int64_t increment);
    uint64_t brk(uint64_t addr); // TODO: Figure out if we need this
//
    bool isValidVirtAddr(const uint64_t addr);
  private:
    RevMem* Mem = nullptr; ///< RevHeap: Pointer to the RevMem object
    size_t Size = 0; ///< RevHeap: Current size of the heap in bytes
    size_t MaxSize = 0; ///< RevHeap: Current size of the heap in bytes
    std::vector<bool> FreeList = {}; ///< RevHeap: Free list of memory chunks (64B chunks)
    uint64_t BaseAddr = 0; ///< RevHeap: Base address of the heap (end of static data)
    SST::Output *output;
};
} // namespace SST::RevCPU

#endif
