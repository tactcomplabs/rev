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

    using RiscvArch = typename SystemArch<RiscvArchType>::RiscvArch;
    using IsRiscv32  = typename SystemArch<RiscvArchType>::IsRiscv32;
    using IsRiscv64  = typename SystemArch<RiscvArchType>::IsRiscv64;
    using IsRiscv128 = typename SystemArch<RiscvArchType>::IsRiscv128;
    using RiscvModeIntegerType = typename SystemArch<RiscvArchType>::RiscvModeIntegerType;

    public:

    enum Codes : RiscvModeIntegerType {
        EXIT             = ExitSystemCall<RiscvArchType>::code_value,
        EXITGROUP        = ExitGroupSystemCall<RiscvArchType>::code_value,
        GETPID           = GetPidSystemCall<RiscvArchType>::code_value,
        KILL             = KillSystemCall<RiscvArchType>::code_value,
        TGKILL           = TGKillSystemCall<RiscvArchType>::code_value,
        READ             = ReadSystemCall<RiscvArchType>::code_value,
        WRITE            = WriteSystemCall<RiscvArchType>::code_value,
        OPENAT           = OpenAtSystemCall<RiscvArchType>::code_value,
        CLOSE            = CloseSystemCall<RiscvArchType>::code_value,
        LSEEK            = LseekSystemCall<RiscvArchType>::code_value,
        BRK              = LseekSystemCall<RiscvArchType>::code_value,
        LINKAT           = LinkAtSystemCall<RiscvArchType>::code_value,
        UNLINKAT         = UnlinkAtSystemCall<RiscvArchType>::code_value,
        LINK             = LinkSystemCall<RiscvArchType>::code_value,
        UNLINK           = UnlinkSystemCall<RiscvArchType>::code_value,
        RENAMEAT         = UnlinkSystemCall<RiscvArchType>::code_value,
        MKDIRAT          = UnlinkSystemCall<RiscvArchType>::code_value,
        MKDIR            = UnlinkSystemCall<RiscvArchType>::code_value,
        CHDIR            = ChdirSystemCall<RiscvArchType>::code_value,
        GETCWD           = GetcwdSystemCall<RiscvArchType>::code_value,
        FSTAT            = FstatSystemCall<RiscvArchType>::code_value,
        FSTATAT          = FstatatSystemCall<RiscvArchType>::code_value,
        FACCESSAT        = FaccessatSystemCall<RiscvArchType>::code_value,
        PREAD            = PreadSystemCall<RiscvArchType>::code_value,
        PWRITE           = PwriteSystemCall<RiscvArchType>::code_value,
        UNAME            = UnameSystemCall<RiscvArchType>::code_value,
        GETUID           = GetuidSystemCall<RiscvArchType>::code_value,
        GETEUID          = GeteuidSystemCall<RiscvArchType>::code_value,
        GETGID           = GetgidSystemCall<RiscvArchType>::code_value,
        GETEGID          = GetegidSystemCall<RiscvArchType>::code_value,
        GETTID           = GettidSystemCall<RiscvArchType>::code_value,
        MMAP             = MmapSystemCall<RiscvArchType>::code_value,
        MUMAP            = MunmapSystemCall<RiscvArchType>::code_value,
        MREMAP           = MremapSystemCall<RiscvArchType>::code_value,
        MPROTECT         = MprotectSystemCall<RiscvArchType>::code_value,
        PRLIMIT64        = Prlimit64SystemCall<RiscvArchType>::code_value,
        SIGACTION        = SigactionSystemCall<RiscvArchType>::code_value,
        WRITEV           = WritevSystemCall<RiscvArchType>::code_value,
        GETTOD           = GettimeofdaySystemCall<RiscvArchType>::code_value,
        TIMES            = TimesSystemCall<RiscvArchType>::code_value,
        FCNTL            = FcntlSystemCall<RiscvArchType>::code_value,
        FTRUNCATE        = FtruncateSystemCall<RiscvArchType>::code_value,
        GETDENTS         = GetdentsSystemCall<RiscvArchType>::code_value,
        DUP              = DupSystemCall<RiscvArchType>::code_value,
        DUP3             = Dup3SystemCall<RiscvArchType>::code_value,        
        READLINKAT       = ReadLinkAtSystemCall<RiscvArchType>::code_value,        
        SIGPROCMASK      = SigprocmaskSystemCall<RiscvArchType>::code_value,                
        IOCTL            = IoctlSystemCall<RiscvArchType>::code_value,                
        GETRLIMIT        = GetrlimitSystemCall<RiscvArchType>::code_value,                
        SETRLIMIT        = SetrlimitSystemCall<RiscvArchType>::code_value,                
        GETRUSAGE        = GetrusageSystemCall<RiscvArchType>::code_value,                
        CLOCKGETTIME     = Clock_gettimeSystemCall<RiscvArchType>::code_value,
        SETTIDADDRESS    = SettidaddressSystemCall<RiscvArchType>::code_value,
        SETROBUSTLIST    = SetrobustlistSystemCall<RiscvArchType>::code_value,
        MADVISE          = MadviseSystemCall<RiscvArchType>::code_value,
        STATX            = StatxSystemCall<RiscvArchType>::code_value,        
        OPEN             = OpenSystemCall<RiscvArchType>::code_value,        
        LINK             = LinkSystemCall<RiscvArchType>::code_value,        
        UNLINK           = UnlinkSystemCall<RiscvArchType>::code_value,        
        MKDIR            = MkdirSystemCall<RiscvArchType>::code_value,        
        ACCESS           = AccessSystemCall<RiscvArchType>::code_value,        
        STAT             = StatSystemCall<RiscvArchType>::code_value,        
        LSTAT            = LstatSystemCall<RiscvArchType>::code_value,        
        TIME             = TimeSystemCall<RiscvArchType>::code_value,        
};        
    };

    static std::unordered_map<Codes, SystemCallInterface<RiscvArchType>> jump_table;
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF