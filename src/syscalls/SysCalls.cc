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
        { SystemCalls<RiscvArchType>::Codes::EXIT           , Exit<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::EXITGROUP      , Exitgroup<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::GETPID         , Getpid<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::KILL           , Kill<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::TGKILL         , Tgkill<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::READ           , Read<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::WRITE          , Write<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::OPENAT         , Openat<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::CLOSE          , Close<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::LSEEK          , Lseek<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::BRK            , Brk<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::LINKAT         , Linkat<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::UNLINKAT       , Unlinkat<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::LINK           , Link<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::UNLINK         , Unlink<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::RENAMEAT       , Renameat<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::MKDIRAT        , Mkdirat<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::MKDIR          , Mkdir<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::CHDIR          , Chdir<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::GETCWD         , Getcwd<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::FSTAT          , Fstat<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::FSTATAT        , Fstatat<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::FACCESSAT      , Faccessat<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::PREAD          , Pread<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::PWRITE         , Pwrite<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::UNAME          , Uname<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::GETUID         , Getuid<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::GETEUID        , Geteuid<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::GETGID         , Getgid<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::GETEGID        , Getegid<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::GETTID         , Gettid<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::MMAP           , Mmap<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::MUMAP          , Munmap<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::MREMAP         , Mremap<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::MPROTECT       , Mprotect<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::PRLIMIT64      , Prlimit64<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::SIGACTION      , Sigaction<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::WRITEV         , Writev<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::GETTOD         , Gettimeofday<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::TIMES          , Time<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::FCNTL          , Fcntl<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::FTRUNCATE      , Ftruncate<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::GETDENTS       , Getdents<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::DUP            , Dup<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::DUP3           , Dup3<RiscvArchType>{} },        
        { SystemCalls<RiscvArchType>::Codes::READLINKAT     , Readlinkat<RiscvArchType>{} },        
        { SystemCalls<RiscvArchType>::Codes::SIGPROCMASK    , Sigprocmask<RiscvArchType>{} },                
        { SystemCalls<RiscvArchType>::Codes::IOCTL          , Ioctl<RiscvArchType>{} },                
        { SystemCalls<RiscvArchType>::Codes::GETRLIMIT      , Getrlimit<RiscvArchType>{} },                
        { SystemCalls<RiscvArchType>::Codes::SETRLIMIT      , Setrlimit<RiscvArchType>{} },                
        { SystemCalls<RiscvArchType>::Codes::GETRUSAGE      , Getrusage<RiscvArchType>{} },                
        { SystemCalls<RiscvArchType>::Codes::CLOCKGETTIME   , Clockgettime<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::SETTIDADDRESS  , Settidaddress<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::SETROBUSTLIST  , Setrobustlist<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::MADVISE        , Madvise<RiscvArchType>{} },
        { SystemCalls<RiscvArchType>::Codes::STATX          , Statx<RiscvArchType>{} },        
        { SystemCalls<RiscvArchType>::Codes::OPEN           , Open<RiscvArchType>{} },        
        { SystemCalls<RiscvArchType>::Codes::LINK           , Link<RiscvArchType>{} },        
        { SystemCalls<RiscvArchType>::Codes::UNLINK         , Unlink<RiscvArchType>{} },        
        { SystemCalls<RiscvArchType>::Codes::MKDIR          , Mkdir<RiscvArchType>{} },        
        { SystemCalls<RiscvArchType>::Codes::ACCESS         , Access<RiscvArchType>{} },        
        { SystemCalls<RiscvArchType>::Codes::STAT           , Stat<RiscvArchType>{} },        
        { SystemCalls<RiscvArchType>::Codes::LSTAT          , Lstat<RiscvArchType>{} },        
        { SystemCalls<RiscvArchType>::Codes::TIME           , Time<RiscvArchType>{} }
};

} /* end namespace RevCPU */ } // end namespace SST

// EOF
