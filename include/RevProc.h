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
#include <sst/core/sst_config.h>
#include <sst/core/component.h>
#include <sst/core/statapi/stataccumulator.h>

// -- Standard Headers
#include <iostream>
#include <fstream>
#include <bitset>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <random>
#include <queue>
#include <functional>
#include <tuple>
#include <list>
#include <inttypes.h>

// -- RevCPU Headers
#include "RevOpts.h"
#include "RevMem.h"
#include "RevFeature.h"
#include "RevLoader.h"
#include "RevInstTable.h"
#include "RevInstTables.h"
#include "PanExec.h"
#include "RevPrefetcher.h"
#include "RevCoProc.h"
#include "RevThread.h"
#include "../common/syscalls/SysFlags.h"

#define _PAN_FWARE_JUMP_            0x0000000000010000

using namespace SST::RevCPU;

namespace SST{
  namespace RevCPU{

    class RevProc{
    public:
      /// RevProc: standard constructor
      RevProc( unsigned Id, RevOpts *Opts, RevMem *Mem, RevLoader *Loader,
               std::vector<std::shared_ptr<RevThread>>& AssignedThreads,
               RevCoProc* CoProc, SST::Output *Output );

      /// RevProc: standard desctructor
      ~RevProc();

      /// RevProc: per-processor clock function
      bool ClockTick( SST::Cycle_t currentCycle );

      /// RevProc: halt the CPU
      bool Halt();

      /// RevProc: resume the CPU
      bool Resume();

      /// RevProc: execute a single step
      bool SingleStepHart();

      /// RevProc: retrieve the local PC for the correct feature set
      uint64_t GetPC();

      /// RevProc: Debug mode read a register
      bool DebugReadReg(unsigned Idx, uint64_t *Value);

      /// RevProc: Debug mode write a register
      bool DebugWriteReg(unsigned Idx, uint64_t Value);

      /// RevProc: Is this an RV32 machine?
      bool DebugIsRV32() { return feature->IsRV32(); }

      /// RevProc: Set the PAN execution context
      void SetExecCtx(PanExec *P) { PExec = P; }

      /// RevProc: Retrieve a random memory cost value
      unsigned RandCost() { return mem->RandCost(feature->GetMinCost(),feature->GetMaxCost()); }

      /// RevProc: Handle register faults
      void HandleRegFault(unsigned width);

      /// RevProc: Handle crack+decode faults
      void HandleCrackFault(unsigned width);

      /// RevProc: Handle ALU faults
      void HandleALUFault(unsigned width);

      /// RevProc: Initialize ThreadTable & First Thread
      bool InitThreadTable();

      class RevProcStats {
        public:
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

      RevProcStats GetStats();

      RevMem& GetMem(){ return *mem; }

      /// RevProc: Add a RevThread to the Proc's ThreadTable
      bool AddCtx(RevThread& Ctx);

      /// RevProc: Create a new RevThread w/ Parent is currently executing thread
      uint32_t CreateChildCtx();
 
      /// RevProc: SpawnThread creates a new thread and returns its ThreadID
      uint32_t SpawnThread(uint64_t fn);

      /// RevProc: Returns the current HartToExec active pid 
      uint32_t GetActiveThreadID();

      std::queue<std::pair<uint64_t, std::shared_ptr<MemSegment>>>& GetNewThreadInfo() { return NewThreadInfo; }

      /// RevProc: Returns the active pid for HartID 
      // uint32_t GetActiveThreadID(const uint32_t HartID){ return ActiveThreadIDs.at(HartID); } 

      /// RevProc: Returns full list of ThreadIDs (keys in ThreadTable) 
      std::vector<uint32_t> GetThreadIDs();

      /// RevProc: Retires currently executing thread & then swaps to its parent. If no parent terminates program
      uint32_t RetireAndSwap(); // Returns new pid
  
      /// RevProc: Used to raise an exception indicating a thread switch is coming (NewThreadID = ThreadID of Ctx to switch to)
      // void CtxSwitchAlert(uint32_t NewThreadID) { NextThreadID=NewThreadID;PendingCtxSwitch = true; }

      uint32_t HartToExecThreadID();
      uint32_t HartToDecodeThreadID();

      ///< RevProc: Returns pointer to current ctx loaded into HartToExec
      std::shared_ptr<RevThread> HartToExecCtx();

      ///< RevProc: Returns pointer to current ctx loaded into HartToDecode
      std::shared_ptr<RevThread> HartToDecodeCtx();

      ///< RevProc: Change HartToExec active pid
      bool ChangeActiveThreadID(uint32_t ThreadID); 
  
      ///< RevProc: Change HartID active pid
      bool ChangeActiveThreadID(uint32_t ThreadID, uint16_t HartID); 

      std::queue<std::pair<uint64_t, std::shared_ptr<MemSegment>>> NewThreadInfo;

      ///< RevProc: Used for scheduling in RevCPU (if Utilization < 1, there is at least 1 unoccupied HART )
      uint16_t GetUtilization(){ return (AssignedThreads.size() / _REV_HART_COUNT_) * 100; }
      // BEFORE MERGE
      // TODO: Implement the proc scheduling (ie. Moving N=HART threads to the end of the AssignedThreads)      
      

    private:
      bool Halted;              ///< RevProc: determines if the core is halted
      bool Stalled;             ///< RevProc: determines if the core is stalled on instruction fetch
      bool SingleStep;          ///< RevProc: determines if we are in a single step
      bool CrackFault;          ///< RevProc: determiens if we need to handle a crack fault
      bool ALUFault;            ///< RevProc: determines if we need to handle an ALU fault
      unsigned fault_width;     ///< RevProc: the width of the target fault
      unsigned id;              ///< RevProc: processor id
      uint64_t ExecPC;          ///< RevProc: executing PC
      uint16_t HartToDecode;   ///< RevProc: Current executing ThreadID
      uint16_t HartToExec;     ///< RevProc: Thread to dispatch instruction
      uint64_t Retired;         ///< RevProc: number of retired instructions

      RevOpts *opts;            ///< RevProc: options object
      RevMem *mem;              ///< RevProc: memory object
      RevCoProc* coProc;        ///< RevProc: attached co-processor
      RevLoader *loader;        ///< RevProc: loader object
  
      /// ThreadIDs assigned to this RevProc (Index into this vector = Hart that's executing)
      std::vector<std::shared_ptr<RevThread>>& AssignedThreads;

