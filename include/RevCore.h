//
// _RevCore_h_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_REVPROC_H_
#define _SST_REVCPU_REVPROC_H_

// -- SST Headers
#include "SST.h"

// -- Standard Headers
#include <array>
#include <bitset>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <queue>
#include <string>
#include <sys/xattr.h>
#include <time.h>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

// -- RevCPU Headers
#include "AllRevInstTables.h"
#include "RevCoProc.h"
#include "RevCorePasskey.h"
#include "RevFeature.h"
#include "RevHart.h"
#include "RevInstTable.h"
#include "RevLoader.h"
#include "RevMem.h"
#include "RevOpts.h"
#include "RevPrefetcher.h"
#include "RevRand.h"
#include "RevThread.h"
#include "RevTracer.h"
#define SYSCALL_TYPES_ONLY
#include "../common/include/RevCommon.h"
#include "../common/syscalls/syscalls.h"

namespace SST::RevCPU {
class RevCoProc;

class RevCore {
public:
  /// RevCore: standard constructor
  RevCore(
    unsigned                  Id,
    RevOpts*                  Opts,
    unsigned                  NumHarts,
    RevMem*                   Mem,
    RevLoader*                Loader,
    std::function<uint32_t()> GetNewThreadID,
    SST::Output*              Output
  );

  /// RevCore: standard destructor
  ~RevCore()                           = default;

  /// RevCore: disallow copying and assignment
  RevCore( const RevCore& )            = delete;
  RevCore& operator=( const RevCore& ) = delete;

  /// RevCore: per-processor clock function
  bool ClockTick( SST::Cycle_t currentCycle );

  /// RevCore: Called by RevCPU when there is no more work to do (ie. All RevThreads are ThreadState::DONE )
  void PrintStatSummary();

  /// RevCore: halt the CPU
  bool Halt();

  /// RevCore: resume the CPU
  bool Resume();

  /// RevCore: execute a single step
  bool SingleStepHart();

  /// RevCore: retrieve the local PC for the correct feature set
  uint64_t GetPC() const { return RegFile->GetPC(); }

  /// RevCore: set time converter for RTC
  void SetTimeConverter( TimeConverter* tc ) { timeConverter = tc; }

  /// RevCore: Debug mode read a register
  bool DebugReadReg( unsigned Idx, uint64_t* Value ) const;

  /// RevCore: Debug mode write a register
  bool DebugWriteReg( unsigned Idx, uint64_t Value ) const;

  /// RevCore: Is this an RV32 machine?
  bool DebugIsRV32() { return feature->IsRV32(); }

  /// RevCore: Set an optional tracer
  void SetTracer( RevTracer* T ) { Tracer = T; }

  /// RevCore: Retrieve a random memory cost value
  unsigned RandCost() { return mem->RandCost( feature->GetMinCost(), feature->GetMaxCost() ); }

  /// RevCore: Handle register faults
  void HandleRegFault( unsigned width );

  /// RevCore: Handle crack+decode faults
  void HandleCrackFault( unsigned width );

  /// RevCore: Handle ALU faults
  void HandleALUFault( unsigned width );

  /// RevCore: Handle ALU faults
  void InjectALUFault( std::pair<unsigned, unsigned> EToE, RevInst& Inst );

  struct RevCoreStats {
    uint64_t totalCycles;
    uint64_t cyclesBusy;
    uint64_t cyclesIdle_Total;
    uint64_t cyclesStalled;
    uint64_t floatsExec;
    uint64_t cyclesIdle_Pipeline;
    uint64_t cyclesIdle_MemoryFetch;
    uint64_t retired;
  };

  auto GetAndClearStats() {
    // Add each field from Stats into StatsTotal
    for( auto stat :
         { &RevCoreStats::totalCycles,
           &RevCoreStats::cyclesBusy,
           &RevCoreStats::cyclesIdle_Total,
           &RevCoreStats::cyclesStalled,
           &RevCoreStats::floatsExec,
           &RevCoreStats::cyclesIdle_Pipeline,
           &RevCoreStats::retired } ) {
      StatsTotal.*stat += Stats.*stat;
    }

    auto memStats = mem->GetAndClearStats();
    auto ret      = std::make_pair( Stats, memStats );
    Stats         = {};  // Zero out Stats
    return ret;
  }

  RevMem& GetMem() const { return *mem; }

  ///< RevCore: Called by RevCPU to handle the state changes threads may have happened during this Proc's ClockTick
  auto TransferThreadsThatChangedState() { return std::move( ThreadsThatChangedState ); }

  ///< RevCore: Add
  void AddThreadsThatChangedState( std::unique_ptr<RevThread>&& thread ) {
    ThreadsThatChangedState.push_back( std::move( thread ) );
  }

  ///< RevCore: SpawnThread creates a new thread and returns its ThreadID
  void CreateThread( uint32_t NewTid, uint64_t fn, void* arg );

  ///< RevCore: Returns the current HartToExecID active pid
  uint32_t GetActiveThreadID() { return Harts.at( HartToDecodeID )->GetAssignedThreadID(); }

  ///< RevCore: Get this Proc's feature
  RevFeature* GetRevFeature() const { return feature; }

  ///< RevCore: Mark a current request as complete
  void MarkLoadComplete( const MemReq& req );

  ///< RevCore: Get pointer to Load / Store queue used to track memory operations
  std::shared_ptr<std::unordered_multimap<uint64_t, MemReq>> GetLSQueue() const { return LSQueue; }

  ///< RevCore: Add a co-processor to the RevCore
  void SetCoProc( RevCoProc* coproc );

  //--------------- External Interface for use with Co-Processor -------------------------
  ///< RevCore: Allow a co-processor to query the bits in scoreboard. Note the RevCorePassKey may only
  ///  be created by a RevCoProc (or a class derived from RevCoProc) so this function may not be called from even within
  ///  RevCore
  [[deprecated( "RevRegClass regClass is used instead of bool isFloat" )]] bool
    ExternalDepCheck( RevCorePasskey<RevCoProc>, uint16_t HartID, uint16_t reg, bool IsFloat ) {
    RevRegClass       regClass = IsFloat ? RevRegClass::RegFLOAT : RevRegClass::RegGPR;
    const RevRegFile* regFile  = GetRegFile( HartID );
    return LSQCheck( HartID, regFile, reg, regClass ) || ScoreboardCheck( regFile, reg, regClass );
  }

  ///< RevCore: Allow a co-processor to manipulate the scoreboard by setting a bit. Note the RevCorePassKey may only
  ///  be created by a RevCoProc (or a class derived from RevCoProc) so this funciton may not be called from even within
  ///  RevCore
  [[deprecated( "RevRegClass regClass is used instead of bool isFloat" )]] void
    ExternalDepSet( RevCorePasskey<RevCoProc>, uint16_t HartID, uint16_t RegNum, bool isFloat, bool value = true ) {
    RevRegClass regClass = isFloat ? RevRegClass::RegFLOAT : RevRegClass::RegGPR;
    DependencySet( HartID, RegNum, regClass, value );
  }

  ///< RevCore: Allow a co-processor to manipulate the scoreboard by clearing a bit. Note the RevCorePassKey may only
  ///  be created by a RevCoProc (or a class derived from RevCoProc) so this funciton may not be called from even within
  ///  RevCore
  [[deprecated( "RevRegClass regClass is used instead of bool isFloat" )]] void
    ExternalDepClear( RevCorePasskey<RevCoProc>, uint16_t HartID, uint16_t RegNum, bool isFloat ) {
    RevRegClass regClass = isFloat ? RevRegClass::RegFLOAT : RevRegClass::RegGPR;
    DependencyClear( HartID, RegNum, regClass );
  }

  //--------------- External Interface for use with Co-Processor -------------------------
  ///< RevCore: Allow a co-processor to query the bits in scoreboard. Note the RevCorePassKey may only
  ///  be created by a RevCoProc (or a class derived from RevCoProc) so this function may not be called from even within
  ///  RevCore
  bool ExternalDepCheck( RevCorePasskey<RevCoProc>, uint16_t HartID, uint16_t reg, RevRegClass regClass ) {
    const RevRegFile* regFile = GetRegFile( HartID );
    return LSQCheck( HartID, regFile, reg, regClass ) || ScoreboardCheck( regFile, reg, regClass );
  }

  ///< RevCore: Allow a co-processor to manipulate the scoreboard by setting a bit. Note the RevCorePassKey may only
  ///  be created by a RevCoProc (or a class derived from RevCoProc) so this funciton may not be called from even within
  ///  RevCore
  void ExternalDepSet( RevCorePasskey<RevCoProc>, uint16_t HartID, uint16_t RegNum, RevRegClass regClass, bool value = true ) {
    DependencySet( HartID, RegNum, regClass, value );
  }

