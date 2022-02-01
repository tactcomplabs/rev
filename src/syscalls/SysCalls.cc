//
// SysCalls.cc
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#include "SysCalls.h"

#include <unordered_map>

namespace SST { namespace RevCPU {

template<typename RiscvArchType>
std::unordered_map<typename SystemCalls<RiscvArchType>::Codes, SystemCallInterface<RiscvArchType>> SystemCalls<RiscvArchType>::jump_table {
        { SystemCalls<RiscvArchType>::Codes::EXIT           , ExitSystemCall<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::EXITGROUP      , ExitGroupSystemCall<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::GETPID         , GetPidSystemCall<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::KILL           , KillSystemCall<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::TGKILL         , TGKillSystemCall<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::READ           , ReadSystemCall<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::WRITE          , WriteSystemCall<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::OPENAT         , OpenAtSystemCall<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::CLOSE          , CloseSystemCall<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::LSEEK          , LseekSystemCall<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::BRK            , BrkSystemCall<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::LINKAT         , LinkAtSystemCall<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::UNLINKAT       , UnlinkAtSystemCall<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::LINK           , LinkSystemCall<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::UNLINK         , UnlinkSystemCall<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::RENAMEAT       , RenameAtSystemCall<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::MKDIRAT        , MkdirAtSystemCall<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::MKDIR          , MkdirSystemCall<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::CHDIR          , ChdirSystemCall<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::GETCWD         , GetcwdSystemCall<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::FSTAT          , FstatSystemCall<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::FSTATAT        , FstatatSystemCall<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::FACCESSAT      , FaccessatSystemCall<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::PREAD          , PreadSystemCall<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::PWRITE         , PwriteSystemCall<RiscvArchType>{} },
};

} /* end namespace RevCPU */ } // end namespace SST

// EOF