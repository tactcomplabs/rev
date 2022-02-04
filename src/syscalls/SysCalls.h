//
// SysCalls.h
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
#pragma once
#ifndef __SYSTEMCALLS_H__
#define __SYSTEMCALLS_H__

#include "SystemCallInterface.h"
#include "SysCallExit.h"
#include "SysCallExitGroup.h"
#include "SysCallGetPid.h"
#include "SysCallKill.h"
#include "SysCallTGKill.h"
#include "SysCallRead.h"
#include "SysCallWrite.h"
#include "SysCallOpenAt.h"
#include "SysCallClose.h"
#include "SysCallLseek.h"
#include "SysCallBrk.h"
#include "SysCallLinkAt.h"
#include "SysCallUnlinkAt.h"
#include "SysCallLink.h"
#include "SysCallUnlink.h"
#include "SysCallMkdirAt.h"
#include "SysCallMkdir.h"
#include "SysCallRenameat.h"
#include "SysCallChdir.h"
#include "SysCallGetcwd.h"
#include "SysCallFstat.h"
#include "SysCallFstatat.h"
#include "SysCallFaccessat.h"
#include "SysCallPread.h"
#include "SysCallPwrite.h"
#include "SysCallUname.h"
#include "SysCallGetuid.h"
#include "SysCallGeteuid.h"
#include "SysCallGetgid.h"
#include "SysCallGetegid.h"
#include "SysCallGettid.h"
#include "SysCallMmap.h"
#include "SysCallMunmap.h"
#include "SysCallMremap.h"
#include "SysCallMprotect.h"
#include "SysCallPrlimit64.h"
#include "SysCallSigaction.h"
#include "SysCallWritev.h"
#include "SysCallGettimeofday.h"
#include "SysCallTimes.h"
#include "SysCallFcntl.h"
#include "SysCallGetdents.h"
#include "SysCallDup.h"
#include "SysCallDup3.h"
#include "SysCallReadlinkAt.h"
#include "SysCallSigprocmask.h"
#include "SysCallIoctl.h"
#include "SysCallGetrlimit.h"
#include "SysCallSetrlimit.h"
#include "SysCallGetrusage.h"
#include "SysCallClockgettime.h"
#include "SysCallSettidaddress.h"
#include "SysCallSetrobustlist.h"
#include "SysCallMadvise.h"
#include "SysCallStatx.h"
#include "SysCallOpen.h"
#include "SysCallAccess.h"
#include "SysCallStat.h"
#include "SysCallLstat.h"
#include "SysCallTime.h"
#include "SysCallFtruncate.h"

#include <unordered_map>
#include <type_traits>

// https://github.com/riscv-software-src/riscv-pk/blob/master/pk/syscall.h
//
// https://github.com/riscv-software-src/riscv-isa-sim/tree/master/fesvr
//

namespace SST { namespace RevCPU {

template<typename RiscvArchType=Riscv32>
class SystemCalls : public SystemArch<RiscvArchType> {

    public:

    using RiscvArch = typename SystemArch<RiscvArchType>::RiscvArch;
    using IsRiscv32  = typename SystemArch<RiscvArchType>::IsRiscv32;
    using IsRiscv64  = typename SystemArch<RiscvArchType>::IsRiscv64;
    using IsRiscv128 = typename SystemArch<RiscvArchType>::IsRiscv128;
    using RiscvModeIntegerType = typename SystemArch<RiscvArchType>::RiscvModeIntegerType;

