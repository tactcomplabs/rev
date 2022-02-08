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
std::unordered_map< typename SystemCalls<RiscvArchType>::Codes, SystemCallInterface<RiscvArchType> > SystemCalls<RiscvArchType>::jump_table = {
        { SystemCalls<RiscvArchType>::Codes::EXIT           , ExitInterfaceType<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::EXITGROUP      , ExitgroupInterfaceType<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::GETPID         , GetpidInterfaceType<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::KILL           , KillInterfaceType<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::TGKILL         , TgkillInterfaceType<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::READ           , ReadInterfaceType<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::WRITE          , WriteInterfaceType<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::OPENAT         , OpenatInterfaceType<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::CLOSE          , CloseInterfaceType<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::LSEEK          , LseekInterfaceType<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::BRK            , BrkInterfaceType<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::LINKAT         , LinkatInterfaceType<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::UNLINKAT       , UnlinkatInterfaceType<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::LINK           , LinkInterfaceType<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::UNLINK         , UnlinkInterfaceType<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::RENAMEAT       , RenameatInterfaceType<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::MKDIRAT        , MkdiratInterfaceType<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::MKDIR          , MkdirInterfaceType<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::CHDIR          , ChdirInterfaceType<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::GETCWD         , GetcwdInterfaceType<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::FSTAT          , FstatInterfaceType<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::FSTATAT        , FstatatInterfaceType<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::FACCESSAT      , FaccessatInterfaceType<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::PREAD          , PreadInterfaceType<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::PWRITE         , PwriteInterfaceType<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::UNAME          , UnameInterfaceType<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::GETUID         , GetuidInterfaceType<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::GETEUID        , GeteuidInterfaceType<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::GETGID         , GetgidInterfaceType<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::GETEGID        , GetegidInterfaceType<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::GETTID         , GettidInterfaceType<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::MMAP           , MmapInterfaceType<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::MUMAP          , MunmapInterfaceType<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::MREMAP         , MremapInterfaceType<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::MPROTECT       , MprotectInterfaceType<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::PRLIMIT64      , Prlimit64InterfaceType<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::SIGACTION      , SigactionInterfaceType<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::WRITEV         , WritevInterfaceType<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::GETTOD         , GettimeofdayInterfaceType<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::TIMES          , TimeInterfaceType<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::FCNTL          , FcntlInterfaceType<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::FTRUNCATE      , FtruncateInterfaceType<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::GETDENTS       , GetdentsInterfaceType<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::DUP            , DupInterfaceType<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::DUP3           , Dup3InterfaceType<RiscvArchType>{} },        
        { SystemCalls<RiscvArchType>::Codes::READLINKAT     , ReadlinkatInterfaceType<RiscvArchType>{} },        
        { SystemCalls<RiscvArchType>::Codes::SIGPROCMASK    , SigprocmaskInterfaceType<RiscvArchType>{} },                
        { SystemCalls<RiscvArchType>::Codes::IOCTL          , IoctlInterfaceType<RiscvArchType>{} },                
        { SystemCalls<RiscvArchType>::Codes::GETRLIMIT      , GetrlimitInterfaceType<RiscvArchType>{} },                
        { SystemCalls<RiscvArchType>::Codes::SETRLIMIT      , SetrlimitInterfaceType<RiscvArchType>{} },                
        { SystemCalls<RiscvArchType>::Codes::GETRUSAGE      , GetrusageInterfaceType<RiscvArchType>{} },                
        { SystemCalls<RiscvArchType>::Codes::CLOCKGETTIME   , ClockgettimeInterfaceType<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::SETTIDADDRESS  , SettidaddressInterfaceType<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::SETROBUSTLIST  , SetrobustlistInterfaceType<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::MADVISE        , MadviseInterfaceType<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::STATX          , StatxInterfaceType<RiscvArchType>{} },        
        { SystemCalls<RiscvArchType>::Codes::OPEN           , OpenInterfaceType<RiscvArchType>{} },        
        { SystemCalls<RiscvArchType>::Codes::LINK           , LinkInterfaceType<RiscvArchType>{} },        
        { SystemCalls<RiscvArchType>::Codes::UNLINK         , UnlinkInterfaceType<RiscvArchType>{} },        
        { SystemCalls<RiscvArchType>::Codes::MKDIR          , MkdirInterfaceType<RiscvArchType>{} },        
        { SystemCalls<RiscvArchType>::Codes::ACCESS         , AccessInterfaceType<RiscvArchType>{} },        
        { SystemCalls<RiscvArchType>::Codes::STAT           , StatInterfaceType<RiscvArchType>{} },        
        { SystemCalls<RiscvArchType>::Codes::LSTAT          , LstatInterfaceType<RiscvArchType>{} },        
        { SystemCalls<RiscvArchType>::Codes::TIME           , TimeInterfaceType<RiscvArchType>{} }
};

} /* end namespace RevCPU */ } // end namespace SST

// EOF
