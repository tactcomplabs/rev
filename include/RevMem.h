//
// _RevMem_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_REVMEM_H_
#define _SST_REVCPU_REVMEM_H_

#define __PAGE_SIZE__ 4096

// -- C++ Headers
#include <ctime>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <random>
#include <tuple>

// -- SST Headers
#include <sst/core/sst_config.h>
#include <sst/core/component.h>
#include <sst/core/interfaces/stdMem.h>

// -- RevCPU Headers
#include "RevOpts.h"
#include "RevMemCtrl.h"

#ifndef _REVMEM_BASE_
#define _REVMEM_BASE_ 0x00000000
#endif

#define REVMEM_FLAGS(x) ((StandardMem::Request::flags_t)(x))

#define _INVALID_ADDR_ 0xFFFFFFFFFFFFFFFF

#define _STACK_SIZE_ (1024*1024*sizeof(char))

using namespace SST::RevCPU;

namespace SST {
  namespace RevCPU {

    class RevMem {
    public:
      /// RevMem: standard constructor
      RevMem( unsigned long MemSize, RevOpts *Opts, SST::Output *Output );

      /// RevMem: standard memory controller constructor
      RevMem( unsigned long MemSize, RevOpts *Opts, RevMemCtrl *Ctrl, SST::Output *Output );

      /// RevMem: standard destructor
      ~RevMem();

      /* Virtual Memory Blocks  */
      class MemSegment {
      public:
          MemSegment(uint64_t baseAddr, uint64_t size)
              : BaseAddr(baseAddr), Size(size) {
                    TopAddr = baseAddr + size;
                    } // Permissions(permissions) {}

          uint64_t getTopAddr() const { return BaseAddr + Size; }
          uint64_t getBaseAddr() const { return BaseAddr; }
          uint64_t getSize() const { return Size; }

          void setBaseAddr(uint64_t baseAddr) {
            BaseAddr = baseAddr;
            if( Size ){
              TopAddr = Size + BaseAddr;
            }
          }

          void setSize(uint64_t size) { Size = size; TopAddr = BaseAddr + size; }

          /// MemSegment: Check if vAddr is included in this segment
          bool contains(const uint64_t& vAddr){
            return (vAddr >= BaseAddr && vAddr < TopAddr);
          };

          // Check if a given range is inside a segment
          bool contains(const uint64_t& vBaseAddr, const uint64_t& Size){
            // exclusive top address
            uint64_t vTopAddr = vBaseAddr + Size - 1;
            return (this->contains(vBaseAddr) && this->contains(vTopAddr));
          };


      // Override for easy std::cout << *Seg << std::endl;
      friend std::ostream& operator<<(std::ostream& os, MemSegment& obj) {
        std::cout << "---------------------------------------------------------------" << std::endl;
        return os << " | 0x" << std::hex << obj.getBaseAddr()
                  << " | 0x" << std::hex << obj.getTopAddr()
                  << " | Size = " << std::dec << obj.getSize();
      }

      private:
          uint64_t BaseAddr;
          uint64_t Size;
          uint64_t TopAddr;
      };

      /// RevMem: determine if there are any outstanding requests
      bool outstandingRqsts();

      /// RevMem: handle incoming memory event
      void handleEvent(Interfaces::StandardMem::Request* ev) { }

      /// RevMem: handle memory injection
      void HandleMemFault(unsigned width);

      /// RevMem: get the stack_top address
      uint64_t GetStackTop() { return stacktop; }

      /// RevMem: set the stack_top address
      void SetStackTop(uint64_t Addr) { stacktop = Addr; }

      /// RevMem: retrieve the address of the top of memory (not stack)
      uint64_t GetMemTop() { return (_REVMEM_BASE_ + memSize); }

      /// RevMem: get the stack_top address
      uint64_t GetStackBottom() { return stacktop - _STACK_SIZE_; }

      /// RevMem: initiate a memory fence
      bool FenceMem(unsigned Hart);

