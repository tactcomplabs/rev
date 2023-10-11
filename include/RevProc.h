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
#include "RevOpts.h"
#include "RevMem.h"
#include "RevFeature.h"
#include "RevLoader.h"
#include "RevInstTable.h"
#include "AllRevInstTables.h"
#include "PanExec.h"
#include "RevPrefetcher.h"
#include "RevCoProc.h"
#include "RevTracer.h"
#include "RevThread.h"
#include "RevRand.h"
#include "RevProcPasskey.h"
#include "RevHart.h"
#define SYSCALL_TYPES_ONLY
#include "../common/syscalls/syscalls.h"
#include "../common/include/RevCommon.h"

#define _PAN_FWARE_JUMP_            0x0000000000010000

namespace SST::RevCPU{
class RevCoProc;

class RevProc{
public:
  /// RevProc: standard constructor
  RevProc( unsigned Id, RevOpts *Opts, unsigned NumHarts, RevMem *Mem, RevLoader *Loader,
           std::unordered_map<uint32_t, std::shared_ptr<RevThread>>& AssignedThreads,
           std::function<uint32_t()> GetNewThreadID, SST::Output *Output );

  /// RevProc: standard destructor
  ~RevProc() = default;

  /// RevProc: per-processor clock function
  bool ClockTick( SST::Cycle_t currentCycle );

  /// RevProc: Called by RevCPU when there is no more work to do (ie. All RevThreads are ThreadState::DONE )
  void PrintStatSummary();

  /// RevProc: halt the CPU
  bool Halt();

  /// RevProc: resume the CPU
  bool Resume();

  /// RevProc: execute a single step
  bool SingleStepHart();

  /// RevProc: retrieve the local PC for the correct feature set
  uint64_t GetPC() const { return RegFile->GetPC(); }

  /// RevProc: Debug mode read a register
  bool DebugReadReg(unsigned Idx, uint64_t *Value) const;

  /// RevProc: Debug mode write a register
  bool DebugWriteReg(unsigned Idx, uint64_t Value) const;

  /// RevProc: Is this an RV32 machine?
  bool DebugIsRV32() { return feature->IsRV32(); }

  /// RevProc: Set the PAN execution context
  void SetExecCtx(PanExec *P) { PExec = P; }

  /// RevProc: Set an optional tracer
  void SetTracer(RevTracer *T) { Tracer = T; }

  /// RevProc: Retrieve a random memory cost value
  unsigned RandCost() { return mem->RandCost(feature->GetMinCost(), feature->GetMaxCost()); }

  /// RevProc: Handle register faults
  void HandleRegFault(unsigned width);

  /// RevProc: Handle crack+decode faults
  void HandleCrackFault(unsigned width);

  /// RevProc: Handle ALU faults
  void HandleALUFault(unsigned width);

  struct RevProcStats {
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

  RevProcStats GetStats() { Stats.memStats = mem->memStats; return Stats; }

  RevMem& GetMem() const { return *mem; }

  // Called by RevCPU to handle the state changes threads may have happened during this Proc's ClockTick
  std::queue<std::shared_ptr<RevThread>>& GetThreadsThatChangedState() { return ThreadsThatChangedState; }
  const std::queue<std::shared_ptr<RevThread>>& GetThreadsThatChangedState() const { return ThreadsThatChangedState; }

  /// RevProc: SpawnThread creates a new thread and returns its ThreadID
  void CreateThread(uint32_t NewTid, uint64_t fn, void* arg);

  /// RevProc: Returns the current HartToExec active pid
  uint32_t GetActiveThreadID();

  // Checks if this thread has any reason it cannot be removed by RevCPU (ie. Dependencies / outstanding loads)
  bool ThreadCanBeRemoved(const uint32_t& ThreadID) ;

  /// RevProc: Gets the thread that is currently executing on the Hart
  const std::shared_ptr<RevThread>& GetThreadOnHart(unsigned HartID) const {
    static const std::shared_ptr<RevThread> null;
    auto it = AssignedThreads.find(Harts.at(HartID)->GetAssignedThreadID());
    return it != AssignedThreads.end() ? it->second : null;
  }

  ///< RevProc: Used for scheduling in RevCPU (if Utilization < 1, there is at least 1 unoccupied HART )
  double GetHartUtilization() const { return (AssignedThreads.size() * 100.0) / Harts.size(); }

  std::bitset<_MAX_HARTS_> HART_CTS; ///< RevProc: Thread is clear to start (proceed with decode)
  std::bitset<_MAX_HARTS_> HART_CTE; ///< RevProc: Thread is clear to execute (no register dependencides)

  ///< RevProc: Get this Proc's feature
  RevFeature* GetRevFeature() const { return feature; }

  ///< RevProc: Mark a current request as complete
  void MarkLoadComplete(const MemReq& req);

  ///< RevProc: Get pointer to Load / Store queue used to track memory operations
  std::shared_ptr<std::unordered_map<uint64_t, MemReq>> GetLSQueue(){ return LSQueue; }

  ///< RevProc: Add a co-processor to the RevProc
  void SetCoProc(RevCoProc* coproc);

//--------------- External Interface for use with Co-Processor -------------------------
  ///< RevProc: Allow a co-processor to manipulate the scoreboard by setting a bit. Note the RevProcPassKey may only
  ///  be created by a RevCoProc (or a class derived from RevCoProc) so this funciton may not be called from even within
  ///  RevProc
  void ExternalDepSet(RevProcPasskey<RevCoProc>, uint16_t HartID, uint16_t RegNum, bool isFloat, bool value = true){
    DependencySet(HartID, RegNum, isFloat, value);
  }

  ///< RevProc: Allow a co-processor to manipulate the scoreboard by clearing a bit. Note the RevProcPassKey may only
  ///  be created by a RevCoProc (or a class derived from RevCoProc) so this funciton may not be called from even within
  ///  RevProc 
  void ExternalDepClear(RevProcPasskey<RevCoProc>, uint16_t HartID, uint16_t RegNum, bool isFloat){
    DependencyClear(HartID, RegNum, isFloat);
  }

  ///< RevProc: Allow a co-processor to stall the pipeline of this proc and hold it in a stall condition
  ///  unitl ExternalReleaseHart() is called. Note the RevProcPassKey may only
  ///  be created by a RevCoProc (or a class derived from RevCoProc) so this funciton may not be called from even within
  ///  RevProc
  void ExternalStallHart(RevProcPasskey<RevCoProc>, uint16_t HartID);

  ///< RevProc: Allow a co-processor to release the pipeline of this proc and allow a hart to continue
  ///  execution (this un-does the ExternalStallHart() function ). Note the RevProcPassKey may only
  ///  be created by a RevCoProc (or a class derived from RevCoProc) so this funciton may not be called from even within
  ///  RevProc
  void ExternalReleaseHart(RevProcPasskey<RevCoProc>, uint16_t HartID);
  //------------- END External Interface -------------------------------
  // Find a Hart to assign ThreadID to
  void AssignThread(uint32_t ThreadID);

private:
  bool Halted;              ///< RevProc: determines if the core is halted
  bool Stalled;             ///< RevProc: determines if the core is stalled on instruction fetch
  bool SingleStep;          ///< RevProc: determines if we are in a single step
  bool CrackFault;          ///< RevProc: determiens if we need to handle a crack fault
  bool ALUFault;            ///< RevProc: determines if we need to handle an ALU fault
  unsigned fault_width;     ///< RevProc: the width of the target fault
  unsigned id;              ///< RevProc: processor id
  uint64_t ExecPC;          ///< RevProc: executing PC
  unsigned HartToDecode;    ///< RevProc: Current executing ThreadID
  unsigned HartToExec;      ///< RevProc: Thread to dispatch instruction
  std::vector<std::shared_ptr<RevHart>> Harts; ///< RevProc: vector of Harts
  uint64_t Retired;         ///< RevProc: number of retired instructions

  unsigned numHarts;        ///< RevProc: Number of Harts for this core
  RevOpts *opts;            ///< RevProc: options object
  RevMem *mem;              ///< RevProc: memory object
  RevCoProc* coProc;        ///< RevProc: attached co-processor
  RevLoader *loader;        ///< RevProc: loader object

  /// ThreadIDs and their corresponding RevThread (Size will never be greater than numHarts)
  std::unordered_map<uint32_t, std::shared_ptr<RevThread>>& AssignedThreads;

