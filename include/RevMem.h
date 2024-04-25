//
// _RevMem_h_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_REVMEM_H_
#define _SST_REVCPU_REVMEM_H_

#define __PAGE_SIZE__ 4096

// -- C++ Headers
#include <algorithm>
#include <cinttypes>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <random>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

// -- SST Headers
#include "SST.h"

// -- RevCPU Headers
#include "../common/include/RevCommon.h"
#include "RevHeap.h"
#include "RevMemCtrl.h"
#include "RevOpts.h"
#include "RevRand.h"
#include "RevTracer.h"

#ifndef _REVMEM_BASE_
#define _REVMEM_BASE_ 0x00000000
#endif

#define _STACK_SIZE_ ( size_t{ 1024 * 1024 } )

namespace SST::RevCPU {

class RevMem {
public:
  /// RevMem: standard constructor
  RevMem( uint64_t MemSize, RevOpts* Opts, SST::Output* Output );

  /// RevMem: standard memory controller constructor
  RevMem( uint64_t     MemSize,
          RevOpts*     Opts,
          RevMemCtrl*  Ctrl,
          SST::Output* Output );

  /// RevMem: standard destructor
  ~RevMem() {
    delete[] physMem;
  }

  /* Virtual Memory Blocks  */
  class MemSegment {
  public:
    MemSegment( uint64_t baseAddr, uint64_t size ) :
      BaseAddr( baseAddr ), Size( size ) {
      TopAddr = baseAddr + size;
    }  // Permissions(permissions) {}

    uint64_t getTopAddr() const {
      return BaseAddr + Size;
    }

    uint64_t getBaseAddr() const {
      return BaseAddr;
    }

    uint64_t getSize() const {
      return Size;
    }

    void setBaseAddr( uint64_t baseAddr ) {
      BaseAddr = baseAddr;
      if( Size ) {
        TopAddr = Size + BaseAddr;
      }
    }

    void setSize( uint64_t size ) {
      Size    = size;
      TopAddr = BaseAddr + size;
    }

    /// MemSegment: Check if vAddr is included in this segment
    bool contains( const uint64_t& vAddr ) {
      return ( vAddr >= BaseAddr && vAddr < TopAddr );
    };

    // Check if a given range is inside a segment
    bool contains( const uint64_t& vBaseAddr, const uint64_t& Size ) {
      // exclusive top address
      uint64_t vTopAddr = vBaseAddr + Size - 1;
      return ( this->contains( vBaseAddr ) && this->contains( vTopAddr ) );
    };

    // Override for easy std::cout << *Seg << std::endl;
    friend std::ostream& operator<<( std::ostream& os, const MemSegment& Seg ) {
      return os << " | BaseAddr:  0x" << std::hex << Seg.getBaseAddr()
                << " | TopAddr: 0x" << std::hex << Seg.getTopAddr()
                << " | Size: " << std::dec << Seg.getSize() << " Bytes";
    }

  private:
    uint64_t BaseAddr;
    uint64_t Size;
    uint64_t TopAddr;
  };

  /// RevMem: determine if there are any outstanding requests
  bool outstandingRqsts();

  /// RevMem: handle incoming memory event
  void handleEvent( Interfaces::StandardMem::Request* ev ) {
  }

  /// RevMem: handle memory injection
  void     HandleMemFault( unsigned width );

  /// RevMem: get the stack_top address
  uint64_t GetStackTop() {
    return stacktop;
  }

  /// RevMem: set the stack_top address
  void SetStackTop( uint64_t Addr ) {
    stacktop = Addr;
  }

  /// RevMem: tracer pointer
  RevTracer* Tracer = nullptr;

  /// RevMem: retrieve the address of the top of memory (not stack)
  uint64_t   GetMemTop() {
    return ( _REVMEM_BASE_ + memSize );
  }

  /// RevMem: get the stack_top address
  uint64_t GetStackBottom() {
    return stacktop - _STACK_SIZE_;
  }

  /// RevMem: initiate a memory fence
  bool     FenceMem( unsigned Hart );

  /// RevMem: retrieves the cache line size.  Returns 0 if no cache is configured
  unsigned getLineSize() {
    return ctrl ? ctrl->getLineSize() : 64;
  }

  /// RevMem: Enable tracing of load and store instructions.
  void SetTracer( RevTracer* tracer ) {
    Tracer = tracer;
  }