      /// RevMem: retrieves the cache line size.  Returns 0 if no cache is configured
      unsigned getLineSize(){ if( ctrl ){return ctrl->getLineSize();}else{return 64;} }

      // ----------------------------------------------------
      // ---- Base Memory Interfaces
      // ----------------------------------------------------
      /// RevMem: write to the target memory location
      bool WriteMem( unsigned Hart, uint64_t Addr, size_t Len, void *Data );

      /// RevMem: write to the target memory location with the target flags
      bool WriteMem( unsigned Hart, uint64_t Addr, size_t Len, void *Data,
                     StandardMem::Request::flags_t flags );

      /// RevMem: read data from the target memory location
      bool ReadMem( unsigned Hart, uint64_t Addr, size_t Len, void *Target,
                    bool *Hazard,
                    StandardMem::Request::flags_t flags);

      /// RevMem: DEPRECATED: read data from the target memory location
      [[deprecated("Simple RevMem interfaces have been deprecated")]]
      bool ReadMem( uint64_t Addr, size_t Len, void *Data );

      // ----------------------------------------------------
      // ---- Read Memory Interfaces
      // ----------------------------------------------------
      /// RevMem: template read memory interface
      template <typename T>
      bool ReadVal( unsigned Hart, uint64_t Addr, T *Target,
                    bool *Hazard,
                    StandardMem::Request::flags_t flags){
        return ReadMem(Hart, Addr, sizeof(T), (void *)(Target), Hazard, flags);
      }

      ///  RevMem: template LOAD RESERVE memory interface
      template <typename T>
      bool LR( unsigned Hart, uint64_t Addr, T *Target,
               uint8_t aq, uint8_t rl, bool *Hazard,
               StandardMem::Request::flags_t flags){
        return LRBase(Hart, Addr, sizeof(T), (void *)(Target), aq, rl, Hazard, flags);
      }

      ///  RevMem: template STORE CONDITIONAL memory interface
      template <typename T>
      bool SC( unsigned Hart, uint64_t Addr, T *Data, T *Target,
               uint8_t aq, uint8_t rl,
               StandardMem::Request::flags_t flags){
        return SCBase(Hart, Addr, sizeof(T), (void *)(Data), (void *)(Target), aq, rl, flags);
      }

      /// RevMem: template AMO memory interface
      template <typename T>
      bool AMOVal( unsigned Hart, uint64_t Addr, T *Data, T *Target,
                   bool *Hazard,
                   StandardMem::Request::flags_t flags){
        return AMOMem(Hart, Addr, sizeof(T), (void *)(Data), (void *)(Target), Hazard, flags);
      }

      /// RevMem: DEPRECATED: Read uint8 from the target memory location
      [[deprecated("Simple RevMem interfaces have been deprecated")]]
      uint8_t ReadU8( uint64_t Addr );

      /// RevMem: DEPRECATED: Read uint16 from the target memory location
      [[deprecated("Simple RevMem interfaces have been deprecated")]]
      uint16_t ReadU16( uint64_t Addr );

      /// RevMem: DEPRECATED: Read uint32 from the target memory location
      [[deprecated("Simple RevMem interfaces have been deprecated")]]
      uint32_t ReadU32( uint64_t Addr );

      /// RevMem: DEPRECATED: Read uint64 from the target memory location
      [[deprecated("Simple RevMem interfaces have been deprecated")]]
      uint64_t ReadU64( uint64_t Addr );

      /// RevMem: DEPRECATED: Read float from the target memory location
      [[deprecated("Simple RevMem interfaces have been deprecated")]]
      float ReadFloat( uint64_t Addr );

      /// RevMem: DEPRECATED: Read double from the target memory location
      [[deprecated("Simple RevMem interfaces have been deprecated")]]
      double ReadDouble( uint64_t Addr );

      // ----------------------------------------------------
      // ---- Write Memory Interfaces
      // ----------------------------------------------------
      /// RevMem: Write a uint8 to the target memory location
      void WriteU8( unsigned Hart, uint64_t Addr, uint8_t Value );

      /// RevMem: Write a uint16 to the target memory location
      void WriteU16( unsigned Hart, uint64_t Addr, uint16_t Value );

      /// RevMem: Write a uint32 to the target memory location
      void WriteU32( unsigned Hart, uint64_t Addr, uint32_t Value );

      /// RevMem: Write a uint64 to the target memory location
      void WriteU64( unsigned Hart, uint64_t Addr, uint64_t Value );

      /// RevMem: Write a float to the target memory location
      void WriteFloat( unsigned Hart, uint64_t Addr, float Value );

      /// RevMem: Write a double to the target memory location
      void WriteDouble( unsigned Hart, uint64_t Addr, double Value );

      // ----------------------------------------------------
      // ---- Atomic/Future/LRSC Interfaces
      // ----------------------------------------------------
      /// RevMem: Add a memory reservation for the target address
      bool LRBase(unsigned Hart, uint64_t Addr, size_t Len,
                  void *Data, uint8_t aq, uint8_t rl, bool *Hazard,
                  StandardMem::Request::flags_t flags);

      /// RevMem: Clear a memory reservation for the target address
      bool SCBase(unsigned Hart, uint64_t Addr, size_t Len,
                  void *Data, void *Target, uint8_t aq, uint8_t rl,
                  StandardMem::Request::flags_t flags);

      /// RevMem: Initiated an AMO request
      bool AMOMem(unsigned Hart, uint64_t Addr, size_t Len,
                  void *Data, void *Target, bool *Hazard,
                  StandardMem::Request::flags_t flags);

      /// RevMem: Initiates a future operation [RV64P only]
      bool SetFuture( uint64_t Addr );

      /// RevMem: Revokes a future operation [RV64P only]
      bool RevokeFuture( uint64_t Addr );

      /// RevMem: Interrogates the target address and returns 'true' if a future reservation is present [RV64P only]
      bool StatusFuture( uint64_t Addr );

      /// RevMem: Randomly assign a memory cost
      unsigned RandCost( unsigned Min, unsigned Max );

      /// RevMem: Used to access & incremenet the global software PID counter
      uint32_t GetNewThreadPID();

      /// RevMem: Used to set the size of the TLBSize
      void SetTLBSize(unsigned numEntries){ tlbSize = numEntries; }

      /// RevMem: Used to set the size of the TLBSize
      void SetMaxHeapSize(const unsigned MaxHeapSize){ maxHeapSize = MaxHeapSize; }

      /// RevMem: Get memSize value set in .py file
      const uint64_t GetMemSize(){ return memSize; }

      ///< RevMem: Get MemSegs vector
      std::vector<std::shared_ptr<MemSegment>>& GetMemSegs(){ return MemSegs; } 

      ///< RevMem: Get FreeMemSegs vector
      std::vector<std::shared_ptr<MemSegment>>& GetFreeMemSegs(){ return FreeMemSegs; } 

      /// RevMem: Add new MemSegment (anywhere) --- Returns BaseAddr of segment
      uint64_t AddMemSeg(const uint64_t& SegSize);

      /// RevMem: Add new MemSegment (starting at BaseAddr)
      uint64_t AddMemSegAt(const uint64_t& BaseAddr, const uint64_t& SegSize);

      /// RevMem: Add new MemSegment (starting at BaseAddr) and round it up to the nearest page
      uint64_t AddRoundedMemSeg(uint64_t BaseAddr, const uint64_t& SegSize, size_t RoundUpSize);

      /// RevMem: Removes or shrinks segment
      uint64_t DeallocMem(uint64_t BaseAddr, uint64_t Size);

      /// RevMem: Removes or shrinks segment
      uint64_t AllocMem(const uint64_t& Size);

      /// RevMem: Attempts to allocate memory at a specific address
      uint64_t AllocMemAt(const uint64_t& BaseAddr, const uint64_t& Size);