  ///< RevCore: Allow a co-processor to manipulate the scoreboard by clearing a bit. Note the RevCorePassKey may only
  ///  be created by a RevCoProc (or a class derived from RevCoProc) so this funciton may not be called from even within
  ///  RevCore
  void ExternalDepClear( RevCorePasskey<RevCoProc>, uint16_t HartID, uint16_t RegNum, RevRegClass regClass ) {
    DependencyClear( HartID, RegNum, regClass );
  }

  ///< RevCore: Allow a co-processor to stall the pipeline of this proc and hold it in a stall condition
  ///  unitl ExternalReleaseHart() is called. Note the RevCorePassKey may only
  ///  be created by a RevCoProc (or a class derived from RevCoProc) so this funciton may not be called from even within
  ///  RevCore
  void ExternalStallHart( RevCorePasskey<RevCoProc>, uint16_t HartID );

  ///< RevCore: Allow a co-processor to release the pipeline of this proc and allow a hart to continue
  ///  execution (this un-does the ExternalStallHart() function ). Note the RevCorePassKey may only
  ///  be created by a RevCoProc (or a class derived from RevCoProc) so this funciton may not be called from even within
  ///  RevCore
  void ExternalReleaseHart( RevCorePasskey<RevCoProc>, uint16_t HartID );
  //------------- END External Interface -------------------------------

  ///< RevCore: Used for loading a software thread into a RevHart
  void AssignThread( std::unique_ptr<RevThread> ThreadToAssign );

  ///< RevCore:
  void UpdateStatusOfHarts();

  ///< RevCore: Returns the id of an idle hart (or _INVALID_HART_ID_ if none are idle)
  unsigned FindIdleHartID() const;

  ///< RevCore: Returns true if all harts are available (ie. There is nothing executing on this Proc)
  bool HasNoBusyHarts() const { return IdleHarts == ValidHarts; }

  ///< RevCore: Used by RevCPU to determine if it can disable this proc
  ///           based on the criteria there are no threads assigned to it and the
  ///           CoProc is done
  bool HasNoWork() const;

  ///< RevCore: Returns true if there are any IdleHarts
  bool HasIdleHart() const { return IdleHarts.any(); }

private:
  bool           Halted      = false;  ///< RevCore: determines if the core is halted
  bool           Stalled     = false;  ///< RevCore: determines if the core is stalled on instruction fetch
  bool           SingleStep  = false;  ///< RevCore: determines if we are in a single step
  bool           CrackFault  = false;  ///< RevCore: determines if we need to handle a crack fault
  bool           ALUFault    = false;  ///< RevCore: determines if we need to handle an ALU fault
  unsigned       fault_width = 0;      ///< RevCore: the width of the target fault
  unsigned const id;                   ///< RevCore: processor id
  uint64_t       ExecPC         = 0;   ///< RevCore: executing PC
  unsigned       HartToDecodeID = 0;   ///< RevCore: Current executing ThreadID
  unsigned       HartToExecID   = 0;   ///< RevCore: Thread to dispatch instruction

  std::vector<std::shared_ptr<RevHart>> Harts{};                ///< RevCore: vector of Harts without a thread assigned to them
  std::bitset<_MAX_HARTS_>              IdleHarts{};            ///< RevCore: bitset of Harts with no thread assigned
  std::bitset<_MAX_HARTS_>              ValidHarts{};           ///< RevCore: Bits 0 -> numHarts are 1
  std::bitset<_MAX_HARTS_>              HartsClearToDecode{};   ///< RevCore: Thread is clear to start (proceed with decode)
  std::bitset<_MAX_HARTS_>              HartsClearToExecute{};  ///< RevCore: Thread is clear to execute (no register dependencides)

  unsigned   numHarts;  ///< RevCore: Number of Harts for this core
  RevOpts*   opts;      ///< RevCore: options object
  RevMem*    mem;       ///< RevCore: memory object
  RevCoProc* coProc;    ///< RevCore: attached co-processor
  RevLoader* loader;    ///< RevCore: loader object

  // Function pointer to the GetNewThreadID function in RevCPU (monotonically increasing thread ID counter)
  std::function<uint32_t()> GetNewThreadID;

  // If a given assigned thread experiences a change of state, it sets the corresponding bit
  std::vector<std::unique_ptr<RevThread>>
    ThreadsThatChangedState{};  ///< RevCore: used to signal to RevCPU that the thread assigned to HART has changed state

  SST::Output* const             output;       ///< RevCore: output handler
  std::unique_ptr<RevFeature>    featureUP{};  ///< RevCore: feature handler
  RevFeature*                    feature{};
  RevCoreStats                   Stats{};       ///< RevCore: collection of performance stats
  RevCoreStats                   StatsTotal{};  ///< RevCore: collection of total performance stats
  std::unique_ptr<RevPrefetcher> sfetch{};      ///< RevCore: stream instruction prefetcher

  std::shared_ptr<std::unordered_multimap<uint64_t, MemReq>>
    LSQueue{};  ///< RevCore: Load / Store queue used to track memory operations. Currently only tracks outstanding loads.
  TimeConverter* timeConverter{};  ///< RevCore: Time converter for RTC

  RevRegFile* RegFile        = nullptr;        ///< RevCore: Initial pointer to HartToDecodeID RegFile
  uint32_t    ActiveThreadID = _INVALID_TID_;  ///< Software ThreadID (Not the Hart) that belongs to the Hart currently decoding
  RevTracer*  Tracer         = nullptr;        ///< RevCore: Tracer object

  std::bitset<_MAX_HARTS_> CoProcStallReq{};

  ///< RevCore: Utility function for system calls that involve reading a string from memory
  EcallStatus EcallLoadAndParseString( uint64_t straddr, std::function<void()> );

  // - Many of these are not implemented
  // - Their existence in the ECalls table is solely to not throw errors
  // - This _should_ be a comprehensive list of system calls supported on RISC-V
  // - Beside each function declaration is the system call code followed by its corresponding declaration
  //   that you can find in `common/syscalls.h` (the file to be included to use system calls inside of rev)
  //
  // clang-format off
  EcallStatus ECALL_io_setup();               // 0, rev_io_setup(unsigned nr_reqs, aio_context_t  *ctx)
  EcallStatus ECALL_io_destroy();             // 1, rev_io_destroy(aio_context_t ctx)
  EcallStatus ECALL_io_submit();              // 2, rev_io_submit(aio_context_t, long, struct iocb  *  *)
  EcallStatus ECALL_io_cancel();              // 3, rev_io_cancel(aio_context_t ctx_id, struct iocb  *iocb, struct io_event  *result)
  EcallStatus ECALL_io_getevents();           // 4, rev_io_getevents(aio_context_t ctx_id, long min_nr, long nr, struct io_event  *events, struct __kernel_timespec  *timeout)
  EcallStatus ECALL_setxattr();               // 5, rev_setxattr(const char  *path, const char  *name, const void  *value, size_t size, int flags)
  EcallStatus ECALL_lsetxattr();              // 6, rev_lsetxattr(const char  *path, const char  *name, const void  *value, size_t size, int flags)
  EcallStatus ECALL_fsetxattr();              // 7, rev_fsetxattr(int fd, const char  *name, const void  *value, size_t size, int flags)
  EcallStatus ECALL_getxattr();               // 8, rev_getxattr(const char  *path, const char  *name, void  *value, size_t size)
  EcallStatus ECALL_lgetxattr();              // 9, rev_lgetxattr(const char  *path, const char  *name, void  *value, size_t size)
  EcallStatus ECALL_fgetxattr();              // 10, rev_fgetxattr(int fd, const char  *name, void  *value, size_t size)
  EcallStatus ECALL_listxattr();              // 11, rev_listxattr(const char  *path, char  *list, size_t size)
  EcallStatus ECALL_llistxattr();             // 12, rev_llistxattr(const char  *path, char  *list, size_t size)
  EcallStatus ECALL_flistxattr();             // 13, rev_flistxattr(int fd, char  *list, size_t size)
  EcallStatus ECALL_removexattr();            // 14, rev_removexattr(const char  *path, const char  *name)
  EcallStatus ECALL_lremovexattr();           // 15, rev_lremovexattr(const char  *path, const char  *name)
  EcallStatus ECALL_fremovexattr();           // 16, rev_fremovexattr(int fd, const char  *name)
  EcallStatus ECALL_getcwd();                 // 17, rev_getcwd(char  *buf, unsigned long size)
  EcallStatus ECALL_lookup_dcookie();         // 18, rev_lookup_dcookie(u64 cookie64, char  *buf, size_t len)
  EcallStatus ECALL_eventfd2();               // 19, rev_eventfd2(unsigned int count, int flags)
  EcallStatus ECALL_epoll_create1();          // 20, rev_epoll_create1(int flags)
  EcallStatus ECALL_epoll_ctl();              // 21, rev_epoll_ctl(int epfd, int op, int fd, struct epoll_event  *event)
  EcallStatus ECALL_epoll_pwait();            // 22, rev_epoll_pwait(int epfd, struct epoll_event  *events, int maxevents, int timeout, const sigset_t  *sigmask, size_t sigsetsize)
  EcallStatus ECALL_dup();                    // 23, rev_dup(unsigned int fildes)
  EcallStatus ECALL_dup3();                   // 24, rev_dup3(unsigned int oldfd, unsigned int newfd, int flags)
  EcallStatus ECALL_fcntl64();                // 25, rev_fcntl64(unsigned int fd, unsigned int cmd, unsigned long arg)
  EcallStatus ECALL_inotify_init1();          // 26, rev_inotify_init1(int flags)
  EcallStatus ECALL_inotify_add_watch();      // 27, rev_inotify_add_watch(int fd, const char  *path, u32 mask)
  EcallStatus ECALL_inotify_rm_watch();       // 28, rev_inotify_rm_watch(int fd, __s32 wd)
  EcallStatus ECALL_ioctl();                  // 29, rev_ioctl(unsigned int fd, unsigned int cmd, unsigned long arg)
  EcallStatus ECALL_ioprio_set();             // 30, rev_ioprio_set(int which, int who, int ioprio)
  EcallStatus ECALL_ioprio_get();             // 31, rev_ioprio_get(int which, int who)
  EcallStatus ECALL_flock();                  // 32, rev_flock(unsigned int fd, unsigned int cmd)
  EcallStatus ECALL_mknodat();                // 33, rev_mknodat(int dfd, const char  * filename, umode_t mode, unsigned dev)
  EcallStatus ECALL_mkdirat();                // 34, rev_mkdirat(int dfd, const char  * pathname, umode_t mode)
  EcallStatus ECALL_unlinkat();               // 35, rev_unlinkat(int dfd, const char  * pathname, int flag)
  EcallStatus ECALL_symlinkat();              // 36, rev_symlinkat(const char  * oldname, int newdfd, const char  * newname)
  EcallStatus ECALL_linkat();                 // 37, rev_linkat(int dfd, const char  * pathname, int flag)
  EcallStatus ECALL_renameat();               // 38, rev_renameat(int olddfd, const char  * oldname, int newdfd, const char  * newname)
  EcallStatus ECALL_umount();                 // 39, rev_umount(char  *name, int flags)
  EcallStatus ECALL_mount();                  // 40, rev_mount(char  *name, int flags)
  EcallStatus ECALL_pivot_root();             // 41, rev_pivot_root(const char  *new_root, const char  *put_old)
  EcallStatus ECALL_ni_syscall();             // 42, rev_ni_syscall(void)
  EcallStatus ECALL_statfs64();               // 43, rev_statfs64(const char  *path, size_t sz, struct statfs64  *buf)
  EcallStatus ECALL_fstatfs64();              // 44, rev_fstatfs64(unsigned int fd, size_t sz, struct statfs64  *buf)
  EcallStatus ECALL_truncate64();             // 45, rev_truncate64(const char  *path, loff_t length)
  EcallStatus ECALL_ftruncate64();            // 46, rev_ftruncate64(unsigned int fd, loff_t length)
  EcallStatus ECALL_fallocate();              // 47, rev_fallocate(int fd, int mode, loff_t offset, loff_t len)
  EcallStatus ECALL_faccessat();              // 48, rev_faccessat(int dfd, const char  *filename, int mode)
  EcallStatus ECALL_chdir();                  // 49, rev_chdir(const char  *filename)
  EcallStatus ECALL_fchdir();                 // 50, rev_fchdir(unsigned int fd)
  EcallStatus ECALL_chroot();                 // 51, rev_chroot(const char  *filename)
  EcallStatus ECALL_fchmod();                 // 52, rev_fchmod(unsigned int fd, umode_t mode)
  EcallStatus ECALL_fchmodat();               // 53, rev_fchmodat(int dfd, const char  * filename, umode_t mode)
  EcallStatus ECALL_fchownat();               // 54, rev_fchownat(int dfd, const char  *filename, uid_t user, gid_t group, int flag)
  EcallStatus ECALL_fchown();                 // 55, rev_fchown(unsigned int fd, uid_t user, gid_t group)
  EcallStatus ECALL_openat();                 // 56, rev_openat(int dfd, const char  *filename, int flags, umode_t mode)
  EcallStatus ECALL_close();                  // 57, rev_close(unsigned int fd)
  EcallStatus ECALL_vhangup();                // 58, rev_vhangup(void)
  EcallStatus ECALL_pipe2();                  // 59, rev_pipe2(int  *fildes, int flags)
  EcallStatus ECALL_quotactl();               // 60, rev_quotactl(unsigned int cmd, const char  *special, qid_t id, void  *addr)
  EcallStatus ECALL_getdents64();             // 61, rev_getdents64(unsigned int fd, struct linux_dirent64  *dirent, unsigned int count)
  EcallStatus ECALL_lseek();                  // 62, rev_llseek(unsigned int fd, unsigned long offset_high, unsigned long offset_low, loff_t  *result, unsigned int whence)
  EcallStatus ECALL_read();                   // 63, rev_read(unsigned int fd, char  *buf, size_t count)
  EcallStatus ECALL_write();                  // 64, rev_write(unsigned int fd, const char  *buf, size_t count)
  EcallStatus ECALL_readv();                  // 65, rev_readv(unsigned long fd, const struct iovec  *vec, unsigned long vlen)
  EcallStatus ECALL_writev();                 // 66, rev_writev(unsigned long fd, const struct iovec  *vec, unsigned long vlen)
  EcallStatus ECALL_pread64();                // 67, rev_pread64(unsigned int fd, char  *buf, size_t count, loff_t pos)
  EcallStatus ECALL_pwrite64();               // 68, rev_pwrite64(unsigned int fd, const char  *buf, size_t count, loff_t pos)
  EcallStatus ECALL_preadv();                 // 69, rev_preadv(unsigned long fd, const struct iovec  *vec, unsigned long vlen, unsigned long pos_l, unsigned long pos_h)
  EcallStatus ECALL_pwritev();                // 70, rev_pwritev(unsigned long fd, const struct iovec  *vec, unsigned long vlen, unsigned long pos_l, unsigned long pos_h)
  EcallStatus ECALL_sendfile64();             // 71, rev_sendfile64(int out_fd, int in_fd, loff_t  *offset, size_t count)
  EcallStatus ECALL_pselect6_time32();        // 72, rev_pselect6_time32(int, fd_set  *, fd_set  *, fd_set  *, struct old_timespec32  *, void  *)
  EcallStatus ECALL_ppoll_time32();           // 73, rev_ppoll_time32(struct pollfd  *, unsigned int, struct old_timespec32  *, const sigset_t  *, size_t)
  EcallStatus ECALL_signalfd4();              // 74, rev_signalfd4(int ufd, sigset_t  *user_mask, size_t sizemask, int flags)
  EcallStatus ECALL_vmsplice();               // 75, rev_vmsplice(int fd, const struct iovec  *iov, unsigned long nr_segs, unsigned int flags)
  EcallStatus ECALL_splice();                 // 76, rev_splice(int fd, const struct iovec  *iov, unsigned long nr_segs, unsigned int flags)
  EcallStatus ECALL_tee();                    // 77, rev_tee(int fdin, int fdout, size_t len, unsigned int flags)
  EcallStatus ECALL_readlinkat();             // 78, rev_readlinkat(int dfd, const char  *path, char  *buf, int bufsiz)
  EcallStatus ECALL_newfstatat();             // 79, rev_newfstatat(int dfd, const char  *filename, struct stat  *statbuf, int flag)
  EcallStatus ECALL_newfstat();               // 80, rev_newfstat(unsigned int fd, struct stat  *statbuf)
  EcallStatus ECALL_sync();                   // 81, rev_sync(void)
  EcallStatus ECALL_fsync();                  // 82, rev_fsync(unsigned int fd)
  EcallStatus ECALL_fdatasync();              // 83, rev_fdatasync(unsigned int fd)
  EcallStatus ECALL_sync_file_range2();       // 84, rev_sync_file_range2(int fd, unsigned int flags, loff_t offset, loff_t nbytes)
  EcallStatus ECALL_sync_file_range();        // 84, rev_sync_file_range(int fd, loff_t offset, loff_t nbytes, unsigned int flags)
  EcallStatus ECALL_timerfd_create();         // 85, rev_timerfd_create(int clockid, int flags)
  EcallStatus ECALL_timerfd_settime();        // 86, rev_timerfd_settime(int ufd, int flags, const struct __kernel_itimerspec  *utmr, struct __kernel_itimerspec  *otmr)
  EcallStatus ECALL_timerfd_gettime();        // 87, rev_timerfd_gettime(int ufd, struct __kernel_itimerspec  *otmr)
  EcallStatus ECALL_utimensat();              // 88, rev_utimensat(int dfd, const char  *filename, struct __kernel_timespec  *utimes, int flags)
  EcallStatus ECALL_acct();                   // 89, rev_acct(const char  *name)
  EcallStatus ECALL_capget();                 // 90, rev_capget(cap_user_header_t header, cap_user_data_t dataptr)
  EcallStatus ECALL_capset();                 // 91, rev_capset(cap_user_header_t header, const cap_user_data_t data)
  EcallStatus ECALL_personality();            // 92, rev_personality(unsigned int personality)
  EcallStatus ECALL_exit();                   // 93, rev_exit(int error_code)
  EcallStatus ECALL_exit_group();             // 94, rev_exit_group(int error_code)
  EcallStatus ECALL_waitid();                 // 95, rev_waitid(int which, pid_t pid, struct siginfo  *infop, int options, struct rusage  *ru)
  EcallStatus ECALL_set_tid_address();        // 96, rev_set_tid_address(int  *tidptr)
  EcallStatus ECALL_unshare();                // 97, rev_unshare(unsigned long unshare_flags)
  EcallStatus ECALL_futex();                  // 98, rev_futex(u32  *uaddr, int op, u32 val, struct __kernel_timespec  *utime, u32  *uaddr2, u32 val3)
  EcallStatus ECALL_set_robust_list();        // 99, rev_set_robust_list(struct robust_list_head  *head, size_t len)
  EcallStatus ECALL_get_robust_list();        // 100, rev_get_robust_list(int pid, struct robust_list_head  *  *head_ptr, size_t  *len_ptr)
  EcallStatus ECALL_nanosleep();              // 101, rev_nanosleep(struct __kernel_timespec  *rqtp, struct __kernel_timespec  *rmtp)
  EcallStatus ECALL_getitimer();              // 102, rev_getitimer(int which, struct __kernel_old_itimerval  *value)
  EcallStatus ECALL_setitimer();              // 103, rev_setitimer(int which, struct __kernel_old_itimerval  *value, struct __kernel_old_itimerval  *ovalue)
  EcallStatus ECALL_kexec_load();             // 104, rev_kexec_load(unsigned long entry, unsigned long nr_segments, struct kexec_segment  *segments, unsigned long flags)
  EcallStatus ECALL_init_module();            // 105, rev_init_module(void  *umod, unsigned long len, const char  *uargs)
  EcallStatus ECALL_delete_module();          // 106, rev_delete_module(const char  *name_user, unsigned int flags)
  EcallStatus ECALL_timer_create();           // 107, rev_timer_create(clockid_t which_clock, struct sigevent  *timer_event_spec, timer_t  * created_timer_id)
  EcallStatus ECALL_timer_gettime();          // 108, rev_timer_gettime(timer_t timer_id, struct __kernel_itimerspec  *setting)
  EcallStatus ECALL_timer_getoverrun();       // 109, rev_timer_getoverrun(timer_t timer_id)
  EcallStatus ECALL_timer_settime();          // 110, rev_timer_settime(timer_t timer_id, int flags, const struct __kernel_itimerspec  *new_setting, struct __kernel_itimerspec  *old_setting)
  EcallStatus ECALL_timer_delete();           // 111, rev_timer_delete(timer_t timer_id)
  EcallStatus ECALL_clock_settime();          // 112, rev_clock_settime(clockid_t which_clock, const struct __kernel_timespec  *tp)
  EcallStatus ECALL_clock_gettime();          // 113, rev_clock_gettime(clockid_t which_clock, struct __kernel_timespec  *tp)
  EcallStatus ECALL_clock_getres();           // 114, rev_clock_getres(clockid_t which_clock, struct __kernel_timespec  *tp)
  EcallStatus ECALL_clock_nanosleep();        // 115, rev_clock_nanosleep(clockid_t which_clock, int flags, const struct __kernel_timespec  *rqtp, struct __kernel_timespec  *rmtp)
  EcallStatus ECALL_syslog();                 // 116, rev_syslog(int type, char  *buf, int len)
  EcallStatus ECALL_ptrace();                 // 117, rev_ptrace(long request, long pid, unsigned long addr, unsigned long data)
  EcallStatus ECALL_sched_setparam();         // 118, rev_sched_setparam(pid_t pid, struct sched_param  *param)
  EcallStatus ECALL_sched_setscheduler();     // 119, rev_sched_setscheduler(pid_t pid, int policy, struct sched_param  *param)
  EcallStatus ECALL_sched_getscheduler();     // 120, rev_sched_getscheduler(pid_t pid)
  EcallStatus ECALL_sched_getparam();         // 121, rev_sched_getparam(pid_t pid, struct sched_param  *param)
  EcallStatus ECALL_sched_setaffinity();      // 122, rev_sched_setaffinity(pid_t pid, unsigned int len, unsigned long  *user_mask_ptr)
  EcallStatus ECALL_sched_getaffinity();      // 123, rev_sched_getaffinity(pid_t pid, unsigned int len, unsigned long  *user_mask_ptr)
  EcallStatus ECALL_sched_yield();            // 124, rev_sched_yield(void)
  EcallStatus ECALL_sched_get_priority_max(); // 125, rev_sched_get_priority_max(int policy)
  EcallStatus ECALL_sched_get_priority_min(); // 126, rev_sched_get_priority_min(int policy)
  EcallStatus ECALL_sched_rr_get_interval();  // 127, rev_sched_rr_get_interval(pid_t pid, struct __kernel_timespec  *interval)
  EcallStatus ECALL_restart_syscall();        // 128, rev_restart_syscall(void)
  EcallStatus ECALL_kill();                   // 129, rev_kill(pid_t pid, int sig)
  EcallStatus ECALL_tkill();                  // 130, rev_tkill(pid_t pid, int sig)
  EcallStatus ECALL_tgkill();                 // 131, rev_tgkill(pid_t tgid, pid_t pid, int sig)
  EcallStatus ECALL_sigaltstack();            // 132, rev_sigaltstack(const struct sigaltstack  *uss, struct sigaltstack  *uoss)
  EcallStatus ECALL_rt_sigsuspend();          // 133, rev_rt_sigsuspend(sigset_t  *unewset, size_t sigsetsize)
  EcallStatus ECALL_rt_sigaction();           // 134, rev_rt_sigaction(int, const struct sigaction  *, struct sigaction  *, size_t)
  EcallStatus ECALL_rt_sigprocmask();         // 135, rev_rt_sigprocmask(int how, sigset_t  *set, sigset_t  *oset, size_t sigsetsize)
  EcallStatus ECALL_rt_sigpending();          // 136, rev_rt_sigpending(sigset_t  *set, size_t sigsetsize)
  EcallStatus ECALL_rt_sigtimedwait_time32(); // 137, rev_rt_sigtimedwait_time32(const sigset_t  *uthese, siginfo_t  *uinfo, const struct old_timespec32  *uts, size_t sigsetsize)
  EcallStatus ECALL_rt_sigqueueinfo();        // 138, rev_rt_sigqueueinfo(pid_t pid, int sig, siginfo_t  *uinfo)
  EcallStatus ECALL_setpriority();            // 140, rev_setpriority(int which, int who, int niceval)
  EcallStatus ECALL_getpriority();            // 141, rev_getpriority(int which, int who)
  EcallStatus ECALL_reboot();                 // 142, rev_reboot(int magic1, int magic2, unsigned int cmd, void  *arg)
  EcallStatus ECALL_setregid();               // 143, rev_setregid(gid_t rgid, gid_t egid)
  EcallStatus ECALL_setgid();                 // 144, rev_setgid(gid_t gid)
  EcallStatus ECALL_setreuid();               // 145, rev_setreuid(uid_t ruid, uid_t euid)
  EcallStatus ECALL_setuid();                 // 146, rev_setuid(uid_t uid)
  EcallStatus ECALL_setresuid();              // 147, rev_setresuid(uid_t ruid, uid_t euid, uid_t suid)
  EcallStatus ECALL_getresuid();              // 148, rev_getresuid(uid_t  *ruid, uid_t  *euid, uid_t  *suid)
  EcallStatus ECALL_setresgid();              // 149, rev_setresgid(gid_t rgid, gid_t egid, gid_t sgid)
  EcallStatus ECALL_getresgid();              // 150, rev_getresgid(gid_t  *rgid, gid_t  *egid, gid_t  *sgid)
  EcallStatus ECALL_setfsuid();               // 151, rev_setfsuid(uid_t uid)
  EcallStatus ECALL_setfsgid();               // 152, rev_setfsgid(gid_t gid)
  EcallStatus ECALL_times();                  // 153, rev_times(struct tms  *tbuf)
  EcallStatus ECALL_setpgid();                // 154, rev_setpgid(pid_t pid, pid_t pgid)
  EcallStatus ECALL_getpgid();                // 155, rev_getpgid(pid_t pid)
  EcallStatus ECALL_getsid();                 // 156, rev_getsid(pid_t pid)
  EcallStatus ECALL_setsid();                 // 157, rev_setsid(void)
  EcallStatus ECALL_getgroups();              // 158, rev_getgroups(int gidsetsize, gid_t  *grouplist)
  EcallStatus ECALL_setgroups();              // 159, rev_setgroups(int gidsetsize, gid_t  *grouplist)
  EcallStatus ECALL_newuname();               // 160, rev_newuname(struct new_utsname  *name)
  EcallStatus ECALL_sethostname();            // 161, rev_sethostname(char  *name, int len)
  EcallStatus ECALL_setdomainname();          // 162, rev_setdomainname(char  *name, int len)
  EcallStatus ECALL_getrlimit();              // 163, rev_getrlimit(unsigned int resource, struct rlimit  *rlim)
  EcallStatus ECALL_setrlimit();              // 164, rev_setrlimit(unsigned int resource, struct rlimit  *rlim)
  EcallStatus ECALL_getrusage();              // 165, rev_getrusage(int who, struct rusage  *ru)
  EcallStatus ECALL_umask();                  // 166, rev_umask(int mask)
  EcallStatus ECALL_prctl();                  // 167, rev_prctl(int option, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5)
  EcallStatus ECALL_getcpu();                 // 168, rev_getcpu(unsigned  *cpu, unsigned  *node, struct getcpu_cache  *cache)
  EcallStatus ECALL_gettimeofday();           // 169, rev_gettimeofday(struct __kernel_old_timeval  *tv, struct timezone  *tz)
  EcallStatus ECALL_settimeofday();           // 170, rev_settimeofday(struct __kernel_old_timeval  *tv, struct timezone  *tz)
  EcallStatus ECALL_adjtimex();               // 171, rev_adjtimex(struct __kernel_timex  *txc_p)
  EcallStatus ECALL_getpid();                 // 172, rev_getpid(void)
  EcallStatus ECALL_getppid();                // 173, rev_getppid(void)
  EcallStatus ECALL_getuid();                 // 174, rev_getuid(void)
  EcallStatus ECALL_geteuid();                // 175, rev_geteuid(void)
  EcallStatus ECALL_getgid();                 // 176, rev_getgid(void)
  EcallStatus ECALL_getegid();                // 177, rev_getegid(void)
  EcallStatus ECALL_gettid();                 // 178, rev_gettid(void)
  EcallStatus ECALL_sysinfo();                // 179, rev_sysinfo(struct sysinfo  *info)
  EcallStatus ECALL_mq_open();                // 180, rev_mq_open(const char  *name, int oflag, umode_t mode, struct mq_attr  *attr)
  EcallStatus ECALL_mq_unlink();              // 181, rev_mq_unlink(const char  *name)
  EcallStatus ECALL_mq_timedsend();           // 182, rev_mq_timedsend(mqd_t mqdes, const char  *msg_ptr, size_t msg_len, unsigned int msg_prio, const struct __kernel_timespec  *abs_timeout)
  EcallStatus ECALL_mq_timedreceive();        // 183, rev_mq_timedreceive(mqd_t mqdes, char  *msg_ptr, size_t msg_len, unsigned int  *msg_prio, const struct __kernel_timespec  *abs_timeout)
  EcallStatus ECALL_mq_notify();              // 184, rev_mq_notify(mqd_t mqdes, const struct sigevent  *notification)
  EcallStatus ECALL_mq_getsetattr();          // 185, rev_mq_getsetattr(mqd_t mqdes, const struct mq_attr  *mqstat, struct mq_attr  *omqstat)
  EcallStatus ECALL_msgget();                 // 186, rev_msgget(key_t key, int msgflg)
  EcallStatus ECALL_msgctl();                 // 187, rev_old_msgctl(int msqid, int cmd, struct msqid_ds  *buf)
  EcallStatus ECALL_msgrcv();                 // 188, rev_msgrcv(int msqid, struct msgbuf  *msgp, size_t msgsz, long msgtyp, int msgflg)
  EcallStatus ECALL_msgsnd();                 // 189, rev_msgsnd(int msqid, struct msgbuf  *msgp, size_t msgsz, int msgflg)
  EcallStatus ECALL_semget();                 // 190, rev_semget(key_t key, int nsems, int semflg)
  EcallStatus ECALL_semctl();                 // 191, rev_semctl(int semid, int semnum, int cmd, unsigned long arg)
  EcallStatus ECALL_semtimedop();             // 192, rev_semtimedop(int semid, struct sembuf  *sops, unsigned nsops, const struct __kernel_timespec  *timeout)
  EcallStatus ECALL_semop();                  // 193, rev_semop(int semid, struct sembuf  *sops, unsigned nsops)
  EcallStatus ECALL_shmget();                 // 194, rev_shmget(key_t key, size_t size, int flag)
  EcallStatus ECALL_shmctl();                 // 195, rev_old_shmctl(int shmid, int cmd, struct shmid_ds  *buf)
  EcallStatus ECALL_shmat();                  // 196, rev_shmat(int shmid, char  *shmaddr, int shmflg)
  EcallStatus ECALL_shmdt();                  // 197, rev_shmdt(char  *shmaddr)
  EcallStatus ECALL_socket();                 // 198, rev_socket(int, int, int)
  EcallStatus ECALL_socketpair();             // 199, rev_socketpair(int, int, int, int  *)
  EcallStatus ECALL_bind();                   // 200, rev_bind(int, struct sockaddr  *, int)
  EcallStatus ECALL_listen();                 // 201, rev_listen(int, int)
  EcallStatus ECALL_accept();                 // 202, rev_accept(int, struct sockaddr  *, int  *)
  EcallStatus ECALL_connect();                // 203, rev_connect(int, struct sockaddr  *, int)
  EcallStatus ECALL_getsockname();            // 204, rev_getsockname(int, struct sockaddr  *, int  *)
  EcallStatus ECALL_getpeername();            // 205, rev_getpeername(int, struct sockaddr  *, int  *)
  EcallStatus ECALL_sendto();                 // 206, rev_sendto(int, void  *, size_t, unsigned, struct sockaddr  *, int)
  EcallStatus ECALL_recvfrom();               // 207, rev_recvfrom(int, void  *, size_t, unsigned, struct sockaddr  *, int  *)
  EcallStatus ECALL_setsockopt();             // 208, rev_setsockopt(int fd, int level, int optname, char  *optval, int optlen)
  EcallStatus ECALL_getsockopt();             // 209, rev_getsockopt(int fd, int level, int optname, char  *optval, int  *optlen)
  EcallStatus ECALL_shutdown();               // 210, rev_shutdown(int, int)
  EcallStatus ECALL_sendmsg();                // 211, rev_sendmsg(int fd, struct user_msghdr  *msg, unsigned flags)
  EcallStatus ECALL_recvmsg();                // 212, rev_recvmsg(int fd, struct user_msghdr  *msg, unsigned flags)
  EcallStatus ECALL_readahead();              // 213, rev_readahead(int fd, loff_t offset, size_t count)
  EcallStatus ECALL_brk();                    // 214, rev_brk(unsigned long brk)
  EcallStatus ECALL_munmap();                 // 215, rev_munmap(unsigned long addr, size_t len)
  EcallStatus ECALL_mremap();                 // 216, rev_mremap(unsigned long addr, unsigned long old_len, unsigned long new_len, unsigned long flags, unsigned long new_addr)
  EcallStatus ECALL_add_key();                // 217, rev_add_key(const char  *_type, const char  *_description, const void  *_payload, size_t plen, key_serial_t destringid)
  EcallStatus ECALL_request_key();            // 218, rev_request_key(const char  *_type, const char  *_description, const char  *_callout_info, key_serial_t destringid)
  EcallStatus ECALL_keyctl();                 // 219, rev_keyctl(int cmd, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5)
  EcallStatus ECALL_clone();                  // 220, rev_clone(unsigned long, unsigned long, int  *, unsigned long, int  *)
  EcallStatus ECALL_execve();                 // 221, rev_execve(const char  *filename, const char  *const  *argv, const char  *const  *envp)
  EcallStatus ECALL_mmap();                   // 222, rev_old_mmap(struct mmap_arg_struct  *arg)
  EcallStatus ECALL_fadvise64_64();           // 223, rev_fadvise64_64(int fd, loff_t offset, loff_t len, int advice)
  EcallStatus ECALL_swapon();                 // 224, rev_swapon(const char  *specialfile, int swap_flags)
  EcallStatus ECALL_swapoff();                // 225, rev_swapoff(const char  *specialfile)
  EcallStatus ECALL_mprotect();               // 226, rev_mprotect(unsigned long start, size_t len, unsigned long prot)
  EcallStatus ECALL_msync();                  // 227, rev_msync(unsigned long start, size_t len, int flags)
  EcallStatus ECALL_mlock();                  // 228, rev_mlock(unsigned long start, size_t len)
  EcallStatus ECALL_munlock();                // 229, rev_munlock(unsigned long start, size_t len)
  EcallStatus ECALL_mlockall();               // 230, rev_mlockall(int flags)
  EcallStatus ECALL_munlockall();             // 231, rev_munlockall(void)
  EcallStatus ECALL_mincore();                // 232, rev_mincore(unsigned long start, size_t len, unsigned char  * vec)
  EcallStatus ECALL_madvise();                // 233, rev_madvise(unsigned long start, size_t len, int behavior)
  EcallStatus ECALL_remap_file_pages();       // 234, rev_remap_file_pages(unsigned long start, unsigned long size, unsigned long prot, unsigned long pgoff, unsigned long flags)
  EcallStatus ECALL_mbind();                  // 235, rev_mbind(unsigned long start, unsigned long len, unsigned long mode, const unsigned long  *nmask, unsigned long maxnode, unsigned flags)
  EcallStatus ECALL_get_mempolicy();          // 236, rev_get_mempolicy(int  *policy, unsigned long  *nmask, unsigned long maxnode, unsigned long addr, unsigned long flags)
  EcallStatus ECALL_set_mempolicy();          // 237, rev_set_mempolicy(int mode, const unsigned long  *nmask, unsigned long maxnode)
  EcallStatus ECALL_migrate_pages();          // 238, rev_migrate_pages(pid_t pid, unsigned long maxnode, const unsigned long  *from, const unsigned long  *to)
  EcallStatus ECALL_move_pages();             // 239, rev_move_pages(pid_t pid, unsigned long nr_pages, const void  *  *pages, const int  *nodes, int  *status, int flags)
  EcallStatus ECALL_rt_tgsigqueueinfo();      // 240, rev_rt_tgsigqueueinfo(pid_t tgid, pid_t pid, int sig, siginfo_t  *uinfo)
  EcallStatus ECALL_perf_event_open();        // 241, rev_perf_event_open(")
  EcallStatus ECALL_accept4();                // 242, rev_accept4(int, struct sockaddr  *, int  *, int)
  EcallStatus ECALL_recvmmsg_time32();        // 243, rev_recvmmsg_time32(int fd, struct mmsghdr  *msg, unsigned int vlen, unsigned flags, struct old_timespec32  *timeout)
  EcallStatus ECALL_wait4();                  // 260, rev_wait4(pid_t pid, int  *stat_addr, int options, struct rusage  *ru)
  EcallStatus ECALL_prlimit64();              // 261, rev_prlimit64(pid_t pid, unsigned int resource, const struct rlimit64  *new_rlim, struct rlimit64  *old_rlim)
  EcallStatus ECALL_fanotify_init();          // 262, rev_fanotify_init(unsigned int flags, unsigned int event_f_flags)
  EcallStatus ECALL_fanotify_mark();          // 263, rev_fanotify_mark(int fanotify_fd, unsigned int flags, u64 mask, int fd, const char  *pathname)
  EcallStatus ECALL_name_to_handle_at();      // 264, rev_name_to_handle_at(int dfd, const char  *name, struct file_handle  *handle, int  *mnt_id, int flag)
  EcallStatus ECALL_open_by_handle_at();      // 265, rev_open_by_handle_at(int mountdirfd, struct file_handle  *handle, int flags)
  EcallStatus ECALL_clock_adjtime();          // 266, rev_clock_adjtime(clockid_t which_clock, struct __kernel_timex  *tx)
  EcallStatus ECALL_syncfs();                 // 267, rev_syncfs(int fd)
  EcallStatus ECALL_setns();                  // 268, rev_setns(int fd, int nstype)
  EcallStatus ECALL_sendmmsg();               // 269, rev_sendmmsg(int fd, struct mmsghdr  *msg, unsigned int vlen, unsigned flags)
  EcallStatus ECALL_process_vm_readv();       // 270, rev_process_vm_readv(pid_t pid, const struct iovec  *lvec, unsigned long liovcnt, const struct iovec  *rvec, unsigned long riovcnt, unsigned long flags)
  EcallStatus ECALL_process_vm_writev();      // 271, rev_process_vm_writev(pid_t pid, const struct iovec  *lvec, unsigned long liovcnt, const struct iovec  *rvec, unsigned long riovcnt, unsigned long flags)
  EcallStatus ECALL_kcmp();                   // 272, rev_kcmp(pid_t pid1, pid_t pid2, int type, unsigned long idx1, unsigned long idx2)
  EcallStatus ECALL_finit_module();           // 273, rev_finit_module(int fd, const char  *uargs, int flags)
  EcallStatus ECALL_sched_setattr();          // 274, rev_sched_setattr(pid_t pid, struct sched_attr  *attr, unsigned int flags)
  EcallStatus ECALL_sched_getattr();          // 275, rev_sched_getattr(pid_t pid, struct sched_attr  *attr, unsigned int size, unsigned int flags)
  EcallStatus ECALL_renameat2();              // 276, rev_renameat2(int olddfd, const char  *oldname, int newdfd, const char  *newname, unsigned int flags)
  EcallStatus ECALL_seccomp();                // 277, rev_seccomp(unsigned int op, unsigned int flags, void  *uargs)
  EcallStatus ECALL_getrandom();              // 278, rev_getrandom(char  *buf, size_t count, unsigned int flags)
  EcallStatus ECALL_memfd_create();           // 279, rev_memfd_create(const char  *uname_ptr, unsigned int flags)
  EcallStatus ECALL_bpf();                    // 280, rev_bpf(int cmd, union bpf_attr *attr, unsigned int size)
  EcallStatus ECALL_execveat();               // 281, rev_execveat(int dfd, const char  *filename, const char  *const  *argv, const char  *const  *envp, int flags)
  EcallStatus ECALL_userfaultfd();            // 282, rev_userfaultfd(int flags)
  EcallStatus ECALL_membarrier();             // 283, rev_membarrier(int cmd, unsigned int flags, int cpu_id)
  EcallStatus ECALL_mlock2();                 // 284, rev_mlock2(unsigned long start, size_t len, int flags)
  EcallStatus ECALL_copy_file_range();        // 285, rev_copy_file_range(int fd_in, loff_t  *off_in, int fd_out, loff_t  *off_out, size_t len, unsigned int flags)
  EcallStatus ECALL_preadv2();                // 286, rev_preadv2(unsigned long fd, const struct iovec  *vec, unsigned long vlen, unsigned long pos_l, unsigned long pos_h, rwf_t flags)
  EcallStatus ECALL_pwritev2();               // 287, rev_pwritev2(unsigned long fd, const struct iovec  *vec, unsigned long vlen, unsigned long pos_l, unsigned long pos_h, rwf_t flags)
  EcallStatus ECALL_pkey_mprotect();          // 288, rev_pkey_mprotect(unsigned long start, size_t len, unsigned long prot, int pkey)
  EcallStatus ECALL_pkey_alloc();             // 289, rev_pkey_alloc(unsigned long flags, unsigned long init_val)
  EcallStatus ECALL_pkey_free();              // 290, rev_pkey_free(int pkey)
  EcallStatus ECALL_statx();                  // 291, rev_statx(int dfd, const char  *path, unsigned flags, unsigned mask, struct statx  *buffer)
  EcallStatus ECALL_io_pgetevents();          // 292, rev_io_pgetevents(aio_context_t ctx_id, long min_nr, long nr, struct io_event  *events, struct __kernel_timespec  *timeout, const struct __aio_sigset *sig)
  EcallStatus ECALL_rseq();                   // 293, rev_rseq(struct rseq  *rseq, uint32_t rseq_len, int flags, uint32_t sig)
  EcallStatus ECALL_kexec_file_load();        // 294, rev_kexec_file_load(int kernel_fd, int initrd_fd, unsigned long cmdline_len, const char  *cmdline_ptr, unsigned long flags)
  // EcallStatus ECALL_clock_gettime();          // 403, rev_clock_gettime(clockid_t which_clock, struct __kernel_timespec  *tp)
  // EcallStatus ECALL_clock_settime();          // 404, rev_clock_settime(clockid_t which_clock, const struct __kernel_timespec  *tp)
  // EcallStatus ECALL_clock_adjtime();          // 405, rev_clock_adjtime(clockid_t which_clock, struct __kernel_timex  *tx)
  // EcallStatus ECALL_clock_getres();           // 406, rev_clock_getres(clockid_t which_clock, struct __kernel_timespec  *tp)
  // EcallStatus ECALL_clock_nanosleep();        // 407, rev_clock_nanosleep(clockid_t which_clock, int flags, const struct __kernel_timespec  *rqtp, struct __kernel_timespec  *rmtp)
  // EcallStatus ECALL_timer_gettime();          // 408, rev_timer_gettime(timer_t timer_id, struct __kernel_itimerspec  *setting)
  // EcallStatus ECALL_timer_settime();          // 409, rev_timer_settime(timer_t timer_id, int flags, const struct __kernel_itimerspec  *new_setting, struct __kernel_itimerspec  *old_setting)
  // EcallStatus ECALL_timerfd_gettime();        // 410, rev_timerfd_gettime(int ufd, struct __kernel_itimerspec  *otmr)
  // EcallStatus ECALL_timerfd_settime();        // 411, rev_timerfd_settime(int ufd, int flags, const struct __kernel_itimerspec  *utmr, struct __kernel_itimerspec  *otmr)
  // EcallStatus ECALL_utimensat();              // 412, rev_utimensat(int dfd, const char  *filename, struct __kernel_timespec  *utimes, int flags)
  // EcallStatus ECALL_io_pgetevents();          // 416, rev_io_pgetevents(aio_context_t ctx_id, long min_nr, long nr, struct io_event  *events, struct __kernel_timespec  *timeout, const struct __aio_sigset *sig)
  // EcallStatus ECALL_mq_timedsend();           // 418, rev_mq_timedsend(mqd_t mqdes, const char  *msg_ptr, size_t msg_len, unsigned int msg_prio, const struct __kernel_timespec  *abs_timeout)
  // EcallStatus ECALL_mq_timedreceive();        // 419, rev_mq_timedreceive(mqd_t mqdes, char  *msg_ptr, size_t msg_len, unsigned int  *msg_prio, const struct __kernel_timespec  *abs_timeout)
  // EcallStatus ECALL_semtimedop();             // 420, rev_semtimedop(int semid, struct sembuf  *sops, unsigned nsops, const struct __kernel_timespec  *timeout)
  // EcallStatus ECALL_futex();                  // 422, rev_futex(u32  *uaddr, int op, u32 val, struct __kernel_timespec  *utime, u32  *uaddr2, u32 val3)
  // EcallStatus ECALL_sched_rr_get_interval();  // 423, rev_sched_rr_get_interval(pid_t pid, struct __kernel_timespec  *interval)
  EcallStatus ECALL_pidfd_send_signal();      // 424, rev_pidfd_send_signal(int pidfd, int sig, siginfo_t  *info, unsigned int flags)
  EcallStatus ECALL_io_uring_setup();         // 425, rev_io_uring_setup(u32 entries, struct io_uring_params  *p)
  EcallStatus ECALL_io_uring_enter();         // 426, rev_io_uring_enter(unsigned int fd, u32 to_submit, u32 min_complete, u32 flags, const sigset_t  *sig, size_t sigsz)
  EcallStatus ECALL_io_uring_register();      // 427, rev_io_uring_register(unsigned int fd, unsigned int op, void  *arg, unsigned int nr_args)
  EcallStatus ECALL_open_tree();              // 428, rev_open_tree(int dfd, const char  *path, unsigned flags)
  EcallStatus ECALL_move_mount();             // 429, rev_move_mount(int from_dfd, const char  *from_path, int to_dfd, const char  *to_path, unsigned int ms_flags)
  EcallStatus ECALL_fsopen();                 // 430, rev_fsopen(const char  *fs_name, unsigned int flags)
  EcallStatus ECALL_fsconfig();               // 431, rev_fsconfig(int fs_fd, unsigned int cmd, const char  *key, const void  *value, int aux)
  EcallStatus ECALL_fsmount();                // 432, rev_fsmount(int fs_fd, unsigned int flags, unsigned int ms_flags)
  EcallStatus ECALL_fspick();                 // 433, rev_fspick(int dfd, const char  *path, unsigned int flags)
  EcallStatus ECALL_pidfd_open();             // 434, rev_pidfd_open(pid_t pid, unsigned int flags)
  EcallStatus ECALL_clone3();                 // 435, rev_clone3(struct clone_args  *uargs, size_t size)
  EcallStatus ECALL_close_range();            // 436, rev_close_range(unsigned int fd, unsigned int max_fd, unsigned int flags)
  EcallStatus ECALL_openat2();                // 437, rev_openat2(int dfd, const char  *filename, struct open_how *how, size_t size)
  EcallStatus ECALL_pidfd_getfd();            // 438, rev_pidfd_getfd(int pidfd, int fd, unsigned int flags)
  EcallStatus ECALL_faccessat2();             // 439, rev_faccessat2(int dfd, const char  *filename, int mode, int flags)
  EcallStatus ECALL_process_madvise();        // 440, rev_process_madvise(int pidfd, const struct iovec  *vec, size_t vlen, int behavior, unsigned int flags)

  // =============== REV specific functions
  EcallStatus ECALL_cpuinfo();                // 500, rev_cpuinfo(struct rev_cpuinfo *info);
  EcallStatus ECALL_perf_stats();             // 501, rev_perf_stats(struct rev_stats *stats);

  // =============== REV pthread functions
  EcallStatus ECALL_pthread_create();         // 1000, rev_pthread_create(pthread_t *thread, const pthread_attr_t  *attr, void  *(*start_routine)(void  *), void  *arg)
  EcallStatus ECALL_pthread_join();           // 1001, rev_pthread_join(pthread_t thread, void **retval);
  EcallStatus ECALL_pthread_exit();           // 1002, rev_pthread_exit(void* retval);

  EcallStatus ECALL_dump_mem_range();         // 9000, dump_mem_range(uint64_t addr, uint64_t size)
  EcallStatus ECALL_dump_mem_range_to_file(); // 9001, dump_mem_range_to_file(const char* outputFile, uint64_t addr, uint64_t size)
  EcallStatus ECALL_dump_stack();             // 9002, dump_stack()
  EcallStatus ECALL_dump_stack_to_file();     // 9003, dump_stack(const char* outputFile)

  EcallStatus ECALL_dump_valid_mem();         // 9004, dump_valid_mem()
  EcallStatus ECALL_dump_valid_mem_to_file(); // 9005, dump_valid_mem_to_file(const char* outputFile)

  EcallStatus ECALL_dump_thread_mem();         // 9006, dump_thread_mem()
  EcallStatus ECALL_dump_thread_mem_to_file(); // 9007, dump_thread_mem_to_file(const char* outputFile)
  // clang-format on

  /// RevCore: Table of ecall codes w/ corresponding function pointer implementations
  static const std::unordered_map<uint32_t, EcallStatus ( RevCore::* )()> Ecalls;

  /// RevCore: Execute the Ecall based on the code loaded in RegFile->GetSCAUSE()
  bool ExecEcall();

  /// RevCore: Get a pointer to the register file loaded into Hart w/ HartID
  RevRegFile* GetRegFile( unsigned HartID ) const;

  std::vector<RevInstEntry>            InstTable{};   ///< RevCore: target instruction table
  std::vector<std::unique_ptr<RevExt>> Extensions{};  ///< RevCore: vector of enabled extensions
  //std::vector<std::tuple<uint16_t, RevInst, bool>>  Pipeline; ///< RevCore: pipeline of instructions
  std::deque<std::pair<uint16_t, RevInst>>    Pipeline{};     ///< RevCore: pipeline of instructions
  std::unordered_map<std::string, unsigned>   NameToEntry{};  ///< RevCore: instruction mnemonic to table entry mapping
  std::unordered_multimap<uint32_t, unsigned> EncToEntry{};   ///< RevCore: instruction encoding to table entry mapping
  std::unordered_multimap<uint32_t, unsigned> CEncToEntry{};  ///< RevCore: compressed instruction encoding to table entry mapping
  std::unordered_map<unsigned, std::pair<unsigned, unsigned>> EntryToExt{};  ///< RevCore: instruction entry to extension mapping
  ///           first = Master table entry number
  ///           second = pair<Extension Index, Extension Entry>

  /// RevCore: finds an entry which matches an encoding whose predicate is true
  auto matchInst(
    const std::unordered_multimap<uint32_t, unsigned>& map,
    uint32_t                                           encoding,
    const std::vector<RevInstEntry>&                   InstTable,
    uint32_t                                           Inst
  ) const;

  /// RevCore: splits a string into tokens
  void splitStr( const std::string& s, char c, std::vector<std::string>& v );

  /// RevCore: parses the feature string for the target core
  bool ParseFeatureStr( std::string Feature );

  /// RevCore: loads the instruction table using the target features
  bool LoadInstructionTable();

  /// RevCore: see the instruction table the target features
  bool SeedInstTable();

  /// RevCore: enable the target extension by merging its instruction table with the master
  bool EnableExt( RevExt* Ext, bool Opt );

  /// RevCore: initializes the internal mapping tables
  bool InitTableMapping();

  /// RevCore: read in the user defined cost tables
  bool ReadOverrideTables();

  /// RevCore: compresses the encoding structure to a single value
  uint32_t CompressEncoding( RevInstEntry Entry );

  /// RevCore: compressed the compressed encoding structure to a single value
  uint32_t CompressCEncoding( RevInstEntry Entry );

  /// RevCore: extracts the instruction mnemonic from the table entry
  std::string ExtractMnemonic( RevInstEntry Entry );

