//
// _RevProc_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#pragma once

#ifndef _SST_REVCPU_REVPROC_H_
#define _SST_REVCPU_REVPROC_H_

// -- SST Headers
#include <algorithm>
#include <cstdint>
#include <iterator>
#include <map>
#include <sst/core/sst_config.h>
#include <sst/core/component.h>
#include <sst/core/statapi/stataccumulator.h>


// -- Standard Headers
#include <iostream>
#include <fstream>
#include <bitset>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <random>
#include <queue>
#include <inttypes.h>
#include <utility>

// -- RevCPU Headers
#include "RevOpts.h"
#include "RevMem.h"
#include "RevFeature.h"
#include "RevLoader.h"
#include "RevInstTable.h"
#include "RevInstTables.h"
#include "PanExec.h"
#include "RevPrefetcher.h"
#include "RevProcCtx.h"

#define _PAN_FWARE_JUMP_            0x0000000000010000

namespace SST::RevCPU {
  class RevProc;
}

using namespace SST::RevCPU;

namespace SST{
  namespace RevCPU{

  class RevProc{
    public:
      /// RevProc: standard constructor
      RevProc( unsigned Id, RevOpts *Opts, RevMem *Mem, RevLoader *Loader,
               SST::Output *Output );

      /// RevProc: standard desctructor
      ~RevProc();

      /// RevProc: per-processor clock function
      bool ClockTick( SST::Cycle_t currentCycle );

      /// RevProc: halt the CPU
      bool Halt();

      /// RevProc: resume the CPU
      bool Resume();

      /// RevProc: execute a single step
      bool SingleStepHart();

      /// RevProc: retrieve the local PC for the correct feature set
      uint64_t GetPC();

      /// RevProc: Debug mode read a register
      bool DebugReadReg(unsigned Idx, uint64_t *Value);

      /// RevProc: Debug mode write a register
      bool DebugWriteReg(unsigned Idx, uint64_t Value);

      /// RevProc: Is this an RV32 machine?
      bool DebugIsRV32() { return feature->IsRV32(); }

      /// RevProc: Set the PAN execution context
      void SetExecCtx(PanExec *P) { PExec = P; }

      /// RevProc: Retrieve a random memory cost value
      unsigned RandCost() { return mem->RandCost(feature->GetMinCost(),feature->GetMaxCost()); }

      /// RevProc: Handle register faults
      void HandleRegFault(unsigned width);

      /// RevProc: Handle crack+decode faults
      void HandleCrackFault(unsigned width);

      /// RevProc: Handle ALU faults
      void HandleALUFault(unsigned width);

      class RevProcStats {
        public:
          uint64_t totalCycles;
          uint64_t cyclesBusy;
          uint64_t cyclesIdle_Total;
          uint64_t cyclesStalled;
          uint64_t floatsExec;
          float    percentEff;
          RevMem::RevMemStats memStats;
          uint64_t cyclesIdle_Pipeline;
          uint64_t cyclesIdle_MemoryFetch;
      };

      RevProcStats GetStats();

    private:
      bool Halted;              ///< RevProc: determines if the core is halted
      bool Stalled;             ///< RevProc: determines if the core is stalled on instruction fetch
      bool SingleStep;          ///< RevProc: determines if we are in a single step
      bool CrackFault;          ///< RevProc: determiens if we need to handle a crack fault
      bool ALUFault;            ///< RevProc: determines if we need to handle an ALU fault
      unsigned fault_width;     ///< RevProc: the width of the target fault
      unsigned id;              ///< RevProc: processor id
      uint64_t ExecPC;          ///< RevProc: executing PC
      uint8_t threadToDecode;   ///< RevProc: Current executing ThreadID
      uint8_t threadToExec;     ///< RevProc: Thread to dispatch instruction
      uint64_t Retired;         ///< RevProc: number of retired instructions

      RevOpts *opts;            ///< RevProc: options object
      RevMem *mem;              ///< RevProc: memory object
      RevLoader *loader;        ///< RevProc: loader object
      SST::Output *output;      ///< RevProc: output handler
      RevFeature *feature;      ///< RevProc: feature handler
      PanExec *PExec;           ///< RevProc: PAN exeuction context
      RevProcStats Stats;       ///< RevProc: collection of performance stats
      RevPrefetcher *sfetch;    ///< RevProc: stream instruction prefetcher

public:
      RevRegFile RegFile[_REV_THREAD_COUNT_];      ///< RevProc: register file
private:
      RevInst Inst;             ///< RevProc: instruction payload

      std::vector<RevInstEntry> InstTable;        ///< RevProc: target instruction table

      std::vector<RevExt *> Extensions;           ///< RevProc: vector of enabled extensions

      std::queue<std::pair<uint16_t, RevInst>>   Pipeline; ///< RevProc: pipeline of instructions - bypass paths not supported

      std::map<std::string,unsigned> NameToEntry; ///< RevProc: instruction mnemonic to table entry mapping
      std::map<uint32_t,unsigned> EncToEntry;     ///< RevProc: instruction encoding to table entry mapping
      std::map<uint32_t,unsigned> CEncToEntry;    ///< RevProc: compressed instruction encoding to table entry mapping