  // Checks to see if a given HartID has an assigned thread by checking if the threadid on that Hart still exists in the AssignedThreads map
  bool HartHasThread(unsigned HartID) const { return ( AssignedThreads.find(Harts.at(HartID)->GetAssignedThreadID()) != AssignedThreads.end() );}

  // Function pointer to the GetNewThreadID function in RevCPU (monotonically increasing thread ID counter)
  std::function<uint32_t()> GetNewThreadID;

  // If a given assigned thread experiences a change of state, it sets the corresponding bit
  std::queue<std::shared_ptr<RevThread>> ThreadsThatChangedState; ///< RevProc: used to signal to RevCPU that the thread assigned to HART has changed state

  SST::Output *output;                   ///< RevProc: output handler
  std::unique_ptr<RevFeature> featureUP; ///< RevProc: feature handler
  RevFeature* feature;
  PanExec *PExec;                        ///< RevProc: PAN exeuction context
  RevProcStats Stats{};                  ///< RevProc: collection of performance stats
  std::unique_ptr<RevPrefetcher> sfetch; ///< RevProc: stream instruction prefetcher

  std::shared_ptr<std::unordered_map<uint64_t, MemReq>> LSQueue; ///< RevProc: Load / Store queue used to track memory operations. Currently only tracks outstanding loads.

  RevRegFile* RegFile = nullptr; ///< RevProc: Initial pointer to HartToDecode RegFile

  RevTracer* Tracer = nullptr;            ///< RevProc: Tracer object
  
  std::bitset<_MAX_HARTS_> CoProcStallReq;
  ///< RevProc: Utility function for system calls that involve reading a string from memory
  EcallStatus EcallLoadAndParseString(RevInst& inst, uint64_t straddr, std::function<void()>);

