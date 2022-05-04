//
// _RevMem_h_
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
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

// -- RevCPU Headers
#include "RevOpts.h"

#ifndef _REVMEM_BASE_
#define _REVMEM_BASE_ 0x00000000
#endif

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

      /// RevMem: standard destructor
      ~RevMem();

      /// RevMem: handle memory injection
      void HandleMemFault(unsigned width);

      /// RevMem: get the stack_top address
      uint64_t GetStackTop() { return stacktop; }

      /// RevMem: set the stack_top address
      void SetStackTop(uint64_t Addr) { stacktop = Addr; }

      /// RevMem: write to the target memory location
      bool WriteMem( uint64_t Addr, size_t Len, void *Data );

      /// RevMem: read data from the target memory location
      bool ReadMem( uint64_t Addr, size_t Len, void *Data );

      /// RevMem: Read uint8 from the target memory location
      uint8_t ReadU8( uint64_t Addr );

      /// RevMem: Read uint16 from the target memory location
      uint16_t ReadU16( uint64_t Addr );

      /// RevMem: Read uint32 from the target memory location
      uint32_t ReadU32( uint64_t Addr );

      /// RevMem: Read uint64 from the target memory location
      uint64_t ReadU64( uint64_t Addr );

      /// RevMem: Read float from the target memory location
      float ReadFloat( uint64_t Addr );

      /// RevMem: Read double from the target memory location
      double ReadDouble( uint64_t Addr );

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

      /// RevMem: Randomly assign a memory cost
      unsigned RandCost( unsigned Min, unsigned Max );

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

    private:
      unsigned long memSize;    ///< RevMem: size of the target memory
      RevOpts *opts;            ///< RevMem: options object
      SST::Output *output;      ///< RevMem: output handler

      uint64_t CalcPhysAddr(uint64_t pageNum, uint64_t Addr);

      char *physMem;                          ///< RevMem: memory container
      
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