      std::map<unsigned,std::pair<unsigned,unsigned>> EntryToExt;     ///< RevProc: instruction entry to extension object mapping
                                                                      ///           first = Master table entry number
                                                                      ///           second = pair<Extension Index, Extension Entry>

      /// RevProc: splits a string into tokens
      void splitStr(const std::string& s, char c, std::vector<std::string>& v);

      /// RevProc: parses the feature string for the target core
      bool ParseFeatureStr(std::string Feature);

      /// RevProc: loads the instruction table using the target features
      bool LoadInstructionTable();

      /// RevProc: see the instruction table the target features
      bool SeedInstTable();

      /// RevProc: enable the target extension by merging its instruction table with the master
      bool EnableExt(RevExt *Ext, bool Opt);

      /// RevProc: initializes the internal mapping tables
      bool InitTableMapping();

      /// RevProc: read in the user defined cost tables
      bool ReadOverrideTables();

      /// RevProc: compresses the encoding structure to a single value
      uint32_t CompressEncoding(RevInstEntry Entry);

      /// RevProc: compressed the compressed encoding structure to a single value
      uint32_t CompressCEncoding(RevInstEntry Entry);

      /// RevProc: extracts the instruction mnemonic from the table entry
      std::string ExtractMnemonic(RevInstEntry Entry);

      /// RevProc: reset the core and its associated features
      bool Reset();

      /// RevProc: set the PC
      void SetPC(uint64_t PC);

      /// RevProc: prefetch the next instruction
      bool PrefetchInst();

      /// RevProc: decode the instruction at the current PC
      RevInst DecodeInst();

      /// RevProc: decode a compressed instruction
      RevInst DecodeCompressed(uint32_t Inst);

      /// RevProc: decode an R-type instruction
      RevInst DecodeRInst(uint32_t Inst, unsigned Entry);

      /// RevProc: decode an I-type instruction
      RevInst DecodeIInst(uint32_t Inst, unsigned Entry);

      /// RevProc: decode an S-type instruction
      RevInst DecodeSInst(uint32_t Inst, unsigned Entry);

      /// RevProc: decode a U-type instruction
      RevInst DecodeUInst(uint32_t Inst, unsigned Entry);

      /// RevProc: decode a B-type instruction
      RevInst DecodeBInst(uint32_t Inst, unsigned Entry);

      /// RevProc: decode a J-type instruction
      RevInst DecodeJInst(uint32_t Inst, unsigned Entry);

      /// RevProc: decode an R4-type instruction
      RevInst DecodeR4Inst(uint32_t Inst, unsigned Entry);

      /// RevProc: decode a compressed CR-type isntruction
      RevInst DecodeCRInst(uint16_t Inst, unsigned Entry);

      /// RevProc: decode a compressed CI-type isntruction
      RevInst DecodeCIInst(uint16_t Inst, unsigned Entry);

      /// RevProc: decode a compressed CSS-type isntruction
      RevInst DecodeCSSInst(uint16_t Inst, unsigned Entry);

      /// RevProc: decode a compressed CIW-type isntruction
      RevInst DecodeCIWInst(uint16_t Inst, unsigned Entry);

      /// RevProc: decode a compressed CL-type isntruction
      RevInst DecodeCLInst(uint16_t Inst, unsigned Entry);

      /// RevProc: decode a compressed CS-type isntruction
      RevInst DecodeCSInst(uint16_t Inst, unsigned Entry);

      /// RevProc: decode a compressed CA-type isntruction
      RevInst DecodeCAInst(uint16_t Inst, unsigned Entry);

      /// RevProc: decode a compressed CB-type isntruction
      RevInst DecodeCBInst(uint16_t Inst, unsigned Entry);

      /// RevProc: decode a compressed CJ-type isntruction
      RevInst DecodeCJInst(uint16_t Inst, unsigned Entry);

      /// RevProc: determine if the instruction is an SP/FP float
      bool IsFloat(unsigned Entry);

      /// RevProc: reset the inst structure
      void ResetInst(RevInst *Inst);

public:
      /// RevProc: Determine next thread to execute
      uint16_t GetThreadID();

      /// RevProc: Context switching utils
      void LoadCtx(RevMem& Mem, const RevProcCtx& ProcCtx);
      void SaveCtx(const RevMem& Mem, RevProcCtx& ProcCtx);

private:

      /// RevProc: Check scoreboard for pipeline hazards
      bool DependencyCheck(uint16_t threadID, RevInst* Inst);

      /// RevProc: Set scoreboard based on instruction destination
      void DependencySet(uint16_t threadID, RevInst* Inst);

      /// RevProc: Clear scoreboard on instruction retirement
      void DependencyClear(uint16_t threadID, RevInst* Inst);

    }; // class RevProc 
    
    
    

  } // namespace RevCPU
} // namespace SST

#endif

// EOF