  /// RevCore: reset the core and its associated features
  bool Reset();

  /// RevCore: set the PC
  void SetPC( uint64_t PC ) { RegFile->SetPC( PC ); }

  /// RevCore: prefetch the next instruction
  bool PrefetchInst();

  /// RevCore: decode the instruction at the current PC
  RevInst FetchAndDecodeInst();

  /// RevCore: decode a particular instruction opcode
  RevInst DecodeInst( uint32_t Inst ) const;

  /// RevCore: decode a compressed instruction
  RevInst DecodeCompressed( uint32_t Inst ) const;

  /// RevCore: decode an R-type instruction
  RevInst DecodeRInst( uint32_t Inst, unsigned Entry ) const;

  /// RevCore: decode an I-type instruction
  RevInst DecodeIInst( uint32_t Inst, unsigned Entry ) const;

  /// RevCore: decode an S-type instruction
  RevInst DecodeSInst( uint32_t Inst, unsigned Entry ) const;

  /// RevCore: decode a U-type instruction
  RevInst DecodeUInst( uint32_t Inst, unsigned Entry ) const;

  /// RevCore: decode a B-type instruction
  RevInst DecodeBInst( uint32_t Inst, unsigned Entry ) const;

  /// RevCore: decode a J-type instruction
  RevInst DecodeJInst( uint32_t Inst, unsigned Entry ) const;

