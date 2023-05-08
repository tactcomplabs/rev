//
// _RevProc_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_REVPROC_H_
#define _SST_REVCPU_REVPROC_H_

// -- SST Headers
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
#include <optional>
#include <inttypes.h>
#include <functional>

// -- RevCPU Headers
#include "RevOpts.h"
#include "RevMem.h"
#include "RevFeature.h"
#include "RevLoader.h"
#include "RevInstTable.h"
#include "RevInstTables.h"
#include "PanExec.h"
#include "RevPrefetcher.h"
#include "RevThreadCtx.h"

/* NOTE:
 * The following are included to allow for a notion of "Priviledge"
 * Eventually adding full CSR support (ie. Supervisor Mode, Machine Mode, etc.)
 * will be better but by including the ECALL implementations in RevProc gives access
 * RevProc and allows for higher priviledge access to operate on Ctx objects
*/
// #include "RevSysCallInterface.h"
// #include "RevSysCalls.h"

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

      /// RevProc: Initialize ThreadTable & First Thread
      bool InitThreadTable();

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

      RevMem& GetMem(){ return *mem; }

      /* Software Process Table */
      /* NOTE: Not all of these are needed/work */
      bool AddCtx(RevThreadCtx& Ctx);
      uint32_t CreateChildCtx(); // returns the childs pid
      ThreadState GetThreadState(uint32_t pid);
      bool SetState(ThreadState, uint32_t pid);
      bool PauseThread(uint32_t pid);
      bool ReadyThread(uint32_t pid);

      /* Returns the current HartToExec active pid */
      uint32_t GetActivePID(){ return ActivePIDs.at(HartToExec); } 
      /* Returns the active pid for HartID */
      uint32_t GetActivePID(const uint32_t HartID){ return ActivePIDs.at(HartID); } 

      /* Returns the current HartToExec active pid */
      // void UpdateRegFile(const uint32_t NewPID){ RegFiles.at(HartToExec) = ThreadTable.at(NewPID).GetRegFile();}

      /* Returns the active pid for HartID */
      // RevRegFile* GetRegFile(uint32_t pid){ return ThreadTable.at(pid).RegFile; }

      const char&& GetThreadMemRef(uint32_t pid);
  
      std::vector<uint32_t> GetPIDs();

      uint32_t RetireAndSwap(); // Returns new pid
      
      bool UpdateCtx(); // FIXME: This will change when RegFiles get changed to pointers
      
      std::unordered_map<uint32_t, std::shared_ptr<RevThreadCtx>> ThreadTable; ///< RevProc: PIDs & corresponding RevThreadCtx objects (Software Threads)
      
      bool LoadCtx(uint32_t pid);
    
    //   RevThreadCtx& GetCtx(uint32_t PID){ return ThreadTable.at(PID); }
    //   RevThreadCtx& GetActiveCtx(){ return ThreadTable.at(ActivePIDs.at(HartToExec));}; ///< RevProc: Get the active ThreadCtx of HartToExec
    //   RevThreadCtx& GetActiveCtx(uint8_t HartID){return ThreadTable.at(ActivePIDs.at(HartID)); } ///< RevProc: Get the active ThreadCtx of HartID

      void CtxSwitchAlert(uint32_t NewPID) { NextPID=NewPID;PendingCtxSwitch = true; }

      uint32_t HartToExecActivePID();
      uint32_t HartToDecodeActivePID();

      std::shared_ptr<RevThreadCtx> HartToExecActiveCtx();
      std::shared_ptr<RevThreadCtx> HartToDecodeActiveCtx();

      RevRegFile* HartToExecRegFile();
      RevRegFile* HartToDecodeRegFile();
      
      bool ChangeActivePID(uint32_t PID); ///< RevProc: Change HartToExec active pid
      bool ChangeActivePID(uint32_t PID, uint16_t HartID); ///< RevProc: Change HartID active pid

    private:
      bool Halted;              ///< RevProc: determines if the core is halted
      bool Stalled;             ///< RevProc: determines if the core is stalled on instruction fetch
      bool SingleStep;          ///< RevProc: determines if we are in a single step
      bool CrackFault;          ///< RevProc: determiens if we need to handle a crack fault
      bool ALUFault;            ///< RevProc: determines if we need to handle an ALU fault
      unsigned fault_width;     ///< RevProc: the width of the target fault
      unsigned id;              ///< RevProc: processor id
      uint64_t ExecPC;          ///< RevProc: executing PC
      uint16_t HartToDecode;   ///< RevProc: Current executing ThreadID
      uint16_t HartToExec;     ///< RevProc: Thread to dispatch instruction
      uint64_t Retired;         ///< RevProc: number of retired instructions
      bool PendingCtxSwitch = false; ///< RevProc: determines if the core is halted
      bool SwapToParent = false; ///< RevProc: determines if the core is halted
      uint32_t NextPID = 0; 

      RevOpts *opts;            ///< RevProc: options object
      RevMem *mem;              ///< RevProc: memory object
      RevLoader *loader;        ///< RevProc: loader object
      SST::Output *output;      ///< RevProc: output handler
      RevFeature *feature;      ///< RevProc: feature handler
      PanExec *PExec;           ///< RevProc: PAN exeuction context
      RevProcStats Stats;       ///< RevProc: collection of performance stats
      RevPrefetcher *sfetch;    ///< RevProc: stream instruction prefetcher

      /*
      * New Ecall Stuff
      */
      void ECALL_clone();
      void ECALL_sync();
      void ECALL_chown();
      void ECALL_getcwd();
      void ECALL_chdir();
      void ECALL_mkdir();
      void ECALL_write();
      void ECALL_exit();
      void ECALL_sigprocrtmask(); // FIXME: Double check name
      void ECALL_rev99(); // FIXME: Figure out what this actually is (ie. what syscall has code = 99)

      std::unordered_map<uint32_t, std::function<void(RevProc*)>> Ecalls;
      void InitEcallTable();
      void ExecEcall();

      // std::vector<RevRegFile*> RegFiles; // TODO: Maybe rename
      RevRegFile* RegFile(uint16_t HartID);
      // Return current executing HART regfile
      RevRegFile* RegFile();
      
      std::vector<uint32_t> ActivePIDs;

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

      /// RevProc: Determine next thread to execute
      uint16_t GetHartID();

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