      /// RevMem: Sets the HeapStart & HeapEnd to EndOfStaticData
      void InitHeap(const uint64_t& EndOfStaticData);
      void SetHeapStart(const uint64_t& HeapStart){ heapstart = HeapStart; }
      void SetHeapEnd(const uint64_t& HeapEnd){ heapend = HeapEnd; }
      const uint64_t& GetHeapEnd(){ return heapend; }

      uint64_t ExpandHeap(uint64_t Size);

      void SetTLSInfo(const uint64_t& BaseAddr, const uint64_t& Size);

      // RevMem: Used to get the TLS BaseAddr & Size
      const uint64_t& GetTLSBaseAddr(){ return TLSBaseAddr; }
      const uint64_t& GetTLSSize(){ return TLSSize; }

    class RevMemStats {
    public:
      uint64_t TLBHits;
      uint64_t TLBMisses;
      uint32_t floatsRead;
      uint32_t floatsWritten;
      uint32_t doublesWritten;
      uint32_t doublesRead;
      uint32_t bytesRead;
      uint32_t bytesWritten;
    };

    RevMemStats memStats;

    protected:
      char *physMem;                          ///< RevMem: memory container

    private:
      std::unordered_map<uint64_t, std::pair<uint64_t, std::list<uint64_t>::iterator>> TLB;
      std::list<uint64_t> LRUQueue; ///< RevMem: List ordered by last access for implementing LRU policy when TLB fills up
      std::vector<std::shared_ptr<MemSegment>> MemSegs;     // Currently Allocated MemSegs
      std::vector<std::shared_ptr<MemSegment>> FreeMemSegs; // MemSegs that have been unallocated
  
      uint64_t TLSBaseAddr = 0x0;
      uint64_t TLSSize = 0x0;
      

      unsigned long memSize;        ///< RevMem: size of the target memory
      unsigned tlbSize;             ///< RevMem: size of the target memory
      unsigned maxHeapSize;             ///< RevMem: size of the target memory
      RevOpts *opts;                ///< RevMem: options object
      RevMemCtrl *ctrl;             ///< RevMem: memory controller object
      SST::Output *output;          ///< RevMem: output handler

      uint64_t SearchTLB(uint64_t vAddr);                       ///< RevMem: Used to check the TLB for an entry
      void AddToTLB(uint64_t vAddr, uint64_t physAddr);         ///< RevMem: Used to add a new entry to TLB & LRUQueue
      void FlushTLB();                                          ///< RevMem: Used to flush the TLB & LRUQueue
      uint64_t CalcPhysAddr(uint64_t pageNum, uint64_t vAddr);  ///< RevMem: Used to calculate the physical address based on virtual address
      bool isValidVirtAddr(const uint64_t vAddr);               ///< RevMem: Used to check if a virtual address exists in MemSegs

      uint32_t PIDCount = 1023;   ///< RevMem: Monotonically increasing PID counter for assigning new PIDs without conflicts

      std::map<uint64_t, std::pair<uint32_t, bool>> pageMap;   ///< RevMem: map of logical to pair<physical addresses, allocated>
      uint32_t                                      pageSize;  ///< RevMem: size of allocated pages
      uint32_t                                      addrShift; ///< RevMem: Bits to shift to caclulate page of address 
      uint32_t                                      nextPage;  ///< RevMem: next physical page to be allocated. Will result in index 
                                                                    /// nextPage * pageSize into physMem

      uint64_t heapend;        ///< RevMem: top of the stack
      uint64_t heapstart;        ///< RevMem: top of the stack
      uint64_t stacktop;        ///< RevMem: top of the stack

      std::vector<uint64_t> FutureRes;  ///< RevMem: future operation reservations

      // these are LRSC tuple index macros
      #define LRSC_HART 0
      #define LRSC_ADDR 1
      #define LRSC_AQRL 2
      #define LRSC_VAL  3
      std::vector<std::tuple<unsigned,uint64_t,
                             unsigned,uint64_t*>> LRSC;   ///< RevMem: load reserve/store conditional vector

    }; // class RevMem
  } // namespace RevCPU
} // namespace SST

#endif

// EOF
