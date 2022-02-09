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
#include "SysCallGetpid.h"
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
#include "SysCallRenameAt.h"
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
#include "SysCallTime.h"
#include "SysCallFcntl.h"
#include "SysCallGetdents.h"
#include "SysCallDup.h"
#include "SysCallDup3.h"
#include "SysCallReadLinkAt.h"
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
#include "SysCallTimes.h"
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
        EXIT             = ExitInterfaceType<RiscvArchType>::value,
        EXITGROUP        = ExitgroupInterfaceType<RiscvArchType>::value,
        GETPID           = GetpidInterfaceType<RiscvArchType>::value,
        KILL             = KillInterfaceType<RiscvArchType>::value,
        TGKILL           = TgkillInterfaceType<RiscvArchType>::value,
        READ             = ReadInterfaceType<RiscvArchType>::value,
        WRITE            = WriteInterfaceType<RiscvArchType>::value,
        OPENAT           = OpenatInterfaceType<RiscvArchType>::value,
        CLOSE            = CloseInterfaceType<RiscvArchType>::value,
        LSEEK            = LseekInterfaceType<RiscvArchType>::value,
        BRK              = LseekInterfaceType<RiscvArchType>::value,
        LINKAT           = LinkatInterfaceType<RiscvArchType>::value,
        UNLINKAT         = UnlinkatInterfaceType<RiscvArchType>::value,
        LINK             = LinkInterfaceType<RiscvArchType>::value,
        UNLINK           = UnlinkInterfaceType<RiscvArchType>::value,
        RENAMEAT         = RenameatInterfaceType<RiscvArchType>::value,
        MKDIRAT          = MkdiratInterfaceType<RiscvArchType>::value,
        MKDIR            = MkdirInterfaceType<RiscvArchType>::value,
        CHDIR            = ChdirInterfaceType<RiscvArchType>::value,
        GETCWD           = GetcwdInterfaceType<RiscvArchType>::value,
        FSTAT            = FstatInterfaceType<RiscvArchType>::value,
        FSTATAT          = FstatatInterfaceType<RiscvArchType>::value,
        FACCESSAT        = FaccessatInterfaceType<RiscvArchType>::value,
        PREAD            = PreadInterfaceType<RiscvArchType>::value,
        PWRITE           = PwriteInterfaceType<RiscvArchType>::value,
        UNAME            = UnameInterfaceType<RiscvArchType>::value,
        GETUID           = GetuidInterfaceType<RiscvArchType>::value,
        GETEUID          = GeteuidInterfaceType<RiscvArchType>::value,
        GETGID           = GetgidInterfaceType<RiscvArchType>::value,
        GETEGID          = GetegidInterfaceType<RiscvArchType>::value,
        GETTID           = GettidInterfaceType<RiscvArchType>::value,
        MMAP             = MmapInterfaceType<RiscvArchType>::value,
        MUMAP            = MunmapInterfaceType<RiscvArchType>::value,
        MREMAP           = MremapInterfaceType<RiscvArchType>::value,
        MPROTECT         = MprotectInterfaceType<RiscvArchType>::value,
        PRLIMIT64        = Prlimit64InterfaceType<RiscvArchType>::value,
        SIGACTION        = SigactionInterfaceType<RiscvArchType>::value,
        WRITEV           = WritevInterfaceType<RiscvArchType>::value,
        GETTOD           = GettimeofdayInterfaceType<RiscvArchType>::value,
        TIMES            = TimesInterfaceType<RiscvArchType>::value,
        FCNTL            = FcntlInterfaceType<RiscvArchType>::value,
        FTRUNCATE        = FtruncateInterfaceType<RiscvArchType>::value,
        GETDENTS         = GetdentsInterfaceType<RiscvArchType>::value,
        DUP              = DupInterfaceType<RiscvArchType>::value,
        DUP3             = Dup3InterfaceType<RiscvArchType>::value,        
        READLINKAT       = ReadlinkatInterfaceType<RiscvArchType>::value,        
        SIGPROCMASK      = SigprocmaskInterfaceType<RiscvArchType>::value,                
        IOCTL            = IoctlInterfaceType<RiscvArchType>::value,                
        GETRLIMIT        = GetrlimitInterfaceType<RiscvArchType>::value,                
        SETRLIMIT        = SetrlimitInterfaceType<RiscvArchType>::value,                
        GETRUSAGE        = GetrusageInterfaceType<RiscvArchType>::value,                
        CLOCKGETTIME     = ClockgettimeInterfaceType<RiscvArchType>::value,
        SETTIDADDRESS    = SettidaddressInterfaceType<RiscvArchType>::value,
        SETROBUSTLIST    = SetrobustlistInterfaceType<RiscvArchType>::value,
        MADVISE          = MadviseInterfaceType<RiscvArchType>::value,
        STATX            = StatxInterfaceType<RiscvArchType>::value,        
        OPEN             = OpenInterfaceType<RiscvArchType>::value,        
        ACCESS           = AccessInterfaceType<RiscvArchType>::value,        
        STAT             = StatInterfaceType<RiscvArchType>::value,        
        LSTAT            = LstatInterfaceType<RiscvArchType>::value,        
        TIME             = TimeInterfaceType<RiscvArchType>::value,
    };

    static std::unordered_map< Codes, SystemCallInterface<RiscvArchType> > jump_table;
};

} /* end namespace RevCPU */ } // end namespace SST

#endif

// EOF