  // - Many of these are not implemented
  // - Their existence in the ECalls table is solely to not throw errors
  // - This _should_ be a comprehensive list of system calls supported on RISC-V
  // - Beside each function declaration is the system call code followed by its corresponding declaration
  //   that you can find in `common/syscalls.h` (the file to be included to use system calls inside of rev)
  //
  EcallStatus ECALL_io_setup(RevInst& inst);               // 0, rev_io_setup(unsigned nr_reqs, aio_context_t  *ctx)
  EcallStatus ECALL_io_destroy(RevInst& inst);             // 1, rev_io_destroy(aio_context_t ctx)
  EcallStatus ECALL_io_submit(RevInst& inst);              // 2, rev_io_submit(aio_context_t, long, struct iocb  *  *)
  EcallStatus ECALL_io_cancel(RevInst& inst);              // 3, rev_io_cancel(aio_context_t ctx_id, struct iocb  *iocb, struct io_event  *result)
  EcallStatus ECALL_io_getevents(RevInst& inst);           // 4, rev_io_getevents(aio_context_t ctx_id, long min_nr, long nr, struct io_event  *events, struct __kernel_timespec  *timeout)
  EcallStatus ECALL_setxattr(RevInst& inst);               // 5, rev_setxattr(const char  *path, const char  *name, const void  *value, size_t size, int flags)
  EcallStatus ECALL_lsetxattr(RevInst& inst);              // 6, rev_lsetxattr(const char  *path, const char  *name, const void  *value, size_t size, int flags)
  EcallStatus ECALL_fsetxattr(RevInst& inst);              // 7, rev_fsetxattr(int fd, const char  *name, const void  *value, size_t size, int flags)
  EcallStatus ECALL_getxattr(RevInst& inst);               // 8, rev_getxattr(const char  *path, const char  *name, void  *value, size_t size)
  EcallStatus ECALL_lgetxattr(RevInst& inst);              // 9, rev_lgetxattr(const char  *path, const char  *name, void  *value, size_t size)
  EcallStatus ECALL_fgetxattr(RevInst& inst);              // 10, rev_fgetxattr(int fd, const char  *name, void  *value, size_t size)
  EcallStatus ECALL_listxattr(RevInst& inst);              // 11, rev_listxattr(const char  *path, char  *list, size_t size)
  EcallStatus ECALL_llistxattr(RevInst& inst);             // 12, rev_llistxattr(const char  *path, char  *list, size_t size)
  EcallStatus ECALL_flistxattr(RevInst& inst);             // 13, rev_flistxattr(int fd, char  *list, size_t size)
  EcallStatus ECALL_removexattr(RevInst& inst);            // 14, rev_removexattr(const char  *path, const char  *name)
  EcallStatus ECALL_lremovexattr(RevInst& inst);           // 15, rev_lremovexattr(const char  *path, const char  *name)
  EcallStatus ECALL_fremovexattr(RevInst& inst);           // 16, rev_fremovexattr(int fd, const char  *name)
  EcallStatus ECALL_getcwd(RevInst& inst);                 // 17, rev_getcwd(char  *buf, unsigned long size)
  EcallStatus ECALL_lookup_dcookie(RevInst& inst);         // 18, rev_lookup_dcookie(u64 cookie64, char  *buf, size_t len)
  EcallStatus ECALL_eventfd2(RevInst& inst);               // 19, rev_eventfd2(unsigned int count, int flags)
  EcallStatus ECALL_epoll_create1(RevInst& inst);          // 20, rev_epoll_create1(int flags)
  EcallStatus ECALL_epoll_ctl(RevInst& inst);              // 21, rev_epoll_ctl(int epfd, int op, int fd, struct epoll_event  *event)
  EcallStatus ECALL_epoll_pwait(RevInst& inst);            // 22, rev_epoll_pwait(int epfd, struct epoll_event  *events, int maxevents, int timeout, const sigset_t  *sigmask, size_t sigsetsize)
  EcallStatus ECALL_dup(RevInst& inst);                    // 23, rev_dup(unsigned int fildes)
  EcallStatus ECALL_dup3(RevInst& inst);                   // 24, rev_dup3(unsigned int oldfd, unsigned int newfd, int flags)
  EcallStatus ECALL_fcntl64(RevInst& inst);                // 25, rev_fcntl64(unsigned int fd, unsigned int cmd, unsigned long arg)
  EcallStatus ECALL_inotify_init1(RevInst& inst);          // 26, rev_inotify_init1(int flags)
  EcallStatus ECALL_inotify_add_watch(RevInst& inst);      // 27, rev_inotify_add_watch(int fd, const char  *path, u32 mask)
  EcallStatus ECALL_inotify_rm_watch(RevInst& inst);       // 28, rev_inotify_rm_watch(int fd, __s32 wd)
  EcallStatus ECALL_ioctl(RevInst& inst);                  // 29, rev_ioctl(unsigned int fd, unsigned int cmd, unsigned long arg)
  EcallStatus ECALL_ioprio_set(RevInst& inst);             // 30, rev_ioprio_set(int which, int who, int ioprio)
  EcallStatus ECALL_ioprio_get(RevInst& inst);             // 31, rev_ioprio_get(int which, int who)
  EcallStatus ECALL_flock(RevInst& inst);                  // 32, rev_flock(unsigned int fd, unsigned int cmd)
  EcallStatus ECALL_mknodat(RevInst& inst);                // 33, rev_mknodat(int dfd, const char  * filename, umode_t mode, unsigned dev)
  EcallStatus ECALL_mkdirat(RevInst& inst);                // 34, rev_mkdirat(int dfd, const char  * pathname, umode_t mode)
  EcallStatus ECALL_unlinkat(RevInst& inst);               // 35, rev_unlinkat(int dfd, const char  * pathname, int flag)
  EcallStatus ECALL_symlinkat(RevInst& inst);              // 36, rev_symlinkat(const char  * oldname, int newdfd, const char  * newname)
  EcallStatus ECALL_linkat(RevInst& inst);                 // 37, rev_unlinkat(int dfd, const char  * pathname, int flag)
  EcallStatus ECALL_renameat(RevInst& inst);               // 38, rev_renameat(int olddfd, const char  * oldname, int newdfd, const char  * newname)
  EcallStatus ECALL_umount(RevInst& inst);                 // 39, rev_umount(char  *name, int flags)
  EcallStatus ECALL_mount(RevInst& inst);                  // 40, rev_umount(char  *name, int flags)
  EcallStatus ECALL_pivot_root(RevInst& inst);             // 41, rev_pivot_root(const char  *new_root, const char  *put_old)
  EcallStatus ECALL_ni_syscall(RevInst& inst);             // 42, rev_ni_syscall(void)
  EcallStatus ECALL_statfs64(RevInst& inst);               // 43, rev_statfs64(const char  *path, size_t sz, struct statfs64  *buf)
  EcallStatus ECALL_fstatfs64(RevInst& inst);              // 44, rev_fstatfs64(unsigned int fd, size_t sz, struct statfs64  *buf)
  EcallStatus ECALL_truncate64(RevInst& inst);             // 45, rev_truncate64(const char  *path, loff_t length)
  EcallStatus ECALL_ftruncate64(RevInst& inst);            // 46, rev_ftruncate64(unsigned int fd, loff_t length)
  EcallStatus ECALL_fallocate(RevInst& inst);              // 47, rev_fallocate(int fd, int mode, loff_t offset, loff_t len)
  EcallStatus ECALL_faccessat(RevInst& inst);              // 48, rev_faccessat(int dfd, const char  *filename, int mode)
  EcallStatus ECALL_chdir(RevInst& inst);                  // 49, rev_chdir(const char  *filename)
  EcallStatus ECALL_fchdir(RevInst& inst);                 // 50, rev_fchdir(unsigned int fd)
  EcallStatus ECALL_chroot(RevInst& inst);                 // 51, rev_chroot(const char  *filename)
  EcallStatus ECALL_fchmod(RevInst& inst);                 // 52, rev_fchmod(unsigned int fd, umode_t mode)
  EcallStatus ECALL_fchmodat(RevInst& inst);               // 53, rev_fchmodat(int dfd, const char  * filename, umode_t mode)
  EcallStatus ECALL_fchownat(RevInst& inst);               // 54, rev_fchownat(int dfd, const char  *filename, uid_t user, gid_t group, int flag)
  EcallStatus ECALL_fchown(RevInst& inst);                 // 55, rev_fchown(unsigned int fd, uid_t user, gid_t group)
  EcallStatus ECALL_openat(RevInst& inst);                 // 56, rev_openat(int dfd, const char  *filename, int flags, umode_t mode)
  EcallStatus ECALL_close(RevInst& inst);                  // 57, rev_close(unsigned int fd)
  EcallStatus ECALL_vhangup(RevInst& inst);                // 58, rev_vhangup(void)
  EcallStatus ECALL_pipe2(RevInst& inst);                  // 59, rev_pipe2(int  *fildes, int flags)
  EcallStatus ECALL_quotactl(RevInst& inst);               // 60, rev_quotactl(unsigned int cmd, const char  *special, qid_t id, void  *addr)
  EcallStatus ECALL_getdents64(RevInst& inst);             // 61, rev_getdents64(unsigned int fd, struct linux_dirent64  *dirent, unsigned int count)
  EcallStatus ECALL_lseek(RevInst& inst);                  // 62, rev_llseek(unsigned int fd, unsigned long offset_high, unsigned long offset_low, loff_t  *result, unsigned int whence)
  EcallStatus ECALL_read(RevInst& inst);                   // 63, rev_read(unsigned int fd, char  *buf, size_t count)
  EcallStatus ECALL_write(RevInst& inst);                  // 64, rev_write(unsigned int fd, const char  *buf, size_t count)
  EcallStatus ECALL_readv(RevInst& inst);                  // 65, rev_readv(unsigned long fd, const struct iovec  *vec, unsigned long vlen)
  EcallStatus ECALL_writev(RevInst& inst);                 // 66, rev_writev(unsigned long fd, const struct iovec  *vec, unsigned long vlen)
  EcallStatus ECALL_pread64(RevInst& inst);                // 67, rev_pread64(unsigned int fd, char  *buf, size_t count, loff_t pos)
  EcallStatus ECALL_pwrite64(RevInst& inst);               // 68, rev_pwrite64(unsigned int fd, const char  *buf, size_t count, loff_t pos)
  EcallStatus ECALL_preadv(RevInst& inst);                 // 69, rev_preadv(unsigned long fd, const struct iovec  *vec, unsigned long vlen, unsigned long pos_l, unsigned long pos_h)
  EcallStatus ECALL_pwritev(RevInst& inst);                // 70, rev_pwritev(unsigned long fd, const struct iovec  *vec, unsigned long vlen, unsigned long pos_l, unsigned long pos_h)
  EcallStatus ECALL_sendfile64(RevInst& inst);             // 71, rev_sendfile64(int out_fd, int in_fd, loff_t  *offset, size_t count)
  EcallStatus ECALL_pselect6_time32(RevInst& inst);        // 72, rev_pselect6_time32(int, fd_set  *, fd_set  *, fd_set  *, struct old_timespec32  *, void  *)
  EcallStatus ECALL_ppoll_time32(RevInst& inst);           // 73, rev_ppoll_time32(struct pollfd  *, unsigned int, struct old_timespec32  *, const sigset_t  *, size_t)
  EcallStatus ECALL_signalfd4(RevInst& inst);              // 74, rev_signalfd4(int ufd, sigset_t  *user_mask, size_t sizemask, int flags)
  EcallStatus ECALL_vmsplice(RevInst& inst);               // 75, rev_vmsplice(int fd, const struct iovec  *iov, unsigned long nr_segs, unsigned int flags)
  EcallStatus ECALL_splice(RevInst& inst);                 // 76, rev_vmsplice(int fd, const struct iovec  *iov, unsigned long nr_segs, unsigned int flags)
  EcallStatus ECALL_tee(RevInst& inst);                    // 77, rev_tee(int fdin, int fdout, size_t len, unsigned int flags)
  EcallStatus ECALL_readlinkat(RevInst& inst);             // 78, rev_readlinkat(int dfd, const char  *path, char  *buf, int bufsiz)
  EcallStatus ECALL_newfstatat(RevInst& inst);             // 79, rev_newfstatat(int dfd, const char  *filename, struct stat  *statbuf, int flag)
  EcallStatus ECALL_newfstat(RevInst& inst);               // 80, rev_newfstat(unsigned int fd, struct stat  *statbuf)
  EcallStatus ECALL_sync(RevInst& inst);                   // 81, rev_sync(void)
  EcallStatus ECALL_fsync(RevInst& inst);                  // 82, rev_fsync(unsigned int fd)
  EcallStatus ECALL_fdatasync(RevInst& inst);              // 83, rev_fdatasync(unsigned int fd)
  EcallStatus ECALL_sync_file_range2(RevInst& inst);       // 84, rev_sync_file_range2(int fd, unsigned int flags, loff_t offset, loff_t nbytes)
  EcallStatus ECALL_sync_file_range(RevInst& inst);        // 84, rev_sync_file_range(int fd, loff_t offset, loff_t nbytes, unsigned int flags)
  EcallStatus ECALL_timerfd_create(RevInst& inst);         // 85, rev_timerfd_create(int clockid, int flags)
  EcallStatus ECALL_timerfd_settime(RevInst& inst);        // 86, rev_timerfd_settime(int ufd, int flags, const struct __kernel_itimerspec  *utmr, struct __kernel_itimerspec  *otmr)
  EcallStatus ECALL_timerfd_gettime(RevInst& inst);        // 87, rev_timerfd_gettime(int ufd, struct __kernel_itimerspec  *otmr)
  EcallStatus ECALL_utimensat(RevInst& inst);              // 88, rev_utimensat(int dfd, const char  *filename, struct __kernel_timespec  *utimes, int flags)
  EcallStatus ECALL_acct(RevInst& inst);                   // 89, rev_acct(const char  *name)
  EcallStatus ECALL_capget(RevInst& inst);                 // 90, rev_capget(cap_user_header_t header, cap_user_data_t dataptr)
  EcallStatus ECALL_capset(RevInst& inst);                 // 91, rev_capset(cap_user_header_t header, const cap_user_data_t data)
  EcallStatus ECALL_personality(RevInst& inst);            // 92, rev_personality(unsigned int personality)
  EcallStatus ECALL_exit(RevInst& inst);                   // 93, rev_exit(int error_code)
  EcallStatus ECALL_exit_group(RevInst& inst);             // 94, rev_exit_group(int error_code)
  EcallStatus ECALL_waitid(RevInst& inst);                 // 95, rev_waitid(int which, pid_t pid, struct siginfo  *infop, int options, struct rusage  *ru)
  EcallStatus ECALL_set_tid_address(RevInst& inst);        // 96, rev_set_tid_address(int  *tidptr)
  EcallStatus ECALL_unshare(RevInst& inst);                // 97, rev_unshare(unsigned long unshare_flags)
  EcallStatus ECALL_futex(RevInst& inst);                  // 98, rev_futex(u32  *uaddr, int op, u32 val, struct __kernel_timespec  *utime, u32  *uaddr2, u32 val3)
  EcallStatus ECALL_set_robust_list(RevInst& inst);        // 99, rev_set_robust_list(struct robust_list_head  *head, size_t len)
  EcallStatus ECALL_get_robust_list(RevInst& inst);        // 100, rev_get_robust_list(int pid, struct robust_list_head  *  *head_ptr, size_t  *len_ptr)
  EcallStatus ECALL_nanosleep(RevInst& inst);              // 101, rev_nanosleep(struct __kernel_timespec  *rqtp, struct __kernel_timespec  *rmtp)
  EcallStatus ECALL_getitimer(RevInst& inst);              // 102, rev_getitimer(int which, struct __kernel_old_itimerval  *value)
  EcallStatus ECALL_setitimer(RevInst& inst);              // 103, rev_setitimer(int which, struct __kernel_old_itimerval  *value, struct __kernel_old_itimerval  *ovalue)
  EcallStatus ECALL_kexec_load(RevInst& inst);             // 104, rev_kexec_load(unsigned long entry, unsigned long nr_segments, struct kexec_segment  *segments, unsigned long flags)
  EcallStatus ECALL_init_module(RevInst& inst);            // 105, rev_init_module(void  *umod, unsigned long len, const char  *uargs)
  EcallStatus ECALL_delete_module(RevInst& inst);          // 106, rev_delete_module(const char  *name_user, unsigned int flags)
  EcallStatus ECALL_timer_create(RevInst& inst);           // 107, rev_timer_create(clockid_t which_clock, struct sigevent  *timer_event_spec, timer_t  * created_timer_id)
  EcallStatus ECALL_timer_gettime(RevInst& inst);          // 108, rev_timer_gettime(timer_t timer_id, struct __kernel_itimerspec  *setting)
  EcallStatus ECALL_timer_getoverrun(RevInst& inst);       // 109, rev_timer_getoverrun(timer_t timer_id)
  EcallStatus ECALL_timer_settime(RevInst& inst);          // 110, rev_timer_settime(timer_t timer_id, int flags, const struct __kernel_itimerspec  *new_setting, struct __kernel_itimerspec  *old_setting)
  EcallStatus ECALL_timer_delete(RevInst& inst);           // 111, rev_timer_delete(timer_t timer_id)
  EcallStatus ECALL_clock_settime(RevInst& inst);          // 112, rev_clock_settime(clockid_t which_clock, const struct __kernel_timespec  *tp)
  EcallStatus ECALL_clock_gettime(RevInst& inst);          // 113, rev_clock_gettime(clockid_t which_clock, struct __kernel_timespec  *tp)
  EcallStatus ECALL_clock_getres(RevInst& inst);           // 114, rev_clock_getres(clockid_t which_clock, struct __kernel_timespec  *tp)
  EcallStatus ECALL_clock_nanosleep(RevInst& inst);        // 115, rev_clock_nanosleep(clockid_t which_clock, int flags, const struct __kernel_timespec  *rqtp, struct __kernel_timespec  *rmtp)
  EcallStatus ECALL_syslog(RevInst& inst);                 // 116, rev_syslog(int type, char  *buf, int len)
  EcallStatus ECALL_ptrace(RevInst& inst);                 // 117, rev_ptrace(long request, long pid, unsigned long addr, unsigned long data)
  EcallStatus ECALL_sched_setparam(RevInst& inst);         // 118, rev_sched_setparam(pid_t pid, struct sched_param  *param)
  EcallStatus ECALL_sched_setscheduler(RevInst& inst);     // 119, rev_sched_setscheduler(pid_t pid, int policy, struct sched_param  *param)
  EcallStatus ECALL_sched_getscheduler(RevInst& inst);     // 120, rev_sched_getscheduler(pid_t pid)
  EcallStatus ECALL_sched_getparam(RevInst& inst);         // 121, rev_sched_getparam(pid_t pid, struct sched_param  *param)
  EcallStatus ECALL_sched_setaffinity(RevInst& inst);      // 122, rev_sched_setaffinity(pid_t pid, unsigned int len, unsigned long  *user_mask_ptr)
  EcallStatus ECALL_sched_getaffinity(RevInst& inst);      // 123, rev_sched_getaffinity(pid_t pid, unsigned int len, unsigned long  *user_mask_ptr)
  EcallStatus ECALL_sched_yield(RevInst& inst);            // 124, rev_sched_yield(void)
  EcallStatus ECALL_sched_get_priority_max(RevInst& inst); // 125, rev_sched_get_priority_max(int policy)
  EcallStatus ECALL_sched_get_priority_min(RevInst& inst); // 126, rev_sched_get_priority_min(int policy)
  EcallStatus ECALL_sched_rr_get_interval(RevInst& inst);  // 127, rev_sched_rr_get_interval(pid_t pid, struct __kernel_timespec  *interval)
  EcallStatus ECALL_restart_syscall(RevInst& inst);        // 128, rev_restart_syscall(void)
  EcallStatus ECALL_kill(RevInst& inst);                   // 129, rev_kill(pid_t pid, int sig)
  EcallStatus ECALL_tkill(RevInst& inst);                  // 130, rev_tkill(pid_t pid, int sig)
  EcallStatus ECALL_tgkill(RevInst& inst);                 // 131, rev_tgkill(pid_t tgid, pid_t pid, int sig)
  EcallStatus ECALL_sigaltstack(RevInst& inst);            // 132, rev_sigaltstack(const struct sigaltstack  *uss, struct sigaltstack  *uoss)
  EcallStatus ECALL_rt_sigsuspend(RevInst& inst);          // 133, rev_rt_sigsuspend(sigset_t  *unewset, size_t sigsetsize)
  EcallStatus ECALL_rt_sigaction(RevInst& inst);           // 134, rev_rt_sigaction(int, const struct sigaction  *, struct sigaction  *, size_t)
  EcallStatus ECALL_rt_sigprocmask(RevInst& inst);         // 135, rev_rt_sigprocmask(int how, sigset_t  *set, sigset_t  *oset, size_t sigsetsize)
  EcallStatus ECALL_rt_sigpending(RevInst& inst);          // 136, rev_rt_sigpending(sigset_t  *set, size_t sigsetsize)
  EcallStatus ECALL_rt_sigtimedwait_time32(RevInst& inst); // 137, rev_rt_sigtimedwait_time32(const sigset_t  *uthese, siginfo_t  *uinfo, const struct old_timespec32  *uts, size_t sigsetsize)
  EcallStatus ECALL_rt_sigqueueinfo(RevInst& inst);        // 138, rev_rt_sigqueueinfo(pid_t pid, int sig, siginfo_t  *uinfo)
  EcallStatus ECALL_setpriority(RevInst& inst);            // 140, rev_setpriority(int which, int who, int niceval)
  EcallStatus ECALL_getpriority(RevInst& inst);            // 141, rev_getpriority(int which, int who)
  EcallStatus ECALL_reboot(RevInst& inst);                 // 142, rev_reboot(int magic1, int magic2, unsigned int cmd, void  *arg)
  EcallStatus ECALL_setregid(RevInst& inst);               // 143, rev_setregid(gid_t rgid, gid_t egid)
  EcallStatus ECALL_setgid(RevInst& inst);                 // 144, rev_setgid(gid_t gid)
  EcallStatus ECALL_setreuid(RevInst& inst);               // 145, rev_setreuid(uid_t ruid, uid_t euid)
  EcallStatus ECALL_setuid(RevInst& inst);                 // 146, rev_setuid(uid_t uid)
  EcallStatus ECALL_setresuid(RevInst& inst);              // 147, rev_setresuid(uid_t ruid, uid_t euid, uid_t suid)
  EcallStatus ECALL_getresuid(RevInst& inst);              // 148, rev_getresuid(uid_t  *ruid, uid_t  *euid, uid_t  *suid)
  EcallStatus ECALL_setresgid(RevInst& inst);              // 149, rev_setresgid(gid_t rgid, gid_t egid, gid_t sgid)
  EcallStatus ECALL_getresgid(RevInst& inst);              // 150, rev_getresgid(gid_t  *rgid, gid_t  *egid, gid_t  *sgid)
  EcallStatus ECALL_setfsuid(RevInst& inst);               // 151, rev_setfsuid(uid_t uid)
  EcallStatus ECALL_setfsgid(RevInst& inst);               // 152, rev_setfsgid(gid_t gid)
  EcallStatus ECALL_times(RevInst& inst);                  // 153, rev_times(struct tms  *tbuf)
  EcallStatus ECALL_setpgid(RevInst& inst);                // 154, rev_setpgid(pid_t pid, pid_t pgid)
  EcallStatus ECALL_getpgid(RevInst& inst);                // 155, rev_getpgid(pid_t pid)
  EcallStatus ECALL_getsid(RevInst& inst);                 // 156, rev_getsid(pid_t pid)
  EcallStatus ECALL_setsid(RevInst& inst);                 // 157, rev_setsid(void)
  EcallStatus ECALL_getgroups(RevInst& inst);              // 158, rev_getgroups(int gidsetsize, gid_t  *grouplist)
  EcallStatus ECALL_setgroups(RevInst& inst);              // 159, rev_setgroups(int gidsetsize, gid_t  *grouplist)
  EcallStatus ECALL_newuname(RevInst& inst);               // 160, rev_newuname(struct new_utsname  *name)
  EcallStatus ECALL_sethostname(RevInst& inst);            // 161, rev_sethostname(char  *name, int len)
  EcallStatus ECALL_setdomainname(RevInst& inst);          // 162, rev_setdomainname(char  *name, int len)
  EcallStatus ECALL_getrlimit(RevInst& inst);              // 163, rev_getrlimit(unsigned int resource, struct rlimit  *rlim)
  EcallStatus ECALL_setrlimit(RevInst& inst);              // 164, rev_setrlimit(unsigned int resource, struct rlimit  *rlim)
  EcallStatus ECALL_getrusage(RevInst& inst);              // 165, rev_getrusage(int who, struct rusage  *ru)
  EcallStatus ECALL_umask(RevInst& inst);                  // 166, rev_umask(int mask)
  EcallStatus ECALL_prctl(RevInst& inst);                  // 167, rev_prctl(int option, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5)
  EcallStatus ECALL_getcpu(RevInst& inst);                 // 168, rev_getcpu(unsigned  *cpu, unsigned  *node, struct getcpu_cache  *cache)
  EcallStatus ECALL_gettimeofday(RevInst& inst);           // 169, rev_gettimeofday(struct __kernel_old_timeval  *tv, struct timezone  *tz)
  EcallStatus ECALL_settimeofday(RevInst& inst);           // 170, rev_settimeofday(struct __kernel_old_timeval  *tv, struct timezone  *tz)
  EcallStatus ECALL_adjtimex(RevInst& inst);               // 171, rev_adjtimex(struct __kernel_timex  *txc_p)
  EcallStatus ECALL_getpid(RevInst& inst);                 // 172, rev_getpid(void)
  EcallStatus ECALL_getppid(RevInst& inst);                // 173, rev_getppid(void)
  EcallStatus ECALL_getuid(RevInst& inst);                 // 174, rev_getuid(void)
  EcallStatus ECALL_geteuid(RevInst& inst);                // 175, rev_geteuid(void)
  EcallStatus ECALL_getgid(RevInst& inst);                 // 176, rev_getgid(void)
  EcallStatus ECALL_getegid(RevInst& inst);                // 177, rev_getegid(void)
  EcallStatus ECALL_gettid(RevInst& inst);                 // 178, rev_gettid(void)
  EcallStatus ECALL_sysinfo(RevInst& inst);                // 179, rev_sysinfo(struct sysinfo  *info)
  EcallStatus ECALL_mq_open(RevInst& inst);                // 180, rev_mq_open(const char  *name, int oflag, umode_t mode, struct mq_attr  *attr)
  EcallStatus ECALL_mq_unlink(RevInst& inst);              // 181, rev_mq_unlink(const char  *name)
  EcallStatus ECALL_mq_timedsend(RevInst& inst);           // 182, rev_mq_timedsend(mqd_t mqdes, const char  *msg_ptr, size_t msg_len, unsigned int msg_prio, const struct __kernel_timespec  *abs_timeout)
  EcallStatus ECALL_mq_timedreceive(RevInst& inst);        // 183, rev_mq_timedreceive(mqd_t mqdes, char  *msg_ptr, size_t msg_len, unsigned int  *msg_prio, const struct __kernel_timespec  *abs_timeout)
  EcallStatus ECALL_mq_notify(RevInst& inst);              // 184, rev_mq_notify(mqd_t mqdes, const struct sigevent  *notification)
  EcallStatus ECALL_mq_getsetattr(RevInst& inst);          // 185, rev_mq_getsetattr(mqd_t mqdes, const struct mq_attr  *mqstat, struct mq_attr  *omqstat)
  EcallStatus ECALL_msgget(RevInst& inst);                 // 186, rev_msgget(key_t key, int msgflg)
  EcallStatus ECALL_msgctl(RevInst& inst);                 // 187, rev_old_msgctl(int msqid, int cmd, struct msqid_ds  *buf)
  EcallStatus ECALL_msgrcv(RevInst& inst);                 // 188, rev_msgrcv(int msqid, struct msgbuf  *msgp, size_t msgsz, long msgtyp, int msgflg)
  EcallStatus ECALL_msgsnd(RevInst& inst);                 // 189, rev_msgsnd(int msqid, struct msgbuf  *msgp, size_t msgsz, int msgflg)
  EcallStatus ECALL_semget(RevInst& inst);                 // 190, rev_semget(key_t key, int nsems, int semflg)
  EcallStatus ECALL_semctl(RevInst& inst);                 // 191, rev_semctl(int semid, int semnum, int cmd, unsigned long arg)
  EcallStatus ECALL_semtimedop(RevInst& inst);             // 192, rev_semtimedop(int semid, struct sembuf  *sops, unsigned nsops, const struct __kernel_timespec  *timeout)
  EcallStatus ECALL_semop(RevInst& inst);                  // 193, rev_semop(int semid, struct sembuf  *sops, unsigned nsops)
  EcallStatus ECALL_shmget(RevInst& inst);                 // 194, rev_shmget(key_t key, size_t size, int flag)
  EcallStatus ECALL_shmctl(RevInst& inst);                 // 195, rev_old_shmctl(int shmid, int cmd, struct shmid_ds  *buf)
  EcallStatus ECALL_shmat(RevInst& inst);                  // 196, rev_shmat(int shmid, char  *shmaddr, int shmflg)
  EcallStatus ECALL_shmdt(RevInst& inst);                  // 197, rev_shmdt(char  *shmaddr)
  EcallStatus ECALL_socket(RevInst& inst);                 // 198, rev_socket(int, int, int)
  EcallStatus ECALL_socketpair(RevInst& inst);             // 199, rev_socketpair(int, int, int, int  *)
  EcallStatus ECALL_bind(RevInst& inst);                   // 200, rev_bind(int, struct sockaddr  *, int)
  EcallStatus ECALL_listen(RevInst& inst);                 // 201, rev_listen(int, int)
  EcallStatus ECALL_accept(RevInst& inst);                 // 202, rev_accept(int, struct sockaddr  *, int  *)
  EcallStatus ECALL_connect(RevInst& inst);                // 203, rev_connect(int, struct sockaddr  *, int)
  EcallStatus ECALL_getsockname(RevInst& inst);            // 204, rev_getsockname(int, struct sockaddr  *, int  *)
  EcallStatus ECALL_getpeername(RevInst& inst);            // 205, rev_getpeername(int, struct sockaddr  *, int  *)
  EcallStatus ECALL_sendto(RevInst& inst);                 // 206, rev_sendto(int, void  *, size_t, unsigned, struct sockaddr  *, int)
  EcallStatus ECALL_recvfrom(RevInst& inst);               // 207, rev_recvfrom(int, void  *, size_t, unsigned, struct sockaddr  *, int  *)
  EcallStatus ECALL_setsockopt(RevInst& inst);             // 208, rev_setsockopt(int fd, int level, int optname, char  *optval, int optlen)
  EcallStatus ECALL_getsockopt(RevInst& inst);             // 209, rev_getsockopt(int fd, int level, int optname, char  *optval, int  *optlen)
  EcallStatus ECALL_shutdown(RevInst& inst);               // 210, rev_shutdown(int, int)
  EcallStatus ECALL_sendmsg(RevInst& inst);                // 211, rev_sendmsg(int fd, struct user_msghdr  *msg, unsigned flags)
  EcallStatus ECALL_recvmsg(RevInst& inst);                // 212, rev_recvmsg(int fd, struct user_msghdr  *msg, unsigned flags)
  EcallStatus ECALL_readahead(RevInst& inst);              // 213, rev_readahead(int fd, loff_t offset, size_t count)
  EcallStatus ECALL_brk(RevInst& inst);                    // 214, rev_brk(unsigned long brk)
  EcallStatus ECALL_munmap(RevInst& inst);                 // 215, rev_munmap(unsigned long addr, size_t len)
  EcallStatus ECALL_mremap(RevInst& inst);                 // 216, rev_mremap(unsigned long addr, unsigned long old_len, unsigned long new_len, unsigned long flags, unsigned long new_addr)
  EcallStatus ECALL_add_key(RevInst& inst);                // 217, rev_add_key(const char  *_type, const char  *_description, const void  *_payload, size_t plen, key_serial_t destringid)
  EcallStatus ECALL_request_key(RevInst& inst);            // 218, rev_request_key(const char  *_type, const char  *_description, const char  *_callout_info, key_serial_t destringid)
  EcallStatus ECALL_keyctl(RevInst& inst);                 // 219, rev_keyctl(int cmd, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5)
  EcallStatus ECALL_clone(RevInst& inst);                  // 220, rev_clone(unsigned long, unsigned long, int  *, unsigned long, int  *)
  EcallStatus ECALL_execve(RevInst& inst);                 // 221, rev_execve(const char  *filename, const char  *const  *argv, const char  *const  *envp)
  EcallStatus ECALL_mmap(RevInst& inst);                   // 222, rev_old_mmap(struct mmap_arg_struct  *arg)
  EcallStatus ECALL_fadvise64_64(RevInst& inst);           // 223, rev_fadvise64_64(int fd, loff_t offset, loff_t len, int advice)
  EcallStatus ECALL_swapon(RevInst& inst);                 // 224, rev_swapon(const char  *specialfile, int swap_flags)
  EcallStatus ECALL_swapoff(RevInst& inst);                // 225, rev_swapoff(const char  *specialfile)
  EcallStatus ECALL_mprotect(RevInst& inst);               // 226, rev_mprotect(unsigned long start, size_t len, unsigned long prot)
  EcallStatus ECALL_msync(RevInst& inst);                  // 227, rev_msync(unsigned long start, size_t len, int flags)
  EcallStatus ECALL_mlock(RevInst& inst);                  // 228, rev_mlock(unsigned long start, size_t len)
  EcallStatus ECALL_munlock(RevInst& inst);                // 229, rev_munlock(unsigned long start, size_t len)
  EcallStatus ECALL_mlockall(RevInst& inst);               // 230, rev_mlockall(int flags)
  EcallStatus ECALL_munlockall(RevInst& inst);             // 231, rev_munlockall(void)
  EcallStatus ECALL_mincore(RevInst& inst);                // 232, rev_mincore(unsigned long start, size_t len, unsigned char  * vec)
  EcallStatus ECALL_madvise(RevInst& inst);                // 233, rev_madvise(unsigned long start, size_t len, int behavior)
  EcallStatus ECALL_remap_file_pages(RevInst& inst);       // 234, rev_remap_file_pages(unsigned long start, unsigned long size, unsigned long prot, unsigned long pgoff, unsigned long flags)
  EcallStatus ECALL_mbind(RevInst& inst);                  // 235, rev_mbind(unsigned long start, unsigned long len, unsigned long mode, const unsigned long  *nmask, unsigned long maxnode, unsigned flags)
  EcallStatus ECALL_get_mempolicy(RevInst& inst);          // 236, rev_get_mempolicy(int  *policy, unsigned long  *nmask, unsigned long maxnode, unsigned long addr, unsigned long flags)
  EcallStatus ECALL_set_mempolicy(RevInst& inst);          // 237, rev_set_mempolicy(int mode, const unsigned long  *nmask, unsigned long maxnode)
  EcallStatus ECALL_migrate_pages(RevInst& inst);          // 238, rev_migrate_pages(pid_t pid, unsigned long maxnode, const unsigned long  *from, const unsigned long  *to)
  EcallStatus ECALL_move_pages(RevInst& inst);             // 239, rev_move_pages(pid_t pid, unsigned long nr_pages, const void  *  *pages, const int  *nodes, int  *status, int flags)
  EcallStatus ECALL_rt_tgsigqueueinfo(RevInst& inst);      // 240, rev_rt_tgsigqueueinfo(pid_t tgid, pid_t pid, int sig, siginfo_t  *uinfo)
  EcallStatus ECALL_perf_event_open(RevInst& inst);        // 241, rev_perf_event_open(")
  EcallStatus ECALL_accept4(RevInst& inst);                // 242, rev_accept4(int, struct sockaddr  *, int  *, int)
  EcallStatus ECALL_recvmmsg_time32(RevInst& inst);        // 243, rev_recvmmsg_time32(int fd, struct mmsghdr  *msg, unsigned int vlen, unsigned flags, struct old_timespec32  *timeout)
  EcallStatus ECALL_wait4(RevInst& inst);                  // 260, rev_wait4(pid_t pid, int  *stat_addr, int options, struct rusage  *ru)
  EcallStatus ECALL_prlimit64(RevInst& inst);              // 261, rev_prlimit64(pid_t pid, unsigned int resource, const struct rlimit64  *new_rlim, struct rlimit64  *old_rlim)
  EcallStatus ECALL_fanotify_init(RevInst& inst);          // 262, rev_fanotify_init(unsigned int flags, unsigned int event_f_flags)
  EcallStatus ECALL_fanotify_mark(RevInst& inst);          // 263, rev_fanotify_mark(int fanotify_fd, unsigned int flags, u64 mask, int fd, const char  *pathname)
  EcallStatus ECALL_name_to_handle_at(RevInst& inst);      // 264, rev_name_to_handle_at(int dfd, const char  *name, struct file_handle  *handle, int  *mnt_id, int flag)
  EcallStatus ECALL_open_by_handle_at(RevInst& inst);      // 265, rev_open_by_handle_at(int mountdirfd, struct file_handle  *handle, int flags)
  EcallStatus ECALL_clock_adjtime(RevInst& inst);          // 266, rev_clock_adjtime(clockid_t which_clock, struct __kernel_timex  *tx)
  EcallStatus ECALL_syncfs(RevInst& inst);                 // 267, rev_syncfs(int fd)
  EcallStatus ECALL_setns(RevInst& inst);                  // 268, rev_setns(int fd, int nstype)
  EcallStatus ECALL_sendmmsg(RevInst& inst);               // 269, rev_sendmmsg(int fd, struct mmsghdr  *msg, unsigned int vlen, unsigned flags)
  EcallStatus ECALL_process_vm_readv(RevInst& inst);       // 270, rev_process_vm_readv(pid_t pid, const struct iovec  *lvec, unsigned long liovcnt, const struct iovec  *rvec, unsigned long riovcnt, unsigned long flags)
  EcallStatus ECALL_process_vm_writev(RevInst& inst);      // 271, rev_process_vm_writev(pid_t pid, const struct iovec  *lvec, unsigned long liovcnt, const struct iovec  *rvec, unsigned long riovcnt, unsigned long flags)
  EcallStatus ECALL_kcmp(RevInst& inst);                   // 272, rev_kcmp(pid_t pid1, pid_t pid2, int type, unsigned long idx1, unsigned long idx2)
  EcallStatus ECALL_finit_module(RevInst& inst);           // 273, rev_finit_module(int fd, const char  *uargs, int flags)
  EcallStatus ECALL_sched_setattr(RevInst& inst);          // 274, rev_sched_setattr(pid_t pid, struct sched_attr  *attr, unsigned int flags)
  EcallStatus ECALL_sched_getattr(RevInst& inst);          // 275, rev_sched_getattr(pid_t pid, struct sched_attr  *attr, unsigned int size, unsigned int flags)
  EcallStatus ECALL_renameat2(RevInst& inst);              // 276, rev_renameat2(int olddfd, const char  *oldname, int newdfd, const char  *newname, unsigned int flags)
  EcallStatus ECALL_seccomp(RevInst& inst);                // 277, rev_seccomp(unsigned int op, unsigned int flags, void  *uargs)
  EcallStatus ECALL_getrandom(RevInst& inst);              // 278, rev_getrandom(char  *buf, size_t count, unsigned int flags)
  EcallStatus ECALL_memfd_create(RevInst& inst);           // 279, rev_memfd_create(const char  *uname_ptr, unsigned int flags)
  EcallStatus ECALL_bpf(RevInst& inst);                    // 280, rev_bpf(int cmd, union bpf_attr *attr, unsigned int size)
  EcallStatus ECALL_execveat(RevInst& inst);               // 281, rev_execveat(int dfd, const char  *filename, const char  *const  *argv, const char  *const  *envp, int flags)
  EcallStatus ECALL_userfaultfd(RevInst& inst);            // 282, rev_userfaultfd(int flags)
  EcallStatus ECALL_membarrier(RevInst& inst);             // 283, rev_membarrier(int cmd, unsigned int flags, int cpu_id)
  EcallStatus ECALL_mlock2(RevInst& inst);                 // 284, rev_mlock2(unsigned long start, size_t len, int flags)
  EcallStatus ECALL_copy_file_range(RevInst& inst);        // 285, rev_copy_file_range(int fd_in, loff_t  *off_in, int fd_out, loff_t  *off_out, size_t len, unsigned int flags)
  EcallStatus ECALL_preadv2(RevInst& inst);                // 286, rev_preadv2(unsigned long fd, const struct iovec  *vec, unsigned long vlen, unsigned long pos_l, unsigned long pos_h, rwf_t flags)
  EcallStatus ECALL_pwritev2(RevInst& inst);               // 287, rev_pwritev2(unsigned long fd, const struct iovec  *vec, unsigned long vlen, unsigned long pos_l, unsigned long pos_h, rwf_t flags)
  EcallStatus ECALL_pkey_mprotect(RevInst& inst);          // 288, rev_pkey_mprotect(unsigned long start, size_t len, unsigned long prot, int pkey)
  EcallStatus ECALL_pkey_alloc(RevInst& inst);             // 289, rev_pkey_alloc(unsigned long flags, unsigned long init_val)
  EcallStatus ECALL_pkey_free(RevInst& inst);              // 290, rev_pkey_free(int pkey)
  EcallStatus ECALL_statx(RevInst& inst);                  // 291, rev_statx(int dfd, const char  *path, unsigned flags, unsigned mask, struct statx  *buffer)
  EcallStatus ECALL_io_pgetevents(RevInst& inst);          // 292, rev_io_pgetevents(aio_context_t ctx_id, long min_nr, long nr, struct io_event  *events, struct __kernel_timespec  *timeout, const struct __aio_sigset *sig)
  EcallStatus ECALL_rseq(RevInst& inst);                   // 293, rev_rseq(struct rseq  *rseq, uint32_t rseq_len, int flags, uint32_t sig)
  EcallStatus ECALL_kexec_file_load(RevInst& inst);        // 294, rev_kexec_file_load(int kernel_fd, int initrd_fd, unsigned long cmdline_len, const char  *cmdline_ptr, unsigned long flags)
  // EcallStatus ECALL_clock_gettime(RevInst& inst);          // 403, rev_clock_gettime(clockid_t which_clock, struct __kernel_timespec  *tp)
  // EcallStatus ECALL_clock_settime(RevInst& inst);          // 404, rev_clock_settime(clockid_t which_clock, const struct __kernel_timespec  *tp)
  // EcallStatus ECALL_clock_adjtime(RevInst& inst);          // 405, rev_clock_adjtime(clockid_t which_clock, struct __kernel_timex  *tx)
  // EcallStatus ECALL_clock_getres(RevInst& inst);           // 406, rev_clock_getres(clockid_t which_clock, struct __kernel_timespec  *tp)
  // EcallStatus ECALL_clock_nanosleep(RevInst& inst);        // 407, rev_clock_nanosleep(clockid_t which_clock, int flags, const struct __kernel_timespec  *rqtp, struct __kernel_timespec  *rmtp)
  // EcallStatus ECALL_timer_gettime(RevInst& inst);          // 408, rev_timer_gettime(timer_t timer_id, struct __kernel_itimerspec  *setting)
  // EcallStatus ECALL_timer_settime(RevInst& inst);          // 409, rev_timer_settime(timer_t timer_id, int flags, const struct __kernel_itimerspec  *new_setting, struct __kernel_itimerspec  *old_setting)
  // EcallStatus ECALL_timerfd_gettime(RevInst& inst);        // 410, rev_timerfd_gettime(int ufd, struct __kernel_itimerspec  *otmr)
  // EcallStatus ECALL_timerfd_settime(RevInst& inst);        // 411, rev_timerfd_settime(int ufd, int flags, const struct __kernel_itimerspec  *utmr, struct __kernel_itimerspec  *otmr)
  // EcallStatus ECALL_utimensat(RevInst& inst);              // 412, rev_utimensat(int dfd, const char  *filename, struct __kernel_timespec  *utimes, int flags)
  // EcallStatus ECALL_io_pgetevents(RevInst& inst);          // 416, rev_io_pgetevents(aio_context_t ctx_id, long min_nr, long nr, struct io_event  *events, struct __kernel_timespec  *timeout, const struct __aio_sigset *sig)
  // EcallStatus ECALL_mq_timedsend(RevInst& inst);           // 418, rev_mq_timedsend(mqd_t mqdes, const char  *msg_ptr, size_t msg_len, unsigned int msg_prio, const struct __kernel_timespec  *abs_timeout)
  // EcallStatus ECALL_mq_timedreceive(RevInst& inst);        // 419, rev_mq_timedreceive(mqd_t mqdes, char  *msg_ptr, size_t msg_len, unsigned int  *msg_prio, const struct __kernel_timespec  *abs_timeout)
  // EcallStatus ECALL_semtimedop(RevInst& inst);             // 420, rev_semtimedop(int semid, struct sembuf  *sops, unsigned nsops, const struct __kernel_timespec  *timeout)
  // EcallStatus ECALL_futex(RevInst& inst);                  // 422, rev_futex(u32  *uaddr, int op, u32 val, struct __kernel_timespec  *utime, u32  *uaddr2, u32 val3)
  // EcallStatus ECALL_sched_rr_get_interval(RevInst& inst);  // 423, rev_sched_rr_get_interval(pid_t pid, struct __kernel_timespec  *interval)
  EcallStatus ECALL_pidfd_send_signal(RevInst& inst);      // 424, rev_pidfd_send_signal(int pidfd, int sig, siginfo_t  *info, unsigned int flags)
  EcallStatus ECALL_io_uring_setup(RevInst& inst);         // 425, rev_io_uring_setup(u32 entries, struct io_uring_params  *p)
  EcallStatus ECALL_io_uring_enter(RevInst& inst);         // 426, rev_io_uring_enter(unsigned int fd, u32 to_submit, u32 min_complete, u32 flags, const sigset_t  *sig, size_t sigsz)
  EcallStatus ECALL_io_uring_register(RevInst& inst);      // 427, rev_io_uring_register(unsigned int fd, unsigned int op, void  *arg, unsigned int nr_args)
  EcallStatus ECALL_open_tree(RevInst& inst);              // 428, rev_open_tree(int dfd, const char  *path, unsigned flags)
  EcallStatus ECALL_move_mount(RevInst& inst);             // 429, rev_move_mount(int from_dfd, const char  *from_path, int to_dfd, const char  *to_path, unsigned int ms_flags)
  EcallStatus ECALL_fsopen(RevInst& inst);                 // 430, rev_fsopen(const char  *fs_name, unsigned int flags)
  EcallStatus ECALL_fsconfig(RevInst& inst);               // 431, rev_fsconfig(int fs_fd, unsigned int cmd, const char  *key, const void  *value, int aux)
  EcallStatus ECALL_fsmount(RevInst& inst);                // 432, rev_fsmount(int fs_fd, unsigned int flags, unsigned int ms_flags)
  EcallStatus ECALL_fspick(RevInst& inst);                 // 433, rev_fspick(int dfd, const char  *path, unsigned int flags)
  EcallStatus ECALL_pidfd_open(RevInst& inst);             // 434, rev_pidfd_open(pid_t pid, unsigned int flags)
  EcallStatus ECALL_clone3(RevInst& inst);                 // 435, rev_clone3(struct clone_args  *uargs, size_t size)
  EcallStatus ECALL_close_range(RevInst& inst);            // 436, rev_close_range(unsigned int fd, unsigned int max_fd, unsigned int flags)
  EcallStatus ECALL_openat2(RevInst& inst);                // 437, rev_openat2(int dfd, const char  *filename, struct open_how *how, size_t size)
  EcallStatus ECALL_pidfd_getfd(RevInst& inst);            // 438, rev_pidfd_getfd(int pidfd, int fd, unsigned int flags)
  EcallStatus ECALL_faccessat2(RevInst& inst);             // 439, rev_faccessat2(int dfd, const char  *filename, int mode, int flags)
  EcallStatus ECALL_process_madvise(RevInst& inst);        // 440, rev_process_madvise(int pidfd, const struct iovec  *vec, size_t vlen, int behavior, unsigned int flags)

  // =============== REV specific functions
  EcallStatus ECALL_cpuinfo(RevInst& inst);                // 500, rev_cpuinfo(struct rev_cpuinfo *info);

  // =============== REV pthread functions
  EcallStatus ECALL_pthread_create(RevInst& inst);         // 1000, rev_pthread_create(pthread_t *thread, const pthread_attr_t  *attr, void  *(*start_routine)(void  *), void  *arg)
  EcallStatus ECALL_pthread_join(RevInst& inst);           // 1001, rev_pthread_join(pthread_t thread, void **retval);
  EcallStatus ECALL_pthread_exit(RevInst& inst);           // 1002, rev_pthread_exit(void* retval);

  /// RevProc: Table of ecall codes w/ corresponding function pointer implementations
  std::unordered_map<uint32_t, std::function<EcallStatus(RevProc*, RevInst&)>> Ecalls;

  /// RevProc: Initialize all of the ecalls inside the above table
  void InitEcallTable();

  /// RevProc: Execute the Ecall based on the code loaded in RegFile->RV64_SCAUSE
  void ExecEcall(RevInst &inst);

  /// RevProc: Get a pointer to the register file loaded into Hart w/ HartID
  RevRegFile* GetRegFile(unsigned HartID) const;

  std::vector<RevInstEntry> InstTable;        ///< RevProc: target instruction table

  std::vector<std::unique_ptr<RevExt>> Extensions;           ///< RevProc: vector of enabled extensions

  //std::vector<std::tuple<uint16_t, RevInst, bool>>  Pipeline; ///< RevProc: pipeline of instructions
  std::vector<std::pair<uint16_t, RevInst>> Pipeline;  ///< RevProc: pipeline of instructions
  std::map<std::string, unsigned> NameToEntry; ///< RevProc: instruction mnemonic to table entry mapping
  std::map<uint32_t, unsigned> EncToEntry;     ///< RevProc: instruction encoding to table entry mapping
  std::map<uint32_t, unsigned> CEncToEntry;    ///< RevProc: compressed instruction encoding to table entry mapping

  std::map<unsigned, std::pair<unsigned, unsigned>> EntryToExt;     ///< RevProc: instruction entry to extension object mapping
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
  void SetPC(uint64_t PC) { RegFile->SetPC(PC); }

  /// RevProc: prefetch the next instruction
  bool PrefetchInst();

  /// RevProc: decode the instruction at the current PC
  RevInst DecodeInst();

  /// RevProc: decode a compressed instruction
  RevInst DecodeCompressed(uint32_t Inst) const;

  /// RevProc: decode an R-type instruction
  RevInst DecodeRInst(uint32_t Inst, unsigned Entry) const;

  /// RevProc: decode an I-type instruction
  RevInst DecodeIInst(uint32_t Inst, unsigned Entry) const;

  /// RevProc: decode an S-type instruction
  RevInst DecodeSInst(uint32_t Inst, unsigned Entry) const;

  /// RevProc: decode a U-type instruction
  RevInst DecodeUInst(uint32_t Inst, unsigned Entry) const;

  /// RevProc: decode a B-type instruction
  RevInst DecodeBInst(uint32_t Inst, unsigned Entry) const;

  /// RevProc: decode a J-type instruction
  RevInst DecodeJInst(uint32_t Inst, unsigned Entry) const;

  /// RevProc: decode an R4-type instruction
  RevInst DecodeR4Inst(uint32_t Inst, unsigned Entry) const;

  /// RevProc: decode a compressed CR-type isntruction
  RevInst DecodeCRInst(uint16_t Inst, unsigned Entry) const;

  /// RevProc: decode a compressed CI-type isntruction
  RevInst DecodeCIInst(uint16_t Inst, unsigned Entry) const;

  /// RevProc: decode a compressed CSS-type isntruction
  RevInst DecodeCSSInst(uint16_t Inst, unsigned Entry) const;

  /// RevProc: decode a compressed CIW-type isntruction
  RevInst DecodeCIWInst(uint16_t Inst, unsigned Entry) const;

  /// RevProc: decode a compressed CL-type isntruction
  RevInst DecodeCLInst(uint16_t Inst, unsigned Entry) const;

  /// RevProc: decode a compressed CS-type isntruction
  RevInst DecodeCSInst(uint16_t Inst, unsigned Entry) const;

  /// RevProc: decode a compressed CA-type isntruction
  RevInst DecodeCAInst(uint16_t Inst, unsigned Entry) const;

  /// RevProc: decode a compressed CB-type isntruction
  RevInst DecodeCBInst(uint16_t Inst, unsigned Entry) const;

  /// RevProc: decode a compressed CJ-type isntruction
  RevInst DecodeCJInst(uint16_t Inst, unsigned Entry) const;

  /// RevProc: determine if the instruction is floating-point
  bool IsFloat(unsigned Entry) const {
    // Note: This is crude and looks for ANY FP register operands;
    // InstTable[...].r<reg>Class should be used when doing hazard
    // detection on particular registers, since some instructions
    // combine integer and FP register operands. See DependencySet().
    return( InstTable[Entry].rdClass  == RevRegClass::RegFLOAT ||
            InstTable[Entry].rs1Class == RevRegClass::RegFLOAT ||
            InstTable[Entry].rs2Class == RevRegClass::RegFLOAT ||
            InstTable[Entry].rs3Class == RevRegClass::RegFLOAT );
  }

  /// RevProc: Determine next thread to execute
  unsigned GetHartID() const;

  /// RevProc: Whether any scoreboard bits are set
  bool AnyDependency(unsigned HartID, bool isFloat) const {
    const RevRegFile* regFile = GetRegFile(HartID);
    return isFloat ? regFile->FP_Scoreboard.any() : regFile->RV_Scoreboard.any();
  }

  /// RevProc: Checks if a given Thread w/ ThreadID has any dependencies
  bool ThreadHasDependencies(const uint32_t& ThreadID) const {
    // Make sure this ThreadID is in AssignedThreads
    if (AssignedThreads.find(ThreadID) == AssignedThreads.end()) {
      output->fatal(CALL_INFO, -1, "Error: Tried to check if thread %" PRIu32 " has dependencies but that thread was not found in AssignedThreads. This is a bug\n", ThreadID);
    }
    const RevRegFile* regFile = AssignedThreads.at(ThreadID)->GetRegFile();
    return regFile->RV_Scoreboard.any() || regFile->FP_Scoreboard.any();
  }

  /// RevProc: Check scoreboard for pipeline hazards
  bool DependencyCheck(unsigned HartID, const RevInst* Inst) const;

  /// RevProc: Set or clear scoreboard based on instruction destination
  void DependencySet(unsigned HartID, const RevInst* Inst, bool value = true){
    DependencySet(HartID,
                  Inst->rd,
                  InstTable[Inst->entry].rdClass == RevRegClass::RegFLOAT,
                  value);
  }

  /// RevProc: Clear scoreboard on instruction retirement
  void DependencyClear(unsigned HartID, const RevInst* Inst){
    DependencySet(HartID, Inst, false);
  }

  /// RevProc: Set or clear scoreboard based on register number and floating point.
  void DependencySet(unsigned HartID, uint16_t RegNum, bool isFloat, bool value = true);

  /// RevProc: Clear scoreboard on instruction retirement
  void DependencyClear(unsigned HartID, uint16_t RegNum, bool isFloat){
    DependencySet(HartID, RegNum, isFloat, false);
  }

}; // class RevProc

} // namespace SST::RevCPU

#endif

// EOF
