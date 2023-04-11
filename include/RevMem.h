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

// -- C++ Headers
#include <ctime>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <random>

// -- SST Headers
#include <sst/core/sst_config.h>
#include <sst/core/component.h>
#include <sst/core/interfaces/stdMem.h>

// -- RevCPU Headers
#include "RevOpts.h"
#include "RevMemCtrl.h"
#include "RevProc.h"

#ifndef _REVMEM_BASE_
#define _REVMEM_BASE_ 0x00000000
#endif

#define REVMEM_FLAGS(x) ((StandardMem::Request::flags_t)(x))

namespace SST::RevCPU {
  class RevMem;
}

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

      /// RevMem: determine if there are any outstanding requests
      bool outstandingRqsts();

      /// RevMem: handle incoming memory event
      void handleEvent(Interfaces::StandardMem::Request* ev) { }

      /// RevMem: handle memory injection
      void HandleMemFault(unsigned width);

      /// RevMem: Set Rev Proc Ctx
      void StoreProcCtx(RevProcCtx& Ctx){
        // Stack Pointer
        Ctx.ParentSP = GetStackTop();
      }
  
      void LoadProcCtx(const RevProcCtx& Ctx){
        // Stack Pointer
        SetStackTop(Ctx.ParentSP);
      }

private:
  
      /// RevMem: get the stack_top address
      uint64_t GetStackTop() { return stacktop; }

      /// RevMem: set the stack_top address
      void SetStackTop(uint64_t Addr) { stacktop = Addr; }

      /// RevMem: initiate a memory fence
      bool FenceMem();

      // ----------------------------------------------------
      // ---- Base Memory Interfaces
      // ----------------------------------------------------
      /// RevMem: write to the target memory location
      bool WriteMem( uint64_t Addr, size_t Len, void *Data );

      /// RevMem: write to the target memory location with the target flags
      bool WriteMem( uint64_t Addr, size_t Len, void *Data,
                     StandardMem::Request::flags_t flags );

      /// RevMem: read data from the target memory location
      bool ReadMem( uint64_t Addr, size_t Len, void *Target,
                    StandardMem::Request::flags_t flags);

      /// RevMem: DEPRECATED: read data from the target memory location
      [[deprecated("Simple RevMem interfaces have been deprecated")]]
      bool ReadMem( uint64_t Addr, size_t Len, void *Data );

      // ----------------------------------------------------
      // ---- Read Memory Interfaces
      // ----------------------------------------------------
      /// RevMem: template read memory interface
      template <typename T>
      bool ReadVal( uint64_t Addr, T *Target,
                    StandardMem::Request::flags_t flags){
        return ReadMem(Addr, sizeof(T), (void *)(Target), flags);
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
      void WriteU8( uint64_t Addr, uint8_t Value );

      /// RevMem: Write a uint16 to the target memory location
      void WriteU16( uint64_t Addr, uint16_t Value );

      /// RevMem: Write a uint32 to the target memory location
      void WriteU32( uint64_t Addr, uint32_t Value );

      /// RevMem: Write a uint64 to the target memory location
      void WriteU64( uint64_t Addr, uint64_t Value );

      /// RevMem: Write a float to the target memory location
      void WriteFloat( uint64_t Addr, float Value );

      /// RevMem: Write a double to the target memory location
      void WriteDouble( uint64_t Addr, double Value );

      // ----------------------------------------------------
      // ---- Atomic/Future/LRSC Interfaces
      // ----------------------------------------------------
      /// RevMem: Add a memory reservation for the target address
      bool LR(unsigned Hart, uint64_t Addr);

      /// RevMem: Clear a memory reservation for the target address
      bool SC(unsigned Hart, uint64_t Addr);

      /// RevMem: Initiates a future operation [RV64P only]
      bool SetFuture( uint64_t Addr );

      /// RevMem: Revokes a future operation [RV64P only]
      bool RevokeFuture( uint64_t Addr );

      /// RevMem: Interrogates the target address and returns 'true' if a future reservation is present [RV64P only]
      bool StatusFuture( uint64_t Addr );

      /// RevMem: Randomly assign a memory cost
      unsigned RandCost( unsigned Min, unsigned Max );

    class RevMemStats {
    public:
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
      unsigned long memSize;    ///< RevMem: size of the target memory
      RevOpts *opts;            ///< RevMem: options object
      RevMemCtrl *ctrl;         ///< RevMem: memory controller object
      SST::Output *output;      ///< RevMem: output handler

      uint64_t CalcPhysAddr(uint64_t pageNum, uint64_t Addr);


      //c++11 should guarentee that these are all zero-initializaed
      std::map<uint64_t, std::pair<uint32_t, bool>> pageMap;   ///< RevMem: map of logical to pair<physical addresses, allocated>
      uint32_t                                      pageSize;  ///< RevMem: size of allocated pages
      uint32_t                                      addrShift; ///< RevMem: Bits to shift to caclulate page of address 
      uint32_t                                      nextPage;  ///< RevMem: next physical page to be allocated. Will result in index 
                                                                    /// nextPage * pageSize into physMem

      uint64_t stacktop;        ///< RevMem: top of the stack

      std::vector<uint64_t> FutureRes;  ///< RevMem: future operation reservations

      std::vector<std::pair<unsigned,uint64_t>> LRSC;   ///< RevMem: load reserve/store conditional vector

    }; // class RevMem
  } // namespace RevCPU
} // namespace SST

#endif

// EOF
