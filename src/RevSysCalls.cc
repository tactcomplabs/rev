#include "../include/RevProc.h"
#include "../include/RevSysCalls.h"
#include "../../common/include/RevCommon.h"
#include "RevMem.h"
#include <bitset>
#include <filesystem>
#include <sys/xattr.h>

using namespace SST::RevCPU;

/// Parse a string for an ECALL starting at address straddr, updating the state
/// as characters are read, and call action() when the end of string is reached.
EcallStatus RevProc::EcallLoadAndParseString(RevInst& inst,
                                             uint64_t straddr,
                                             std::function<void()> action){
  auto  rtval = EcallStatus::ERROR;
  auto& EcallState = Harts.at(HartToExec)->GetEcallState();

  if( RegFile->GetLSQueue()->count(make_lsq_hash(10, RevRegClass::RegGPR, HartToExec)) > 0 ){
    rtval = EcallStatus::CONTINUE;
  } else {
    // we don't know how long the path string is so read a byte (char)
    // at a time and search for the string terminator character '\0'
    if(EcallState.bytesRead != 0){
      EcallState.string += std::string_view(EcallState.buf.data(), EcallState.bytesRead);
      EcallState.bytesRead = 0;
    }

    // We store the 0-terminator byte in EcallState.string to distinguish an empty
    // C string from no data read at all. If we read an empty string in the
    // program, EcallState.string.size() == 1 with front() == back() == '\0'. If no
    // data has been read yet, EcallState.string.size() == 0.
    if(EcallState.string.size() && !EcallState.string.back()){
      //found the null terminator - we're done
      // action is usually passed in as a lambda with local code and captures
      // from the caller, such as performing a syscall using EcallState.string.
      action();

      EcallState.string.clear();   //reset the ECALL buffers
      EcallState.bytesRead = 0;

      DependencyClear(HartToExec, 10, false);
      rtval = EcallStatus::SUCCESS;
    }else{
      //We are in the middle of the string - read one byte
      MemReq req{straddr + EcallState.string.size(), 10,
                 RevRegClass::RegGPR, HartToExec, MemOp::MemOpREAD,
                 true, [=](const MemReq& req){this->MarkLoadComplete(req);}};
      LSQueue->insert({make_lsq_hash(req.DestReg, req.RegType, req.Hart), req});
      mem->ReadVal(HartToExec,
                  straddr + EcallState.string.size(),
                  EcallState.buf.data(),
                  req,
                  REVMEM_FLAGS(0));
      EcallState.bytesRead = 1;
      DependencySet(HartToExec, 10, false);
      rtval = EcallStatus::CONTINUE;
    }
  }
  return rtval;
}

// 0, rev_io_setup(unsigned nr_reqs, aio_context_t  *ctx)
EcallStatus RevProc::ECALL_io_setup(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: io_setup called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 1, rev_io_destroy(aio_context_t ctx)
EcallStatus RevProc::ECALL_io_destroy(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: io_destroy called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 2, rev_io_submit(aio_context_t, long, struct iocb  *  *)
EcallStatus RevProc::ECALL_io_submit(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: io_submit called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 3, rev_io_cancel(aio_context_t ctx_id, struct iocb  *iocb, struct io_event  *result)
EcallStatus RevProc::ECALL_io_cancel(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: io_cancel called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 4, rev_io_getevents(aio_context_t ctx_id, long min_nr, long nr, struct io_event  *events, struct __kernel_timespec  *timeout)
EcallStatus RevProc::ECALL_io_getevents(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: io_getevents called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 5, rev_setxattr(const char  *path, const char  *name, const void  *value, size_t size, int flags)
EcallStatus RevProc::ECALL_setxattr(RevInst& inst){
#if 0
  // TODO: Need to load the data from (value, size bytes) into
  // hostValue vector before it can be passed to setxattr() on host.

  auto path = RegFile->GetX<uint64_t>(RevReg::a0);
  auto name = RegFile->GetX<uint64_t>(RevReg::a1);
  auto value = RegFile->GetX<uint64_t>(RevReg::a2);
  auto size = RegFile->GetX<size_t>(RevReg::a3);
  auto flags = RegFile->GetX<uint64_t>(RevReg::a4);

  // host-side value which has size bytes
  std::vector<char> hostValue(size);

  if(ECALL.path_string.empty()){
    // We are still parsing the path string. When it is finished, we
    // will move the ECALL.string to ECALL.path_string and continue below
    auto action = [&]{
      ECALL.path_string = std::move(ECALL.string);
    };
    auto rtv = EcallLoadAndParseString(inst, path, action);

    // When the parsing of path_string returns SUCCESS, we change it to
    // CONTINUE to continue the later stages
    return rtv == EcallStatus::SUCCESS ? EcallStatus::CONTINUE : rtv;
  }else{
    // We have set path_string. Now we are parsing the name string.
    // When it is finished, we will call setxattr(path, name, ...);
    auto action = [&]{

#ifdef __APPLE__
      uint32_t position = 0;
      int rc = setxattr(ECALL.path_string.c_str(),
                        ECALL.string.c_str(),
                        &hostValue[0],
                        size,
                        position,
                        flags);
#else
      int rc = setxattr(ECALL.path_string.c_str(),
                        ECALL.string.c_str(),
                        &hostValue[0],
                        size,
                        flags);
#endif

      // Clear path_string so that later calls parse path_string first
      ECALL.path_string.clear();

      // setxattr return code
      RegFile->SetX(RevReg::a0, rc);
    };

    // Parse the name string, then call setxattr() using path and name
    return EcallLoadAndParseString(inst, name, action);
  }
#else
  return EcallStatus::SUCCESS;
#endif
}

// 6, rev_lsetxattr(const char  *path, const char  *name, const void  *value, size_t size, int flags)
EcallStatus RevProc::ECALL_lsetxattr(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: lsetxattr called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 7, rev_fsetxattr(int fd, const char  *name, const void  *value, size_t size, int flags)
EcallStatus RevProc::ECALL_fsetxattr(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: fsetxattr called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 8, rev_getxattr(const char  *path, const char  *name, void  *value, size_t size)
EcallStatus RevProc::ECALL_getxattr(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: getxattr called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 9, rev_lgetxattr(const char  *path, const char  *name, void  *value, size_t size)
EcallStatus RevProc::ECALL_lgetxattr(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: lgetxattr called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 10, rev_fgetxattr(int fd, const char  *name, void  *value, size_t size)
EcallStatus RevProc::ECALL_fgetxattr(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: fgetxattr called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 11, rev_listxattr(const char  *path, char  *list, size_t size)
EcallStatus RevProc::ECALL_listxattr(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: listxattr called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 12, rev_llistxattr(const char  *path, char  *list, size_t size)
EcallStatus RevProc::ECALL_llistxattr(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: llistxattr called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 13, rev_flistxattr(int fd, char  *list, size_t size)
EcallStatus RevProc::ECALL_flistxattr(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: flistxattr called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 14, rev_removexattr(const char  *path, const char  *name)
EcallStatus RevProc::ECALL_removexattr(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: removexattr called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 15, rev_lremovexattr(const char  *path, const char  *name)
EcallStatus RevProc::ECALL_lremovexattr(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: lremovexattr called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 16, rev_fremovexattr(int fd, const char  *name)
EcallStatus RevProc::ECALL_fremovexattr(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: fremovexattr called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 17, rev_getcwd(char  *buf, unsigned long size)
EcallStatus RevProc::ECALL_getcwd(RevInst& inst){
  auto BufAddr = RegFile->GetX<uint64_t>(RevReg::a0);
  auto size = RegFile->GetX<uint64_t>(RevReg::a1);
  auto CWD = std::filesystem::current_path();
  mem->WriteMem(HartToExec, BufAddr, size, CWD.c_str());

  // Returns null-terminated string in buf
  // (no need to set x10 since it's already got BufAddr)
  // RegFile->SetX(RevReg::a0, BufAddr);

  return EcallStatus::SUCCESS;
}

// 18, rev_lookup_dcookie(u64 cookie64, char  *buf, size_t len)
EcallStatus RevProc::ECALL_lookup_dcookie(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: lookup_dcookie called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 19, rev_eventfd2(unsigned int count, int flags)
EcallStatus RevProc::ECALL_eventfd2(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: eventfd2 called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 20, rev_epoll_create1(int flags)
EcallStatus RevProc::ECALL_epoll_create1(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: epoll_create1 called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 21, rev_epoll_ctl(int epfd, int op, int fd, struct epoll_event  *event)
EcallStatus RevProc::ECALL_epoll_ctl(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: epoll_ctl called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 22, rev_epoll_pwait(int epfd, struct epoll_event  *events, int maxevents, int timeout, const sigset_t  *sigmask, size_t sigsetsize)
EcallStatus RevProc::ECALL_epoll_pwait(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: epoll_pwait called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 23, rev_dup(unsigned int fildes)
EcallStatus RevProc::ECALL_dup(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: dup called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 24, rev_dup3(unsigned int oldfd, unsigned int newfd, int flags)
EcallStatus RevProc::ECALL_dup3(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: dup3 called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 25, rev_fcntl64(unsigned int fd, unsigned int cmd, unsigned long arg)
EcallStatus RevProc::ECALL_fcntl64(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: fcntl64 called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 26, rev_inotify_init1(int flags)
EcallStatus RevProc::ECALL_inotify_init1(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: inotify_init1 called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 27, rev_inotify_add_watch(int fd, const char  *path, u32 mask)
EcallStatus RevProc::ECALL_inotify_add_watch(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: inotify_add_watch called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 28, rev_inotify_rm_watch(int fd, __s32 wd)
EcallStatus RevProc::ECALL_inotify_rm_watch(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: inotify_rm_watch called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 29, rev_ioctl(unsigned int fd, unsigned int cmd, unsigned long arg)
EcallStatus RevProc::ECALL_ioctl(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: ioctl called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 30, rev_ioprio_set(int which, int who, int ioprio)
EcallStatus RevProc::ECALL_ioprio_set(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: ioprio_set called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 31, rev_ioprio_get(int which, int who)
EcallStatus RevProc::ECALL_ioprio_get(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: ioprio_get called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 32, rev_flock(unsigned int fd, unsigned int cmd)
EcallStatus RevProc::ECALL_flock(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: flock called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 33, rev_mknodat(int dfd, const char  * filename, umode_t mode, unsigned dev)
EcallStatus RevProc::ECALL_mknodat(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: mknodat called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// TODO: 34, rev_mkdirat(int dfd, const char  * pathname, umode_t mode)
EcallStatus RevProc::ECALL_mkdirat(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: mkdirat called");
  EcallState& ECALL = Harts.at(HartToExec)->GetEcallState();
  auto dirfd = RegFile->GetX<int>(RevReg::a0);
  auto path = RegFile->GetX<uint64_t>(RevReg::a1);
  auto mode = RegFile->GetX<unsigned short>(RevReg::a2);

  auto action = [&]{
    // Do the mkdirat on the host
    int rc = mkdirat(dirfd, ECALL.string.c_str(), mode);
    RegFile->SetX(RevReg::a0, rc);
  };
  return EcallLoadAndParseString(inst, path, action);
}

// 35, rev_unlinkat(int dfd, const char  * pathname, int flag)
EcallStatus RevProc::ECALL_unlinkat(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,

                  "ECALL: unlinkat called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 36, rev_symlinkat(const char  * oldname, int newdfd, const char  * newname)
EcallStatus RevProc::ECALL_symlinkat(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: symlinkat called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 37, rev_unlinkat(int dfd, const char  * pathname, int flag)
EcallStatus RevProc::ECALL_linkat(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: linkat called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 38, rev_renameat(int olddfd, const char  * oldname, int newdfd, const char  * newname)
EcallStatus RevProc::ECALL_renameat(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: renameat called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 39, rev_umount(char  *name, int flags)
EcallStatus RevProc::ECALL_umount(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: umount called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 40, rev_umount(char  *name, int flags)
EcallStatus RevProc::ECALL_mount(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: mount called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 41, rev_pivot_root(const char  *new_root, const char  *put_old)
EcallStatus RevProc::ECALL_pivot_root(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: pivot_root called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 42, rev_ni_syscall(void)
EcallStatus RevProc::ECALL_ni_syscall(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: ni_syscall called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 43, rev_statfs64(const char  *path, size_t sz, struct statfs64  *buf)
EcallStatus RevProc::ECALL_statfs64(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: statfs64 called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 44, rev_fstatfs64(unsigned int fd, size_t sz, struct statfs64  *buf)
EcallStatus RevProc::ECALL_fstatfs64(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: fstatfs64 called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 45, rev_truncate64(const char  *path, loff_t length)
EcallStatus RevProc::ECALL_truncate64(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: truncate64 called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 46, rev_ftruncate64(unsigned int fd, loff_t length)
EcallStatus RevProc::ECALL_ftruncate64(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: ftruncate64 called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 47, rev_fallocate(int fd, int mode, loff_t offset, loff_t len)
EcallStatus RevProc::ECALL_fallocate(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: fallocate called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 48, rev_faccessat(int dfd, const char  *filename, int mode)
EcallStatus RevProc::ECALL_faccessat(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: faccessat called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 49, rev_chdir(const char  *filename)
EcallStatus RevProc::ECALL_chdir(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: chdir called\n");
  auto path = RegFile->GetX<uint64_t>(RevReg::a0);
  auto action = [&]{
    int rc = chdir(Harts.at(HartToExec)->GetEcallState().string.c_str());
    RegFile->SetX(RevReg::a0, rc);
  };
  return EcallLoadAndParseString(inst, path, action);
}

// 50, rev_fchdir(unsigned int fd)
EcallStatus RevProc::ECALL_fchdir(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,

                  "ECALL: fchdir called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 51, rev_chroot(const char  *filename)
EcallStatus RevProc::ECALL_chroot(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: chroot called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 52, rev_fchmod(unsigned int fd, umode_t mode)
EcallStatus RevProc::ECALL_fchmod(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: fchmod called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 53, rev_fchmodat(int dfd, const char  * filename, umode_t mode)
EcallStatus RevProc::ECALL_fchmodat(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: fchmodat called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 54, rev_fchownat(int dfd, const char  *filename, uid_t user, gid_t group, int flag)
EcallStatus RevProc::ECALL_fchownat(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: fchownat called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 55, rev_fchown(unsigned int fd, uid_t user, gid_t group)
EcallStatus RevProc::ECALL_fchown(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: fchown called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 56, rev_openat(int dfd, const char  *filename, int flags, umode_t mode)
EcallStatus RevProc::ECALL_openat(RevInst& inst){
  auto& EcallState = Harts.at(HartToExec)->GetEcallState();
  if( EcallState.bytesRead == 0 ){
    output->verbose(CALL_INFO, 2, 0,
                    "ECALL: openat called by thread %" PRIu32
                    " on hart %" PRIu32 "\n",  GetActiveThreadID(), HartToExec);
  }
  auto dirfd = RegFile->GetX<int>(RevReg::a0);
  auto pathname = RegFile->GetX<uint64_t>(RevReg::a1);

  // commented out to remove warnings
  // auto flags = RegFile->GetX<int>(RevReg::a2);
  auto mode = RegFile->GetX<int>(RevReg::a3);

  /*
   * NOTE: this is currently only opening files in the current directory
   *       because of some oddities in parsing the arguments & flags
   *       but this will be fixed in the near future
   */

  /* Read the filename from memory one character at a time until we find '\0' */
  auto& Thread = GetThreadOnHart(HartToExec);


  auto action = [&]{
    // Do the openat on the host
    dirfd = open(std::filesystem::current_path().c_str(), mode);
    int fd = openat(dirfd, EcallState.string.c_str(), mode);

    // Add the file descriptor to this thread
    Thread->AddFD(fd);

    // openat returns the file descriptor of the opened file
    Thread->GetRegFile()->SetX(RevReg::a0, fd);

  };

  return EcallLoadAndParseString(inst, pathname, action);
}

// 57, rev_close(unsigned int fd)
EcallStatus RevProc::ECALL_close(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: close called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  auto fd = RegFile->GetX<int>(RevReg::a0);
  auto& ActiveThread = GetThreadOnHart(HartToExec);

  // Check if CurrCtx has fd in fildes vector
  if( !ActiveThread->FindFD(fd) ){
    output->fatal(CALL_INFO, -1,
                  "Core %" PRIu32 "; Hart %" PRIu32 "; Thread %" PRIu32
                  " tried to close file descriptor %" PRIu32
                  " but did not have access to it\n",
                  id, HartToExec, GetActiveThreadID(), fd);
    return EcallStatus::SUCCESS;
  }
  // Close file on host
  int rc = close(fd);

  // Remove from Ctx's fildes
  ActiveThread->RemoveFD(fd);

  // rc is propogated to rev from host
  RegFile->SetX(RevReg::a0, rc);

  return EcallStatus::SUCCESS;
}

// 58, rev_vhangup(void)
EcallStatus RevProc::ECALL_vhangup(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: vhangup called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 59, rev_pipe2(int  *fildes, int flags)
EcallStatus RevProc::ECALL_pipe2(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: pipe2 called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 60, rev_quotactl(unsigned int cmd, const char  *special, qid_t id, void  *addr)
EcallStatus RevProc::ECALL_quotactl(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: quotactl called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 61, rev_getdents64(unsigned int fd, struct linux_dirent64  *dirent, unsigned int count)
EcallStatus RevProc::ECALL_getdents64(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: getdents64 called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 62, rev_llseek(unsigned int fd, unsigned long offset_high, unsigned long offset_low, loff_t  *result, unsigned int whence)
EcallStatus RevProc::ECALL_lseek(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: lseek called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 63, rev_read(unsigned int fd
EcallStatus RevProc::ECALL_read(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: read called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  auto fd = RegFile->GetX<int>(RevReg::a0);
  auto BufAddr = RegFile->GetX<uint64_t>(RevReg::a1);
  auto BufSize = RegFile->GetX<uint64_t>(RevReg::a2);

  // Check if Current Ctx has access to the fd
  auto& ActiveThread = GetThreadOnHart(HartToExec);

  if( !ActiveThread->FindFD(fd) ){
    output->fatal(CALL_INFO, -1,
                  "Core %" PRIu32 "; Hart %" PRIu32 "; Thread %" PRIu32
                  " tried to read from file descriptor: %" PRIi32
                  ", but did not have access to it\n",
                  id, HartToExec, GetActiveThreadID(), fd);
    return EcallStatus::SUCCESS;
  }

  // This buffer is an intermediate buffer for storing the data read from host
  // for later use in writing to RevMem
  std::vector<char> TmpBuf(BufSize);

  // Do the read on the host
  int rc = read(fd, &TmpBuf[0], BufSize);

  // Write that data to the buffer inside of Rev
  mem->WriteMem(HartToExec, BufAddr, BufSize, &TmpBuf[0]);

  RegFile->SetX(RevReg::a0, rc);
  return EcallStatus::SUCCESS;
}


EcallStatus RevProc::ECALL_write(RevInst& inst){
  auto& EcallState = Harts.at(HartToExec)->GetEcallState();
  if( EcallState.bytesRead == 0 ){
    output->verbose(CALL_INFO, 2, 0,
                    "ECALL: write called by thread %" PRIu32
                    " on hart %" PRIu32 "\n",  GetActiveThreadID(), HartToExec);
  }
  auto fd = RegFile->GetX<int>(RevReg::a0);
  auto addr = RegFile->GetX<uint64_t>(RevReg::a1);
  auto nbytes = RegFile->GetX<uint64_t>(RevReg::a2);

  auto lsq_hash = make_lsq_hash(10, RevRegClass::RegGPR, HartToExec); // Cached hash value

  if(EcallState.bytesRead && LSQueue->count(lsq_hash) == 0){
    EcallState.string += std::string_view(EcallState.buf.data(), EcallState.bytesRead);
    EcallState.bytesRead = 0;
  }

  auto nleft = nbytes - EcallState.string.size();
  if(nleft == 0 && LSQueue->count(lsq_hash) == 0){
    int rc = write(fd, EcallState.string.data(), EcallState.string.size());
    RegFile->SetX(RevReg::a0, rc);
    EcallState.clear();
    DependencyClear(HartToExec, 10, false);
    return EcallStatus::SUCCESS;
  }

  if (LSQueue->count(lsq_hash) == 0) {
    MemReq req (addr + EcallState.string.size(), 10, RevRegClass::RegGPR,
                HartToExec, MemOp::MemOpREAD, true, RegFile->GetMarkLoadComplete());
    LSQueue->insert({lsq_hash, req});

    if(nleft >= 8){
      mem->ReadVal(HartToExec, addr+EcallState.string.size(),
                   reinterpret_cast<uint64_t*>(EcallState.buf.data()),
                   req, REVMEM_FLAGS(0));
      EcallState.bytesRead = 8;
    } else if(nleft >= 4){
      mem->ReadVal(HartToExec, addr+EcallState.string.size(),
                   reinterpret_cast<uint32_t*>(EcallState.buf.data()),
                   req, REVMEM_FLAGS(0));
      EcallState.bytesRead = 4;
    } else if(nleft >= 2){
      mem->ReadVal(HartToExec, addr+EcallState.string.size(),
                   reinterpret_cast<uint16_t*>(EcallState.buf.data()),
                   req, REVMEM_FLAGS(0));
      EcallState.bytesRead = 2;
    } else{
      mem->ReadVal(HartToExec, addr+EcallState.string.size(),
                   reinterpret_cast<uint8_t*>(EcallState.buf.data()),
                   req, REVMEM_FLAGS(0));
      EcallState.bytesRead = 1;
    }

    DependencySet(HartToExec, 10, false);
    return EcallStatus::CONTINUE;
  }

  return EcallStatus::CONTINUE;
}
// 65, rev_readv(unsigned long fd, const struct iovec  *vec, unsigned long vlen)
EcallStatus RevProc::ECALL_readv(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: readv called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 66, rev_writev(unsigned long fd, const struct iovec  *vec, unsigned long vlen)
EcallStatus RevProc::ECALL_writev(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: writev called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 67, rev_pread64(unsigned int fd, char  *buf, size_t count, loff_t pos)
EcallStatus RevProc::ECALL_pread64(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: pread64 called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 68, rev_pwrite64(unsigned int fd, const char  *buf, size_t count, loff_t pos)
EcallStatus RevProc::ECALL_pwrite64(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: pwrite64 called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 69, rev_preadv(unsigned long fd, const struct iovec  *vec, unsigned long vlen, unsigned long pos_l, unsigned long pos_h)
EcallStatus RevProc::ECALL_preadv(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: preadv called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 70, rev_pwritev(unsigned long fd, const struct iovec  *vec, unsigned long vlen, unsigned long pos_l, unsigned long pos_h)
EcallStatus RevProc::ECALL_pwritev(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: pwritev called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 71, rev_sendfile64(int out_fd, int in_fd, loff_t  *offset, size_t count)
EcallStatus RevProc::ECALL_sendfile64(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: sendfile64 called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 72, rev_pselect6_time32(int, fd_set  *, fd_set  *, fd_set  *, struct old_timespec32  *, void  *)
EcallStatus RevProc::ECALL_pselect6_time32(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: pselect6_time32 called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 73, rev_ppoll_time32(struct pollfd  *, unsigned int, struct old_timespec32  *, const sigset_t  *, size_t)
EcallStatus RevProc::ECALL_ppoll_time32(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: ppoll_time32 called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 74, rev_signalfd4(int ufd, sigset_t  *user_mask, size_t sizemask, int flags)
EcallStatus RevProc::ECALL_signalfd4(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: signalfd4 called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 75, rev_vmsplice(int fd, const struct iovec  *iov, unsigned long nr_segs, unsigned int flags)
EcallStatus RevProc::ECALL_vmsplice(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: vmsplice called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 76, rev_vmsplice(int fd, const struct iovec  *iov, unsigned long nr_segs, unsigned int flags)
EcallStatus RevProc::ECALL_splice(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: splice called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 77, rev_tee(int fdin, int fdout, size_t len, unsigned int flags)
EcallStatus RevProc::ECALL_tee(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: tee called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 78, rev_readlinkat(int dfd, const char  *path, char  *buf, int bufsiz)
EcallStatus RevProc::ECALL_readlinkat(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: readlinkat called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 79, rev_newfstatat(int dfd, const char  *filename, struct stat  *statbuf, int flag)
EcallStatus RevProc::ECALL_newfstatat(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: newfstatat called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 80, rev_newfstat(unsigned int fd, struct stat  *statbuf)
EcallStatus RevProc::ECALL_newfstat(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: newfstat called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 81, rev_sync(void)
EcallStatus RevProc::ECALL_sync(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: sync called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 82, rev_fsync(unsigned int fd)
EcallStatus RevProc::ECALL_fsync(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: fsync called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 83, rev_fdatasync(unsigned int fd)
EcallStatus RevProc::ECALL_fdatasync(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: fdatasync called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 84, rev_sync_file_range2(int fd, unsigned int flags, loff_t offset, loff_t nbytes)
EcallStatus RevProc::ECALL_sync_file_range2(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: sync_file_range2 called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 84, rev_sync_file_range(int fd, loff_t offset, loff_t nbytes, unsigned int flags)
EcallStatus RevProc::ECALL_sync_file_range(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: sync_file_range called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 85, rev_timerfd_create(int clockid, int flags)
EcallStatus RevProc::ECALL_timerfd_create(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: timerfd_create called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 86, rev_timerfd_settime(int ufd, int flags, const struct __kernel_itimerspec  *utmr, struct __kernel_itimerspec  *otmr)
EcallStatus RevProc::ECALL_timerfd_settime(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: timerfd_settime called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 87, rev_timerfd_gettime(int ufd, struct __kernel_itimerspec  *otmr)
EcallStatus RevProc::ECALL_timerfd_gettime(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: timerfd_gettime called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 88, rev_utimensat(int dfd, const char  *filename, struct __kernel_timespec  *utimes, int flags)
EcallStatus RevProc::ECALL_utimensat(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: utimensat called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 89, rev_acct(const char  *name)
EcallStatus RevProc::ECALL_acct(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: acct called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 90, rev_capget(cap_user_header_t header, cap_user_data_t dataptr)
EcallStatus RevProc::ECALL_capget(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: capget called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 91, rev_capset(cap_user_header_t header, const cap_user_data_t data)
EcallStatus RevProc::ECALL_capset(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: capset called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 92, rev_personality(unsigned int personality)
EcallStatus RevProc::ECALL_personality(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: personality called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 93, rev_exit(int error_code)
EcallStatus RevProc::ECALL_exit(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: exit called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  auto& ActiveThread = GetThreadOnHart(HartToExec);
  auto status = RegFile->GetX<uint64_t>(RevReg::a0);

  output->verbose(CALL_INFO, 0, 0,
                  "thread %" PRIu32 " on hart %" PRIu32 "exiting with"
                  " status %" PRIu64 "\n",
                  ActiveThread->GetThreadID(), HartToExec, status );
  exit(status);
  return EcallStatus::SUCCESS;
}


// 94, rev_exit_group(int error_code)
EcallStatus RevProc::ECALL_exit_group(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: exit_group called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 95, rev_waitid(int which, pid_t pid, struct siginfo  *infop, int options, struct rusage  *ru)
EcallStatus RevProc::ECALL_waitid(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: waitid called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 96, rev_set_tid_address(int  *tidptr)
EcallStatus RevProc::ECALL_set_tid_address(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: set_tid_address called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 97, rev_unshare(unsigned long unshare_flags)
EcallStatus RevProc::ECALL_unshare(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: unshare called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 98, rev_futex(u32  *uaddr, int op, u32 val, struct __kernel_timespec  *utime, u32  *uaddr2, u32 val3)
EcallStatus RevProc::ECALL_futex(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: futex called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 99, rev_set_robust_list(struct robust_list_head  *head, size_t len)
EcallStatus RevProc::ECALL_set_robust_list(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: set_robust_list called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 100, rev_get_robust_list(int pid, struct robust_list_head  *  *head_ptr, size_t  *len_ptr)
EcallStatus RevProc::ECALL_get_robust_list(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: get_robust_list called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 101, rev_nanosleep(struct __kernel_timespec  *rqtp, struct __kernel_timespec  *rmtp)
EcallStatus RevProc::ECALL_nanosleep(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: nanosleep called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 102, rev_getitimer(int which, struct __kernel_old_itimerval  *value)
EcallStatus RevProc::ECALL_getitimer(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: getitimer called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 103, rev_setitimer(int which, struct __kernel_old_itimerval  *value, struct __kernel_old_itimerval  *ovalue)
EcallStatus RevProc::ECALL_setitimer(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: setitimer called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 104, rev_kexec_load(unsigned long entry, unsigned long nr_segments, struct kexec_segment  *segments, unsigned long flags)
EcallStatus RevProc::ECALL_kexec_load(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: kexec_load called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 105, rev_init_module(void  *umod, unsigned long len, const char  *uargs)
EcallStatus RevProc::ECALL_init_module(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: init_module called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 106, rev_delete_module(const char  *name_user, unsigned int flags)
EcallStatus RevProc::ECALL_delete_module(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: delete_module called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 107, rev_timer_create(clockid_t which_clock, struct sigevent  *timer_event_spec, timer_t  * created_timer_id)
EcallStatus RevProc::ECALL_timer_create(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: timer_create called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 108, rev_timer_gettime(timer_t timer_id, struct __kernel_itimerspec  *setting)
EcallStatus RevProc::ECALL_timer_gettime(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: timer_gettime called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 109, rev_timer_getoverrun(timer_t timer_id)
EcallStatus RevProc::ECALL_timer_getoverrun(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: timer_getoverrun called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 110, rev_timer_settime(timer_t timer_id, int flags, const struct __kernel_itimerspec  *new_setting, struct __kernel_itimerspec  *old_setting)
EcallStatus RevProc::ECALL_timer_settime(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: timer_settime called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 111, rev_timer_delete(timer_t timer_id)
EcallStatus RevProc::ECALL_timer_delete(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: timer_delete called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 112, rev_clock_settime(clockid_t which_clock, const struct __kernel_timespec  *tp)
EcallStatus RevProc::ECALL_clock_settime(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: clock_settime called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 113, rev_clock_gettime(clockid_t which_clock, struct __kernel_timespec  *tp)
EcallStatus RevProc::ECALL_clock_gettime(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: clock_gettime called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 114, rev_clock_getres(clockid_t which_clock, struct __kernel_timespec  *tp)
EcallStatus RevProc::ECALL_clock_getres(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: clock_getres called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 115, rev_clock_nanosleep(clockid_t which_clock, int flags, const struct __kernel_timespec  *rqtp, struct __kernel_timespec  *rmtp)
EcallStatus RevProc::ECALL_clock_nanosleep(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: clock_nanosleep called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 116, rev_syslog(int type, char  *buf, int len)
EcallStatus RevProc::ECALL_syslog(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: syslog called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 117, rev_ptrace(long request, long pid, unsigned long addr, unsigned long data)
EcallStatus RevProc::ECALL_ptrace(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: ptrace called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 118, rev_sched_setparam(pid_t pid, struct sched_param  *param)
EcallStatus RevProc::ECALL_sched_setparam(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: sched_setparam called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 119, rev_sched_setscheduler(pid_t pid, int policy, struct sched_param  *param)
EcallStatus RevProc::ECALL_sched_setscheduler(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: sched_setscheduler called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 120, rev_sched_getscheduler(pid_t pid)
EcallStatus RevProc::ECALL_sched_getscheduler(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: sched_getscheduler called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 121, rev_sched_getparam(pid_t pid, struct sched_param  *param)
EcallStatus RevProc::ECALL_sched_getparam(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: sched_getparam called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 122, rev_sched_setaffinity(pid_t pid, unsigned int len, unsigned long  *user_mask_ptr)
EcallStatus RevProc::ECALL_sched_setaffinity(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: sched_setaffinity called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 123, rev_sched_getaffinity(pid_t pid, unsigned int len, unsigned long  *user_mask_ptr)
EcallStatus RevProc::ECALL_sched_getaffinity(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: sched_getaffinity called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 124, rev_sched_yield(void)
EcallStatus RevProc::ECALL_sched_yield(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: sched_yield called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 125, rev_sched_get_priority_max(int policy)
EcallStatus RevProc::ECALL_sched_get_priority_max(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: sched_get_priority_max called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 126, rev_sched_get_priority_min(int policy)
EcallStatus RevProc::ECALL_sched_get_priority_min(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: sched_get_priority_min called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 127, rev_sched_rr_get_interval(pid_t pid, struct __kernel_timespec  *interval)
EcallStatus RevProc::ECALL_sched_rr_get_interval(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: sched_rr_get_interval called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 128, rev_restart_syscall(void)
EcallStatus RevProc::ECALL_restart_syscall(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: restart_syscall called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 129, rev_kill(pid_t pid, int sig)
EcallStatus RevProc::ECALL_kill(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: kill called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 130, rev_tkill(pid_t pid, int sig)
EcallStatus RevProc::ECALL_tkill(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: tkill called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 131, rev_tgkill(pid_t tgid, pid_t pid, int sig)
EcallStatus RevProc::ECALL_tgkill(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: tgkill called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 132, rev_sigaltstack(const struct sigaltstack  *uss, struct sigaltstack  *uoss)
EcallStatus RevProc::ECALL_sigaltstack(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: sigaltstack called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 133, rev_rt_sigsuspend(sigset_t  *unewset, size_t sigsetsize)
EcallStatus RevProc::ECALL_rt_sigsuspend(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: rt_sigsuspend called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 134, rev_rt_sigaction(int, const struct sigaction  *, struct sigaction  *, size_t)
EcallStatus RevProc::ECALL_rt_sigaction(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: rt_sigaction called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 135, rev_rt_sigprocmask(int how, sigset_t  *set, sigset_t  *oset, size_t sigsetsize)
EcallStatus RevProc::ECALL_rt_sigprocmask(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: rt_sigprocmask called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 136, rev_rt_sigpending(sigset_t  *set, size_t sigsetsize)
EcallStatus RevProc::ECALL_rt_sigpending(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: rt_sigpending called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 137, rev_rt_sigtimedwait_time32(const sigset_t  *uthese, siginfo_t  *uinfo, const struct old_timespec32  *uts, size_t sigsetsize)
EcallStatus RevProc::ECALL_rt_sigtimedwait_time32(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: rt_sigtimedwait_time32 called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 138, rev_rt_sigqueueinfo(pid_t pid, int sig, siginfo_t  *uinfo)
EcallStatus RevProc::ECALL_rt_sigqueueinfo(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: rt_sigqueueinfo called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 140, rev_setpriority(int which, int who, int niceval)
EcallStatus RevProc::ECALL_setpriority(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: setpriority called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 141, rev_getpriority(int which, int who)
EcallStatus RevProc::ECALL_getpriority(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: getpriority called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 142, rev_reboot(int magic1, int magic2, unsigned int cmd, void  *arg)
EcallStatus RevProc::ECALL_reboot(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: reboot called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 143, rev_setregid(gid_t rgid, gid_t egid)
EcallStatus RevProc::ECALL_setregid(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: setregid called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 144, rev_setgid(gid_t gid)
EcallStatus RevProc::ECALL_setgid(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: setgid called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 145, rev_setreuid(uid_t ruid, uid_t euid)
EcallStatus RevProc::ECALL_setreuid(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: setreuid called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 146, rev_setuid(uid_t uid)
EcallStatus RevProc::ECALL_setuid(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: setuid called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 147, rev_setresuid(uid_t ruid, uid_t euid, uid_t suid)
EcallStatus RevProc::ECALL_setresuid(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: setresuid called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 148, rev_getresuid(uid_t  *ruid, uid_t  *euid, uid_t  *suid)
EcallStatus RevProc::ECALL_getresuid(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: getresuid called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 149, rev_setresgid(gid_t rgid, gid_t egid, gid_t sgid)
EcallStatus RevProc::ECALL_setresgid(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: setresgid called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 150, rev_getresgid(gid_t  *rgid, gid_t  *egid, gid_t  *sgid)
EcallStatus RevProc::ECALL_getresgid(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: getresgid called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 151, rev_setfsuid(uid_t uid)
EcallStatus RevProc::ECALL_setfsuid(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: setfsuid called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 152, rev_setfsgid(gid_t gid)
EcallStatus RevProc::ECALL_setfsgid(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: setfsgid called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 153, rev_times(struct tms  *tbuf)
EcallStatus RevProc::ECALL_times(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: times called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 154, rev_setpgid(pid_t pid, pid_t pgid)
EcallStatus RevProc::ECALL_setpgid(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: setpgid called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 155, rev_getpgid(pid_t pid)
EcallStatus RevProc::ECALL_getpgid(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: getpgid called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 156, rev_getsid(pid_t pid)
EcallStatus RevProc::ECALL_getsid(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: getsid called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 157, rev_setsid(void)
EcallStatus RevProc::ECALL_setsid(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: setsid called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 158, rev_getgroups(int gidsetsize, gid_t  *grouplist)
EcallStatus RevProc::ECALL_getgroups(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: getgroups called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 159, rev_setgroups(int gidsetsize, gid_t  *grouplist)
EcallStatus RevProc::ECALL_setgroups(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: setgroups called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 160, rev_newuname(struct new_utsname  *name)
EcallStatus RevProc::ECALL_newuname(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: newuname called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 161, rev_sethostname(char  *name, int len)
EcallStatus RevProc::ECALL_sethostname(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: sethostname called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 162, rev_setdomainname(char  *name, int len)
EcallStatus RevProc::ECALL_setdomainname(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: setdomainname called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 163, rev_getrlimit(unsigned int resource, struct rlimit  *rlim)
EcallStatus RevProc::ECALL_getrlimit(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: getrlimit called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 164, rev_setrlimit(unsigned int resource, struct rlimit  *rlim)
EcallStatus RevProc::ECALL_setrlimit(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: setrlimit called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 165, rev_getrusage(int who, struct rusage  *ru)
EcallStatus RevProc::ECALL_getrusage(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: getrusage called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 166, rev_umask(int mask)
EcallStatus RevProc::ECALL_umask(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: umask called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 167, rev_prctl(int option, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5)
EcallStatus RevProc::ECALL_prctl(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: prctl called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 168, rev_getcpu(unsigned  *cpu, unsigned  *node, struct getcpu_cache  *cache)
EcallStatus RevProc::ECALL_getcpu(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: getcpu called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 169, rev_gettimeofday(struct __kernel_old_timeval  *tv, struct timezone  *tz)
EcallStatus RevProc::ECALL_gettimeofday(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: gettimeofday called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 170, rev_settimeofday(struct __kernel_old_timeval  *tv, struct timezone  *tz)
EcallStatus RevProc::ECALL_settimeofday(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: settimeofday called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 171, rev_adjtimex(struct __kernel_timex  *txc_p)
EcallStatus RevProc::ECALL_adjtimex(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: adjtimex called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 172, rev_getpid(void)
EcallStatus RevProc::ECALL_getpid(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: getpid called (Rev only supports a single process)\n");
  return EcallStatus::SUCCESS;
}

//  173, rev_getppid(void)
EcallStatus RevProc::ECALL_getppid(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,

                  "ECALL: getppid called (Rev only supports a single process)\n");
  return EcallStatus::SUCCESS;
}

// 174, rev_getuid(void)
EcallStatus RevProc::ECALL_getuid(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,

                  "ECALL: getuid called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 175, rev_geteuid(void)
EcallStatus RevProc::ECALL_geteuid(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: geteuid called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 176, rev_getgid(void)
EcallStatus RevProc::ECALL_getgid(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: getgid called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 177, rev_getegid(void)
EcallStatus RevProc::ECALL_getegid(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: getegid called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 178, rev_gettid(void)
EcallStatus RevProc::ECALL_gettid(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: gettid called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);

  /* rc = Currently Executing Hart */
  RegFile->SetX(RevReg::a0, GetThreadOnHart(HartToExec)->GetThreadID());
  return EcallStatus::SUCCESS;
}

// 179, rev_sysinfo(struct sysinfo  *info)
EcallStatus RevProc::ECALL_sysinfo(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: sysinfo called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 180, rev_mq_open(const char  *name, int oflag, umode_t mode, struct mq_attr  *attr)
EcallStatus RevProc::ECALL_mq_open(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: mq_open called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 181, rev_mq_unlink(const char  *name)
EcallStatus RevProc::ECALL_mq_unlink(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: mq_unlink called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 182, rev_mq_timedsend(mqd_t mqdes, const char  *msg_ptr, size_t msg_len, unsigned int msg_prio, const struct __kernel_timespec  *abs_timeout)
EcallStatus RevProc::ECALL_mq_timedsend(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: mq_timedsend called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 183, rev_mq_timedreceive(mqd_t mqdes, char  *msg_ptr, size_t msg_len, unsigned int  *msg_prio, const struct __kernel_timespec  *abs_timeout)
EcallStatus RevProc::ECALL_mq_timedreceive(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: mq_timedreceive called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 184, rev_mq_notify(mqd_t mqdes, const struct sigevent  *notification)
EcallStatus RevProc::ECALL_mq_notify(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: mq_notify called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 185, rev_mq_getsetattr(mqd_t mqdes, const struct mq_attr  *mqstat, struct mq_attr  *omqstat)
EcallStatus RevProc::ECALL_mq_getsetattr(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: mq_getsetattr called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 186, rev_msgget(key_t key, int msgflg)
EcallStatus RevProc::ECALL_msgget(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: msgget called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 187, rev_old_msgctl(int msqid, int cmd, struct msqid_ds  *buf)
EcallStatus RevProc::ECALL_msgctl(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: msgctl called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 188, rev_msgrcv(int msqid, struct msgbuf  *msgp, size_t msgsz, long msgtyp, int msgflg)
EcallStatus RevProc::ECALL_msgrcv(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: msgrcv called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 189, rev_msgsnd(int msqid, struct msgbuf  *msgp, size_t msgsz, int msgflg)
EcallStatus RevProc::ECALL_msgsnd(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: msgsnd called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 190, rev_semget(key_t key, int nsems, int semflg)
EcallStatus RevProc::ECALL_semget(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: semget called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 191, rev_semctl(int semid, int semnum, int cmd, unsigned long arg)
EcallStatus RevProc::ECALL_semctl(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: semctl called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 192, rev_semtimedop(int semid, struct sembuf  *sops, unsigned nsops, const struct __kernel_timespec  *timeout)
EcallStatus RevProc::ECALL_semtimedop(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: semtimedop called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 193, rev_semop(int semid, struct sembuf  *sops, unsigned nsops)
EcallStatus RevProc::ECALL_semop(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: semop called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 194, rev_shmget(key_t key, size_t size, int flag)
EcallStatus RevProc::ECALL_shmget(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: shmget called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 195, rev_old_shmctl(int shmid, int cmd, struct shmid_ds  *buf)
EcallStatus RevProc::ECALL_shmctl(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: shmctl called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 196, rev_shmat(int shmid, char  *shmaddr, int shmflg)
EcallStatus RevProc::ECALL_shmat(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: shmat called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 197, rev_shmdt(char  *shmaddr)
EcallStatus RevProc::ECALL_shmdt(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: shmdt called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 198, rev_socket(int, int, int)
EcallStatus RevProc::ECALL_socket(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: socket called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 199, rev_socketpair(int, int, int, int  *)
EcallStatus RevProc::ECALL_socketpair(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: socketpair called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 200, rev_bind(int, struct sockaddr  *, int)
EcallStatus RevProc::ECALL_bind(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: bind called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 201, rev_listen(int, int)
EcallStatus RevProc::ECALL_listen(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: listen called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 202, rev_accept(int, struct sockaddr  *, int  *)
EcallStatus RevProc::ECALL_accept(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: accept called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 203, rev_connect(int, struct sockaddr  *, int)
EcallStatus RevProc::ECALL_connect(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: connect called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 204, rev_getsockname(int, struct sockaddr  *, int  *)
EcallStatus RevProc::ECALL_getsockname(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: getsockname called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 205, rev_getpeername(int, struct sockaddr  *, int  *)
EcallStatus RevProc::ECALL_getpeername(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: getpeername called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 206, rev_sendto(int, void  *, size_t, unsigned, struct sockaddr  *, int)
EcallStatus RevProc::ECALL_sendto(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: sendto called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 207, rev_recvfrom(int, void  *, size_t, unsigned, struct sockaddr  *, int  *)
EcallStatus RevProc::ECALL_recvfrom(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: recvfrom called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 208, rev_setsockopt(int fd, int level, int optname, char  *optval, int optlen)
EcallStatus RevProc::ECALL_setsockopt(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: setsockopt called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 209, rev_getsockopt(int fd, int level, int optname, char  *optval, int  *optlen)
EcallStatus RevProc::ECALL_getsockopt(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: getsockopt called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 210, rev_shutdown(int, int)
EcallStatus RevProc::ECALL_shutdown(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: shutdown called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 211, rev_sendmsg(int fd, struct user_msghdr  *msg, unsigned flags)
EcallStatus RevProc::ECALL_sendmsg(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: sendmsg called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 212, rev_recvmsg(int fd, struct user_msghdr  *msg, unsigned flags)
EcallStatus RevProc::ECALL_recvmsg(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: recvmsg called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 213, rev_readahead(int fd, loff_t offset, size_t count)
EcallStatus RevProc::ECALL_readahead(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: readahead called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 214, rev_brk(unsigned long brk)
EcallStatus RevProc::ECALL_brk(RevInst& inst){
  auto Addr = RegFile->GetX<uint64_t>(RevReg::a0);

  const uint64_t heapend = mem->GetHeapEnd();
  if( Addr > 0 && Addr > heapend ){
    uint64_t Size = Addr - heapend;
    mem->ExpandHeap(Size);
  } else {
    output->fatal(CALL_INFO, 11,
                  "Out of memory / Unable to expand system break (brk) to "
                  "Addr = 0x%" PRIx64 "\n", Addr);
  }
  return EcallStatus::SUCCESS;
}

// 215, rev_munmap(unsigned long addr, size_t len)
EcallStatus RevProc::ECALL_munmap(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: munmap called\n");
  auto Addr = RegFile->GetX<uint64_t>(RevReg::a0);
  auto Size = RegFile->GetX<uint64_t>(RevReg::a1);

  int rc =  mem->DeallocMem(Addr, Size) == uint64_t(-1);
  if(rc == -1){
    output->fatal(CALL_INFO, 11,
                  "Failed to perform munmap(Addr = 0x%" PRIx64 ", Size = %" PRIu64 ")"
                  "likely because the memory was not allocated to begin with" ,
                  Addr, Size);
  }

  RegFile->SetX(RevReg::a0, rc);
  return EcallStatus::SUCCESS;
}

// 216, rev_mremap(unsigned long addr, unsigned long old_len, unsigned long new_len, unsigned long flags, unsigned long new_addr)
EcallStatus RevProc::ECALL_mremap(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: mremap called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 217, rev_add_key(const char  *_type, const char  *_description, const void  *_payload, size_t plen, key_serial_t destringid)
EcallStatus RevProc::ECALL_add_key(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: add_key called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 218, rev_request_key(const char  *_type, const char  *_description, const char  *_callout_info, key_serial_t destringid)
EcallStatus RevProc::ECALL_request_key(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: request_key called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 219, rev_keyctl(int cmd, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5)
EcallStatus RevProc::ECALL_keyctl(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: keyctl called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 220, rev_clone(unsigned long, unsigned long, int  *, unsigned long, int  *)
EcallStatus RevProc::ECALL_clone(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: clone called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  auto rtval = EcallStatus::SUCCESS;
 //  auto CloneArgsAddr = RegFile->GetX<uint64_t>(RevReg::a0);
 //  // auto SizeOfCloneArgs = RegFile()->GetX<size_t>(RevReg::a1);

 // if(0 == ECALL.bytesRead){
 //    // First time through the function...
 //    /* Fetch the clone_args */
 //    // struct clone_args args;  // So while clone_args is a whole struct, we appear to be only
 //                                // using the 1st uint64, so that's all we're going to fetch
 //   uint64_t* args = reinterpret_cast<uint64_t*>(ECALL.buf.data());
 //   mem->ReadVal<uint64_t>(HartToExec, CloneArgsAddr, args, inst.hazard, REVMEM_FLAGS(0x00));
 //   ECALL.bytesRead = sizeof(*args);
 //   rtval = EcallStatus::CONTINUE;
 // }else{
 //    /*
 //    * Parse clone flags
 //    * NOTE: if no flags are set, we get fork() like behavior
 //    */
 //   uint64_t* args = reinterpret_cast<uint64_t*>(ECALL.buf.data());
 //    for( uint64_t bit=1; bit != 0; bit <<= 1 ){
 //      switch (*args & bit) {
 //        case CLONE_VM:
 //          // std::cout << "CLONE_VM is true" << std::endl;
 //          break;
 //        case CLONE_FS: /* Set if fs info shared between processes */
 //          // std::cout << "CLONE_FS is true" << std::endl;
 //          break;
 //        case CLONE_FILES: /* Set if open files shared between processes */
 //          // std::cout << "CLONE_FILES is true" << std::endl;
 //          break;
 //        case CLONE_SIGHAND: /* Set if signal handlers shared */
 //          // std::cout << "CLONE_SIGHAND is true" << std::endl;
 //          break;
 //        case CLONE_PIDFD: /* Set if a pidfd should be placed in the parent */
 //          // std::cout << "CLONE_PIDFD is true" << std::endl;
 //          break;
 //        case CLONE_PTRACE: /* Set if tracing continues on the child */
 //          // std::cout << "CLONE_PTRACE is true" << std::endl;
 //          break;
 //        case CLONE_VFORK: /* Set if the parent wants the child to wake it up on mm_release */
 //          // std::cout << "CLONE_VFORK is true" << std::endl;
 //          break;
 //        case CLONE_PARENT: /* Set if we want to have the same parent as the cloner */
 //          // std::cout << "CLONE_PARENT is true" << std::endl;
 //          break;
 //        case CLONE_THREAD: /* Set to add to same thread group */
 //          // std::cout << "CLONE_THREAD is true" << std::endl;
 //          break;
 //        case CLONE_NEWNS: /* Set to create new namespace */
 //          // std::cout << "CLONE_NEWNS is true" << std::endl;
 //          break;
 //        case CLONE_SYSVSEM: /* Set to shared SVID SEM_UNDO semantics */
 //          // std::cout << "CLONE_SYSVSEM is true" << std::endl;
 //          break;
 //        case CLONE_SETTLS: /* Set TLS info */
 //          // std::cout << "CLONE_SETTLS is true" << std::endl;
 //          break;
 //        case CLONE_PARENT_SETTID: /* Store TID in userlevel buffer before MM copy */
 //          // std::cout << "CLONE_PARENT_SETTID is true" << std::endl;
 //          break;
 //        case CLONE_CHILD_CLEARTID: /* Register exit futex and memory location to clear */
 //          // std::cout << "CLONE_CHILD_CLEARTID is true" << std::endl;
 //          break;
 //        case CLONE_DETACHED: /* Create clone detached */
 //          // std::cout << "CLONE_DETACHED is true" << std::endl;
 //          break;
 //        case CLONE_UNTRACED: /* Set if the tracing process can't force CLONE_PTRACE on this clone */
 //          // std::cout << "CLONE_UNTRACED is true" << std::endl;
 //          break;
 //        case CLONE_CHILD_SETTID: /* New cgroup namespace */
 //          // std::cout << "CLONE_CHILD_SETTID is true" << std::endl;
 //          break;
 //        case CLONE_NEWCGROUP: /* New cgroup namespace */
 //          // std::cout << "CLONE_NEWCGROUP is true" << std::endl;
 //          break;
 //        case CLONE_NEWUTS: /* New utsname group */
 //          // std::cout << "CLONE_NEWUTS is true" << std::endl;
 //          break;
 //        case CLONE_NEWIPC: /* New ipcs */
 //          // std::cout << "CLONE_NEWIPC is true" << std::endl;
 //          break;
 //        case CLONE_NEWUSER: /* New user namespace */
 //          // std::cout << "CLONE_NEWUSER is true" << std::endl;
 //          break;
 //        case CLONE_NEWPID: /* New pid namespace */
 //          // std::cout << "CLONE_NEWPID is true" << std::endl;
 //          break;
 //        case CLONE_NEWNET: /* New network namespace */
 //          // std::cout << "CLONE_NEWNET is true" << std::endl;
 //          break;
 //        case CLONE_IO: /* Clone I/O Context */
 //          // std::cout << "CLONE_IO is true" << std::endl;
 //          break;
 //        default:
 //          break;
 //      } // switch
 //    } // for

 //    /* Get the parent ctx (Current active, executing PID) */
 //    std::shared_ptr<RevThread> ParentCtx = ThreadTable.at(ActivePIDs.at(HartToExec));

 //    /* Create the child ctx */
 //    uint32_t ChildPID = CreateChildCtx();
 //    std::shared_ptr<RevThread> ChildCtx = ThreadTable.at(ChildPID);

 //    /*
 //    * ===========================================================================================
 //    * Register File
 //    * ===========================================================================================
 //    * We need to duplicate the parent's RegFile to to the Childs
 //    * - NOTE: when we return from this function, the return value will
 //    *         be automatically stored in the Proc.RegFile[HartToExec]'s a0
 //    *         register. In a traditional fork code this looks like:
 //    *
 //    *         pid_t pid = fork()
 //    *         if pid < 0: // Error
 //    *         else if pid = 0: // New Child Process
 //    *         else: // Parent Process
 //    *
 //    *         In this case, the value of pid is the value thats returned to a0
 //    *         It follows that
 //    *         - The child's regfile MUST have 0 in its a0 (despite its pid != 0 to the RevProc)
 //    *         - The Parent's a0 register MUST have its PID in it
 //    * ===========================================================================================
 //    */

 //    /*
 //    Alert the Proc there needs to be a Ctx switch
 //    Pass the PID that will be switched to once the
 //    current pipeline is executed until completion
 //    */
 //    CtxSwitchAlert(ChildPID);

 //    // Parent's return value is the child's PID
 //    RegFile->SetX(RevReg::a0, ChildPID);

 //    // Child's return value is 0
 //    ChildCtx->GetRegFile()->SetX(RevReg::a0, 0);

 //    // clean up ecall state
 //    rtval = EcallStatus::SUCCESS;
 //    ECALL.bytesRead = 0;

 //  } //else
  return rtval;
}

// 221, rev_execve(const char  *filename, const char  *const  *argv, const char  *const  *envp)
EcallStatus RevProc::ECALL_execve(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: execve called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 222, rev_old_mmap(struct mmap_arg_struct  *arg)
EcallStatus RevProc::ECALL_mmap(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: mmap called\n");

  auto addr = RegFile->GetX<uint64_t>(RevReg::a0);
  auto size = RegFile->GetX<uint64_t>(RevReg::a1);
  // auto prot = RegFile->GetX<int>(RevReg::a2);
  // auto Flags = RegFile->GetX<int>(RevReg::a3);
  // auto fd = RegFile->GetX<int>(RevReg::a4);
  // auto offset = RegFile->GetX<off_t>(RevReg::a5);

  if( !addr ){
    // If address is NULL... We add it to MemSegs.end()->getTopAddr()+1
    addr = mem->AllocMem(size+1);
    // addr = mem->AddMemSeg(Size);
  } else {
    // We were passed an address... try to put a segment there.
    // Currently there is no handling of getting it 'close' to the
    // suggested address... instead if it can't allocate a new segment
    // there it fails.
    if( !mem->AllocMemAt(addr, size) ){
      output->fatal(CALL_INFO, 11,
                    "Failed to add mem segment\n");
    }
  }
  RegFile->SetX(RevReg::a0, addr);
  return EcallStatus::SUCCESS;
}

// 223, rev_fadvise64_64(int fd, loff_t offset, loff_t len, int advice)
EcallStatus RevProc::ECALL_fadvise64_64(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: fadvise64_64 called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 224, rev_swapon(const char  *specialfile, int swap_flags)
EcallStatus RevProc::ECALL_swapon(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: swapon called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 225, rev_swapoff(const char  *specialfile)
EcallStatus RevProc::ECALL_swapoff(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: swapoff called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 226, rev_mprotect(unsigned long start, size_t len, unsigned long prot)
EcallStatus RevProc::ECALL_mprotect(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: mprotect called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 227, rev_msync(unsigned long start, size_t len, int flags)
EcallStatus RevProc::ECALL_msync(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: msync called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 228, rev_mlock(unsigned long start, size_t len)
EcallStatus RevProc::ECALL_mlock(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: mlock called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 229, rev_munlock(unsigned long start, size_t len)
EcallStatus RevProc::ECALL_munlock(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: munlock called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 230, rev_mlockall(int flags)
EcallStatus RevProc::ECALL_mlockall(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: mlockall called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 231, rev_munlockall(void)
EcallStatus RevProc::ECALL_munlockall(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: munlockall called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 232, rev_mincore(unsigned long start, size_t len, unsigned char  * vec)
EcallStatus RevProc::ECALL_mincore(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: mincore called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 233, rev_madvise(unsigned long start, size_t len, int behavior)
EcallStatus RevProc::ECALL_madvise(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: madvise called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 234, rev_remap_file_pages(unsigned long start, unsigned long size, unsigned long prot, unsigned long pgoff, unsigned long flags)
EcallStatus RevProc::ECALL_remap_file_pages(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: remap_file_pages called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 235, rev_mbind(unsigned long start, unsigned long len, unsigned long mode, const unsigned long  *nmask, unsigned long maxnode, unsigned flags)
EcallStatus RevProc::ECALL_mbind(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: mbind called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 236, rev_get_mempolicy(int  *policy, unsigned long  *nmask, unsigned long maxnode, unsigned long addr, unsigned long flags)
EcallStatus RevProc::ECALL_get_mempolicy(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: get_mempolicy called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 237, rev_set_mempolicy(int mode, const unsigned long  *nmask, unsigned long maxnode)
EcallStatus RevProc::ECALL_set_mempolicy(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: set_mempolicy called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 238, rev_migrate_pages(pid_t pid, unsigned long maxnode, const unsigned long  *from, const unsigned long  *to)
EcallStatus RevProc::ECALL_migrate_pages(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: migrate_pages called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 239, rev_move_pages(pid_t pid, unsigned long nr_pages, const void  *  *pages, const int  *nodes, int  *status, int flags)
EcallStatus RevProc::ECALL_move_pages(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: move_pages called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 240, rev_rt_tgsigqueueinfo(pid_t tgid, pid_t pid, int sig, siginfo_t  *uinfo)
EcallStatus RevProc::ECALL_rt_tgsigqueueinfo(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: rt_tgsigqueueinfo called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 241, rev_perf_event_open(")
EcallStatus RevProc::ECALL_perf_event_open(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: perf_event_open called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 242, rev_accept4(int, struct sockaddr  *, int  *, int)
EcallStatus RevProc::ECALL_accept4(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: accept4 called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 243, rev_recvmmsg_time32(int fd, struct mmsghdr  *msg, unsigned int vlen, unsigned flags, struct old_timespec32  *timeout)
EcallStatus RevProc::ECALL_recvmmsg_time32(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: recvmmsg_time32 called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 260, rev_wait4(pid_t pid, int  *stat_addr, int options, struct rusage  *ru)
EcallStatus RevProc::ECALL_wait4(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: wait4 called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 261, rev_prlimit64(pid_t pid, unsigned int resource, const struct rlimit64  *new_rlim, struct rlimit64  *old_rlim)
EcallStatus RevProc::ECALL_prlimit64(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: prlimit64 called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 262, rev_fanotify_init(unsigned int flags, unsigned int event_f_flags)
EcallStatus RevProc::ECALL_fanotify_init(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: fanotify_init called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 263, rev_fanotify_mark(int fanotify_fd, unsigned int flags, u64 mask, int fd, const char  *pathname)
EcallStatus RevProc::ECALL_fanotify_mark(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: fanotify_mark called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 264, rev_name_to_handle_at(int dfd, const char  *name, struct file_handle  *handle, int  *mnt_id, int flag)
EcallStatus RevProc::ECALL_name_to_handle_at(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: name_to_handle_at called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 265, rev_open_by_handle_at(int mountdirfd, struct file_handle  *handle, int flags)
EcallStatus RevProc::ECALL_open_by_handle_at(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: open_by_handle_at called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 266, rev_clock_adjtime(clockid_t which_clock, struct __kernel_timex  *tx)
EcallStatus RevProc::ECALL_clock_adjtime(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: clock_adjtime called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 267, rev_syncfs(int fd)
EcallStatus RevProc::ECALL_syncfs(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: syncfs called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 268, rev_setns(int fd, int nstype)
EcallStatus RevProc::ECALL_setns(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: setns called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 269, rev_sendmmsg(int fd, struct mmsghdr  *msg, unsigned int vlen, unsigned flags)
EcallStatus RevProc::ECALL_sendmmsg(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: sendmmsg called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 270, rev_process_vm_readv(pid_t pid, const struct iovec  *lvec, unsigned long liovcnt, const struct iovec  *rvec, unsigned long riovcnt, unsigned long flags)
EcallStatus RevProc::ECALL_process_vm_readv(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: process_vm_readv called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 271, rev_process_vm_writev(pid_t pid, const struct iovec  *lvec, unsigned long liovcnt, const struct iovec  *rvec, unsigned long riovcnt, unsigned long flags)
EcallStatus RevProc::ECALL_process_vm_writev(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: process_vm_writev called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);

  return EcallStatus::SUCCESS;
}

// 272, rev_kcmp(pid_t pid1, pid_t pid2, int type, unsigned long idx1, unsigned long idx2)
EcallStatus RevProc::ECALL_kcmp(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: kcmp called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 273, rev_finit_module(int fd, const char  *uargs, int flags)
EcallStatus RevProc::ECALL_finit_module(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: finit_module called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 274, rev_sched_setattr(pid_t pid, struct sched_attr  *attr, unsigned int flags)
EcallStatus RevProc::ECALL_sched_setattr(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: sched_setattr called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 275, rev_sched_getattr(pid_t pid, struct sched_attr  *attr, unsigned int size, unsigned int flags)
EcallStatus RevProc::ECALL_sched_getattr(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: sched_getattr called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 276, rev_renameat2(int olddfd, const char  *oldname, int newdfd, const char  *newname, unsigned int flags)
EcallStatus RevProc::ECALL_renameat2(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: renameat2 called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 277, rev_seccomp(unsigned int op, unsigned int flags, void  *uargs)
EcallStatus RevProc::ECALL_seccomp(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: seccomp called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 278, rev_getrandom(char  *buf, size_t count, unsigned int flags)
EcallStatus RevProc::ECALL_getrandom(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: getrandom called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 279, rev_memfd_create(const char  *uname_ptr, unsigned int flags)
EcallStatus RevProc::ECALL_memfd_create(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: memfd_create called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 280, rev_bpf(int cmd, union bpf_attr *attr, unsigned int size)
EcallStatus RevProc::ECALL_bpf(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: bpf called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 281, rev_execveat(int dfd, const char  *filename, const char  *const  *argv, const char  *const  *envp, int flags)
EcallStatus RevProc::ECALL_execveat(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: execveat called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 282, rev_userfaultfd(int flags)
EcallStatus RevProc::ECALL_userfaultfd(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: userfaultfd called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 283, rev_membarrier(int cmd, unsigned int flags, int cpu_id)
EcallStatus RevProc::ECALL_membarrier(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: membarrier called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 284, rev_mlock2(unsigned long start, size_t len, int flags)
EcallStatus RevProc::ECALL_mlock2(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: mlock2 called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 285, rev_copy_file_range(int fd_in, loff_t  *off_in, int fd_out, loff_t  *off_out, size_t len, unsigned int flags)
EcallStatus RevProc::ECALL_copy_file_range(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: copy_file_range called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 286, rev_preadv2(unsigned long fd, const struct iovec  *vec, unsigned long vlen, unsigned long pos_l, unsigned long pos_h, rwf_t flags)
EcallStatus RevProc::ECALL_preadv2(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: preadv2 called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 287, rev_pwritev2(unsigned long fd, const struct iovec  *vec, unsigned long vlen, unsigned long pos_l, unsigned long pos_h, rwf_t flags)
EcallStatus RevProc::ECALL_pwritev2(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: pwritev2 called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 288, rev_pkey_mprotect(unsigned long start, size_t len, unsigned long prot, int pkey)
EcallStatus RevProc::ECALL_pkey_mprotect(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: pkey_mprotect called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 289, rev_pkey_alloc(unsigned long flags, unsigned long init_val)
EcallStatus RevProc::ECALL_pkey_alloc(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: pkey_alloc called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 290, rev_pkey_free(int pkey)
EcallStatus RevProc::ECALL_pkey_free(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: pkey_free called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 291, rev_statx(int dfd, const char  *path, unsigned flags, unsigned mask, struct statx  *buffer)
EcallStatus RevProc::ECALL_statx(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: statx called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 292, rev_io_pgetevents(aio_context_t ctx_id, long min_nr, long nr, struct io_event  *events, struct __kernel_timespec  *timeout, const struct __aio_sigset *sig)
EcallStatus RevProc::ECALL_io_pgetevents(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: io_pgetevents called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 293, rev_rseq(struct rseq  *rseq, uint32_t rseq_len, int flags, uint32_t sig)
EcallStatus RevProc::ECALL_rseq(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: rseq called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 294, rev_kexec_file_load(int kernel_fd, int initrd_fd, unsigned long cmdline_len, const char  *cmdline_ptr, unsigned long flags)
EcallStatus RevProc::ECALL_kexec_file_load(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: kexec_file_load called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// // 403, rev_clock_gettime(clockid_t which_clock, struct __kernel_timespec  *tp)
// EcallStatus RevProc::ECALL_clock_gettime(RevInst& inst){
//   output->verbose(CALL_INFO, 2, 0,
//   "ECALL: clock_gettime called by thread %" PRIu32
//   " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
//   return EcallStatus::SUCCESS;
// }

// // 404, rev_clock_settime(clockid_t which_clock, const struct __kernel_timespec  *tp)
// EcallStatus RevProc::ECALL_clock_settime(RevInst& inst){
//   output->verbose(CALL_INFO, 2, 0,
//   "ECALL: clock_settime called by thread %" PRIu32
//   " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
//   return EcallStatus::SUCCESS;
// }

// // 405, rev_clock_adjtime(clockid_t which_clock, struct __kernel_timex  *tx)
// EcallStatus RevProc::ECALL_clock_adjtime(RevInst& inst){
//   output->verbose(CALL_INFO, 2, 0,
//   "ECALL: clock_adjtime called by thread %" PRIu32
//   " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
//   return EcallStatus::SUCCESS;
// }

// // 406, rev_clock_getres(clockid_t which_clock, struct __kernel_timespec  *tp)
// EcallStatus RevProc::ECALL_clock_getres(RevInst& inst){
//   output->verbose(CALL_INFO, 2, 0,
//   "ECALL: clock_getres called by thread %" PRIu32
//   " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
//   return EcallStatus::SUCCESS;
// }

// // 407, rev_clock_nanosleep(clockid_t which_clock, int flags, const struct __kernel_timespec  *rqtp, struct __kernel_timespec  *rmtp)
// EcallStatus RevProc::ECALL_clock_nanosleep(RevInst& inst){
//   output->verbose(CALL_INFO, 2, 0,
//   "ECALL: clock_nanosleep called by thread %" PRIu32
//   " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
//   return EcallStatus::SUCCESS;
// }

// // 408, rev_timer_gettime(timer_t timer_id, struct __kernel_itimerspec  *setting)
// EcallStatus RevProc::ECALL_timer_gettime(RevInst& inst){
//   output->verbose(CALL_INFO, 2, 0,
//   "ECALL: timer_gettime called by thread %" PRIu32
//   " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
//   return EcallStatus::SUCCESS;
// }

// // 409, rev_timer_settime(timer_t timer_id, int flags, const struct __kernel_itimerspec  *new_setting, struct __kernel_itimerspec  *old_setting)
// EcallStatus RevProc::ECALL_timer_settime(RevInst& inst){
//   output->verbose(CALL_INFO, 2, 0,
//   "ECALL: timer_settime called by thread %" PRIu32
//   " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
//   return EcallStatus::SUCCESS;
// }

// // 410, rev_timerfd_gettime(int ufd, struct __kernel_itimerspec  *otmr)
// EcallStatus RevProc::ECALL_timerfd_gettime(RevInst& inst){
//   output->verbose(CALL_INFO, 2, 0,
//   "ECALL: timerfd_gettime called by thread %" PRIu32
//   " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
//   return EcallStatus::SUCCESS;
// }

// // 411, rev_timerfd_settime(int ufd, int flags, const struct __kernel_itimerspec  *utmr, struct __kernel_itimerspec  *otmr)
// EcallStatus RevProc::ECALL_timerfd_settime(RevInst& inst){
//   output->verbose(CALL_INFO, 2, 0,
//   "ECALL: timerfd_settime called by thread %" PRIu32
//   " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
//   return EcallStatus::SUCCESS;
// }

// // 412, rev_utimensat(int dfd, const char  *filename, struct __kernel_timespec  *utimes, int flags)
// EcallStatus RevProc::ECALL_utimensat(RevInst& inst){
//   output->verbose(CALL_INFO, 2, 0,
//   "ECALL: utimensat called by thread %" PRIu32
//   " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
//   return EcallStatus::SUCCESS;
// }

// // 416, rev_io_pgetevents(aio_context_t ctx_id, long min_nr, long nr, struct io_event  *events, struct __kernel_timespec  *timeout, const struct __aio_sigset *sig)
// EcallStatus RevProc::ECALL_io_pgetevents(RevInst& inst){
//   output->verbose(CALL_INFO, 2, 0,
//   "ECALL: io_pgetevents called by thread %" PRIu32
//   " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
//   return EcallStatus::SUCCESS;
// }

// // 418, rev_mq_timedsend(mqd_t mqdes, const char  *msg_ptr, size_t msg_len, unsigned int msg_prio, const struct __kernel_timespec  *abs_timeout)
// EcallStatus RevProc::ECALL_mq_timedsend(RevInst& inst){
//   output->verbose(CALL_INFO, 2, 0,
//   "ECALL: mq_timedsend called by thread %" PRIu32
//   " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
//   return EcallStatus::SUCCESS;
// }

// // 419, rev_mq_timedreceive(mqd_t mqdes, char  *msg_ptr, size_t msg_len, unsigned int  *msg_prio, const struct __kernel_timespec  *abs_timeout)
// EcallStatus RevProc::ECALL_mq_timedreceive(RevInst& inst){
//   output->verbose(CALL_INFO, 2, 0,
//   "ECALL: mq_timedreceive called by thread %" PRIu32
//   " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
//   return EcallStatus::SUCCESS;
// }

// // 420, rev_semtimedop(int semid, struct sembuf  *sops, unsigned nsops, const struct __kernel_timespec  *timeout)
// EcallStatus RevProc::ECALL_semtimedop(RevInst& inst){
//   output->verbose(CALL_INFO, 2, 0,
//   "ECALL: semtimedop called by thread %" PRIu32
//   " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
//   return EcallStatus::SUCCESS;
// }

// // 422, rev_futex(u32  *uaddr, int op, u32 val, struct __kernel_timespec  *utime, u32  *uaddr2, u32 val3)
// EcallStatus RevProc::ECALL_futex(RevInst& inst){
//   output->verbose(CALL_INFO, 2, 0,
//   "ECALL: futex called by thread %" PRIu32
//   " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
//   return EcallStatus::SUCCESS;
// }

// // 423, rev_sched_rr_get_interval(pid_t pid, struct __kernel_timespec  *interval)
// EcallStatus RevProc::ECALL_sched_rr_get_interval(RevInst& inst){
//   output->verbose(CALL_INFO, 2, 0,
//   "ECALL: sched_rr_get_interval called by thread %" PRIu32
//   " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
//   return EcallStatus::SUCCESS;
// }
//

// 424, rev_pidfd_send_signal(int pidfd, int sig, siginfo_t  *info, unsigned int flags)
EcallStatus RevProc::ECALL_pidfd_send_signal(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: pidfd_send_signal called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 425, rev_io_uring_setup(u32 entries, struct io_uring_params  *p)
EcallStatus RevProc::ECALL_io_uring_setup(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: io_uring_setup called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 426, rev_io_uring_enter(unsigned int fd, u32 to_submit, u32 min_complete, u32 flags, const sigset_t  *sig, size_t sigsz)
EcallStatus RevProc::ECALL_io_uring_enter(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: io_uring_enter called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 427, rev_io_uring_register(unsigned int fd, unsigned int op, void  *arg, unsigned int nr_args)
EcallStatus RevProc::ECALL_io_uring_register(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: io_uring_register called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 428, rev_open_tree(int dfd, const char  *path, unsigned flags)
EcallStatus RevProc::ECALL_open_tree(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: open_tree called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 429, rev_move_mount(int from_dfd, const char  *from_path, int to_dfd, const char  *to_path, unsigned int ms_flags)
EcallStatus RevProc::ECALL_move_mount(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: move_mount called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 430, rev_fsopen(const char  *fs_name, unsigned int flags)
EcallStatus RevProc::ECALL_fsopen(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: fsopen called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 431, rev_fsconfig(int fs_fd, unsigned int cmd, const char  *key, const void  *value, int aux)
EcallStatus RevProc::ECALL_fsconfig(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: fsconfig called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 432, rev_fsmount(int fs_fd, unsigned int flags, unsigned int ms_flags)
EcallStatus RevProc::ECALL_fsmount(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: fsmount called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 433, rev_fspick(int dfd, const char  *path, unsigned int flags)
EcallStatus RevProc::ECALL_fspick(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: fspick called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 434, rev_pidfd_open(pid_t pid, unsigned int flags)
EcallStatus RevProc::ECALL_pidfd_open(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: pidfd_open called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 435, rev_clone3(struct clone_args  *uargs, size_t size)
EcallStatus RevProc::ECALL_clone3(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: clone3 called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  auto rtval = EcallStatus::SUCCESS;
 //  auto CloneArgsAddr = RegFile->GetX<uint64_t>(RevReg::a0);
 // auto SizeOfCloneArgs = RegFile()->GetX<size_t>(RevReg::a1);

 // if(0 == ECALL.bytesRead){
 //    // First time through the function...
 //    /* Fetch the clone_args */
 //    // struct clone_args args;  // So while clone_args is a whole struct, we appear to be only
 //                                // using the 1st uint64, so that's all we're going to fetch
 //   uint64_t* args = reinterpret_cast<uint64_t*>(ECALL.buf.data());
 //   mem->ReadVal<uint64_t>(HartToExec, CloneArgsAddr, args, inst.hazard, REVMEM_FLAGS(0x00));
 //   ECALL.bytesRead = sizeof(*args);
 //   rtval = EcallStatus::CONTINUE;
 // }else{
 //    /*
 //    * Parse clone flags
 //    * NOTE: if no flags are set, we get fork() like behavior
 //    */
 //   uint64_t* args = reinterpret_cast<uint64_t*>(ECALL.buf.data());
 //    for( uint64_t bit=1; bit != 0; bit <<= 1 ){
 //      switch (*args & bit) {
 //        case CLONE_VM:
 //          // std::cout << "CLONE_VM is true" << std::endl;
 //          break;
 //        case CLONE_FS: /* Set if fs info shared between processes */
 //          // std::cout << "CLONE_FS is true" << std::endl;
 //          break;
 //        case CLONE_FILES: /* Set if open files shared between processes */
 //          // std::cout << "CLONE_FILES is true" << std::endl;
 //          break;
 //        case CLONE_SIGHAND: /* Set if signal handlers shared */
 //          // std::cout << "CLONE_SIGHAND is true" << std::endl;
 //          break;
 //        case CLONE_PIDFD: /* Set if a pidfd should be placed in the parent */
 //          // std::cout << "CLONE_PIDFD is true" << std::endl;
 //          break;
 //        case CLONE_PTRACE: /* Set if tracing continues on the child */
 //          // std::cout << "CLONE_PTRACE is true" << std::endl;
 //          break;
 //        case CLONE_VFORK: /* Set if the parent wants the child to wake it up on mm_release */
 //          // std::cout << "CLONE_VFORK is true" << std::endl;
 //          break;
 //        case CLONE_PARENT: /* Set if we want to have the same parent as the cloner */
 //          // std::cout << "CLONE_PARENT is true" << std::endl;
 //          break;
 //        case CLONE_THREAD: /* Set to add to same thread group */
 //          // std::cout << "CLONE_THREAD is true" << std::endl;
 //          break;
 //        case CLONE_NEWNS: /* Set to create new namespace */
 //          // std::cout << "CLONE_NEWNS is true" << std::endl;
 //          break;
 //        case CLONE_SYSVSEM: /* Set to shared SVID SEM_UNDO semantics */
 //          // std::cout << "CLONE_SYSVSEM is true" << std::endl;
 //          break;
 //        case CLONE_SETTLS: /* Set TLS info */
 //          // std::cout << "CLONE_SETTLS is true" << std::endl;
 //          break;
 //        case CLONE_PARENT_SETTID: /* Store TID in userlevel buffer before MM copy */
 //          // std::cout << "CLONE_PARENT_SETTID is true" << std::endl;
 //          break;
 //        case CLONE_CHILD_CLEARTID: /* Register exit futex and memory location to clear */
 //          // std::cout << "CLONE_CHILD_CLEARTID is true" << std::endl;
 //          break;
 //        case CLONE_DETACHED: /* Create clone detached */
 //          // std::cout << "CLONE_DETACHED is true" << std::endl;
 //          break;
 //        case CLONE_UNTRACED: /* Set if the tracing process can't force CLONE_PTRACE on this clone */
 //          // std::cout << "CLONE_UNTRACED is true" << std::endl;
 //          break;
 //        case CLONE_CHILD_SETTID: /* New cgroup namespace */
 //          // std::cout << "CLONE_CHILD_SETTID is true" << std::endl;
 //          break;
 //        case CLONE_NEWCGROUP: /* New cgroup namespace */
 //          // std::cout << "CLONE_NEWCGROUP is true" << std::endl;
 //          break;
 //        case CLONE_NEWUTS: /* New utsname group */
 //          // std::cout << "CLONE_NEWUTS is true" << std::endl;
 //          break;
 //        case CLONE_NEWIPC: /* New ipcs */
 //          // std::cout << "CLONE_NEWIPC is true" << std::endl;
 //          break;
 //        case CLONE_NEWUSER: /* New user namespace */
 //          // std::cout << "CLONE_NEWUSER is true" << std::endl;
 //          break;
 //        case CLONE_NEWPID: /* New pid namespace */
 //          // std::cout << "CLONE_NEWPID is true" << std::endl;
 //          break;
 //        case CLONE_NEWNET: /* New network namespace */
 //          // std::cout << "CLONE_NEWNET is true" << std::endl;
 //          break;
 //        case CLONE_IO: /* Clone I/O Context */
 //          // std::cout << "CLONE_IO is true" << std::endl;
 //          break;
 //        default:
 //          break;
 //      } // switch
 //    } // for

 //    /* Get the parent ctx (Current active, executing PID) */
 //    std::shared_ptr<RevThread> ParentCtx = ThreadTable.at(ActivePIDs.at(HartToExec));

 //    /* Create the child ctx */
 //    uint32_t ChildPID = CreateChildCtx();
 //    std::shared_ptr<RevThread> ChildCtx = ThreadTable.at(ChildPID);

 //    /*
 //    * ===========================================================================================
 //    * Register File
 //    * ===========================================================================================
 //    * We need to duplicate the parent's RegFile to to the Childs
 //    * - NOTE: when we return from this function, the return value will
 //    *         be automatically stored in the Proc.RegFile[HartToExec]'s a0
 //    *         register. In a traditional fork code this looks like:
 //    *
 //    *         pid_t pid = fork()
 //    *         if pid < 0: // Error
 //    *         else if pid = 0: // New Child Process
 //    *         else: // Parent Process
 //    *
 //    *         In this case, the value of pid is the value thats returned to a0
 //    *         It follows that
 //    *         - The child's regfile MUST have 0 in its a0 (despite its pid != 0 to the RevProc)
 //    *         - The Parent's a0 register MUST have its PID in it
 //    * ===========================================================================================
 //    */

 //    /*
 //    Alert the Proc there needs to be a Ctx switch
 //    Pass the PID that will be switched to once the
 //    current pipeline is executed until completion
 //    */
 //    CtxSwitchAlert(ChildPID);

 //    // Parent's return value is the child's PID
 //    RegFile->SetX(RevReg::a0, ChildPID);

 //    // Child's return value is 0
 //    ChildCtx->GetRegFile()->SetX(RevReg::a0, 0);

 //    // clean up ecall state
 //    rtval = EcallStatus::SUCCESS;
 //    ECALL.bytesRead = 0;

 //  } //else
  return rtval;
}

// 436, rev_close_range(unsigned int fd, unsigned int max_fd, unsigned int flags)
EcallStatus RevProc::ECALL_close_range(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: close_range called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 437, rev_openat2(int dfd, const char  *filename, struct open_how *how, size_t size)
EcallStatus RevProc::ECALL_openat2(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: openat2 called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 438, rev_pidfd_getfd(int pidfd, int fd, unsigned int flags)
EcallStatus RevProc::ECALL_pidfd_getfd(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: pidfd_getfd called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}


// 439, rev_faccessat2(int dfd, const char  *filename, int mode, int flags)
EcallStatus RevProc::ECALL_faccessat2(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: faccessat2 called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}


// 440, rev_process_madvise(int pidfd, const struct iovec  *vec, size_t vlen, int behavior, unsigned int flags)
EcallStatus RevProc::ECALL_process_madvise(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: process_madvise called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  return EcallStatus::SUCCESS;
}

// 500, rev_cpuinfo(struct rev_cpuinfo *info)
EcallStatus RevProc::ECALL_cpuinfo(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: cpuinfoc called by thread %" PRIu32
                  "\n", GetActiveThreadID());
  struct rev_cpuinfo info;
  auto addr = RegFile->GetX<int>(RevReg::a0);
  info.cores = opts->GetNumCores();
  info.harts_per_core = opts->GetNumHarts();
  mem->WriteMem(HartToExec, addr, sizeof(info), &info);
  RegFile->SetX(RevReg::a0, 0);
  return EcallStatus::SUCCESS;
}

// 1000, int pthread_create(pthread_t *restrict thread,
//                          const pthread_attr_t *restrict attr,
//                          void *(*start_routine)(void *),
//                          void *restrict arg);
EcallStatus RevProc::ECALL_pthread_create(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: pthread_create called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);
  uint64_t tidAddr     = RegFile->GetX<uint64_t>(RevReg::a0);
  //uint64_t AttrPtr     = RegFile->GetX<uint64_t>(RevReg::a1);
  uint64_t NewThreadPC = RegFile->GetX<uint64_t>(RevReg::a2);
  uint64_t ArgPtr      = RegFile->GetX<uint64_t>(RevReg::a3);
  unsigned long int NewTID = GetNewThreadID();
  CreateThread(NewTID,
               NewThreadPC, reinterpret_cast<void*>(ArgPtr));

  mem->WriteMem(HartToExec, tidAddr, sizeof(NewTID), &NewTID, REVMEM_FLAGS(0x00));
  return EcallStatus::SUCCESS;
}

// 1001, int rev_pthread_join(pthread_t thread, void **retval);
EcallStatus RevProc::ECALL_pthread_join(RevInst& inst){
  EcallStatus rtval = EcallStatus::CONTINUE;
  output->verbose(CALL_INFO, 2, 0,
                  "ECALL: pthread_join called by thread %" PRIu32
                  " on hart %" PRIu32 "\n", GetActiveThreadID(), HartToExec);

  if( !ThreadHasDependencies(Harts.at(HartToExec)->GetAssignedThreadID()) ){
    rtval = EcallStatus::SUCCESS;

    // Set current thread to blocked
    auto& Thread = GetThreadOnHart(HartToExec);
    Thread->SetState(ThreadState::BLOCKED);

    // Signal to RevCPU this thread is has changed state
    ThreadsThatChangedState.emplace(Thread);

    // Output the ecall buf

    // Set the TID this thread is waiting for
    Thread->SetWaitingToJoinTID(RegFile->RV64[10]);

    // // if retval is not null,
    //
    //store the return value of the thread in retval
    // void **retval = (void **)RegFile->RV64[11];
    // if( retval != NULL ){
    //   *retval = (void *)
    //   GetThreadOnHart(HartToDecode)->GetRegFile()->RV64[10];
    // }
    //
  }
  return rtval;
}