  // ----------------------------------------------------
  // ---- Base Memory Interfaces
  // ----------------------------------------------------
  /// RevMem: write to the target memory location
  bool WriteMem( unsigned Hart, uint64_t Addr, size_t Len, const void* Data );

  /// RevMem: write to the target memory location with the target flags
  bool WriteMem(
    unsigned Hart, uint64_t Addr, size_t Len, const void* Data, RevFlag flags );

  /// RevMem: read data from the target memory location
  bool ReadMem( unsigned      Hart,
                uint64_t      Addr,
                size_t        Len,
                void*         Target,
                const MemReq& req,
                RevFlag       flags );

  /// RevMem: flush a cache line
  bool FlushLine( unsigned Hart, uint64_t Addr );

  /// RevMem: invalidate a cache line
  bool InvLine( unsigned Hart, uint64_t Addr );

  /// RevMem: clean a line
  bool CleanLine( unsigned Hart, uint64_t Addr );

  /// RevMem: DEPRECATED: read data from the target memory location
  [[deprecated( "Simple RevMem interfaces have been deprecated" )]] bool
    ReadMem( uint64_t Addr, size_t Len, void* Data );

  [[deprecated( "ReadU* interfaces have been deprecated" )]] uint64_t
    ReadU64( uint64_t Addr ) {
    uint64_t Value;
    if( !ReadMem( Addr, sizeof( Value ), &Value ) )
      output->fatal( CALL_INFO, -1, "Error: could not read memory (U64)\n" );
    return Value;
  }

  // ----------------------------------------------------
  // ---- Read Memory Interfaces
  // ----------------------------------------------------
  /// RevMem: template read memory interface
  template< typename T >
  bool ReadVal( unsigned      Hart,
                uint64_t      Addr,
                T*            Target,
                const MemReq& req,
                RevFlag       flags ) {
    return ReadMem( Hart, Addr, sizeof( T ), Target, req, flags );
  }

  ///  RevMem: template LOAD RESERVE memory interface
  template< typename T >
  bool LR( unsigned      Hart,
           uint64_t      Addr,
           T*            Target,
           uint8_t       aq,
           uint8_t       rl,
           const MemReq& req,
           RevFlag       flags ) {
    return LRBase( Hart, Addr, sizeof( T ), Target, aq, rl, req, flags );
  }

  ///  RevMem: template STORE CONDITIONAL memory interface
  template< typename T >
  bool SC( unsigned Hart,
           uint64_t Addr,
           T*       Data,
           T*       Target,
           uint8_t  aq,
           uint8_t  rl,
           RevFlag  flags ) {
    return SCBase( Hart, Addr, sizeof( T ), Data, Target, aq, rl, flags );
  }

  /// RevMem: template AMO memory interface
  template< typename T >
  bool AMOVal( unsigned      Hart,
               uint64_t      Addr,
               T*            Data,
               T*            Target,
               const MemReq& req,
               RevFlag       flags ) {
    return AMOMem( Hart, Addr, sizeof( T ), Data, Target, req, flags );
  }

  // ----------------------------------------------------
  // ---- Write Memory Interfaces
  // ----------------------------------------------------

  template< typename T >
  void Write( unsigned Hart, uint64_t Addr, T Value ) {
    if( std::is_same_v< T, float > ) {
      memStats.floatsWritten++;
    } else if( std::is_same_v< T, double > ) {
      memStats.doublesWritten++;
    }

    if( !WriteMem( Hart, Addr, sizeof( T ), &Value ) ) {
      output->fatal( CALL_INFO,
                     -1,
                     std::is_floating_point_v< T > ?
                       "Error: could not write memory (FP%zu)\n" :
                       "Error: could not write memory (U%zu)\n",
                     sizeof( T ) * 8 );
    }
  }

  // ----------------------------------------------------
  // ---- Atomic/Future/LRSC Interfaces
  // ----------------------------------------------------
  /// RevMem: Add a memory reservation for the target address
  bool     LRBase( unsigned      Hart,
                   uint64_t      Addr,
                   size_t        Len,
                   void*         Data,
                   uint8_t       aq,
                   uint8_t       rl,
                   const MemReq& req,
                   RevFlag       flags );

  /// RevMem: Clear a memory reservation for the target address
  bool     SCBase( unsigned Hart,
                   uint64_t Addr,
                   size_t   Len,
                   void*    Data,
                   void*    Target,
                   uint8_t  aq,
                   uint8_t  rl,
                   RevFlag  flags );

