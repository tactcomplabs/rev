//
// _RevHeap_cc_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "RevHeap.h"
#include "RevMem.h"
#include <cmath>
#include <cstring>
#include <functional>
#include <memory>
#include <mutex>
#include <utility>

namespace SST::RevCPU {

RevHeap::RevHeap( RevMem*        Mem,
                  const size_t   MaxHeapSize,
                  const uint64_t baseAddr,
                  SST::Output*   output ) :
  Mem( Mem ), Size( 0 ), MaxSize( MaxHeapSize ), BaseAddr( baseAddr ),
  output( output ) {

  // Reserve enough space for the free list (doesn't add entries... just prevents us from reallocating every time the brk changes)
  FreeList.reserve( MaxHeapSize / 64 );

  if( MaxHeapSize % 64 != 0 ) {
    const size_t AdjustedMaxHeapSize = Mem->AlignDown( MaxHeapSize, 64 );
    output->verbose( CALL_INFO,
                     1,
                     0,
                     "RevHeap: Adjusting MaxHeapSize from %zu to %zu\n",
                     MaxHeapSize,
                     AdjustedMaxHeapSize );
    MaxSize = AdjustedMaxHeapSize;
  }

  if( BaseAddr % 64 != 0 ) {
    output->fatal(
      CALL_INFO, -1, "RevHeap: BaseAddr must be a multiple of 64B\n" );
  }

  if( MaxHeapSize < 64 ) {
    output->fatal(
      CALL_INFO, -1, "RevHeap: MaxHeapSize must be at least 64B\n" );
  }

  output->verbose( CALL_INFO,
                   1,
                   0,
                   "RevHeap: Initialized with MaxHeapSize=%zu "
                   "and BaseAddr=%" PRIx64 "\n",
                   MaxHeapSize,
                   BaseAddr );
}

// This will only be able to verify if addr is valid within the current Rev instance
// Whatever runtime system you use will have to have a mechanism to update each rev if
// you expect global heap allocations to be valid
bool RevHeap::isValidVirtAddr( const uint64_t addr ) {
  // Calculate what chunk it would be in
  // Check to see if the address is within 8 bytes under BaseAddr
  uint64_t Chunk = ( addr - BaseAddr ) / 64;
  if( Chunk >= FreeList.size() ) {
    return false;
  }

  return FreeList[Chunk];
}

void RevHeap::sbrk( const int64_t increment ) {
  if( increment < 0 ) {
    if( Size + increment < 0 ) {
      output->fatal( CALL_INFO,
                     -1,
                     "RevHeap: SBRK Increment Value would result in a negative "
                     "heap size\n" );
    }
    FreeList.resize( FreeList.size() + ( increment / 64 ) );
  } else if( increment > 0 ) {
    // Calculate the number of 64B chunks we need to add
    size_t ChunksToAdd = increment / 64;
    if( increment % 64 != 0 ) {
      ChunksToAdd++;
    }
    // Resize the free list
    FreeList.resize( FreeList.size() + ChunksToAdd, false );
  }
  Size += increment;
}

}  // namespace SST::RevCPU
