//
// _RevProcCtx_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once

#ifndef _SST_REVCPU_REVPROCCTX_H_
#define _SST_REVCPU_REVPROCCTX_H_

#include "RevInstTable.h"

class RevProcCtx{

      public:
        RevProcCtx() = default;
        RevProcCtx(uint16_t tid,  uint16_t pid, uint64_t procStartAddr,  RevRegFile& parentRegFile,  uint64_t parentSP )
        : tID(tid), pID(pid), ProcStartAddr(procStartAddr), ParentRegFile(parentRegFile), ParentSP(parentSP) {

          std::copy_n(parentRegFile.RV32, _REV_NUM_REGS_, ParentRegFile.RV32);    ///< RevRegFile: RV32I register file
          std::copy_n(parentRegFile.RV64,_REV_NUM_REGS_, ParentRegFile.RV64);    ///< RevRegFile: RV64I register file
          std::copy_n(parentRegFile.SPF, _REV_NUM_REGS_, ParentRegFile.SPF);    ///< RevRegFile: SPFI register file
          std::copy_n(parentRegFile.DPF, _REV_NUM_REGS_, ParentRegFile.DPF);    ///< RevRegFile: DPFI register file

          std::copy_n(parentRegFile.RV32_Scoreboard,_REV_NUM_REGS_, ParentRegFile.RV32_Scoreboard);    ///< RevRegFile: RV32_ScoreboardI register file
          std::copy_n(parentRegFile.RV64_Scoreboard,_REV_NUM_REGS_, ParentRegFile.RV64_Scoreboard);    ///< RevRegFile: RV64_ScoreboardI register file
          std::copy_n(parentRegFile.SPF_Scoreboard, _REV_NUM_REGS_, ParentRegFile.SPF_Scoreboard);    ///< RevRegFile: SPF_ScoreboardI register file
          std::copy_n(parentRegFile.DPF_Scoreboard, _REV_NUM_REGS_, ParentRegFile.DPF_Scoreboard);    ///< RevRegFile: DPF_ScoreboardI register file

        }
        uint16_t tID; 
        uint16_t pID; 
        uint64_t ProcStartAddr; 
        RevRegFile ParentRegFile; 
        uint64_t ParentSP; 
    };
    

#endif