      SST::Output *output;      ///< RevProc: output handler
      RevFeature *feature;      ///< RevProc: feature handler
      PanExec *PExec;           ///< RevProc: PAN exeuction context
      RevProcStats Stats;       ///< RevProc: collection of performance stats
      RevPrefetcher *sfetch;    ///< RevProc: stream instruction prefetcher

      RevRegFile* RegFile = nullptr; ///< RevProc: Initial pointer to HartToDecode RegFile
      // uint32_t NextThreadID = 0;     ///< RevProc: ThreadID of next thread to load into HartToExec

      /// RevProc: Get a pointer to the register file loaded into Hart w/ HartID
      RevRegFile* GetRegFile(uint16_t HartID);
      
      RevInst Inst;             ///< RevProc: instruction payload

      std::vector<RevInstEntry> InstTable;        ///< RevProc: target instruction table

      std::vector<RevExt *> Extensions;           ///< RevProc: vector of enabled extensions

      std::map<std::string,unsigned> NameToEntry; ///< RevProc: instruction mnemonic to table entry mapping
      std::map<uint32_t,unsigned> EncToEntry;     ///< RevProc: instruction encoding to table entry mapping
      std::map<uint32_t,unsigned> CEncToEntry;    ///< RevProc: compressed instruction encoding to table entry mapping

      std::map<unsigned,std::pair<unsigned,unsigned>> EntryToExt;     ///< RevProc: instruction entry to extension object mapping
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

      /// RevProc: setup the argc/argc arrays
      void SetupArgs();

      /// RevProc: set the PC
      void SetPC(uint64_t PC);

      /// RevProc: prefetch the next instruction
      bool PrefetchInst();

      /// RevProc: decode the instruction at the current PC
      RevInst DecodeInst();

      /// RevProc: decode a compressed instruction
      RevInst DecodeCompressed(uint32_t Inst);

      /// RevProc: decode an R-type instruction
      RevInst DecodeRInst(uint32_t Inst, unsigned Entry);

      /// RevProc: decode an I-type instruction
      RevInst DecodeIInst(uint32_t Inst, unsigned Entry);

      /// RevProc: decode an S-type instruction
      RevInst DecodeSInst(uint32_t Inst, unsigned Entry);

      /// RevProc: decode a U-type instruction
      RevInst DecodeUInst(uint32_t Inst, unsigned Entry);

      /// RevProc: decode a B-type instruction
      RevInst DecodeBInst(uint32_t Inst, unsigned Entry);

      /// RevProc: decode a J-type instruction
      RevInst DecodeJInst(uint32_t Inst, unsigned Entry);

      /// RevProc: decode an R4-type instruction
      RevInst DecodeR4Inst(uint32_t Inst, unsigned Entry);

      /// RevProc: decode a compressed CR-type isntruction
      RevInst DecodeCRInst(uint16_t Inst, unsigned Entry);

      /// RevProc: decode a compressed CI-type isntruction
      RevInst DecodeCIInst(uint16_t Inst, unsigned Entry);

      /// RevProc: decode a compressed CSS-type isntruction
      RevInst DecodeCSSInst(uint16_t Inst, unsigned Entry);

      /// RevProc: decode a compressed CIW-type isntruction
      RevInst DecodeCIWInst(uint16_t Inst, unsigned Entry);

      /// RevProc: decode a compressed CL-type isntruction
      RevInst DecodeCLInst(uint16_t Inst, unsigned Entry);

      /// RevProc: decode a compressed CS-type isntruction
      RevInst DecodeCSInst(uint16_t Inst, unsigned Entry);

      /// RevProc: decode a compressed CA-type isntruction
      RevInst DecodeCAInst(uint16_t Inst, unsigned Entry);

      /// RevProc: decode a compressed CB-type isntruction
      RevInst DecodeCBInst(uint16_t Inst, unsigned Entry);

      /// RevProc: decode a compressed CJ-type isntruction
      RevInst DecodeCJInst(uint16_t Inst, unsigned Entry);

      /// RevProc: determine if the instruction is an SP/FP float
      bool IsFloat(unsigned Entry);

      /// RevProc: reset the inst structure
      void ResetInst(RevInst *Inst);

      /// RevProc: Determine next Hart to execute
      uint16_t GetHartID();

      /// RevProc: Check scoreboard for pipeline hazards
      bool DependencyCheck(uint16_t threadID, RevInst* Inst);

      /// RevProc: Set scoreboard based on instruction destination
      void DependencySet(uint16_t threadID, RevInst* Inst);

      /// RevProc: Clear scoreboard on instruction retirement
      void DependencyClear(uint16_t threadID, RevInst* Inst);

      #define PIPE_HART     0
      #define PIPE_INST     1
      #define PIPE_HAZARD   2

      //std::vector<std::tuple<uint16_t, RevInst, bool>>  Pipeline; ///< RevProc: pipeline of instructions
      std::vector<std::pair<uint16_t,RevInst>> Pipeline;  ///< RevProc: pipeline of instructions
      std::list<bool *> LoadHazards;                      ///< RevProc: list of allocated load hazards

      /// RevProc: creates a new pipeline load hazard and returns a pointer to it
      bool *createLoadHazard();

      /// RevProc: destroys the target load hazard object and removes it from the list
      void destroyLoadHazard(bool *hazard);

      /*
      * ECALLs 
      * - Many of these are not implemented
      * - Their existence in the ECalls table is solely to not throw errors 
      * - This _should_ be a comprehensive list of system calls supported on RISC-V
      * - Beside each function declaration is the system call code followed by its corresponding declaration 
      *   that you can find in `common/syscalls.h` (the file to be included to use system calls inside of rev)
      */
      void ECALL_io_setup();               // 0, rev_io_setup(unsigned nr_reqs, aio_context_t  *ctx)
      void ECALL_io_destroy();             // 1, rev_io_destroy(aio_context_t ctx)
      void ECALL_io_submit();              // 2, rev_io_submit(aio_context_t, long, struct iocb  *  *)
      void ECALL_io_cancel();              // 3, rev_io_cancel(aio_context_t ctx_id, struct iocb  *iocb, struct io_event  *result)
      void ECALL_io_getevents();           // 4, rev_io_getevents(aio_context_t ctx_id, long min_nr, long nr, struct io_event  *events, struct __kernel_timespec  *timeout)
      void ECALL_setxattr();               // 5, rev_setxattr(const char  *path, const char  *name, const void  *value, size_t size, int flags)
      void ECALL_lsetxattr();              // 6, rev_lsetxattr(const char  *path, const char  *name, const void  *value, size_t size, int flags)
      void ECALL_fsetxattr();              // 7, rev_fsetxattr(int fd, const char  *name, const void  *value, size_t size, int flags)
      void ECALL_getxattr();               // 8, rev_getxattr(const char  *path, const char  *name, void  *value, size_t size)
      void ECALL_lgetxattr();              // 9, rev_lgetxattr(const char  *path, const char  *name, void  *value, size_t size)
      void ECALL_fgetxattr();              // 10, rev_fgetxattr(int fd, const char  *name, void  *value, size_t size)
      void ECALL_listxattr();              // 11, rev_listxattr(const char  *path, char  *list, size_t size)
      void ECALL_llistxattr();             // 12, rev_llistxattr(const char  *path, char  *list, size_t size)
      void ECALL_flistxattr();             // 13, rev_flistxattr(int fd, char  *list, size_t size)
      void ECALL_removexattr();            // 14, rev_removexattr(const char  *path, const char  *name)
      void ECALL_lremovexattr();           // 15, rev_lremovexattr(const char  *path, const char  *name)
      void ECALL_fremovexattr();           // 16, rev_fremovexattr(int fd, const char  *name)
      void ECALL_getcwd();                 // 17, rev_getcwd(char  *buf, unsigned long size)
      void ECALL_lookup_dcookie();         // 18, rev_lookup_dcookie(u64 cookie64, char  *buf, size_t len)
      void ECALL_eventfd2();               // 19, rev_eventfd2(unsigned int count, int flags)
      void ECALL_epoll_create1();          // 20, rev_epoll_create1(int flags)
      void ECALL_epoll_ctl();              // 21, rev_epoll_ctl(int epfd, int op, int fd, struct epoll_event  *event)
      void ECALL_epoll_pwait();            // 22, rev_epoll_pwait(int epfd, struct epoll_event  *events, int maxevents, int timeout, const sigset_t  *sigmask, size_t sigsetsize)
      void ECALL_dup();                    // 23, rev_dup(unsigned int fildes)
      void ECALL_dup3();                   // 24, rev_dup3(unsigned int oldfd, unsigned int newfd, int flags)
      void ECALL_fcntl64();                // 25, rev_fcntl64(unsigned int fd, unsigned int cmd, unsigned long arg)
      void ECALL_inotify_init1();          // 26, rev_inotify_init1(int flags)
      void ECALL_inotify_add_watch();      // 27, rev_inotify_add_watch(int fd, const char  *path, u32 mask)
      void ECALL_inotify_rm_watch();       // 28, rev_inotify_rm_watch(int fd, __s32 wd)
      void ECALL_ioctl();                  // 29, rev_ioctl(unsigned int fd, unsigned int cmd, unsigned long arg)
      void ECALL_ioprio_set();             // 30, rev_ioprio_set(int which, int who, int ioprio)
      void ECALL_ioprio_get();             // 31, rev_ioprio_get(int which, int who)
      void ECALL_flock();                  // 32, rev_flock(unsigned int fd, unsigned int cmd)
      void ECALL_mknodat();                // 33, rev_mknodat(int dfd, const char  * filename, umode_t mode, unsigned dev)
      void ECALL_mkdirat();                // 34, rev_mkdirat(int dfd, const char  * pathname, umode_t mode)
      void ECALL_unlinkat();               // 35, rev_unlinkat(int dfd, const char  * pathname, int flag)
      void ECALL_symlinkat();              // 36, rev_symlinkat(const char  * oldname, int newdfd, const char  * newname)
      void ECALL_linkat();                 // 37, rev_unlinkat(int dfd, const char  * pathname, int flag)
      void ECALL_renameat();               // 38, rev_renameat(int olddfd, const char  * oldname, int newdfd, const char  * newname)
      void ECALL_umount();                 // 39, rev_umount(char  *name, int flags)
      void ECALL_mount();                  // 40, rev_umount(char  *name, int flags)
      void ECALL_pivot_root();             // 41, rev_pivot_root(const char  *new_root, const char  *put_old)
      void ECALL_ni_syscall();             // 42, rev_ni_syscall(void)
      void ECALL_statfs64();               // 43, rev_statfs64(const char  *path, size_t sz, struct statfs64  *buf)
      void ECALL_fstatfs64();              // 44, rev_fstatfs64(unsigned int fd, size_t sz, struct statfs64  *buf)
      void ECALL_truncate64();             // 45, rev_truncate64(const char  *path, loff_t length)
      void ECALL_ftruncate64();            // 46, rev_ftruncate64(unsigned int fd, loff_t length)
      void ECALL_fallocate();              // 47, rev_fallocate(int fd, int mode, loff_t offset, loff_t len)
      void ECALL_faccessat();              // 48, rev_faccessat(int dfd, const char  *filename, int mode)
      void ECALL_chdir();                  // 49, rev_chdir(const char  *filename)
      void ECALL_fchdir();                 // 50, rev_fchdir(unsigned int fd)
      void ECALL_chroot();                 // 51, rev_chroot(const char  *filename)
      void ECALL_fchmod();                 // 52, rev_fchmod(unsigned int fd, umode_t mode)
      void ECALL_fchmodat();               // 53, rev_fchmodat(int dfd, const char  * filename, umode_t mode)
      void ECALL_fchownat();               // 54, rev_fchownat(int dfd, const char  *filename, uid_t user, gid_t group, int flag)
      void ECALL_fchown();                 // 55, rev_fchown(unsigned int fd, uid_t user, gid_t group)
      void ECALL_openat();                 // 56, rev_openat(int dfd, const char  *filename, int flags, umode_t mode)
      void ECALL_close();                  // 57, rev_close(unsigned int fd)
      void ECALL_vhangup();                // 58, rev_vhangup(void)
      void ECALL_pipe2();                  // 59, rev_pipe2(int  *fildes, int flags)
      void ECALL_quotactl();               // 60, rev_quotactl(unsigned int cmd, const char  *special, qid_t id, void  *addr)
      void ECALL_getdents64();             // 61, rev_getdents64(unsigned int fd, struct linux_dirent64  *dirent, unsigned int count)
      void ECALL_lseek();                  // 62, rev_llseek(unsigned int fd, unsigned long offset_high, unsigned long offset_low, loff_t  *result, unsigned int whence)
      void ECALL_read();                   // 63, rev_read(unsigned int fd, char  *buf, size_t count)
      void ECALL_write();                  // 64, rev_write(unsigned int fd, const char  *buf, size_t count)
      void ECALL_readv();                  // 65, rev_readv(unsigned long fd, const struct iovec  *vec, unsigned long vlen)
      void ECALL_writev();                 // 66, rev_writev(unsigned long fd, const struct iovec  *vec, unsigned long vlen)
      void ECALL_pread64();                // 67, rev_pread64(unsigned int fd, char  *buf, size_t count, loff_t pos)
      void ECALL_pwrite64();               // 68, rev_pwrite64(unsigned int fd, const char  *buf, size_t count, loff_t pos)
      void ECALL_preadv();                 // 69, rev_preadv(unsigned long fd, const struct iovec  *vec, unsigned long vlen, unsigned long pos_l, unsigned long pos_h)
      void ECALL_pwritev();                // 70, rev_pwritev(unsigned long fd, const struct iovec  *vec, unsigned long vlen, unsigned long pos_l, unsigned long pos_h)
      void ECALL_sendfile64();             // 71, rev_sendfile64(int out_fd, int in_fd, loff_t  *offset, size_t count)
      void ECALL_pselect6_time32();        // 72, rev_pselect6_time32(int, fd_set  *, fd_set  *, fd_set  *, struct old_timespec32  *, void  *)
      void ECALL_ppoll_time32();           // 73, rev_ppoll_time32(struct pollfd  *, unsigned int, struct old_timespec32  *, const sigset_t  *, size_t)
      void ECALL_signalfd4();              // 74, rev_signalfd4(int ufd, sigset_t  *user_mask, size_t sizemask, int flags)
      void ECALL_vmsplice();               // 75, rev_vmsplice(int fd, const struct iovec  *iov, unsigned long nr_segs, unsigned int flags)
      void ECALL_splice();                 // 76, rev_vmsplice(int fd, const struct iovec  *iov, unsigned long nr_segs, unsigned int flags)
      void ECALL_tee();                    // 77, rev_tee(int fdin, int fdout, size_t len, unsigned int flags)
      void ECALL_readlinkat();             // 78, rev_readlinkat(int dfd, const char  *path, char  *buf, int bufsiz)
      void ECALL_newfstatat();             // 79, rev_newfstatat(int dfd, const char  *filename, struct stat  *statbuf, int flag)
      void ECALL_newfstat();               // 80, rev_newfstat(unsigned int fd, struct stat  *statbuf)
      void ECALL_sync();                   // 81, rev_sync(void)
      void ECALL_fsync();                  // 82, rev_fsync(unsigned int fd)
      void ECALL_fdatasync();              // 83, rev_fdatasync(unsigned int fd)
      void ECALL_sync_file_range2();       // 84, rev_sync_file_range2(int fd, unsigned int flags, loff_t offset, loff_t nbytes)
      void ECALL_sync_file_range();        // 84, rev_sync_file_range(int fd, loff_t offset, loff_t nbytes, unsigned int flags)
      void ECALL_timerfd_create();         // 85, rev_timerfd_create(int clockid, int flags)
      void ECALL_timerfd_settime();        // 86, rev_timerfd_settime(int ufd, int flags, const struct __kernel_itimerspec  *utmr, struct __kernel_itimerspec  *otmr)
      void ECALL_timerfd_gettime();        // 87, rev_timerfd_gettime(int ufd, struct __kernel_itimerspec  *otmr)
      void ECALL_utimensat();              // 88, rev_utimensat(int dfd, const char  *filename, struct __kernel_timespec  *utimes, int flags)
      void ECALL_acct();                   // 89, rev_acct(const char  *name)
      void ECALL_capget();                 // 90, rev_capget(cap_user_header_t header, cap_user_data_t dataptr)
      void ECALL_capset();                 // 91, rev_capset(cap_user_header_t header, const cap_user_data_t data)
      void ECALL_personality();            // 92, rev_personality(unsigned int personality)
      void ECALL_exit();                   // 93, rev_exit(int error_code)
      void ECALL_exit_group();             // 94, rev_exit_group(int error_code)
      void ECALL_waitid();                 // 95, rev_waitid(int which, pid_t pid, struct siginfo  *infop, int options, struct rusage  *ru)
      void ECALL_set_tid_address();        // 96, rev_set_tid_address(int  *tidptr)
      void ECALL_unshare();                // 97, rev_unshare(unsigned long unshare_flags)
      void ECALL_futex();                  // 98, rev_futex(u32  *uaddr, int op, u32 val, struct __kernel_timespec  *utime, u32  *uaddr2, u32 val3)
      void ECALL_set_robust_list();        // 99, rev_set_robust_list(struct robust_list_head  *head, size_t len)
      void ECALL_get_robust_list();        // 100, rev_get_robust_list(int pid, struct robust_list_head  *  *head_ptr, size_t  *len_ptr)
      void ECALL_nanosleep();              // 101, rev_nanosleep(struct __kernel_timespec  *rqtp, struct __kernel_timespec  *rmtp)
      void ECALL_getitimer();              // 102, rev_getitimer(int which, struct __kernel_old_itimerval  *value)
      void ECALL_setitimer();              // 103, rev_setitimer(int which, struct __kernel_old_itimerval  *value, struct __kernel_old_itimerval  *ovalue)
      void ECALL_kexec_load();             // 104, rev_kexec_load(unsigned long entry, unsigned long nr_segments, struct kexec_segment  *segments, unsigned long flags)
      void ECALL_init_module();            // 105, rev_init_module(void  *umod, unsigned long len, const char  *uargs)
      void ECALL_delete_module();          // 106, rev_delete_module(const char  *name_user, unsigned int flags)
      void ECALL_timer_create();           // 107, rev_timer_create(clockid_t which_clock, struct sigevent  *timer_event_spec, timer_t  * created_timer_id)
      void ECALL_timer_gettime();          // 108, rev_timer_gettime(timer_t timer_id, struct __kernel_itimerspec  *setting)
      void ECALL_timer_getoverrun();       // 109, rev_timer_getoverrun(timer_t timer_id)
      void ECALL_timer_settime();          // 110, rev_timer_settime(timer_t timer_id, int flags, const struct __kernel_itimerspec  *new_setting, struct __kernel_itimerspec  *old_setting)
      void ECALL_timer_delete();           // 111, rev_timer_delete(timer_t timer_id)
      void ECALL_clock_settime();          // 112, rev_clock_settime(clockid_t which_clock, const struct __kernel_timespec  *tp)
      void ECALL_clock_gettime();          // 113, rev_clock_gettime(clockid_t which_clock, struct __kernel_timespec  *tp)
      void ECALL_clock_getres();           // 114, rev_clock_getres(clockid_t which_clock, struct __kernel_timespec  *tp)
      void ECALL_clock_nanosleep();        // 115, rev_clock_nanosleep(clockid_t which_clock, int flags, const struct __kernel_timespec  *rqtp, struct __kernel_timespec  *rmtp)
      void ECALL_syslog();                 // 116, rev_syslog(int type, char  *buf, int len)
      void ECALL_ptrace();                 // 117, rev_ptrace(long request, long pid, unsigned long addr, unsigned long data)
      void ECALL_sched_setparam();         // 118, rev_sched_setparam(pid_t pid, struct sched_param  *param)
      void ECALL_sched_setscheduler();     // 119, rev_sched_setscheduler(pid_t pid, int policy, struct sched_param  *param)
      void ECALL_sched_getscheduler();     // 120, rev_sched_getscheduler(pid_t pid)
      void ECALL_sched_getparam();         // 121, rev_sched_getparam(pid_t pid, struct sched_param  *param)
      void ECALL_sched_setaffinity();      // 122, rev_sched_setaffinity(pid_t pid, unsigned int len, unsigned long  *user_mask_ptr)
      void ECALL_sched_getaffinity();      // 123, rev_sched_getaffinity(pid_t pid, unsigned int len, unsigned long  *user_mask_ptr)
      void ECALL_sched_yield();            // 124, rev_sched_yield(void)
      void ECALL_sched_get_priority_max(); // 125, rev_sched_get_priority_max(int policy)
      void ECALL_sched_get_priority_min(); // 126, rev_sched_get_priority_min(int policy)
      void ECALL_sched_rr_get_interval();  // 127, rev_sched_rr_get_interval(pid_t pid, struct __kernel_timespec  *interval)
      void ECALL_restart_syscall();        // 128, rev_restart_syscall(void)
      void ECALL_kill();                   // 129, rev_kill(pid_t pid, int sig)
      void ECALL_tkill();                  // 130, rev_tkill(pid_t pid, int sig)
      void ECALL_tgkill();                 // 131, rev_tgkill(pid_t tgid, pid_t pid, int sig)
      void ECALL_sigaltstack();            // 132, rev_sigaltstack(const struct sigaltstack  *uss, struct sigaltstack  *uoss)
      void ECALL_rt_sigsuspend();          // 133, rev_rt_sigsuspend(sigset_t  *unewset, size_t sigsetsize)
      void ECALL_rt_sigaction();           // 134, rev_rt_sigaction(int, const struct sigaction  *, struct sigaction  *, size_t)
      void ECALL_rt_sigprocmask();         // 135, rev_rt_sigprocmask(int how, sigset_t  *set, sigset_t  *oset, size_t sigsetsize)
      void ECALL_rt_sigpending();          // 136, rev_rt_sigpending(sigset_t  *set, size_t sigsetsize)
      void ECALL_rt_sigtimedwait_time32(); // 137, rev_rt_sigtimedwait_time32(const sigset_t  *uthese, siginfo_t  *uinfo, const struct old_timespec32  *uts, size_t sigsetsize)
      void ECALL_rt_sigqueueinfo();        // 138, rev_rt_sigqueueinfo(pid_t pid, int sig, siginfo_t  *uinfo)
      void ECALL_setpriority();            // 140, rev_setpriority(int which, int who, int niceval)
      void ECALL_getpriority();            // 141, rev_getpriority(int which, int who)
      void ECALL_reboot();                 // 142, rev_reboot(int magic1, int magic2, unsigned int cmd, void  *arg)
      void ECALL_setregid();               // 143, rev_setregid(gid_t rgid, gid_t egid)
      void ECALL_setgid();                 // 144, rev_setgid(gid_t gid)
      void ECALL_setreuid();               // 145, rev_setreuid(uid_t ruid, uid_t euid)
      void ECALL_setuid();                 // 146, rev_setuid(uid_t uid)
      void ECALL_setresuid();              // 147, rev_setresuid(uid_t ruid, uid_t euid, uid_t suid)
      void ECALL_getresuid();              // 148, rev_getresuid(uid_t  *ruid, uid_t  *euid, uid_t  *suid)
      void ECALL_setresgid();              // 149, rev_setresgid(gid_t rgid, gid_t egid, gid_t sgid)
      void ECALL_getresgid();              // 150, rev_getresgid(gid_t  *rgid, gid_t  *egid, gid_t  *sgid)
      void ECALL_setfsuid();               // 151, rev_setfsuid(uid_t uid)
      void ECALL_setfsgid();               // 152, rev_setfsgid(gid_t gid)
      void ECALL_times();                  // 153, rev_times(struct tms  *tbuf)
      void ECALL_setpgid();                // 154, rev_setpgid(pid_t pid, pid_t pgid)
      void ECALL_getpgid();                // 155, rev_getpgid(pid_t pid)
      void ECALL_getsid();                 // 156, rev_getsid(pid_t pid)
      void ECALL_setsid();                 // 157, rev_setsid(void)
      void ECALL_getgroups();              // 158, rev_getgroups(int gidsetsize, gid_t  *grouplist)
      void ECALL_setgroups();              // 159, rev_setgroups(int gidsetsize, gid_t  *grouplist)
      void ECALL_newuname();               // 160, rev_newuname(struct new_utsname  *name)
      void ECALL_sethostname();            // 161, rev_sethostname(char  *name, int len)
      void ECALL_setdomainname();          // 162, rev_setdomainname(char  *name, int len)
      void ECALL_getrlimit();              // 163, rev_getrlimit(unsigned int resource, struct rlimit  *rlim)
      void ECALL_setrlimit();              // 164, rev_setrlimit(unsigned int resource, struct rlimit  *rlim)
      void ECALL_getrusage();              // 165, rev_getrusage(int who, struct rusage  *ru)
      void ECALL_umask();                  // 166, rev_umask(int mask)
      void ECALL_prctl();                  // 167, rev_prctl(int option, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5)
      void ECALL_getcpu();                 // 168, rev_getcpu(unsigned  *cpu, unsigned  *node, struct getcpu_cache  *cache)
      void ECALL_gettimeofday();           // 169, rev_gettimeofday(struct __kernel_old_timeval  *tv, struct timezone  *tz)
      void ECALL_settimeofday();           // 170, rev_settimeofday(struct __kernel_old_timeval  *tv, struct timezone  *tz)
      void ECALL_adjtimex();               // 171, rev_adjtimex(struct __kernel_timex  *txc_p)
      void ECALL_getpid();                 // 172, rev_getpid(void)
      void ECALL_getppid();                // 173, rev_getppid(void)
      void ECALL_getuid();                 // 174, rev_getuid(void)
      void ECALL_geteuid();                // 175, rev_geteuid(void)
      void ECALL_getgid();                 // 176, rev_getgid(void)
      void ECALL_getegid();                // 177, rev_getegid(void)
      void ECALL_gettid();                 // 178, rev_gettid(void)
      void ECALL_sysinfo();                // 179, rev_sysinfo(struct sysinfo  *info)
      void ECALL_mq_open();                // 180, rev_mq_open(const char  *name, int oflag, umode_t mode, struct mq_attr  *attr)
      void ECALL_mq_unlink();              // 181, rev_mq_unlink(const char  *name)
      void ECALL_mq_timedsend();           // 182, rev_mq_timedsend(mqd_t mqdes, const char  *msg_ptr, size_t msg_len, unsigned int msg_prio, const struct __kernel_timespec  *abs_timeout)
      void ECALL_mq_timedreceive();        // 183, rev_mq_timedreceive(mqd_t mqdes, char  *msg_ptr, size_t msg_len, unsigned int  *msg_prio, const struct __kernel_timespec  *abs_timeout)
      void ECALL_mq_notify();              // 184, rev_mq_notify(mqd_t mqdes, const struct sigevent  *notification)
      void ECALL_mq_getsetattr();          // 185, rev_mq_getsetattr(mqd_t mqdes, const struct mq_attr  *mqstat, struct mq_attr  *omqstat)
      void ECALL_msgget();                 // 186, rev_msgget(key_t key, int msgflg)
      void ECALL_msgctl();                 // 187, rev_old_msgctl(int msqid, int cmd, struct msqid_ds  *buf)
      void ECALL_msgrcv();                 // 188, rev_msgrcv(int msqid, struct msgbuf  *msgp, size_t msgsz, long msgtyp, int msgflg)
      void ECALL_msgsnd();                 // 189, rev_msgsnd(int msqid, struct msgbuf  *msgp, size_t msgsz, int msgflg)
      void ECALL_semget();                 // 190, rev_semget(key_t key, int nsems, int semflg)
      void ECALL_semctl();                 // 191, rev_semctl(int semid, int semnum, int cmd, unsigned long arg)
      void ECALL_semtimedop();             // 192, rev_semtimedop(int semid, struct sembuf  *sops, unsigned nsops, const struct __kernel_timespec  *timeout)
      void ECALL_semop();                  // 193, rev_semop(int semid, struct sembuf  *sops, unsigned nsops)
      void ECALL_shmget();                 // 194, rev_shmget(key_t key, size_t size, int flag)
      void ECALL_shmctl();                 // 195, rev_old_shmctl(int shmid, int cmd, struct shmid_ds  *buf)
      void ECALL_shmat();                  // 196, rev_shmat(int shmid, char  *shmaddr, int shmflg)
      void ECALL_shmdt();                  // 197, rev_shmdt(char  *shmaddr)
      void ECALL_socket();                 // 198, rev_socket(int, int, int)
      void ECALL_socketpair();             // 199, rev_socketpair(int, int, int, int  *)
      void ECALL_bind();                   // 200, rev_bind(int, struct sockaddr  *, int)
      void ECALL_listen();                 // 201, rev_listen(int, int)
      void ECALL_accept();                 // 202, rev_accept(int, struct sockaddr  *, int  *)
      void ECALL_connect();                // 203, rev_connect(int, struct sockaddr  *, int)
      void ECALL_getsockname();            // 204, rev_getsockname(int, struct sockaddr  *, int  *)
      void ECALL_getpeername();            // 205, rev_getpeername(int, struct sockaddr  *, int  *)
      void ECALL_sendto();                 // 206, rev_sendto(int, void  *, size_t, unsigned, struct sockaddr  *, int)
      void ECALL_recvfrom();               // 207, rev_recvfrom(int, void  *, size_t, unsigned, struct sockaddr  *, int  *)
      void ECALL_setsockopt();             // 208, rev_setsockopt(int fd, int level, int optname, char  *optval, int optlen)
      void ECALL_getsockopt();             // 209, rev_getsockopt(int fd, int level, int optname, char  *optval, int  *optlen)
      void ECALL_shutdown();               // 210, rev_shutdown(int, int)
      void ECALL_sendmsg();                // 211, rev_sendmsg(int fd, struct user_msghdr  *msg, unsigned flags)
      void ECALL_recvmsg();                // 212, rev_recvmsg(int fd, struct user_msghdr  *msg, unsigned flags)
      void ECALL_readahead();              // 213, rev_readahead(int fd, loff_t offset, size_t count)
      void ECALL_brk();                    // 214, rev_brk(unsigned long brk)
      void ECALL_munmap();                 // 215, rev_munmap(unsigned long addr, size_t len)
      void ECALL_mremap();                 // 216, rev_mremap(unsigned long addr, unsigned long old_len, unsigned long new_len, unsigned long flags, unsigned long new_addr)
      void ECALL_add_key();                // 217, rev_add_key(const char  *_type, const char  *_description, const void  *_payload, size_t plen, key_serial_t destringid)
      void ECALL_request_key();            // 218, rev_request_key(const char  *_type, const char  *_description, const char  *_callout_info, key_serial_t destringid)
      void ECALL_keyctl();                 // 219, rev_keyctl(int cmd, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5)
      void ECALL_clone();                  // 220, rev_clone(unsigned long, unsigned long, int  *, unsigned long, int  *)
      void ECALL_execve();                 // 221, rev_execve(const char  *filename, const char  *const  *argv, const char  *const  *envp)
      void ECALL_mmap();                   // 222, rev_old_mmap(struct mmap_arg_struct  *arg)
      void ECALL_fadvise64_64();           // 223, rev_fadvise64_64(int fd, loff_t offset, loff_t len, int advice)
      void ECALL_swapon();                 // 224, rev_swapon(const char  *specialfile, int swap_flags)
      void ECALL_swapoff();                // 225, rev_swapoff(const char  *specialfile)
      void ECALL_mprotect();               // 226, rev_mprotect(unsigned long start, size_t len, unsigned long prot)
      void ECALL_msync();                  // 227, rev_msync(unsigned long start, size_t len, int flags)
      void ECALL_mlock();                  // 228, rev_mlock(unsigned long start, size_t len)
      void ECALL_munlock();                // 229, rev_munlock(unsigned long start, size_t len)
      void ECALL_mlockall();               // 230, rev_mlockall(int flags)
      void ECALL_munlockall();             // 231, rev_munlockall(void)
      void ECALL_mincore();                // 232, rev_mincore(unsigned long start, size_t len, unsigned char  * vec)
      void ECALL_madvise();                // 233, rev_madvise(unsigned long start, size_t len, int behavior)
      void ECALL_remap_file_pages();       // 234, rev_remap_file_pages(unsigned long start, unsigned long size, unsigned long prot, unsigned long pgoff, unsigned long flags)
      void ECALL_mbind();                  // 235, rev_mbind(unsigned long start, unsigned long len, unsigned long mode, const unsigned long  *nmask, unsigned long maxnode, unsigned flags)
      void ECALL_get_mempolicy();          // 236, rev_get_mempolicy(int  *policy, unsigned long  *nmask, unsigned long maxnode, unsigned long addr, unsigned long flags)
      void ECALL_set_mempolicy();          // 237, rev_set_mempolicy(int mode, const unsigned long  *nmask, unsigned long maxnode)
      void ECALL_migrate_pages();          // 238, rev_migrate_pages(pid_t pid, unsigned long maxnode, const unsigned long  *from, const unsigned long  *to)
      void ECALL_move_pages();             // 239, rev_move_pages(pid_t pid, unsigned long nr_pages, const void  *  *pages, const int  *nodes, int  *status, int flags)
      void ECALL_rt_tgsigqueueinfo();      // 240, rev_rt_tgsigqueueinfo(pid_t tgid, pid_t pid, int sig, siginfo_t  *uinfo)
      void ECALL_perf_event_open();        // 241, rev_perf_event_open(")
      void ECALL_accept4();                // 242, rev_accept4(int, struct sockaddr  *, int  *, int)
      void ECALL_recvmmsg_time32();        // 243, rev_recvmmsg_time32(int fd, struct mmsghdr  *msg, unsigned int vlen, unsigned flags, struct old_timespec32  *timeout)
      void ECALL_wait4();                  // 260, rev_wait4(pid_t pid, int  *stat_addr, int options, struct rusage  *ru)
      void ECALL_prlimit64();              // 261, rev_prlimit64(pid_t pid, unsigned int resource, const struct rlimit64  *new_rlim, struct rlimit64  *old_rlim)
      void ECALL_fanotify_init();          // 262, rev_fanotify_init(unsigned int flags, unsigned int event_f_flags)
      void ECALL_fanotify_mark();          // 263, rev_fanotify_mark(int fanotify_fd, unsigned int flags, u64 mask, int fd, const char  *pathname)
      void ECALL_name_to_handle_at();      // 264, rev_name_to_handle_at(int dfd, const char  *name, struct file_handle  *handle, int  *mnt_id, int flag)
      void ECALL_open_by_handle_at();      // 265, rev_open_by_handle_at(int mountdirfd, struct file_handle  *handle, int flags)
      void ECALL_clock_adjtime();          // 266, rev_clock_adjtime(clockid_t which_clock, struct __kernel_timex  *tx)
      void ECALL_syncfs();                 // 267, rev_syncfs(int fd)
      void ECALL_setns();                  // 268, rev_setns(int fd, int nstype)
      void ECALL_sendmmsg();               // 269, rev_sendmmsg(int fd, struct mmsghdr  *msg, unsigned int vlen, unsigned flags)
      void ECALL_process_vm_readv();       // 270, rev_process_vm_readv(pid_t pid, const struct iovec  *lvec, unsigned long liovcnt, const struct iovec  *rvec, unsigned long riovcnt, unsigned long flags)
      void ECALL_process_vm_writev();      // 271, rev_process_vm_writev(pid_t pid, const struct iovec  *lvec, unsigned long liovcnt, const struct iovec  *rvec, unsigned long riovcnt, unsigned long flags)
      void ECALL_kcmp();                   // 272, rev_kcmp(pid_t pid1, pid_t pid2, int type, unsigned long idx1, unsigned long idx2)
      void ECALL_finit_module();           // 273, rev_finit_module(int fd, const char  *uargs, int flags)
      void ECALL_sched_setattr();          // 274, rev_sched_setattr(pid_t pid, struct sched_attr  *attr, unsigned int flags)
      void ECALL_sched_getattr();          // 275, rev_sched_getattr(pid_t pid, struct sched_attr  *attr, unsigned int size, unsigned int flags)
      void ECALL_renameat2();              // 276, rev_renameat2(int olddfd, const char  *oldname, int newdfd, const char  *newname, unsigned int flags)
      void ECALL_seccomp();                // 277, rev_seccomp(unsigned int op, unsigned int flags, void  *uargs)
      void ECALL_getrandom();              // 278, rev_getrandom(char  *buf, size_t count, unsigned int flags)
      void ECALL_memfd_create();           // 279, rev_memfd_create(const char  *uname_ptr, unsigned int flags)
      void ECALL_bpf();                    // 280, rev_bpf(int cmd, union bpf_attr *attr, unsigned int size)
      void ECALL_execveat();               // 281, rev_execveat(int dfd, const char  *filename, const char  *const  *argv, const char  *const  *envp, int flags)
      void ECALL_userfaultfd();            // 282, rev_userfaultfd(int flags)
      void ECALL_membarrier();             // 283, rev_membarrier(int cmd, unsigned int flags, int cpu_id)
      void ECALL_mlock2();                 // 284, rev_mlock2(unsigned long start, size_t len, int flags)
      void ECALL_copy_file_range();        // 285, rev_copy_file_range(int fd_in, loff_t  *off_in, int fd_out, loff_t  *off_out, size_t len, unsigned int flags)
      void ECALL_preadv2();                // 286, rev_preadv2(unsigned long fd, const struct iovec  *vec, unsigned long vlen, unsigned long pos_l, unsigned long pos_h, rwf_t flags)
      void ECALL_pwritev2();               // 287, rev_pwritev2(unsigned long fd, const struct iovec  *vec, unsigned long vlen, unsigned long pos_l, unsigned long pos_h, rwf_t flags)
      void ECALL_pkey_mprotect();          // 288, rev_pkey_mprotect(unsigned long start, size_t len, unsigned long prot, int pkey)
      void ECALL_pkey_alloc();             // 289, rev_pkey_alloc(unsigned long flags, unsigned long init_val)
      void ECALL_pkey_free();              // 290, rev_pkey_free(int pkey)
      void ECALL_statx();                  // 291, rev_statx(int dfd, const char  *path, unsigned flags, unsigned mask, struct statx  *buffer)
      void ECALL_io_pgetevents();          // 292, rev_io_pgetevents(aio_context_t ctx_id, long min_nr, long nr, struct io_event  *events, struct __kernel_timespec  *timeout, const struct __aio_sigset *sig)
      void ECALL_rseq();                   // 293, rev_rseq(struct rseq  *rseq, uint32_t rseq_len, int flags, uint32_t sig)
      void ECALL_kexec_file_load();        // 294, rev_kexec_file_load(int kernel_fd, int initrd_fd, unsigned long cmdline_len, const char  *cmdline_ptr, unsigned long flags)
      // void ECALL_clock_gettime();          // 403, rev_clock_gettime(clockid_t which_clock, struct __kernel_timespec  *tp)
      // void ECALL_clock_settime();          // 404, rev_clock_settime(clockid_t which_clock, const struct __kernel_timespec  *tp)
      // void ECALL_clock_adjtime();          // 405, rev_clock_adjtime(clockid_t which_clock, struct __kernel_timex  *tx)
      // void ECALL_clock_getres();           // 406, rev_clock_getres(clockid_t which_clock, struct __kernel_timespec  *tp)
      // void ECALL_clock_nanosleep();        // 407, rev_clock_nanosleep(clockid_t which_clock, int flags, const struct __kernel_timespec  *rqtp, struct __kernel_timespec  *rmtp)
      // void ECALL_timer_gettime();          // 408, rev_timer_gettime(timer_t timer_id, struct __kernel_itimerspec  *setting)
      // void ECALL_timer_settime();          // 409, rev_timer_settime(timer_t timer_id, int flags, const struct __kernel_itimerspec  *new_setting, struct __kernel_itimerspec  *old_setting)
      // void ECALL_timerfd_gettime();        // 410, rev_timerfd_gettime(int ufd, struct __kernel_itimerspec  *otmr)
      // void ECALL_timerfd_settime();        // 411, rev_timerfd_settime(int ufd, int flags, const struct __kernel_itimerspec  *utmr, struct __kernel_itimerspec  *otmr)
      // void ECALL_utimensat();              // 412, rev_utimensat(int dfd, const char  *filename, struct __kernel_timespec  *utimes, int flags)
      // void ECALL_io_pgetevents();          // 416, rev_io_pgetevents(aio_context_t ctx_id, long min_nr, long nr, struct io_event  *events, struct __kernel_timespec  *timeout, const struct __aio_sigset *sig)
      // void ECALL_mq_timedsend();           // 418, rev_mq_timedsend(mqd_t mqdes, const char  *msg_ptr, size_t msg_len, unsigned int msg_prio, const struct __kernel_timespec  *abs_timeout)
      // void ECALL_mq_timedreceive();        // 419, rev_mq_timedreceive(mqd_t mqdes, char  *msg_ptr, size_t msg_len, unsigned int  *msg_prio, const struct __kernel_timespec  *abs_timeout)
      // void ECALL_semtimedop();             // 420, rev_semtimedop(int semid, struct sembuf  *sops, unsigned nsops, const struct __kernel_timespec  *timeout)
      // void ECALL_futex();                  // 422, rev_futex(u32  *uaddr, int op, u32 val, struct __kernel_timespec  *utime, u32  *uaddr2, u32 val3)
      // void ECALL_sched_rr_get_interval();  // 423, rev_sched_rr_get_interval(pid_t pid, struct __kernel_timespec  *interval)
      void ECALL_pidfd_send_signal();      // 424, rev_pidfd_send_signal(int pidfd, int sig, siginfo_t  *info, unsigned int flags)
      void ECALL_io_uring_setup();         // 425, rev_io_uring_setup(u32 entries, struct io_uring_params  *p)
      void ECALL_io_uring_enter();         // 426, rev_io_uring_enter(unsigned int fd, u32 to_submit, u32 min_complete, u32 flags, const sigset_t  *sig, size_t sigsz)
      void ECALL_io_uring_register();      // 427, rev_io_uring_register(unsigned int fd, unsigned int op, void  *arg, unsigned int nr_args)
      void ECALL_open_tree();              // 428, rev_open_tree(int dfd, const char  *path, unsigned flags)
      void ECALL_move_mount();             // 429, rev_move_mount(int from_dfd, const char  *from_path, int to_dfd, const char  *to_path, unsigned int ms_flags)
      void ECALL_fsopen();                 // 430, rev_fsopen(const char  *fs_name, unsigned int flags)
      void ECALL_fsconfig();               // 431, rev_fsconfig(int fs_fd, unsigned int cmd, const char  *key, const void  *value, int aux)
      void ECALL_fsmount();                // 432, rev_fsmount(int fs_fd, unsigned int flags, unsigned int ms_flags)
      void ECALL_fspick();                 // 433, rev_fspick(int dfd, const char  *path, unsigned int flags)
      void ECALL_pidfd_open();             // 434, rev_pidfd_open(pid_t pid, unsigned int flags)
      void ECALL_clone3();                 // 435, rev_clone3(struct clone_args  *uargs, size_t size)
      void ECALL_close_range();            // 436, rev_close_range(unsigned int fd, unsigned int max_fd, unsigned int flags)
      void ECALL_openat2();                // 437, rev_openat2(int dfd, const char  *filename, struct open_how *how, size_t size)
      void ECALL_pidfd_getfd();            // 438, rev_pidfd_getfd(int pidfd, int fd, unsigned int flags)
      void ECALL_faccessat2();             // 439, rev_faccessat2(int dfd, const char  *filename, int mode, int flags)
      void ECALL_process_madvise();        // 440, rev_process_madvise(int pidfd, const struct iovec  *vec, size_t vlen, int behavior, unsigned int flags)

      // =============== Begin Rev Specific Thread Functions ===============
      void ECALL_pthread_create();         // 1000, rev_pthread_create(pthread_t  *thread, const pthread_attr_t  *attr, void  *(*start_routine)(void  *), void  *arg)

      /// RevProc: Table of ecall codes w/ corresponding function pointer implementations
      std::unordered_map<uint32_t, std::function<void(RevProc*)>> Ecalls;

      /// RevProc: Initialize all of the ecalls inside the above table
      void InitEcallTable();

      /// RevProc: Execute the Ecall based on the code loaded in RegFile->RV64_SCAUSE
      void ExecEcall();


    }; // class RevProc
  } // namespace RevCPU
} // namespace SST

#endif

// EOF
