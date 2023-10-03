#include "../include/RevProc.h"
#include "RevMem.h"
#include <bitset>
#include <filesystem>
#include <sys/xattr.h>

using namespace SST::RevCPU;

// 0, rev_io_setup(unsigned nr_reqs, aio_context_t  *ctx)
RevProc::ECALL_status_t RevProc::ECALL_io_setup(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: io_setup called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 1, rev_io_destroy(aio_context_t ctx)
RevProc::ECALL_status_t RevProc::ECALL_io_destroy(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: io_destroy called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 2, rev_io_submit(aio_context_t, long, struct iocb  *  *)
RevProc::ECALL_status_t RevProc::ECALL_io_submit(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: io_submit called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 3, rev_io_cancel(aio_context_t ctx_id, struct iocb  *iocb, struct io_event  *result)
RevProc::ECALL_status_t RevProc::ECALL_io_cancel(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: io_cancel called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 4, rev_io_getevents(aio_context_t ctx_id, long min_nr, long nr, struct io_event  *events, struct __kernel_timespec  *timeout)
RevProc::ECALL_status_t RevProc::ECALL_io_getevents(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: io_getevents called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 5, rev_setxattr(const char  *path, const char  *name, const void  *value, size_t size, int flags)
RevProc::ECALL_status_t RevProc::ECALL_setxattr(RevInst& inst){
#if 0
  // TODO: Need to load the data from (value, size bytes) into
  // hostValue vector before it can be passed to setxattr() on host.

  auto path = RegFile->GetX<uint64_t>(10);
  auto name = RegFile->GetX<uint64_t>(11);
  auto value = RegFile->GetX<uint64_t>(12);
  auto size = RegFile->GetX<size_t>(13);
  auto flags = RegFile->GetX<uint64_t>(14);

  // host-side value which has size bytes
  std::vector<char> hostValue(size);

  if(ECALL.path_string.empty()){
    // We are still parsing the path string. When it is finished, we
    // will move the ECALL.string to ECALL.path_string and continue below
    auto action = [&]{
      ECALL.path_string = std::move(ECALL.string);
    };
    auto rtv = ECALL_LoadAndParseString(inst, path, action);

    // When the parsing of path_string returns SUCCESS, we change it to
    // CONTINUE to continue the later stages
    return rtv == ECALL_status_t::SUCCESS ? ECALL_status_t::CONTINUE : rtv;
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
      RegFile->SetX(10, rc);
    };

    // Parse the name string, then call setxattr() using path and name
    return ECALL_LoadAndParseString(inst, name, action);
  }
#else
  return ECALL_status_t::SUCCESS;
#endif
}

// 6, rev_lsetxattr(const char  *path, const char  *name, const void  *value, size_t size, int flags)
RevProc::ECALL_status_t RevProc::ECALL_lsetxattr(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: lsetxattr called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 7, rev_fsetxattr(int fd, const char  *name, const void  *value, size_t size, int flags)
RevProc::ECALL_status_t RevProc::ECALL_fsetxattr(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fsetxattr called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 8, rev_getxattr(const char  *path, const char  *name, void  *value, size_t size)
RevProc::ECALL_status_t RevProc::ECALL_getxattr(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: getxattr called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 9, rev_lgetxattr(const char  *path, const char  *name, void  *value, size_t size)
RevProc::ECALL_status_t RevProc::ECALL_lgetxattr(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: lgetxattr called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 10, rev_fgetxattr(int fd, const char  *name, void  *value, size_t size)
RevProc::ECALL_status_t RevProc::ECALL_fgetxattr(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fgetxattr called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 11, rev_listxattr(const char  *path, char  *list, size_t size)
RevProc::ECALL_status_t RevProc::ECALL_listxattr(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: listxattr called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 12, rev_llistxattr(const char  *path, char  *list, size_t size)
RevProc::ECALL_status_t RevProc::ECALL_llistxattr(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: llistxattr called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 13, rev_flistxattr(int fd, char  *list, size_t size)
RevProc::ECALL_status_t RevProc::ECALL_flistxattr(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: flistxattr called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 14, rev_removexattr(const char  *path, const char  *name)
RevProc::ECALL_status_t RevProc::ECALL_removexattr(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: removexattr called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 15, rev_lremovexattr(const char  *path, const char  *name)
RevProc::ECALL_status_t RevProc::ECALL_lremovexattr(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: lremovexattr called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 16, rev_fremovexattr(int fd, const char  *name)
RevProc::ECALL_status_t RevProc::ECALL_fremovexattr(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fremovexattr called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 17, rev_getcwd(char  *buf, unsigned long size)
RevProc::ECALL_status_t RevProc::ECALL_getcwd(RevInst& inst){
  auto BufAddr = RegFile->GetX<uint64_t>(10);
  auto size = RegFile->GetX<uint64_t>(11);
  auto CWD = std::filesystem::current_path();
  mem->WriteMem(feature->GetHartToExec(), BufAddr, size, CWD.c_str());

  // Returns null-terminated string in buf
  // (no need to set x10 since it's already got BufAddr)
  // RegFile->SetX(10, BufAddr);

  return ECALL_status_t::SUCCESS;
}

// 18, rev_lookup_dcookie(u64 cookie64, char  *buf, size_t len)
RevProc::ECALL_status_t RevProc::ECALL_lookup_dcookie(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: lookup_dcookie called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 19, rev_eventfd2(unsigned int count, int flags)
RevProc::ECALL_status_t RevProc::ECALL_eventfd2(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: eventfd2 called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 20, rev_epoll_create1(int flags)
RevProc::ECALL_status_t RevProc::ECALL_epoll_create1(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: epoll_create1 called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 21, rev_epoll_ctl(int epfd, int op, int fd, struct epoll_event  *event)
RevProc::ECALL_status_t RevProc::ECALL_epoll_ctl(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: epoll_ctl called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 22, rev_epoll_pwait(int epfd, struct epoll_event  *events, int maxevents, int timeout, const sigset_t  *sigmask, size_t sigsetsize)
RevProc::ECALL_status_t RevProc::ECALL_epoll_pwait(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: epoll_pwait called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 23, rev_dup(unsigned int fildes)
RevProc::ECALL_status_t RevProc::ECALL_dup(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: dup called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 24, rev_dup3(unsigned int oldfd, unsigned int newfd, int flags)
RevProc::ECALL_status_t RevProc::ECALL_dup3(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: dup3 called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 25, rev_fcntl64(unsigned int fd, unsigned int cmd, unsigned long arg)
RevProc::ECALL_status_t RevProc::ECALL_fcntl64(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fcntl64 called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 26, rev_inotify_init1(int flags)
RevProc::ECALL_status_t RevProc::ECALL_inotify_init1(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: inotify_init1 called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 27, rev_inotify_add_watch(int fd, const char  *path, u32 mask)
RevProc::ECALL_status_t RevProc::ECALL_inotify_add_watch(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: inotify_add_watch called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 28, rev_inotify_rm_watch(int fd, __s32 wd)
RevProc::ECALL_status_t RevProc::ECALL_inotify_rm_watch(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: inotify_rm_watch called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 29, rev_ioctl(unsigned int fd, unsigned int cmd, unsigned long arg)
RevProc::ECALL_status_t RevProc::ECALL_ioctl(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: ioctl called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 30, rev_ioprio_set(int which, int who, int ioprio)
RevProc::ECALL_status_t RevProc::ECALL_ioprio_set(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: ioprio_set called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 31, rev_ioprio_get(int which, int who)
RevProc::ECALL_status_t RevProc::ECALL_ioprio_get(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: ioprio_get called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 32, rev_flock(unsigned int fd, unsigned int cmd)
RevProc::ECALL_status_t RevProc::ECALL_flock(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: flock called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 33, rev_mknodat(int dfd, const char  * filename, umode_t mode, unsigned dev)
RevProc::ECALL_status_t RevProc::ECALL_mknodat(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: mknodat called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// TODO: 34, rev_mkdirat(int dfd, const char  * pathname, umode_t mode)
RevProc::ECALL_status_t RevProc::ECALL_mkdirat(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: mkdirat called");
  auto dirfd = RegFile->GetX<int>(10);
  auto path = RegFile->GetX<uint64_t>(11);
  auto mode = RegFile->GetX<unsigned short>(12);

  auto action = [&]{
    // Do the mkdirat on the host
    int rc = mkdirat(dirfd, ECALL.string.c_str(), mode);
    RegFile->SetX(10, rc);
  };
  return ECALL_LoadAndParseString(inst, path, action);
}

// 35, rev_unlinkat(int dfd, const char  * pathname, int flag)
RevProc::ECALL_status_t RevProc::ECALL_unlinkat(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: unlinkat called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 36, rev_symlinkat(const char  * oldname, int newdfd, const char  * newname)
RevProc::ECALL_status_t RevProc::ECALL_symlinkat(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: symlinkat called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 37, rev_unlinkat(int dfd, const char  * pathname, int flag)
RevProc::ECALL_status_t RevProc::ECALL_linkat(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: linkat called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 38, rev_renameat(int olddfd, const char  * oldname, int newdfd, const char  * newname)
RevProc::ECALL_status_t RevProc::ECALL_renameat(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: renameat called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 39, rev_umount(char  *name, int flags)
RevProc::ECALL_status_t RevProc::ECALL_umount(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: umount called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 40, rev_umount(char  *name, int flags)
RevProc::ECALL_status_t RevProc::ECALL_mount(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: mount called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 41, rev_pivot_root(const char  *new_root, const char  *put_old)
RevProc::ECALL_status_t RevProc::ECALL_pivot_root(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: pivot_root called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 42, rev_ni_syscall(void)
RevProc::ECALL_status_t RevProc::ECALL_ni_syscall(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: ni_syscall called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 43, rev_statfs64(const char  *path, size_t sz, struct statfs64  *buf)
RevProc::ECALL_status_t RevProc::ECALL_statfs64(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: statfs64 called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 44, rev_fstatfs64(unsigned int fd, size_t sz, struct statfs64  *buf)
RevProc::ECALL_status_t RevProc::ECALL_fstatfs64(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fstatfs64 called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 45, rev_truncate64(const char  *path, loff_t length)
RevProc::ECALL_status_t RevProc::ECALL_truncate64(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: truncate64 called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 46, rev_ftruncate64(unsigned int fd, loff_t length)
RevProc::ECALL_status_t RevProc::ECALL_ftruncate64(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: ftruncate64 called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 47, rev_fallocate(int fd, int mode, loff_t offset, loff_t len)
RevProc::ECALL_status_t RevProc::ECALL_fallocate(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fallocate called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 48, rev_faccessat(int dfd, const char  *filename, int mode)
RevProc::ECALL_status_t RevProc::ECALL_faccessat(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: faccessat called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 49, rev_chdir(const char  *filename)
RevProc::ECALL_status_t RevProc::ECALL_chdir(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: chdir called\n");
  auto path = RegFile->GetX<uint64_t>(10);
  auto action = [&]{
    int rc = chdir(ECALL.string.c_str());
    RegFile->SetX(10, rc);
  };
  return ECALL_LoadAndParseString(inst, path, action);
}

// 50, rev_fchdir(unsigned int fd)
RevProc::ECALL_status_t RevProc::ECALL_fchdir(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fchdir called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 51, rev_chroot(const char  *filename)
RevProc::ECALL_status_t RevProc::ECALL_chroot(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: chroot called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 52, rev_fchmod(unsigned int fd, umode_t mode)
RevProc::ECALL_status_t RevProc::ECALL_fchmod(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fchmod called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 53, rev_fchmodat(int dfd, const char  * filename, umode_t mode)
RevProc::ECALL_status_t RevProc::ECALL_fchmodat(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fchmodat called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 54, rev_fchownat(int dfd, const char  *filename, uid_t user, gid_t group, int flag)
RevProc::ECALL_status_t RevProc::ECALL_fchownat(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fchownat called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 55, rev_fchown(unsigned int fd, uid_t user, gid_t group)
RevProc::ECALL_status_t RevProc::ECALL_fchown(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fchown called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 56, rev_openat(int dfd, const char  *filename, int flags, umode_t mode)
RevProc::ECALL_status_t RevProc::ECALL_openat(RevInst& inst){
  auto dirfd = RegFile->GetX<int>(10);
  auto pathname = RegFile->GetX<uint64_t>(11);

  // commented out to remove warnings
  // auto flags = RegFile->GetX<int>(12);
  // auto mode = RegFile->GetX<int>(13);

  /*
   * NOTE: this is currently only opening files in the current directory
   *       because of some oddities in parsing the arguments & flags
   *       but this will be fixed in the near future
   */

  /* Read the filename from memory one character at a time until we find '\0' */

  auto action = [&]{
    // Do the openat on the host
    dirfd = open(std::filesystem::current_path().c_str(), O_RDONLY);
    int fd = openat(dirfd, ECALL.string.c_str(), O_RDWR);

    AssignedThreads.at(HartToExec)->AddFD(fd);

    // openat returns the file descriptor of the opened file
    RegFile->SetX(10, fd);
  };

  return ECALL_LoadAndParseString(inst, pathname, action);
}
// TODO: RevProc::ECALL_status_t RevProc::ECALL_openat(RevInst& inst){
//   int dfd = RegFile->RV64[10];
//   int filenameAddr = RegFile->RV64[11];
//   int flags = RegFile->RV64[12]; /* NOTE: Unused for now */
//   uint64_t mode = RegFile->RV64[13];

//   RevProc::ECALL_status_t rtval = ECALL_status_t::SUCCESS;
//   /*
//    * NOTE: this is currently only opening files in the current directory
//    *       because of some oddities in parsing the arguments & flags
//    *       but this will be fixed in the near future
//   */


//   /* Read the filename from memory one character at a time until we find '\0' */

//   if('\0' != ECALL.buf[0]){
//     //We are in the middle of the string
//     ECALL.string = ECALL.string + ECALL.buf[0];
//     mem->ReadVal<char>(HartToExec, filenameAddr + sizeof(char)*ECALL.string.length(), &ECALL.buf[0], inst.hazard, REVMEM_FLAGS(0x00));
//     rtval = RevProc::ECALL_status_t::CONTINUE;
//     DependencySet(HartToExec, 10, false);
//   }else if(('\0' == ECALL.buf[0]) && (ECALL.string.length() > 0)) {
//     //found the null terminator - we're done
//     ECALL.string = ECALL.string + ECALL.buf[0];

//     /* Do the openat on the host */
//     dfd = open(std::filesystem::current_path().c_str(), O_RDONLY);
//     int fd = openat(dfd, ECALL.string.c_str(), O_RDWR);

//     AssignedThreads.at(HartToDecode)->AddFD(fd);

//     /* openat returns the file descriptor of the opened file */
//     RegFile->RV64[10] = fd;

//     ECALL.string.clear();   //reset the ECALL buffers
//     ECALL.buf[0] = '\0';
//     rtval = RevProc::ECALL_status_t::SUCCESS;
//     DependencyClear(HartToExec, 10, false);
//   }else{
//     //first time through the ECALL
//     mem->ReadVal<char>(HartToExec, filenameAddr, &ECALL.buf[0], inst.hazard, REVMEM_FLAGS(0x00));
//     DependencySet(HartToExec, 10, false);
//     rtval = RevProc::ECALL_status_t::CONTINUE;
//   }

//   return rtval;
// }

// 57, rev_close(unsigned int fd)
RevProc::ECALL_status_t RevProc::ECALL_close(RevInst& inst){
  auto fd = RegFile->GetX<int>(10);
  auto ActiveThread = AssignedThreads.at(HartToExec);

  // Check if CurrCtx has fd in fildes vector
  if( !ActiveThread->FindFD(fd) ){
    output->fatal(CALL_INFO, -1,
                  "Core %" PRIu32 "; Hart %" PRIu32 "; ThreadID %" PRIu32 " tried to close file descriptor %" PRIu32 " but did not have access to it\n",
                  id, HartToExec, GetActiveThreadID(), fd);
    return ECALL_status_t::SUCCESS;
  }
  // Close file on host
  int rc = close(fd);

  // Remove from Ctx's fildes
  ActiveThread->RemoveFD(fd);

  // rc is propogated to rev from host
  RegFile->SetX(10, rc);

  return ECALL_status_t::SUCCESS;
}

// 58, rev_vhangup(void)
RevProc::ECALL_status_t RevProc::ECALL_vhangup(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: vhangup called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 59, rev_pipe2(int  *fildes, int flags)
RevProc::ECALL_status_t RevProc::ECALL_pipe2(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: pipe2 called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 60, rev_quotactl(unsigned int cmd, const char  *special, qid_t id, void  *addr)
RevProc::ECALL_status_t RevProc::ECALL_quotactl(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: quotactl called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 61, rev_getdents64(unsigned int fd, struct linux_dirent64  *dirent, unsigned int count)
RevProc::ECALL_status_t RevProc::ECALL_getdents64(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: getdents64 called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 62, rev_llseek(unsigned int fd, unsigned long offset_high, unsigned long offset_low, loff_t  *result, unsigned int whence)
RevProc::ECALL_status_t RevProc::ECALL_lseek(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: lseek called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 63, rev_read(unsigned int fd
RevProc::ECALL_status_t RevProc::ECALL_read(RevInst& inst){
  auto fd = RegFile->GetX<int>(10);
  auto BufAddr = RegFile->GetX<uint64_t>(11);
  auto BufSize = RegFile->GetX<uint64_t>(12);

  /* Check if Current Ctx has access to the fd */
  auto ActiveThread = AssignedThreads.at(HartToExec);

  if( !ActiveThread->FindFD(fd) ){
    output->fatal(CALL_INFO, -1,
                  "Core %u; Hart %" PRIu16 "; PID %" PRIu32
                  " tried to read from file descriptor: %d, but did not have access to it\n",
                  id, HartToExec, GetActiveThreadID(), fd);
    return ECALL_status_t::SUCCESS;
  }
  /*
   * This buffer is an intermediate buffer for storing the data read from host
   * for later use in writing to RevMem
   */
  std::vector<char> TmpBuf(BufSize);

  /*
   * Read nbytes of fd from host
   *
   * NOTE: Because the fd is in the Ctx's fildes vector, we can reasonably
   *       assume the file is already open on the host system because we
   *       try to maintain parity between those
   */

  // Do the read on the host
  int rc = read(fd, &TmpBuf[0], BufSize);

  // Write that data to the buffer inside of Rev
  mem->WriteMem(feature->GetHartToExec(), BufAddr, BufSize, &TmpBuf[0]);

  RegFile->SetX(10, rc);
  return ECALL_status_t::SUCCESS;
}

// RevProc::ECALL_status_t RevProc::ECALL_read(RevInst& inst){
//   int fd = RegFile->RV64[10];
//   uint64_t BufAddr = RegFile->RV64[11];
//   size_t BufSize = RegFile->RV64[12];

//   // Check if Current Thread has access to the fd
//   std::shared_ptr<RevThread> CurrThread = AssignedThreads.at(HartToDecode);

//   if( !CurrThread->FindFD(fd) ){
//     output->fatal(CALL_INFO, -1,
//                   "Core %d; Hart %d; ThreadID %" PRIu32 " tried to read from file descriptor: " PRIu32 " but did not have access to it\n",
//                   id, HartToExec, GetActiveThreadID(), fd);
//     return ECALL_status_t::SUCCESS;
//   }
//   /*
//    * This buffer is an intermediate buffer for storing the data read from host
//    * for later use in writing to RevMem
//   */
//   char TmpBuf[BufSize];

//   /*
//    * Read nbytes of fd from host
//    *
//    * NOTE: Because the fd is in the Thread's fildes vector, we can reasonably
//    *       assume the file is already open on the host system because we
//    *       try to maintain parity between those
//    */

//   /* Do the read on the host */
//   uint64_t rc = read(fd, &TmpBuf, BufSize);

//   /* Write that data to the buffer inside of Rev */
//   mem->WriteMem(feature->GetHart(), BufAddr, BufSize, &TmpBuf);

//   RegFile->RV64[10] = rc;
//   return ECALL_status_t::SUCCESS;
// }

// 64, rev_write(unsigned int fd, const char  *buf, size_t count)
RevProc::ECALL_status_t RevProc::ECALL_write(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: write called\n");
  auto fd = RegFile->GetX<int>(10);
  auto addr = RegFile->GetX<uint64_t>(11);
  auto nbytes = RegFile->GetX<uint64_t>(12);
  auto rtv = ECALL_status_t::ERROR;

  if(ECALL.bytesRead){
    // Not our first time through... so capture previous read data
    ECALL.string += std::string_view(ECALL.buf.data(), ECALL.bytesRead);
    ECALL.bytesRead = 0;
  }

  auto nleft = nbytes - ECALL.string.size();
  if(nleft == 0){
    // Perform the write on the host system
    int rc = write(fd, ECALL.string.data(), ECALL.string.size());

    // write returns the number of bytes written
    RegFile->SetX(10, rc);

    // Reset our tracking state
    ECALL.clear();

    DependencyClear(HartToExec, 10, false);
    rtv = ECALL_status_t::SUCCESS;
  }else if (0 == LSQueue->count(make_lsq_hash(10, RevRegClass::RegGPR, HartToExec)))  {
    auto readfunc = [&](auto* buf){
      MemReq req (addr + ECALL.string.size(), 10, RevRegClass::RegGPR, HartToExec, MemOp::MemOpREAD, true, RegFile->GetMarkLoadComplete());
      LSQueue->insert({make_lsq_hash(req.DestReg, req.RegType, req.Hart), req});
      mem->ReadVal(HartToExec,
                   addr + ECALL.string.size(),
                   buf,
                   req,
                   REVMEM_FLAGS(0));
      ECALL.bytesRead = sizeof(*buf);
    };
    if(nleft >= 8){
      readfunc(reinterpret_cast<uint64_t*>(ECALL.buf.data()));
    } else if(nleft >= 4){
      readfunc(reinterpret_cast<uint32_t*>(ECALL.buf.data()));
    } else if(nleft >= 2){
      readfunc(reinterpret_cast<uint16_t*>(ECALL.buf.data()));
    } else{
      readfunc(reinterpret_cast<uint8_t*>(ECALL.buf.data()));
    }
    DependencySet(HartToExec, 10, false);
    rtv = ECALL_status_t::CONTINUE;
  }else{
    rtv = ECALL_status_t::CONTINUE;
  }
  return rtv;
}

// 65, rev_readv(unsigned long fd, const struct iovec  *vec, unsigned long vlen)
RevProc::ECALL_status_t RevProc::ECALL_readv(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: readv called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 66, rev_writev(unsigned long fd, const struct iovec  *vec, unsigned long vlen)
RevProc::ECALL_status_t RevProc::ECALL_writev(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: writev called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 67, rev_pread64(unsigned int fd, char  *buf, size_t count, loff_t pos)
RevProc::ECALL_status_t RevProc::ECALL_pread64(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: pread64 called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 68, rev_pwrite64(unsigned int fd, const char  *buf, size_t count, loff_t pos)
RevProc::ECALL_status_t RevProc::ECALL_pwrite64(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: pwrite64 called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 69, rev_preadv(unsigned long fd, const struct iovec  *vec, unsigned long vlen, unsigned long pos_l, unsigned long pos_h)
RevProc::ECALL_status_t RevProc::ECALL_preadv(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: preadv called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 70, rev_pwritev(unsigned long fd, const struct iovec  *vec, unsigned long vlen, unsigned long pos_l, unsigned long pos_h)
RevProc::ECALL_status_t RevProc::ECALL_pwritev(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: pwritev called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 71, rev_sendfile64(int out_fd, int in_fd, loff_t  *offset, size_t count)
RevProc::ECALL_status_t RevProc::ECALL_sendfile64(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: sendfile64 called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 72, rev_pselect6_time32(int, fd_set  *, fd_set  *, fd_set  *, struct old_timespec32  *, void  *)
RevProc::ECALL_status_t RevProc::ECALL_pselect6_time32(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: pselect6_time32 called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 73, rev_ppoll_time32(struct pollfd  *, unsigned int, struct old_timespec32  *, const sigset_t  *, size_t)
RevProc::ECALL_status_t RevProc::ECALL_ppoll_time32(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: ppoll_time32 called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 74, rev_signalfd4(int ufd, sigset_t  *user_mask, size_t sizemask, int flags)
RevProc::ECALL_status_t RevProc::ECALL_signalfd4(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: signalfd4 called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 75, rev_vmsplice(int fd, const struct iovec  *iov, unsigned long nr_segs, unsigned int flags)
RevProc::ECALL_status_t RevProc::ECALL_vmsplice(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: vmsplice called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 76, rev_vmsplice(int fd, const struct iovec  *iov, unsigned long nr_segs, unsigned int flags)
RevProc::ECALL_status_t RevProc::ECALL_splice(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: splice called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 77, rev_tee(int fdin, int fdout, size_t len, unsigned int flags)
RevProc::ECALL_status_t RevProc::ECALL_tee(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: tee called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 78, rev_readlinkat(int dfd, const char  *path, char  *buf, int bufsiz)
RevProc::ECALL_status_t RevProc::ECALL_readlinkat(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: readlinkat called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 79, rev_newfstatat(int dfd, const char  *filename, struct stat  *statbuf, int flag)
RevProc::ECALL_status_t RevProc::ECALL_newfstatat(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: newfstatat called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 80, rev_newfstat(unsigned int fd, struct stat  *statbuf)
RevProc::ECALL_status_t RevProc::ECALL_newfstat(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: newfstat called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 81, rev_sync(void)
RevProc::ECALL_status_t RevProc::ECALL_sync(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: sync called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 82, rev_fsync(unsigned int fd)
RevProc::ECALL_status_t RevProc::ECALL_fsync(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fsync called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 83, rev_fdatasync(unsigned int fd)
RevProc::ECALL_status_t RevProc::ECALL_fdatasync(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fdatasync called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 84, rev_sync_file_range2(int fd, unsigned int flags, loff_t offset, loff_t nbytes)
RevProc::ECALL_status_t RevProc::ECALL_sync_file_range2(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: sync_file_range2 called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 84, rev_sync_file_range(int fd, loff_t offset, loff_t nbytes, unsigned int flags)
RevProc::ECALL_status_t RevProc::ECALL_sync_file_range(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: sync_file_range called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 85, rev_timerfd_create(int clockid, int flags)
RevProc::ECALL_status_t RevProc::ECALL_timerfd_create(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: timerfd_create called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 86, rev_timerfd_settime(int ufd, int flags, const struct __kernel_itimerspec  *utmr, struct __kernel_itimerspec  *otmr)
RevProc::ECALL_status_t RevProc::ECALL_timerfd_settime(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: timerfd_settime called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 87, rev_timerfd_gettime(int ufd, struct __kernel_itimerspec  *otmr)
RevProc::ECALL_status_t RevProc::ECALL_timerfd_gettime(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: timerfd_gettime called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 88, rev_utimensat(int dfd, const char  *filename, struct __kernel_timespec  *utimes, int flags)
RevProc::ECALL_status_t RevProc::ECALL_utimensat(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: utimensat called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 89, rev_acct(const char  *name)
RevProc::ECALL_status_t RevProc::ECALL_acct(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: acct called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 90, rev_capget(cap_user_header_t header, cap_user_data_t dataptr)
RevProc::ECALL_status_t RevProc::ECALL_capget(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: capget called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 91, rev_capset(cap_user_header_t header, const cap_user_data_t data)
RevProc::ECALL_status_t RevProc::ECALL_capset(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: capset called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 92, rev_personality(unsigned int personality)
RevProc::ECALL_status_t RevProc::ECALL_personality(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: personality called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// TODO: Move exit here
// 93, rev_exit(int error_code)
RevProc::ECALL_status_t RevProc::ECALL_exit(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: exit called by thread %" PRIu32 "\n", GetActiveThreadID());
  auto ActiveThread = AssignedThreads.at(HartToExec);
  auto status = RegFile->GetX<uint64_t>(10);

  output->verbose(CALL_INFO, 0, 0,
                  "Thread %u " PRIu32 "exiting with status %" PRIu64 "\n",
                  ActiveThread->GetThreadID(), status );
  exit(status);
  return ECALL_status_t::SUCCESS;
}


// 94, rev_exit_group(int error_code)
RevProc::ECALL_status_t RevProc::ECALL_exit_group(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: exit_group called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 95, rev_waitid(int which, pid_t pid, struct siginfo  *infop, int options, struct rusage  *ru)
RevProc::ECALL_status_t RevProc::ECALL_waitid(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: waitid called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 96, rev_set_tid_address(int  *tidptr)
RevProc::ECALL_status_t RevProc::ECALL_set_tid_address(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: set_tid_address called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 97, rev_unshare(unsigned long unshare_flags)
RevProc::ECALL_status_t RevProc::ECALL_unshare(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: unshare called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 98, rev_futex(u32  *uaddr, int op, u32 val, struct __kernel_timespec  *utime, u32  *uaddr2, u32 val3)
RevProc::ECALL_status_t RevProc::ECALL_futex(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: futex called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 99, rev_set_robust_list(struct robust_list_head  *head, size_t len)
RevProc::ECALL_status_t RevProc::ECALL_set_robust_list(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: set_robust_list called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 100, rev_get_robust_list(int pid, struct robust_list_head  *  *head_ptr, size_t  *len_ptr)
RevProc::ECALL_status_t RevProc::ECALL_get_robust_list(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: get_robust_list called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 101, rev_nanosleep(struct __kernel_timespec  *rqtp, struct __kernel_timespec  *rmtp)
RevProc::ECALL_status_t RevProc::ECALL_nanosleep(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: nanosleep called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 102, rev_getitimer(int which, struct __kernel_old_itimerval  *value)
RevProc::ECALL_status_t RevProc::ECALL_getitimer(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: getitimer called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 103, rev_setitimer(int which, struct __kernel_old_itimerval  *value, struct __kernel_old_itimerval  *ovalue)
RevProc::ECALL_status_t RevProc::ECALL_setitimer(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: setitimer called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 104, rev_kexec_load(unsigned long entry, unsigned long nr_segments, struct kexec_segment  *segments, unsigned long flags)
RevProc::ECALL_status_t RevProc::ECALL_kexec_load(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: kexec_load called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 105, rev_init_module(void  *umod, unsigned long len, const char  *uargs)
RevProc::ECALL_status_t RevProc::ECALL_init_module(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: init_module called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 106, rev_delete_module(const char  *name_user, unsigned int flags)
RevProc::ECALL_status_t RevProc::ECALL_delete_module(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: delete_module called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 107, rev_timer_create(clockid_t which_clock, struct sigevent  *timer_event_spec, timer_t  * created_timer_id)
RevProc::ECALL_status_t RevProc::ECALL_timer_create(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: timer_create called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 108, rev_timer_gettime(timer_t timer_id, struct __kernel_itimerspec  *setting)
RevProc::ECALL_status_t RevProc::ECALL_timer_gettime(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: timer_gettime called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 109, rev_timer_getoverrun(timer_t timer_id)
RevProc::ECALL_status_t RevProc::ECALL_timer_getoverrun(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: timer_getoverrun called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 110, rev_timer_settime(timer_t timer_id, int flags, const struct __kernel_itimerspec  *new_setting, struct __kernel_itimerspec  *old_setting)
RevProc::ECALL_status_t RevProc::ECALL_timer_settime(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: timer_settime called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 111, rev_timer_delete(timer_t timer_id)
RevProc::ECALL_status_t RevProc::ECALL_timer_delete(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: timer_delete called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 112, rev_clock_settime(clockid_t which_clock, const struct __kernel_timespec  *tp)
RevProc::ECALL_status_t RevProc::ECALL_clock_settime(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: clock_settime called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 113, rev_clock_gettime(clockid_t which_clock, struct __kernel_timespec  *tp)
RevProc::ECALL_status_t RevProc::ECALL_clock_gettime(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: clock_gettime called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 114, rev_clock_getres(clockid_t which_clock, struct __kernel_timespec  *tp)
RevProc::ECALL_status_t RevProc::ECALL_clock_getres(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: clock_getres called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 115, rev_clock_nanosleep(clockid_t which_clock, int flags, const struct __kernel_timespec  *rqtp, struct __kernel_timespec  *rmtp)
RevProc::ECALL_status_t RevProc::ECALL_clock_nanosleep(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: clock_nanosleep called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 116, rev_syslog(int type, char  *buf, int len)
RevProc::ECALL_status_t RevProc::ECALL_syslog(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: syslog called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 117, rev_ptrace(long request, long pid, unsigned long addr, unsigned long data)
RevProc::ECALL_status_t RevProc::ECALL_ptrace(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: ptrace called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 118, rev_sched_setparam(pid_t pid, struct sched_param  *param)
RevProc::ECALL_status_t RevProc::ECALL_sched_setparam(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: sched_setparam called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 119, rev_sched_setscheduler(pid_t pid, int policy, struct sched_param  *param)
RevProc::ECALL_status_t RevProc::ECALL_sched_setscheduler(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: sched_setscheduler called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 120, rev_sched_getscheduler(pid_t pid)
RevProc::ECALL_status_t RevProc::ECALL_sched_getscheduler(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: sched_getscheduler called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 121, rev_sched_getparam(pid_t pid, struct sched_param  *param)
RevProc::ECALL_status_t RevProc::ECALL_sched_getparam(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: sched_getparam called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 122, rev_sched_setaffinity(pid_t pid, unsigned int len, unsigned long  *user_mask_ptr)
RevProc::ECALL_status_t RevProc::ECALL_sched_setaffinity(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: sched_setaffinity called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 123, rev_sched_getaffinity(pid_t pid, unsigned int len, unsigned long  *user_mask_ptr)
RevProc::ECALL_status_t RevProc::ECALL_sched_getaffinity(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: sched_getaffinity called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 124, rev_sched_yield(void)
RevProc::ECALL_status_t RevProc::ECALL_sched_yield(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: sched_yield called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 125, rev_sched_get_priority_max(int policy)
RevProc::ECALL_status_t RevProc::ECALL_sched_get_priority_max(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: sched_get_priority_max called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 126, rev_sched_get_priority_min(int policy)
RevProc::ECALL_status_t RevProc::ECALL_sched_get_priority_min(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: sched_get_priority_min called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 127, rev_sched_rr_get_interval(pid_t pid, struct __kernel_timespec  *interval)
RevProc::ECALL_status_t RevProc::ECALL_sched_rr_get_interval(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: sched_rr_get_interval called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 128, rev_restart_syscall(void)
RevProc::ECALL_status_t RevProc::ECALL_restart_syscall(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: restart_syscall called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 129, rev_kill(pid_t pid, int sig)
RevProc::ECALL_status_t RevProc::ECALL_kill(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: kill called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 130, rev_tkill(pid_t pid, int sig)
RevProc::ECALL_status_t RevProc::ECALL_tkill(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: tkill called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 131, rev_tgkill(pid_t tgid, pid_t pid, int sig)
RevProc::ECALL_status_t RevProc::ECALL_tgkill(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: tgkill called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 132, rev_sigaltstack(const struct sigaltstack  *uss, struct sigaltstack  *uoss)
RevProc::ECALL_status_t RevProc::ECALL_sigaltstack(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: sigaltstack called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 133, rev_rt_sigsuspend(sigset_t  *unewset, size_t sigsetsize)
RevProc::ECALL_status_t RevProc::ECALL_rt_sigsuspend(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: rt_sigsuspend called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 134, rev_rt_sigaction(int, const struct sigaction  *, struct sigaction  *, size_t)
RevProc::ECALL_status_t RevProc::ECALL_rt_sigaction(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: rt_sigaction called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 135, rev_rt_sigprocmask(int how, sigset_t  *set, sigset_t  *oset, size_t sigsetsize)
RevProc::ECALL_status_t RevProc::ECALL_rt_sigprocmask(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: rt_sigprocmask called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 136, rev_rt_sigpending(sigset_t  *set, size_t sigsetsize)
RevProc::ECALL_status_t RevProc::ECALL_rt_sigpending(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: rt_sigpending called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 137, rev_rt_sigtimedwait_time32(const sigset_t  *uthese, siginfo_t  *uinfo, const struct old_timespec32  *uts, size_t sigsetsize)
RevProc::ECALL_status_t RevProc::ECALL_rt_sigtimedwait_time32(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: rt_sigtimedwait_time32 called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 138, rev_rt_sigqueueinfo(pid_t pid, int sig, siginfo_t  *uinfo)
RevProc::ECALL_status_t RevProc::ECALL_rt_sigqueueinfo(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: rt_sigqueueinfo called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 140, rev_setpriority(int which, int who, int niceval)
RevProc::ECALL_status_t RevProc::ECALL_setpriority(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: setpriority called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 141, rev_getpriority(int which, int who)
RevProc::ECALL_status_t RevProc::ECALL_getpriority(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: getpriority called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 142, rev_reboot(int magic1, int magic2, unsigned int cmd, void  *arg)
RevProc::ECALL_status_t RevProc::ECALL_reboot(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: reboot called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 143, rev_setregid(gid_t rgid, gid_t egid)
RevProc::ECALL_status_t RevProc::ECALL_setregid(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: setregid called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 144, rev_setgid(gid_t gid)
RevProc::ECALL_status_t RevProc::ECALL_setgid(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: setgid called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 145, rev_setreuid(uid_t ruid, uid_t euid)
RevProc::ECALL_status_t RevProc::ECALL_setreuid(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: setreuid called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 146, rev_setuid(uid_t uid)
RevProc::ECALL_status_t RevProc::ECALL_setuid(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: setuid called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 147, rev_setresuid(uid_t ruid, uid_t euid, uid_t suid)
RevProc::ECALL_status_t RevProc::ECALL_setresuid(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: setresuid called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 148, rev_getresuid(uid_t  *ruid, uid_t  *euid, uid_t  *suid)
RevProc::ECALL_status_t RevProc::ECALL_getresuid(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: getresuid called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 149, rev_setresgid(gid_t rgid, gid_t egid, gid_t sgid)
RevProc::ECALL_status_t RevProc::ECALL_setresgid(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: setresgid called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 150, rev_getresgid(gid_t  *rgid, gid_t  *egid, gid_t  *sgid)
RevProc::ECALL_status_t RevProc::ECALL_getresgid(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: getresgid called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 151, rev_setfsuid(uid_t uid)
RevProc::ECALL_status_t RevProc::ECALL_setfsuid(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: setfsuid called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 152, rev_setfsgid(gid_t gid)
RevProc::ECALL_status_t RevProc::ECALL_setfsgid(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: setfsgid called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 153, rev_times(struct tms  *tbuf)
RevProc::ECALL_status_t RevProc::ECALL_times(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: times called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 154, rev_setpgid(pid_t pid, pid_t pgid)
RevProc::ECALL_status_t RevProc::ECALL_setpgid(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: setpgid called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 155, rev_getpgid(pid_t pid)
RevProc::ECALL_status_t RevProc::ECALL_getpgid(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: getpgid called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 156, rev_getsid(pid_t pid)
RevProc::ECALL_status_t RevProc::ECALL_getsid(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: getsid called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 157, rev_setsid(void)
RevProc::ECALL_status_t RevProc::ECALL_setsid(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: setsid called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 158, rev_getgroups(int gidsetsize, gid_t  *grouplist)
RevProc::ECALL_status_t RevProc::ECALL_getgroups(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: getgroups called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 159, rev_setgroups(int gidsetsize, gid_t  *grouplist)
RevProc::ECALL_status_t RevProc::ECALL_setgroups(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: setgroups called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 160, rev_newuname(struct new_utsname  *name)
RevProc::ECALL_status_t RevProc::ECALL_newuname(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: newuname called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 161, rev_sethostname(char  *name, int len)
RevProc::ECALL_status_t RevProc::ECALL_sethostname(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: sethostname called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 162, rev_setdomainname(char  *name, int len)
RevProc::ECALL_status_t RevProc::ECALL_setdomainname(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: setdomainname called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 163, rev_getrlimit(unsigned int resource, struct rlimit  *rlim)
RevProc::ECALL_status_t RevProc::ECALL_getrlimit(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: getrlimit called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 164, rev_setrlimit(unsigned int resource, struct rlimit  *rlim)
RevProc::ECALL_status_t RevProc::ECALL_setrlimit(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: setrlimit called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 165, rev_getrusage(int who, struct rusage  *ru)
RevProc::ECALL_status_t RevProc::ECALL_getrusage(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: getrusage called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 166, rev_umask(int mask)
RevProc::ECALL_status_t RevProc::ECALL_umask(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: umask called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 167, rev_prctl(int option, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5)
RevProc::ECALL_status_t RevProc::ECALL_prctl(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: prctl called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 168, rev_getcpu(unsigned  *cpu, unsigned  *node, struct getcpu_cache  *cache)
RevProc::ECALL_status_t RevProc::ECALL_getcpu(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: getcpu called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 169, rev_gettimeofday(struct __kernel_old_timeval  *tv, struct timezone  *tz)
RevProc::ECALL_status_t RevProc::ECALL_gettimeofday(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: gettimeofday called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 170, rev_settimeofday(struct __kernel_old_timeval  *tv, struct timezone  *tz)
RevProc::ECALL_status_t RevProc::ECALL_settimeofday(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: settimeofday called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 171, rev_adjtimex(struct __kernel_timex  *txc_p)
RevProc::ECALL_status_t RevProc::ECALL_adjtimex(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: adjtimex called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 172, rev_getpid(void)
RevProc::ECALL_status_t RevProc::ECALL_getpid(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: getpid called (Rev only supports a single process)\n");
  return ECALL_status_t::SUCCESS;
}

//  173, rev_getppid(void)
RevProc::ECALL_status_t RevProc::ECALL_getppid(RevInst& inst){
  // TODO: Implement error handling
  output->verbose(CALL_INFO, 2, 0, "ECALL: getppid called (Rev only supports a single process)\n");
  return ECALL_status_t::SUCCESS;
}

// 174, rev_getuid(void)
RevProc::ECALL_status_t RevProc::ECALL_getuid(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: getuid called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 175, rev_geteuid(void)
RevProc::ECALL_status_t RevProc::ECALL_geteuid(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: geteuid called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 176, rev_getgid(void)
RevProc::ECALL_status_t RevProc::ECALL_getgid(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: getgid called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 177, rev_getegid(void)
RevProc::ECALL_status_t RevProc::ECALL_getegid(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: getegid called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 178, rev_gettid(void)
RevProc::ECALL_status_t RevProc::ECALL_gettid(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: gettid called by thread %" PRIu32 "\n", GetActiveThreadID());

  /* rc = Currently Executing Hart */
  RegFile->RV64[10] = AssignedThreads.at(HartToExec)->GetThreadID();
  return ECALL_status_t::SUCCESS;
}

// 179, rev_sysinfo(struct sysinfo  *info)
RevProc::ECALL_status_t RevProc::ECALL_sysinfo(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: sysinfo called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 180, rev_mq_open(const char  *name, int oflag, umode_t mode, struct mq_attr  *attr)
RevProc::ECALL_status_t RevProc::ECALL_mq_open(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: mq_open called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 181, rev_mq_unlink(const char  *name)
RevProc::ECALL_status_t RevProc::ECALL_mq_unlink(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: mq_unlink called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 182, rev_mq_timedsend(mqd_t mqdes, const char  *msg_ptr, size_t msg_len, unsigned int msg_prio, const struct __kernel_timespec  *abs_timeout)
RevProc::ECALL_status_t RevProc::ECALL_mq_timedsend(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: mq_timedsend called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 183, rev_mq_timedreceive(mqd_t mqdes, char  *msg_ptr, size_t msg_len, unsigned int  *msg_prio, const struct __kernel_timespec  *abs_timeout)
RevProc::ECALL_status_t RevProc::ECALL_mq_timedreceive(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: mq_timedreceive called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 184, rev_mq_notify(mqd_t mqdes, const struct sigevent  *notification)
RevProc::ECALL_status_t RevProc::ECALL_mq_notify(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: mq_notify called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 185, rev_mq_getsetattr(mqd_t mqdes, const struct mq_attr  *mqstat, struct mq_attr  *omqstat)
RevProc::ECALL_status_t RevProc::ECALL_mq_getsetattr(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: mq_getsetattr called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 186, rev_msgget(key_t key, int msgflg)
RevProc::ECALL_status_t RevProc::ECALL_msgget(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: msgget called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 187, rev_old_msgctl(int msqid, int cmd, struct msqid_ds  *buf)
RevProc::ECALL_status_t RevProc::ECALL_msgctl(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: msgctl called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 188, rev_msgrcv(int msqid, struct msgbuf  *msgp, size_t msgsz, long msgtyp, int msgflg)
RevProc::ECALL_status_t RevProc::ECALL_msgrcv(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: msgrcv called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 189, rev_msgsnd(int msqid, struct msgbuf  *msgp, size_t msgsz, int msgflg)
RevProc::ECALL_status_t RevProc::ECALL_msgsnd(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: msgsnd called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 190, rev_semget(key_t key, int nsems, int semflg)
RevProc::ECALL_status_t RevProc::ECALL_semget(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: semget called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 191, rev_semctl(int semid, int semnum, int cmd, unsigned long arg)
RevProc::ECALL_status_t RevProc::ECALL_semctl(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: semctl called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 192, rev_semtimedop(int semid, struct sembuf  *sops, unsigned nsops, const struct __kernel_timespec  *timeout)
RevProc::ECALL_status_t RevProc::ECALL_semtimedop(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: semtimedop called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 193, rev_semop(int semid, struct sembuf  *sops, unsigned nsops)
RevProc::ECALL_status_t RevProc::ECALL_semop(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: semop called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 194, rev_shmget(key_t key, size_t size, int flag)
RevProc::ECALL_status_t RevProc::ECALL_shmget(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: shmget called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 195, rev_old_shmctl(int shmid, int cmd, struct shmid_ds  *buf)
RevProc::ECALL_status_t RevProc::ECALL_shmctl(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: shmctl called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 196, rev_shmat(int shmid, char  *shmaddr, int shmflg)
RevProc::ECALL_status_t RevProc::ECALL_shmat(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: shmat called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 197, rev_shmdt(char  *shmaddr)
RevProc::ECALL_status_t RevProc::ECALL_shmdt(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: shmdt called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 198, rev_socket(int, int, int)
RevProc::ECALL_status_t RevProc::ECALL_socket(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: socket called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 199, rev_socketpair(int, int, int, int  *)
RevProc::ECALL_status_t RevProc::ECALL_socketpair(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: socketpair called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 200, rev_bind(int, struct sockaddr  *, int)
RevProc::ECALL_status_t RevProc::ECALL_bind(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: bind called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 201, rev_listen(int, int)
RevProc::ECALL_status_t RevProc::ECALL_listen(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: listen called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 202, rev_accept(int, struct sockaddr  *, int  *)
RevProc::ECALL_status_t RevProc::ECALL_accept(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: accept called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 203, rev_connect(int, struct sockaddr  *, int)
RevProc::ECALL_status_t RevProc::ECALL_connect(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: connect called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 204, rev_getsockname(int, struct sockaddr  *, int  *)
RevProc::ECALL_status_t RevProc::ECALL_getsockname(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: getsockname called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 205, rev_getpeername(int, struct sockaddr  *, int  *)
RevProc::ECALL_status_t RevProc::ECALL_getpeername(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: getpeername called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 206, rev_sendto(int, void  *, size_t, unsigned, struct sockaddr  *, int)
RevProc::ECALL_status_t RevProc::ECALL_sendto(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: sendto called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 207, rev_recvfrom(int, void  *, size_t, unsigned, struct sockaddr  *, int  *)
RevProc::ECALL_status_t RevProc::ECALL_recvfrom(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: recvfrom called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 208, rev_setsockopt(int fd, int level, int optname, char  *optval, int optlen)
RevProc::ECALL_status_t RevProc::ECALL_setsockopt(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: setsockopt called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 209, rev_getsockopt(int fd, int level, int optname, char  *optval, int  *optlen)
RevProc::ECALL_status_t RevProc::ECALL_getsockopt(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: getsockopt called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 210, rev_shutdown(int, int)
RevProc::ECALL_status_t RevProc::ECALL_shutdown(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: shutdown called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 211, rev_sendmsg(int fd, struct user_msghdr  *msg, unsigned flags)
RevProc::ECALL_status_t RevProc::ECALL_sendmsg(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: sendmsg called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 212, rev_recvmsg(int fd, struct user_msghdr  *msg, unsigned flags)
RevProc::ECALL_status_t RevProc::ECALL_recvmsg(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: recvmsg called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 213, rev_readahead(int fd, loff_t offset, size_t count)
RevProc::ECALL_status_t RevProc::ECALL_readahead(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: readahead called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 214, rev_brk(unsigned long brk)
RevProc::ECALL_status_t RevProc::ECALL_brk(RevInst& inst){
  auto Addr = RegFile->GetX<uint64_t>(10);

  const uint64_t heapend = mem->GetHeapEnd();
  if( Addr > 0 && Addr > heapend ){
    uint64_t Size = Addr - heapend;
    mem->ExpandHeap(Size);
  } else {
    output->fatal(CALL_INFO, 11,
                  "Out of memory / Unable to expand system break (brk) to Addr = 0x%lx", Addr);
  }
  return ECALL_status_t::SUCCESS;
}

// 215, rev_munmap(unsigned long addr, size_t len)
RevProc::ECALL_status_t RevProc::ECALL_munmap(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: munmap called\n");
  auto Addr = RegFile->GetX<uint64_t>(10);
  auto Size = RegFile->GetX<uint64_t>(11);

  int rc =  mem->DeallocMem(Addr, Size) == uint64_t(-1);
  if(rc == -1){
    output->fatal(CALL_INFO, 11,
                  "Failed to perform munmap(Addr = 0x%lx, Size = 0x%lx)"
                  "likely because the memory was not allocated to begin with" ,
                  Addr, Size);
  }

  RegFile->SetX(10, rc);
  return ECALL_status_t::SUCCESS;
}

// 216, rev_mremap(unsigned long addr, unsigned long old_len, unsigned long new_len, unsigned long flags, unsigned long new_addr)
RevProc::ECALL_status_t RevProc::ECALL_mremap(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: mremap called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 217, rev_add_key(const char  *_type, const char  *_description, const void  *_payload, size_t plen, key_serial_t destringid)
RevProc::ECALL_status_t RevProc::ECALL_add_key(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: add_key called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 218, rev_request_key(const char  *_type, const char  *_description, const char  *_callout_info, key_serial_t destringid)
RevProc::ECALL_status_t RevProc::ECALL_request_key(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: request_key called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 219, rev_keyctl(int cmd, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5)
RevProc::ECALL_status_t RevProc::ECALL_keyctl(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: keyctl called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// TODO: Add ThreadManager Logic
// TODO: Figure out the difference between this and clone3
// 220, rev_clone(unsigned long, unsigned long, int  *, unsigned long, int  *)
RevProc::ECALL_status_t RevProc::ECALL_clone(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: clone called by thread %" PRIu32 "\n", GetActiveThreadID());
  auto rtval = ECALL_status_t::SUCCESS;
 //  auto CloneArgsAddr = RegFile->GetX<uint64_t>(10);
 //  // auto SizeOfCloneArgs = RegFile()->GetX<size_t>(11);

 // if(0 == ECALL.bytesRead){
 //    // First time through the function...
 //    /* Fetch the clone_args */
 //    // struct clone_args args;  // So while clone_args is a whole struct, we appear to be only
 //                                // using the 1st uint64, so that's all we're going to fetch
 //   uint64_t* args = reinterpret_cast<uint64_t*>(ECALL.buf.data());
 //   mem->ReadVal<uint64_t>(HartToExec, CloneArgsAddr, args, inst.hazard, REVMEM_FLAGS(0x00));
 //   ECALL.bytesRead = sizeof(*args);
 //   rtval = ECALL_status_t::CONTINUE;
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

 //    /* TODO: Create a copy of Parents Memory Space (need Demand Paging first) */

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
 //    RegFile->SetX(10, ChildPID);

 //    // Child's return value is 0
 //    ChildCtx->GetRegFile()->SetX(10, 0);

 //    // clean up ecall state
 //    rtval = RevProc::ECALL_status_t::SUCCESS;
 //    ECALL.bytesRead = 0;

 //  } //else
  return rtval;
}

// 221, rev_execve(const char  *filename, const char  *const  *argv, const char  *const  *envp)
RevProc::ECALL_status_t RevProc::ECALL_execve(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: execve called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 222, rev_old_mmap(struct mmap_arg_struct  *arg)
RevProc::ECALL_status_t RevProc::ECALL_mmap(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: mmap called\n");

  auto addr = RegFile->GetX<uint64_t>(10);
  auto size = RegFile->GetX<uint64_t>(11);
  // auto prot = RegFile->GetX<int>(12);
  // auto Flags = RegFile->GetX<int>(13);
  // auto fd = RegFile->GetX<int>(14);
  // auto offset = RegFile->GetX<off_t>(15);

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
      output->fatal(CALL_INFO, 11, "Failed to add mem segment\n");
    }
  }
  RegFile->SetX(10, addr);
  return ECALL_status_t::SUCCESS;
}

// 223, rev_fadvise64_64(int fd, loff_t offset, loff_t len, int advice)
RevProc::ECALL_status_t RevProc::ECALL_fadvise64_64(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fadvise64_64 called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 224, rev_swapon(const char  *specialfile, int swap_flags)
RevProc::ECALL_status_t RevProc::ECALL_swapon(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: swapon called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 225, rev_swapoff(const char  *specialfile)
RevProc::ECALL_status_t RevProc::ECALL_swapoff(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: swapoff called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 226, rev_mprotect(unsigned long start, size_t len, unsigned long prot)
RevProc::ECALL_status_t RevProc::ECALL_mprotect(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: mprotect called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 227, rev_msync(unsigned long start, size_t len, int flags)
RevProc::ECALL_status_t RevProc::ECALL_msync(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: msync called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 228, rev_mlock(unsigned long start, size_t len)
RevProc::ECALL_status_t RevProc::ECALL_mlock(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: mlock called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 229, rev_munlock(unsigned long start, size_t len)
RevProc::ECALL_status_t RevProc::ECALL_munlock(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: munlock called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 230, rev_mlockall(int flags)
RevProc::ECALL_status_t RevProc::ECALL_mlockall(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: mlockall called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 231, rev_munlockall(void)
RevProc::ECALL_status_t RevProc::ECALL_munlockall(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: munlockall called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 232, rev_mincore(unsigned long start, size_t len, unsigned char  * vec)
RevProc::ECALL_status_t RevProc::ECALL_mincore(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: mincore called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 233, rev_madvise(unsigned long start, size_t len, int behavior)
RevProc::ECALL_status_t RevProc::ECALL_madvise(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: madvise called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 234, rev_remap_file_pages(unsigned long start, unsigned long size, unsigned long prot, unsigned long pgoff, unsigned long flags)
RevProc::ECALL_status_t RevProc::ECALL_remap_file_pages(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: remap_file_pages called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 235, rev_mbind(unsigned long start, unsigned long len, unsigned long mode, const unsigned long  *nmask, unsigned long maxnode, unsigned flags)
RevProc::ECALL_status_t RevProc::ECALL_mbind(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: mbind called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 236, rev_get_mempolicy(int  *policy, unsigned long  *nmask, unsigned long maxnode, unsigned long addr, unsigned long flags)
RevProc::ECALL_status_t RevProc::ECALL_get_mempolicy(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: get_mempolicy called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 237, rev_set_mempolicy(int mode, const unsigned long  *nmask, unsigned long maxnode)
RevProc::ECALL_status_t RevProc::ECALL_set_mempolicy(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: set_mempolicy called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 238, rev_migrate_pages(pid_t pid, unsigned long maxnode, const unsigned long  *from, const unsigned long  *to)
RevProc::ECALL_status_t RevProc::ECALL_migrate_pages(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: migrate_pages called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 239, rev_move_pages(pid_t pid, unsigned long nr_pages, const void  *  *pages, const int  *nodes, int  *status, int flags)
RevProc::ECALL_status_t RevProc::ECALL_move_pages(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: move_pages called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 240, rev_rt_tgsigqueueinfo(pid_t tgid, pid_t pid, int sig, siginfo_t  *uinfo)
RevProc::ECALL_status_t RevProc::ECALL_rt_tgsigqueueinfo(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: rt_tgsigqueueinfo called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 241, rev_perf_event_open(")
RevProc::ECALL_status_t RevProc::ECALL_perf_event_open(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: perf_event_open called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 242, rev_accept4(int, struct sockaddr  *, int  *, int)
RevProc::ECALL_status_t RevProc::ECALL_accept4(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: accept4 called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 243, rev_recvmmsg_time32(int fd, struct mmsghdr  *msg, unsigned int vlen, unsigned flags, struct old_timespec32  *timeout)
RevProc::ECALL_status_t RevProc::ECALL_recvmmsg_time32(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: recvmmsg_time32 called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 260, rev_wait4(pid_t pid, int  *stat_addr, int options, struct rusage  *ru)
RevProc::ECALL_status_t RevProc::ECALL_wait4(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: wait4 called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 261, rev_prlimit64(pid_t pid, unsigned int resource, const struct rlimit64  *new_rlim, struct rlimit64  *old_rlim)
RevProc::ECALL_status_t RevProc::ECALL_prlimit64(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: prlimit64 called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 262, rev_fanotify_init(unsigned int flags, unsigned int event_f_flags)
RevProc::ECALL_status_t RevProc::ECALL_fanotify_init(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fanotify_init called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 263, rev_fanotify_mark(int fanotify_fd, unsigned int flags, u64 mask, int fd, const char  *pathname)
RevProc::ECALL_status_t RevProc::ECALL_fanotify_mark(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fanotify_mark called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 264, rev_name_to_handle_at(int dfd, const char  *name, struct file_handle  *handle, int  *mnt_id, int flag)
RevProc::ECALL_status_t RevProc::ECALL_name_to_handle_at(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: name_to_handle_at called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 265, rev_open_by_handle_at(int mountdirfd, struct file_handle  *handle, int flags)
RevProc::ECALL_status_t RevProc::ECALL_open_by_handle_at(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: open_by_handle_at called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 266, rev_clock_adjtime(clockid_t which_clock, struct __kernel_timex  *tx)
RevProc::ECALL_status_t RevProc::ECALL_clock_adjtime(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: clock_adjtime called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 267, rev_syncfs(int fd)
RevProc::ECALL_status_t RevProc::ECALL_syncfs(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: syncfs called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 268, rev_setns(int fd, int nstype)
RevProc::ECALL_status_t RevProc::ECALL_setns(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: setns called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 269, rev_sendmmsg(int fd, struct mmsghdr  *msg, unsigned int vlen, unsigned flags)
RevProc::ECALL_status_t RevProc::ECALL_sendmmsg(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: sendmmsg called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 270, rev_process_vm_readv(pid_t pid, const struct iovec  *lvec, unsigned long liovcnt, const struct iovec  *rvec, unsigned long riovcnt, unsigned long flags)
RevProc::ECALL_status_t RevProc::ECALL_process_vm_readv(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: process_vm_readv called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 271, rev_process_vm_writev(pid_t pid, const struct iovec  *lvec, unsigned long liovcnt, const struct iovec  *rvec, unsigned long riovcnt, unsigned long flags)
RevProc::ECALL_status_t RevProc::ECALL_process_vm_writev(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: process_vm_writev called by thread %" PRIu32 "\n", GetActiveThreadID());

  return ECALL_status_t::SUCCESS;
}

// 272, rev_kcmp(pid_t pid1, pid_t pid2, int type, unsigned long idx1, unsigned long idx2)
RevProc::ECALL_status_t RevProc::ECALL_kcmp(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: kcmp called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 273, rev_finit_module(int fd, const char  *uargs, int flags)
RevProc::ECALL_status_t RevProc::ECALL_finit_module(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: finit_module called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 274, rev_sched_setattr(pid_t pid, struct sched_attr  *attr, unsigned int flags)
RevProc::ECALL_status_t RevProc::ECALL_sched_setattr(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: sched_setattr called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 275, rev_sched_getattr(pid_t pid, struct sched_attr  *attr, unsigned int size, unsigned int flags)
RevProc::ECALL_status_t RevProc::ECALL_sched_getattr(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: sched_getattr called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 276, rev_renameat2(int olddfd, const char  *oldname, int newdfd, const char  *newname, unsigned int flags)
RevProc::ECALL_status_t RevProc::ECALL_renameat2(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: renameat2 called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 277, rev_seccomp(unsigned int op, unsigned int flags, void  *uargs)
RevProc::ECALL_status_t RevProc::ECALL_seccomp(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: seccomp called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 278, rev_getrandom(char  *buf, size_t count, unsigned int flags)
RevProc::ECALL_status_t RevProc::ECALL_getrandom(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: getrandom called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 279, rev_memfd_create(const char  *uname_ptr, unsigned int flags)
RevProc::ECALL_status_t RevProc::ECALL_memfd_create(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: memfd_create called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 280, rev_bpf(int cmd, union bpf_attr *attr, unsigned int size)
RevProc::ECALL_status_t RevProc::ECALL_bpf(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: bpf called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 281, rev_execveat(int dfd, const char  *filename, const char  *const  *argv, const char  *const  *envp, int flags)
RevProc::ECALL_status_t RevProc::ECALL_execveat(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: execveat called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 282, rev_userfaultfd(int flags)
RevProc::ECALL_status_t RevProc::ECALL_userfaultfd(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: userfaultfd called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 283, rev_membarrier(int cmd, unsigned int flags, int cpu_id)
RevProc::ECALL_status_t RevProc::ECALL_membarrier(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: membarrier called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 284, rev_mlock2(unsigned long start, size_t len, int flags)
RevProc::ECALL_status_t RevProc::ECALL_mlock2(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: mlock2 called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 285, rev_copy_file_range(int fd_in, loff_t  *off_in, int fd_out, loff_t  *off_out, size_t len, unsigned int flags)
RevProc::ECALL_status_t RevProc::ECALL_copy_file_range(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: copy_file_range called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 286, rev_preadv2(unsigned long fd, const struct iovec  *vec, unsigned long vlen, unsigned long pos_l, unsigned long pos_h, rwf_t flags)
RevProc::ECALL_status_t RevProc::ECALL_preadv2(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: preadv2 called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 287, rev_pwritev2(unsigned long fd, const struct iovec  *vec, unsigned long vlen, unsigned long pos_l, unsigned long pos_h, rwf_t flags)
RevProc::ECALL_status_t RevProc::ECALL_pwritev2(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: pwritev2 called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 288, rev_pkey_mprotect(unsigned long start, size_t len, unsigned long prot, int pkey)
RevProc::ECALL_status_t RevProc::ECALL_pkey_mprotect(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: pkey_mprotect called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 289, rev_pkey_alloc(unsigned long flags, unsigned long init_val)
RevProc::ECALL_status_t RevProc::ECALL_pkey_alloc(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: pkey_alloc called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 290, rev_pkey_free(int pkey)
RevProc::ECALL_status_t RevProc::ECALL_pkey_free(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: pkey_free called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 291, rev_statx(int dfd, const char  *path, unsigned flags, unsigned mask, struct statx  *buffer)
RevProc::ECALL_status_t RevProc::ECALL_statx(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: statx called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 292, rev_io_pgetevents(aio_context_t ctx_id, long min_nr, long nr, struct io_event  *events, struct __kernel_timespec  *timeout, const struct __aio_sigset *sig)
RevProc::ECALL_status_t RevProc::ECALL_io_pgetevents(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: io_pgetevents called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 293, rev_rseq(struct rseq  *rseq, uint32_t rseq_len, int flags, uint32_t sig)
RevProc::ECALL_status_t RevProc::ECALL_rseq(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: rseq called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 294, rev_kexec_file_load(int kernel_fd, int initrd_fd, unsigned long cmdline_len, const char  *cmdline_ptr, unsigned long flags)
RevProc::ECALL_status_t RevProc::ECALL_kexec_file_load(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: kexec_file_load called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// // 403, rev_clock_gettime(clockid_t which_clock, struct __kernel_timespec  *tp)
// RevProc::ECALL_status_t RevProc::ECALL_clock_gettime(RevInst& inst){
//   output->verbose(CALL_INFO, 2, 0, "ECALL: clock_gettime called by thread %" PRIu32 "\n", GetActiveThreadID());
//   return ECALL_status_t::SUCCESS;
// }

// // 404, rev_clock_settime(clockid_t which_clock, const struct __kernel_timespec  *tp)
// RevProc::ECALL_status_t RevProc::ECALL_clock_settime(RevInst& inst){
//   output->verbose(CALL_INFO, 2, 0, "ECALL: clock_settime called by thread %" PRIu32 "\n", GetActiveThreadID());
//   return ECALL_status_t::SUCCESS;
// }

// // 405, rev_clock_adjtime(clockid_t which_clock, struct __kernel_timex  *tx)
// RevProc::ECALL_status_t RevProc::ECALL_clock_adjtime(RevInst& inst){
//   output->verbose(CALL_INFO, 2, 0, "ECALL: clock_adjtime called by thread %" PRIu32 "\n", GetActiveThreadID());
//   return ECALL_status_t::SUCCESS;
// }

// // 406, rev_clock_getres(clockid_t which_clock, struct __kernel_timespec  *tp)
// RevProc::ECALL_status_t RevProc::ECALL_clock_getres(RevInst& inst){
//   output->verbose(CALL_INFO, 2, 0, "ECALL: clock_getres called by thread %" PRIu32 "\n", GetActiveThreadID());
//   return ECALL_status_t::SUCCESS;
// }

// // 407, rev_clock_nanosleep(clockid_t which_clock, int flags, const struct __kernel_timespec  *rqtp, struct __kernel_timespec  *rmtp)
// RevProc::ECALL_status_t RevProc::ECALL_clock_nanosleep(RevInst& inst){
//   output->verbose(CALL_INFO, 2, 0, "ECALL: clock_nanosleep called by thread %" PRIu32 "\n", GetActiveThreadID());
//   return ECALL_status_t::SUCCESS;
// }

// // 408, rev_timer_gettime(timer_t timer_id, struct __kernel_itimerspec  *setting)
// RevProc::ECALL_status_t RevProc::ECALL_timer_gettime(RevInst& inst){
//   output->verbose(CALL_INFO, 2, 0, "ECALL: timer_gettime called by thread %" PRIu32 "\n", GetActiveThreadID());
//   return ECALL_status_t::SUCCESS;
// }

// // 409, rev_timer_settime(timer_t timer_id, int flags, const struct __kernel_itimerspec  *new_setting, struct __kernel_itimerspec  *old_setting)
// RevProc::ECALL_status_t RevProc::ECALL_timer_settime(RevInst& inst){
//   output->verbose(CALL_INFO, 2, 0, "ECALL: timer_settime called by thread %" PRIu32 "\n", GetActiveThreadID());
//   return ECALL_status_t::SUCCESS;
// }

// // 410, rev_timerfd_gettime(int ufd, struct __kernel_itimerspec  *otmr)
// RevProc::ECALL_status_t RevProc::ECALL_timerfd_gettime(RevInst& inst){
//   output->verbose(CALL_INFO, 2, 0, "ECALL: timerfd_gettime called by thread %" PRIu32 "\n", GetActiveThreadID());
//   return ECALL_status_t::SUCCESS;
// }

// // 411, rev_timerfd_settime(int ufd, int flags, const struct __kernel_itimerspec  *utmr, struct __kernel_itimerspec  *otmr)
// RevProc::ECALL_status_t RevProc::ECALL_timerfd_settime(RevInst& inst){
//   output->verbose(CALL_INFO, 2, 0, "ECALL: timerfd_settime called by thread %" PRIu32 "\n", GetActiveThreadID());
//   return ECALL_status_t::SUCCESS;
// }

// // 412, rev_utimensat(int dfd, const char  *filename, struct __kernel_timespec  *utimes, int flags)
// RevProc::ECALL_status_t RevProc::ECALL_utimensat(RevInst& inst){
//   output->verbose(CALL_INFO, 2, 0, "ECALL: utimensat called by thread %" PRIu32 "\n", GetActiveThreadID());
//   return ECALL_status_t::SUCCESS;
// }

// // 416, rev_io_pgetevents(aio_context_t ctx_id, long min_nr, long nr, struct io_event  *events, struct __kernel_timespec  *timeout, const struct __aio_sigset *sig)
// RevProc::ECALL_status_t RevProc::ECALL_io_pgetevents(RevInst& inst){
//   output->verbose(CALL_INFO, 2, 0, "ECALL: io_pgetevents called by thread %" PRIu32 "\n", GetActiveThreadID());
//   return ECALL_status_t::SUCCESS;
// }

// // 418, rev_mq_timedsend(mqd_t mqdes, const char  *msg_ptr, size_t msg_len, unsigned int msg_prio, const struct __kernel_timespec  *abs_timeout)
// RevProc::ECALL_status_t RevProc::ECALL_mq_timedsend(RevInst& inst){
//   output->verbose(CALL_INFO, 2, 0, "ECALL: mq_timedsend called by thread %" PRIu32 "\n", GetActiveThreadID());
//   return ECALL_status_t::SUCCESS;
// }

// // 419, rev_mq_timedreceive(mqd_t mqdes, char  *msg_ptr, size_t msg_len, unsigned int  *msg_prio, const struct __kernel_timespec  *abs_timeout)
// RevProc::ECALL_status_t RevProc::ECALL_mq_timedreceive(RevInst& inst){
//   output->verbose(CALL_INFO, 2, 0, "ECALL: mq_timedreceive called by thread %" PRIu32 "\n", GetActiveThreadID());
//   return ECALL_status_t::SUCCESS;
// }

// // 420, rev_semtimedop(int semid, struct sembuf  *sops, unsigned nsops, const struct __kernel_timespec  *timeout)
// RevProc::ECALL_status_t RevProc::ECALL_semtimedop(RevInst& inst){
//   output->verbose(CALL_INFO, 2, 0, "ECALL: semtimedop called by thread %" PRIu32 "\n", GetActiveThreadID());
//   return ECALL_status_t::SUCCESS;
// }

// // 422, rev_futex(u32  *uaddr, int op, u32 val, struct __kernel_timespec  *utime, u32  *uaddr2, u32 val3)
// RevProc::ECALL_status_t RevProc::ECALL_futex(RevInst& inst){
//   output->verbose(CALL_INFO, 2, 0, "ECALL: futex called by thread %" PRIu32 "\n", GetActiveThreadID());
//   return ECALL_status_t::SUCCESS;
// }

// // 423, rev_sched_rr_get_interval(pid_t pid, struct __kernel_timespec  *interval)
// RevProc::ECALL_status_t RevProc::ECALL_sched_rr_get_interval(RevInst& inst){
//   output->verbose(CALL_INFO, 2, 0, "ECALL: sched_rr_get_interval called by thread %" PRIu32 "\n", GetActiveThreadID());
//   return ECALL_status_t::SUCCESS;
// }
//

// 424, rev_pidfd_send_signal(int pidfd, int sig, siginfo_t  *info, unsigned int flags)
RevProc::ECALL_status_t RevProc::ECALL_pidfd_send_signal(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: pidfd_send_signal called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 425, rev_io_uring_setup(u32 entries, struct io_uring_params  *p)
RevProc::ECALL_status_t RevProc::ECALL_io_uring_setup(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: io_uring_setup called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 426, rev_io_uring_enter(unsigned int fd, u32 to_submit, u32 min_complete, u32 flags, const sigset_t  *sig, size_t sigsz)
RevProc::ECALL_status_t RevProc::ECALL_io_uring_enter(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: io_uring_enter called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 427, rev_io_uring_register(unsigned int fd, unsigned int op, void  *arg, unsigned int nr_args)
RevProc::ECALL_status_t RevProc::ECALL_io_uring_register(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: io_uring_register called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 428, rev_open_tree(int dfd, const char  *path, unsigned flags)
RevProc::ECALL_status_t RevProc::ECALL_open_tree(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: open_tree called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 429, rev_move_mount(int from_dfd, const char  *from_path, int to_dfd, const char  *to_path, unsigned int ms_flags)
RevProc::ECALL_status_t RevProc::ECALL_move_mount(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: move_mount called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 430, rev_fsopen(const char  *fs_name, unsigned int flags)
RevProc::ECALL_status_t RevProc::ECALL_fsopen(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fsopen called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 431, rev_fsconfig(int fs_fd, unsigned int cmd, const char  *key, const void  *value, int aux)
RevProc::ECALL_status_t RevProc::ECALL_fsconfig(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fsconfig called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 432, rev_fsmount(int fs_fd, unsigned int flags, unsigned int ms_flags)
RevProc::ECALL_status_t RevProc::ECALL_fsmount(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fsmount called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 433, rev_fspick(int dfd, const char  *path, unsigned int flags)
RevProc::ECALL_status_t RevProc::ECALL_fspick(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fspick called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 434, rev_pidfd_open(pid_t pid, unsigned int flags)
RevProc::ECALL_status_t RevProc::ECALL_pidfd_open(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: pidfd_open called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 435, rev_clone3(struct clone_args  *uargs, size_t size)
RevProc::ECALL_status_t RevProc::ECALL_clone3(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: clone3 called by thread %" PRIu32 "\n", GetActiveThreadID());
  auto rtval = ECALL_status_t::SUCCESS;
 //  auto CloneArgsAddr = RegFile->GetX<uint64_t>(10);
 // auto SizeOfCloneArgs = RegFile()->GetX<size_t>(11);

 // if(0 == ECALL.bytesRead){
 //    // First time through the function...
 //    /* Fetch the clone_args */
 //    // struct clone_args args;  // So while clone_args is a whole struct, we appear to be only
 //                                // using the 1st uint64, so that's all we're going to fetch
 //   uint64_t* args = reinterpret_cast<uint64_t*>(ECALL.buf.data());
 //   mem->ReadVal<uint64_t>(HartToExec, CloneArgsAddr, args, inst.hazard, REVMEM_FLAGS(0x00));
 //   ECALL.bytesRead = sizeof(*args);
 //   rtval = ECALL_status_t::CONTINUE;
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

 //    /* TODO: Create a copy of Parents Memory Space (need Demand Paging first) */

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
 //    RegFile->SetX(10, ChildPID);

 //    // Child's return value is 0
 //    ChildCtx->GetRegFile()->SetX(10, 0);

 //    // clean up ecall state
 //    rtval = RevProc::ECALL_status_t::SUCCESS;
 //    ECALL.bytesRead = 0;

 //  } //else
  return rtval;
}

// 436, rev_close_range(unsigned int fd, unsigned int max_fd, unsigned int flags)
RevProc::ECALL_status_t RevProc::ECALL_close_range(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: close_range called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 437, rev_openat2(int dfd, const char  *filename, struct open_how *how, size_t size)
RevProc::ECALL_status_t RevProc::ECALL_openat2(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: openat2 called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 438, rev_pidfd_getfd(int pidfd, int fd, unsigned int flags)
RevProc::ECALL_status_t RevProc::ECALL_pidfd_getfd(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: pidfd_getfd called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}


// 439, rev_faccessat2(int dfd, const char  *filename, int mode, int flags)
RevProc::ECALL_status_t RevProc::ECALL_faccessat2(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: faccessat2 called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}


// 440, rev_process_madvise(int pidfd, const struct iovec  *vec, size_t vlen, int behavior, unsigned int flags)
RevProc::ECALL_status_t RevProc::ECALL_process_madvise(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: process_madvise called by thread %" PRIu32 "\n", GetActiveThreadID());
  return ECALL_status_t::SUCCESS;
}

// 1000, int pthread_create(pthread_t *restrict thread,
//                          const pthread_attr_t *restrict attr,
//                          void *(*start_routine)(void *),
//                          void *restrict arg);
RevProc::ECALL_status_t RevProc::ECALL_pthread_create(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: pthread_create called by thread %" PRIu32 "\n", GetActiveThreadID());
  uint64_t tidAddr     = RegFile->GetX<uint64_t>(10);
  uint64_t NewThreadPC = RegFile->GetX<uint64_t>(11);
  uint64_t ArgPtr      = RegFile->GetX<uint64_t>(12);
  unsigned long int NewTID = GetNewThreadID();
  CreateThread(NewTID, NewThreadPC, reinterpret_cast<void*>(ArgPtr));

  mem->WriteMem(feature->GetHartToExec(), tidAddr, sizeof(NewTID), &NewTID, REVMEM_FLAGS(0x00));
  return ECALL_status_t::SUCCESS;
}

// 1001, int rev_pthread_join(pthread_t thread, void **retval);
RevProc::ECALL_status_t RevProc::ECALL_pthread_join(RevInst& inst){
  RevProc::ECALL_status_t rtval = RevProc::ECALL_status_t::CONTINUE;
  output->verbose(CALL_INFO, 2, 0, "ECALL: pthread_join called by thread %" PRIu32 "\n", GetActiveThreadID());

  rtval = RevProc::ECALL_status_t::SUCCESS;

  // Set current thread to blocked
  AssignedThreads.at(HartToDecode)->SetState(ThreadState::BLOCKED);

  // Signal to RevCPU this thread is has changed state
  ThreadStateChanges.set(HartToDecode);

  // Output the ecall buf

  // Set the TID this thread is waiting for
  AssignedThreads.at(HartToDecode)->SetWaitingToJoinTID(RegFile->RV64[10]);

  // // if retval is not null, store the return value of the thread in retval
  // void **retval = (void **)RegFile->RV64[11];
  // if( retval != NULL ){
  //   *retval = (void *)
  //   AssignedThreads.at(HartToDecode)->GetRegFile()->RV64[10];
  // }
  //
  return rtval;
}