    enum Codes : RiscvModeIntegerType {
        EXIT             = Exit<RiscvArchType>::code_value,
        EXITGROUP        = Exitgroup<RiscvArchType>::code_value,
        GETPID           = Getpid<RiscvArchType>::code_value,
        KILL             = Kill<RiscvArchType>::code_value,
        TGKILL           = Tgkill<RiscvArchType>::code_value,
        READ             = Read<RiscvArchType>::code_value,
        WRITE            = Write<RiscvArchType>::code_value,
        OPENAT           = Openat<RiscvArchType>::code_value,
        CLOSE            = Close<RiscvArchType>::code_value,
        LSEEK            = Lseek<RiscvArchType>::code_value,
        BRK              = Lseek<RiscvArchType>::code_value,
        LINKAT           = LinkAt<RiscvArchType>::code_value,
        UNLINKAT         = Unlinkat<RiscvArchType>::code_value,
        LINK             = Link<RiscvArchType>::code_value,
        UNLINK           = Unlink<RiscvArchType>::code_value,
        RENAMEAT         = Unlink<RiscvArchType>::code_value,
        MKDIRAT          = Unlink<RiscvArchType>::code_value,
        MKDIR            = Unlink<RiscvArchType>::code_value,
        CHDIR            = Chdir<RiscvArchType>::code_value,
        GETCWD           = Getcwd<RiscvArchType>::code_value,
        FSTAT            = Fstat<RiscvArchType>::code_value,
        FSTATAT          = Fstatat<RiscvArchType>::code_value,
        FACCESSAT        = Faccessat<RiscvArchType>::code_value,
        PREAD            = Pread<RiscvArchType>::code_value,
        PWRITE           = Pwrite<RiscvArchType>::code_value,
        UNAME            = Uname<RiscvArchType>::code_value,
        GETUID           = Getuid<RiscvArchType>::code_value,
        GETEUID          = Geteuid<RiscvArchType>::code_value,
        GETGID           = Getgid<RiscvArchType>::code_value,
        GETEGID          = Getegid<RiscvArchType>::code_value,
        GETTID           = Gettid<RiscvArchType>::code_value,
        MMAP             = Mmap<RiscvArchType>::code_value,
        MUMAP            = Munmap<RiscvArchType>::code_value,
        MREMAP           = Mremap<RiscvArchType>::code_value,
        MPROTECT         = Mprotect<RiscvArchType>::code_value,
        PRLIMIT64        = Prlimit64<RiscvArchType>::code_value,
        SIGACTION        = Sigaction<RiscvArchType>::code_value,
        WRITEV           = Writev<RiscvArchType>::code_value,
        GETTOD           = Gettimeofday<RiscvArchType>::code_value,
        TIMES            = Times<RiscvArchType>::code_value,
        FCNTL            = Fcntl<RiscvArchType>::code_value,
        FTRUNCATE        = Ftruncate<RiscvArchType>::code_value,
        GETDENTS         = Getdents<RiscvArchType>::code_value,
        DUP              = Dup<RiscvArchType>::code_value,
        DUP3             = Dup3<RiscvArchType>::code_value,        
        READLINKAT       = Readlinkat<RiscvArchType>::code_value,        
        SIGPROCMASK      = Sigprocmask<RiscvArchType>::code_value,                
        IOCTL            = Ioctl<RiscvArchType>::code_value,                
        GETRLIMIT        = Getrlimit<RiscvArchType>::code_value,                
        SETRLIMIT        = Setrlimit<RiscvArchType>::code_value,                
        GETRUSAGE        = Getrusage<RiscvArchType>::code_value,                
        CLOCKGETTIME     = Clock_gettime<RiscvArchType>::code_value,
        SETTIDADDRESS    = Settidaddress<RiscvArchType>::code_value,
        SETROBUSTLIST    = Setrobustlist<RiscvArchType>::code_value,
        MADVISE          = Madvise<RiscvArchType>::code_value,
        STATX            = Statx<RiscvArchType>::code_value,        
        OPEN             = Open<RiscvArchType>::code_value,        
        LINK             = Link<RiscvArchType>::code_value,        
        UNLINK           = Unlink<RiscvArchType>::code_value,        
        MKDIR            = Mkdir<RiscvArchType>::code_value,        
        ACCESS           = Access<RiscvArchType>::code_value,        
        STAT             = Stat<RiscvArchType>::code_value,        
        LSTAT            = Lstat<RiscvArchType>::code_value,        
        TIME             = Time<RiscvArchType>::code_value,
    };

    static std::unordered_map< Codes, SystemCallInterface<RiscvArchType> > jump_table;
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF