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
        { SystemCalls<RiscvArchType>::Codes::GETPID         , GetpidSystemCall<RiscvArchType>{} },
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
        { SystemCalls<RiscvArchType>::Codes::UNAME          , UnameSystemCall<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::GETUID         , GetuidSystemCall<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::GETEUID        , GeteuidSystemCall<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::GETGID         , GetgidSystemCall<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::GETEGID        , GetegidSystemCall<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::GETTID         , GettidSystemCall<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::MMAP           , MmapSystemCall<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::MUMAP          , MunmapSystemCall<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::MREMAP         , MremapSystemCall<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::MPROTECT       , MprotectSystemCall<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::PRLIMIT64      , Prlimit64SystemCall<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::SIGACTION      , SigactionSystemCall<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::WRITEV         , WritevSystemCall<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::GETTOD         , GettimeofdaySystemCall<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::TIMES          , TimesSystemCall<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::FCNTL          , FcntlSystemCall<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::FTRUNCATE      , FtruncateSystemCall<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::GETDENTS       , GetdentsSystemCall<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::DUP            , DupSystemCall<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::DUP3           , Dup3SystemCall<RiscvArchType>{} },        
        { SystemCalls<RiscvArchType>::Codes::READLINKAT     , ReadLinkAtSystemCall<RiscvArchType>{} },        
        { SystemCalls<RiscvArchType>::Codes::SIGPROCMASK    , SigprocmaskSystemCall<RiscvArchType>{} },                
        { SystemCalls<RiscvArchType>::Codes::IOCTL          , IoctlSystemCall<RiscvArchType>{} },                
        { SystemCalls<RiscvArchType>::Codes::GETRLIMIT      , GetrlimitSystemCall<RiscvArchType>{} },                
        { SystemCalls<RiscvArchType>::Codes::SETRLIMIT      , SetrlimitSystemCall<RiscvArchType>{} },                
        { SystemCalls<RiscvArchType>::Codes::GETRUSAGE      , GetrusageSystemCall<RiscvArchType>{} },                
        { SystemCalls<RiscvArchType>::Codes::CLOCKGETTIME   , Clock_gettimeSystemCall<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::SETTIDADDRESS  , SettidaddressSystemCall<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::SETROBUSTLIST  , SetrobustlistSystemCall<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::MADVISE        , MadviseSystemCall<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::STATX          , StatxSystemCall<RiscvArchType>{} },        
        { SystemCalls<RiscvArchType>::Codes::OPEN           , OpenSystemCall<RiscvArchType>{} },        
        { SystemCalls<RiscvArchType>::Codes::LINK           , LinkSystemCall<RiscvArchType>{} },        
        { SystemCalls<RiscvArchType>::Codes::UNLINK         , UnlinkSystemCall<RiscvArchType>{} },        
        { SystemCalls<RiscvArchType>::Codes::MKDIR          , MkdirSystemCall<RiscvArchType>{} },        
        { SystemCalls<RiscvArchType>::Codes::ACCESS         , AccessSystemCall<RiscvArchType>{} },        
        { SystemCalls<RiscvArchType>::Codes::STAT           , StatSystemCall<RiscvArchType>{} },        
        { SystemCalls<RiscvArchType>::Codes::LSTAT          , LstatSystemCall<RiscvArchType>{} },        
        { SystemCalls<RiscvArchType>::Codes::TIME           , TimeSystemCall<RiscvArchType>{} },        
};

} /* end namespace RevCPU */ } // end namespace SST

// EOF