  /// RevMem: Initiated an AMO request
  bool     AMOMem( unsigned      Hart,
                   uint64_t      Addr,
                   size_t        Len,
                   void*         Data,
                   void*         Target,
                   const MemReq& req,
                   RevFlag       flags );

  /// RevMem: Initiates a future operation [RV64P only]
  bool     SetFuture( uint64_t Addr );

  /// RevMem: Revokes a future operation [RV64P only]
  bool     RevokeFuture( uint64_t Addr );

  /// RevMem: Interrogates the target address and returns 'true' if a future reservation is present [RV64P only]
  bool     StatusFuture( uint64_t Addr );

  /// RevMem: Randomly assign a memory cost
  unsigned RandCost( unsigned Min, unsigned Max ) {
    return RevRand( Min, Max );
  }

  /// RevMem: Used to set the size of the TLBSize
  void SetTLBSize( unsigned numEntries ) {
    tlbSize = numEntries;
  }

  /// RevMem: Used to set the size of the TLBSize
  void SetMaxHeapSize( const unsigned MaxHeapSize ) {
    maxHeapSize = MaxHeapSize;
  }

  /// RevMem: Get memSize value set in .py file
  uint64_t GetMemSize() const {
    return memSize;
  }

  /// RevMem: Sets the next stack top address
  void SetNextThreadMemAddr( const uint64_t NextAddr ) {
    NextThreadMemAddr = NextAddr;
  }

  ///< RevMem: Get MemSegs vector
  std::vector< std::shared_ptr< MemSegment > >& GetStaticMemSegs() {
    return StaticMemSegs;
  }

  ///< RevMem: Get memory mapped MemSegs vector
  std::vector< std::shared_ptr< MemSegment > >& GetMmapMemSegs() {
    return MMapSegs;
  }

  ///< RevMem: Get ThreadMemSegs vector
  std::vector< std::shared_ptr< MemSegment > >& GetThreadMemSegs() {
    return ThreadMemSegs;
  }

  /// RevMem: Add new MemSegment (anywhere) --- Returns BaseAddr of segment
  uint64_t                      AddMemSeg( const uint64_t& SegSize );

  /// TODO: Change
  /// RevMem: Add new thread mem (starting at TopAddr [growing down])
  std::shared_ptr< MemSegment > AddThreadMem();

  uint64_t AddStaticMemSeg( const uint64_t BaseAddr, const uint64_t SegSize );
  uint64_t AddMMapMemSeg( const uint64_t BaseAddr, const uint64_t SegSize );

  /// RevMem: Removes or shrinks segment
  uint64_t AllocMem( const uint64_t Size );

  /// RevMem: Sets the HeapStart & HeapEnd to EndOfStaticData
  void     InitHeap( const uint64_t EndOfStaticData );

  const uint64_t& GetHeapEnd() {
    return heapend;
  }

  uint64_t ExpandHeap( const uint64_t Size );

  void     SetTLSInfo( const uint64_t BaseAddr, const uint64_t Size );

  // RevMem: Used to get the TLS BaseAddr & Size
  uint64_t GetTLSBaseAddr() {
    return TLSBaseAddr;
  }

  uint64_t GetTLSSize() {
    return TLSSize;
  }

  struct RevMemStats {
    uint64_t TLBHits;
    uint64_t TLBMisses;
    uint64_t floatsRead;
    uint64_t floatsWritten;
    uint64_t doublesRead;
    uint64_t doublesWritten;
    uint64_t bytesRead;
    uint64_t bytesWritten;
  };

  RevMemStats GetAndClearStats() {
    // Add each field from memStats into memStatsTotal
    for( auto stat : { &RevMemStats::TLBHits,
                       &RevMemStats::TLBMisses,
                       &RevMemStats::floatsRead,
                       &RevMemStats::floatsWritten,
                       &RevMemStats::doublesRead,
                       &RevMemStats::doublesWritten,
                       &RevMemStats::bytesRead,
                       &RevMemStats::bytesWritten } ) {
      memStatsTotal.*stat += memStats.*stat;
    }

    auto ret = memStats;
    memStats = {};  // Zero out memStats
    return ret;
  }

  RevMemStats GetMemStatsTotal() const {
    return memStatsTotal;
  }

  uint64_t AlignUp( const uint64_t Addr, const uint64_t Align ) const;
  uint64_t AlignDown( const uint64_t Addr, const uint64_t Align ) const;