  /// RevCore: decode an R4-type instruction
  RevInst DecodeR4Inst( uint32_t Inst, unsigned Entry ) const;

  /// RevCore: decode a compressed CR-type isntruction
  RevInst DecodeCRInst( uint16_t Inst, unsigned Entry ) const;

  /// RevCore: decode a compressed CI-type isntruction
  RevInst DecodeCIInst( uint16_t Inst, unsigned Entry ) const;

  /// RevCore: decode a compressed CSS-type isntruction
  RevInst DecodeCSSInst( uint16_t Inst, unsigned Entry ) const;

  /// RevCore: decode a compressed CIW-type isntruction
  RevInst DecodeCIWInst( uint16_t Inst, unsigned Entry ) const;

  /// RevCore: decode a compressed CL-type isntruction
  RevInst DecodeCLInst( uint16_t Inst, unsigned Entry ) const;

  /// RevCore: decode a compressed CS-type isntruction
  RevInst DecodeCSInst( uint16_t Inst, unsigned Entry ) const;

  /// RevCore: decode a compressed CA-type isntruction
  RevInst DecodeCAInst( uint16_t Inst, unsigned Entry ) const;

  /// RevCore: decode a compressed CB-type isntruction
  RevInst DecodeCBInst( uint16_t Inst, unsigned Entry ) const;

  /// RevCore: decode a compressed CJ-type isntruction
  RevInst DecodeCJInst( uint16_t Inst, unsigned Entry ) const;

  /// RevCore: Determine next thread to execute
  unsigned GetNextHartToDecodeID() const;

  /// RevCore: Whether any scoreboard bits are set
  bool AnyDependency( unsigned HartID, RevRegClass regClass = RevRegClass::RegUNKNOWN ) const {
    const RevRegFile* regFile = GetRegFile( HartID );
    switch( regClass ) {
    case RevRegClass::RegGPR: return regFile->RV_Scoreboard.any();
    case RevRegClass::RegFLOAT: return regFile->FP_Scoreboard.any();
    case RevRegClass::RegUNKNOWN: return regFile->RV_Scoreboard.any() || regFile->FP_Scoreboard.any();
    default: return false;
    }
  }

  /// RevCore: Check LS queue for outstanding load - ignore x0
  bool LSQCheck( unsigned HartID, const RevRegFile* regFile, uint16_t reg, RevRegClass regClass ) const {
    if( reg == 0 && regClass == RevRegClass::RegGPR ) {
      return false;  // GPR x0 is not considered
    } else {
      return regFile->GetLSQueue()->count( LSQHash( reg, regClass, HartID ) ) > 0;
    }
  }

  /// RevCore: Check scoreboard for a source register dependency
  bool ScoreboardCheck( const RevRegFile* regFile, uint16_t reg, RevRegClass regClass ) const {
    switch( regClass ) {
    case RevRegClass::RegGPR: return reg != 0 && regFile->RV_Scoreboard[reg];
    case RevRegClass::RegFLOAT: return regFile->FP_Scoreboard[reg];
    default: return false;
    }
  }

  bool HartHasNoDependencies( unsigned HartID ) const { return !AnyDependency( HartID ); }

  ///< Removes thread from Hart and returns it
  std::unique_ptr<RevThread> PopThreadFromHart( unsigned HartID );

  /// RevCore: Check scoreboard for pipeline hazards
  bool DependencyCheck( unsigned HartID, const RevInst* Inst ) const;

  /// RevCore: Set or clear scoreboard based on instruction destination
  void DependencySet( unsigned HartID, const RevInst* Inst, bool value = true ) {
    DependencySet( HartID, Inst->rd, InstTable[Inst->entry].rdClass, value );
  }

  /// RevCore: Clear scoreboard on instruction retirement
  void DependencyClear( unsigned HartID, const RevInst* Inst ) { DependencySet( HartID, Inst, false ); }

  /// RevCore: Set or clear scoreboard based on register number and floating point.
  template<typename T>
  void DependencySet( unsigned HartID, T RegNum, RevRegClass regClass, bool value = true ) {
    if( size_t( RegNum ) < _REV_NUM_REGS_ ) {
      RevRegFile* regFile = GetRegFile( HartID );
      switch( regClass ) {
      case RevRegClass::RegGPR:
        if( size_t( RegNum ) != 0 )
          regFile->RV_Scoreboard[size_t( RegNum )] = value;
        break;
      case RevRegClass::RegFLOAT: regFile->FP_Scoreboard[size_t( RegNum )] = value; break;
      default: break;
      }
    }
  }

  /// RevCore: Clear scoreboard on instruction retirement
  template<typename T>
  void DependencyClear( unsigned HartID, T RegNum, RevRegClass regClass ) {
    DependencySet( HartID, RegNum, regClass, false );
  }

};  // class RevCore

}  // namespace SST::RevCPU

#endif

// EOF