  // TODO: maybe move to RevHeap
  uint64_t GetBRKValueAddr() {
    return BRKValueAddr;
  }

  void sbrk( int64_t increment ) {
    Heap->sbrk( increment );
  }

protected:
  char*                      physMem = nullptr;  ///< RevMem: memory container
  // TODO: Potentially unique_ptr
  std::shared_ptr< RevHeap > Heap;  ///< RevMem: heap object

private:
  RevMemStats   memStats      = {};
  RevMemStats   memStatsTotal = {};

  unsigned long memSize;      ///< RevMem: size of the target memory
  unsigned      tlbSize;      ///< RevMem: number of entries in the TLB
  unsigned      maxHeapSize;  ///< RevMem: maximum size of the heap
  std::unordered_map< uint64_t,
                      std::pair< uint64_t, std::list< uint64_t >::iterator > >
    TLB;
  std::list< uint64_t >
    LRUQueue;  ///< RevMem: List ordered by last access for implementing LRU policy when TLB fills up
  RevOpts*     opts;    ///< RevMem: options object
  RevMemCtrl*  ctrl;    ///< RevMem: memory controller object
  SST::Output* output;  ///< RevMem: output handler

  std::vector< std::shared_ptr< MemSegment > >
    StaticMemSegs;  ///< RevMem: TODO: update comment
  std::vector< std::shared_ptr< MemSegment > >
    MMapSegs;  ///< RevMem: TODO: update comment

  // TODO: Change to be a single mem segment
  std::vector< std::shared_ptr< MemSegment > >
    ThreadMemSegs;  // For each RevThread there is a corresponding MemSeg that contains TLS & Stack

  uint64_t TLSBaseAddr;  ///< RevMem: TLS Base Address
  uint64_t TLSSize = sizeof(
    uint32_t );  ///< RevMem: TLS Size (minimum size is enough to write the TID)
  uint64_t ThreadMemSize =
    _STACK_SIZE_;  ///< RevMem: Size of a thread's memory segment (StackSize + TLSSize)

  /// TODO: Write this to backing store instead
  uint64_t NextThreadMemAddr =
    memSize -
    1024;  ///< RevMem: Next top address for a new thread's memory (starts at the point the 1024 bytes for argc/argv ends)

  uint64_t SearchTLB(
    uint64_t vAddr );  ///< RevMem: Used to check the TLB for an entry
  void AddToTLB(
    uint64_t vAddr,
    uint64_t physAddr );  ///< RevMem: Used to add a new entry to TLB & LRUQueue
  void     FlushTLB();    ///< RevMem: Used to flush the TLB & LRUQueue
  uint64_t CalcPhysAddr(
    uint64_t pageNum,
    uint64_t
      vAddr );  ///< RevMem: Used to calculate the physical address based on virtual address
  bool isValidVirtAddr(
    uint64_t
      vAddr );  ///< RevMem: Used to check if a virtual address exists in MemSegs

  std::map< uint64_t, std::pair< uint32_t, bool > >
    pageMap;  ///< RevMem: map of logical to pair<physical addresses, allocated>
  uint32_t pageSize;   ///< RevMem: size of allocated pages
  uint32_t addrShift;  ///< RevMem: Bits to shift to caclulate page of address
  uint32_t
    nextPage;  ///< RevMem: next physical page to be allocated. Will result in index
  /// nextPage * pageSize into physMem

  uint64_t heapend;       ///< RevMem: top of the stack
  uint64_t heapstart;     ///< RevMem: top of the stack
  uint64_t stacktop = 0;  ///< RevMem: top of the stack
  uint64_t BRK      = 0;  ///< RevMem: TODO: Idk if I like this or not...
  uint64_t BRKValueAddr =
    0;  ///< RevMem: address of the BRK value (This should be the same across any rev executing the same binary)

  std::vector< uint64_t > FutureRes;  ///< RevMem: future operation reservations

  // these are LRSC tuple index macros
#define LRSC_HART 0
#define LRSC_ADDR 1
#define LRSC_AQRL 2
#define LRSC_VAL  3
  std::vector< std::tuple< unsigned, uint64_t, unsigned, uint64_t* > >
    LRSC;  ///< RevMem: load reserve/store conditional vector

};  // class RevMem

}  // namespace SST::RevCPU

#endif

// EOF
