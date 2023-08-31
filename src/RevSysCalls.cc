#include "../include/RevProc.h"
#include "RevMem.h"
#include <bitset>
#include <filesystem>
#include <sys/xattr.h>

// 0, rev_io_setup(unsigned nr_reqs, aio_context_t  *ctx) 
void RevProc::ECALL_io_setup(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: io_setup called\n");
  return;
}


// 1, rev_io_destroy(aio_context_t ctx) 
void RevProc::ECALL_io_destroy(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: io_destroy called\n");
  return;
}


// 2, rev_io_submit(aio_context_t, long, struct iocb  *  *) 
void RevProc::ECALL_io_submit(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: io_submit called\n");
  return;
}

// 3, rev_io_cancel(aio_context_t ctx_id, struct iocb  *iocb, struct io_event  *result) 
void RevProc::ECALL_io_cancel(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: io_cancel called\n");
  return;
}

// 4, rev_io_getevents(aio_context_t ctx_id, long min_nr, long nr, struct io_event  *events, struct __kernel_timespec  *timeout) 
void RevProc::ECALL_io_getevents(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: io_getevents called\n");
  return;
}

// 5, rev_setxattr(const char  *path, const char  *name, const void  *value, size_t size, int flags) 
void RevProc::ECALL_setxattr(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: setxattr called\n");
  const char *path = (char*)RegFile->RV64[10];
  const char *name = (char*)RegFile->RV64[11];
  const void *value = (void*)RegFile->RV64[12];
  size_t size = RegFile->RV64[13];
  uint64_t flags = RegFile->RV64[14];

#ifdef __APPLE__
  uint32_t position = 0;
  uint64_t rc = setxattr(path, name, value, size, position, flags);
#else
  uint64_t rc = setxattr(path, name, value, size, flags);
#endif
  RegFile->RV64[10] = rc;
  return;
}

// 6, rev_lsetxattr(const char  *path, const char  *name, const void  *value, size_t size, int flags) 
void RevProc::ECALL_lsetxattr(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: lsetxattr called\n");
  return;
}             

// 7, rev_fsetxattr(int fd, const char  *name, const void  *value, size_t size, int flags) 
void RevProc::ECALL_fsetxattr(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fsetxattr called\n");
  return;
}             

// 8, rev_getxattr(const char  *path, const char  *name, void  *value, size_t size) 
void RevProc::ECALL_getxattr(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: getxattr called\n");
  return;
}              

// 9, rev_lgetxattr(const char  *path, const char  *name, void  *value, size_t size) 
void RevProc::ECALL_lgetxattr(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: lgetxattr called\n");
  return;
}             

// 10, rev_fgetxattr(int fd, const char  *name, void  *value, size_t size) 
void RevProc::ECALL_fgetxattr(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fgetxattr called\n");
  return;
}             

// 11, rev_listxattr(const char  *path, char  *list, size_t size) 
void RevProc::ECALL_listxattr(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: listxattr called\n");
  return;
}             

// 12, rev_llistxattr(const char  *path, char  *list, size_t size) 
void RevProc::ECALL_llistxattr(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: llistxattr called\n");
  return;
}            

// 13, rev_flistxattr(int fd, char  *list, size_t size) 
void RevProc::ECALL_flistxattr(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: flistxattr called\n");
  return;
}            

// 14, rev_removexattr(const char  *path, const char  *name) 
void RevProc::ECALL_removexattr(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: removexattr called\n");
  return;
}           

// 15, rev_lremovexattr(const char  *path, const char  *name) 
void RevProc::ECALL_lremovexattr(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: lremovexattr called\n");
  return;
}          

// 16, rev_fremovexattr(int fd, const char  *name) 
void RevProc::ECALL_fremovexattr(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fremovexattr called\n");
  return;
}          

// 17, rev_getcwd(char  *buf, unsigned long size) 
void RevProc::ECALL_getcwd(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: getcwd called\n");
  uint64_t BufAddr = RegFile->RV64[10];
  uint64_t size = RegFile->RV64[11];
  std::string CWD = std::filesystem::current_path().c_str();
  mem->WriteMem(feature->GetHart(), BufAddr, size, &CWD);

  /* Returns null-terminated string in buf */
  RegFile->RV64[10] = BufAddr;

  return;
}

// 18, rev_lookup_dcookie(u64 cookie64, char  *buf, size_t len) 
void RevProc::ECALL_lookup_dcookie(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: lookup_dcookie called\n");
  return;
}        

// 19, rev_eventfd2(unsigned int count, int flags) 
void RevProc::ECALL_eventfd2(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: eventfd2 called\n");
  return;
}              

// 20, rev_epoll_create1(int flags) 
void RevProc::ECALL_epoll_create1(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: epoll_create1 called\n");
  return;
}         

// 21, rev_epoll_ctl(int epfd, int op, int fd, struct epoll_event  *event) 
void RevProc::ECALL_epoll_ctl(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: epoll_ctl called\n");
  return;
}             

// 22, rev_epoll_pwait(int epfd, struct epoll_event  *events, int maxevents, int timeout, const sigset_t  *sigmask, size_t sigsetsize) 
void RevProc::ECALL_epoll_pwait(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: epoll_pwait called\n");
  return;
}           

// 23, rev_dup(unsigned int fildes) 
void RevProc::ECALL_dup(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: dup called\n");
  return;
}                   

// 24, rev_dup3(unsigned int oldfd, unsigned int newfd, int flags) 
void RevProc::ECALL_dup3(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: dup3 called\n");
  return;
}                  

// 25, rev_fcntl64(unsigned int fd, unsigned int cmd, unsigned long arg) 
void RevProc::ECALL_fcntl64(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fcntl64 called\n");
  return;
}               

// 26, rev_inotify_init1(int flags) 
void RevProc::ECALL_inotify_init1(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: inotify_init1 called\n");
  return;
}         

// 27, rev_inotify_add_watch(int fd, const char  *path, u32 mask) 
void RevProc::ECALL_inotify_add_watch(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: inotify_add_watch called\n");
  return;
}     

// 28, rev_inotify_rm_watch(int fd, __s32 wd) 
void RevProc::ECALL_inotify_rm_watch(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: inotify_rm_watch called\n");
  return;
}      

// 29, rev_ioctl(unsigned int fd, unsigned int cmd, unsigned long arg) 
void RevProc::ECALL_ioctl(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: ioctl called\n");
  return;
}                 

// 30, rev_ioprio_set(int which, int who, int ioprio) 
void RevProc::ECALL_ioprio_set(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: ioprio_set called\n");
  return;
}            

// 31, rev_ioprio_get(int which, int who) 
void RevProc::ECALL_ioprio_get(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: ioprio_get called\n");
  return;
}            

// 32, rev_flock(unsigned int fd, unsigned int cmd) 
void RevProc::ECALL_flock(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: flock called\n");
  return;
}                 

// 33, rev_mknodat(int dfd, const char  * filename, umode_t mode, unsigned dev) 
void RevProc::ECALL_mknodat(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: mknodat called\n");
  return;
}               

// 34, rev_mkdirat(int dfd, const char  * pathname, umode_t mode) 
void RevProc::ECALL_mkdirat(){
  output->verbose(CALL_INFO, 2, 0, "ECALL_mkdirat called"); 

  unsigned fd = RegFile->RV64[10];
  unsigned Mode = RegFile->RV64[12];

  std::string path = "";
  unsigned i=0;
  
  // we don't know how long the path string is so read a byte (char)
  // at a time and search for the string terminator character '\0'
  do {
    char dirchar;
    // TODO: Reimpliment this with the new memory interface
    // mem->ReadVal(HartToExec, RegFile->RV64[11] + sizeof(char)*i, &dirchar, REVMEM_FLAGS(0));
    path = path + dirchar;
    i++;
  } while( path.back() != '\0');

  const int rc = mkdirat(fd, path.data(), Mode);
  RegFile->RV64[10] = rc;
  return;
}               

// 35, rev_unlinkat(int dfd, const char  * pathname, int flag) 
void RevProc::ECALL_unlinkat(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: unlinkat called\n");
  return;
}              

// 36, rev_symlinkat(const char  * oldname, int newdfd, const char  * newname) 
void RevProc::ECALL_symlinkat(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: symlinkat called\n");
  return;
}             

// 37, rev_unlinkat(int dfd, const char  * pathname, int flag) 
void RevProc::ECALL_linkat(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: linkat called\n");
  return;
}               

// 38, rev_renameat(int olddfd, const char  * oldname, int newdfd, const char  * newname) 
void RevProc::ECALL_renameat(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: renameat called\n");
  return;
}              

// 39, rev_umount(char  *name, int flags) 
void RevProc::ECALL_umount(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: umount called\n");
  return;
}                

// 40, rev_umount(char  *name, int flags) 
void RevProc::ECALL_mount(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: mount called\n");
  return;
}                 

// 41, rev_pivot_root(const char  *new_root, const char  *put_old) 
void RevProc::ECALL_pivot_root(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: pivot_root called\n");
  return;
}            

// 42, rev_ni_syscall(void) 
void RevProc::ECALL_ni_syscall(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: ni_syscall called\n");
  return;
}            

// 43, rev_statfs64(const char  *path, size_t sz, struct statfs64  *buf) 
void RevProc::ECALL_statfs64(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: statfs64 called\n");
  return;
}              

// 44, rev_fstatfs64(unsigned int fd, size_t sz, struct statfs64  *buf) 
void RevProc::ECALL_fstatfs64(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fstatfs64 called\n");
  return;
}             

// 45, rev_truncate64(const char  *path, loff_t length) 
void RevProc::ECALL_truncate64(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: truncate64 called\n");
  return;
}            

// 46, rev_ftruncate64(unsigned int fd, loff_t length) 
void RevProc::ECALL_ftruncate64(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: ftruncate64 called\n");
  return;
}           

// 47, rev_fallocate(int fd, int mode, loff_t offset, loff_t len) 
void RevProc::ECALL_fallocate(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fallocate called\n");
  return;
}             

// 48, rev_faccessat(int dfd, const char  *filename, int mode) 
void RevProc::ECALL_faccessat(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: faccessat called\n");
  return;
}             

// 49, rev_chdir(const char  *filename) 
void RevProc::ECALL_chdir(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: chdir called\n");
  std::string path = "";
  unsigned i=0;

  // we don't know how long the path string is so read a byte (char)
  // at a time and search for the string terminator character '\0'
  do {
    char dirchar;
    // mem->ReadVal(RegFile->RV64[10] + sizeof(char)*i, sizeof(char), &dirchar, REVMEM_FLAGS(0));
    path = path + dirchar;
    i++;
  } while( path.back() != '\0');

  const int rc = chdir(path.data());
  RegFile->RV64[10] = rc;
  return;
}                 

// 50, rev_fchdir(unsigned int fd) 
void RevProc::ECALL_fchdir(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fchdir called\n");
  return;
}                

// 51, rev_chroot(const char  *filename) 
void RevProc::ECALL_chroot(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: chroot called\n");
  return;
}                

// 52, rev_fchmod(unsigned int fd, umode_t mode) 
void RevProc::ECALL_fchmod(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fchmod called\n");
  return;
}                

// 53, rev_fchmodat(int dfd, const char  * filename, umode_t mode) 
void RevProc::ECALL_fchmodat(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fchmodat called\n");
  return;
}              

// 54, rev_fchownat(int dfd, const char  *filename, uid_t user, gid_t group, int flag) 
void RevProc::ECALL_fchownat(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fchownat called\n");
  return;
}              

// 55, rev_fchown(unsigned int fd, uid_t user, gid_t group) 
void RevProc::ECALL_fchown(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fchown called\n");
  return;
}                

// 56, rev_openat(int dfd, const char  *filename, int flags, umode_t mode) 
void RevProc::ECALL_openat(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: openat called\n");
  int dfd = RegFile->RV64[10];
  int filenameAddr = RegFile->RV64[11];
  // int flags = RegFile->RV64[12]; /* NOTE: Unused for now */
  // uint64_t mode = RegFile->RV64[13];

  /*
   * NOTE: this is currently only opening files in the current directory
   *       because of some oddities in parsing the arguments & flags
   *       but this will be fixed in the near future
  */


  /* Read the filename from memory one character at a time until we find '\0' */
  std::string filename = "";
  unsigned i = 0;
  do {
    char filenameChar;
    // mem->ReadVal(filenameAddr + sizeof(char)*i, sizeof(char), &filenameChar, REVMEM_FLAGS(0));
    filename = filename + filenameChar;
    i++;
  } while( filename.back() != '\0');

  dfd = open(std::filesystem::current_path().c_str(), O_RDONLY);

  /* Do the openat on the host */
  int fd = openat(dfd, filename.c_str(), O_RDWR);

  HartToExecCtx()->AddFD(fd);

  /* openat returns the file descriptor of the opened file */
  RegFile->RV64[10] = fd;
  return;
}                

// 57, rev_close(unsigned int fd) 
void RevProc::ECALL_close(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: close called\n");
  int fd = RegFile->RV64[10];
  std::shared_ptr<RevThread> CurrCtx = HartToExecCtx();

  /* Check if CurrCtx has fd in fildes vector */
  if( !CurrCtx->FindFD(fd) ){
    output->fatal(CALL_INFO, -1,
                  "Core %d; Hart %d; ThreadID %" PRIu32 " tried to close file descriptor %d but did not have access to it\n",
                  id, HartToExec, HartToExecThreadID(), fd);
    return;
  }
  /* Close file on host */
  uint64_t rc = close(fd);

  /* Remove from Ctx's fildes */
  CurrCtx->RemoveFD(fd);

  /* rc is propogated to rev from host */
  RegFile->RV64[10] = rc;

  return;
}                 

// 58, rev_vhangup(void) 
void RevProc::ECALL_vhangup(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: vhangup called\n");
  return;
}               

// 59, rev_pipe2(int  *fildes, int flags) 
void RevProc::ECALL_pipe2(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: pipe2 called\n");
  return;
}                 

// 60, rev_quotactl(unsigned int cmd, const char  *special, qid_t id, void  *addr) 
void RevProc::ECALL_quotactl(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: quotactl called\n");
  return;
}              

// 61, rev_getdents64(unsigned int fd, struct linux_dirent64  *dirent, unsigned int count) 
void RevProc::ECALL_getdents64(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: getdents64 called\n");
  return;
}            

// 62, rev_llseek(unsigned int fd, unsigned long offset_high, unsigned long offset_low, loff_t  *result, unsigned int whence) 
void RevProc::ECALL_lseek(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: lseek called\n");
  return;
}                 

// 63, rev_read(unsigned int fd, char  *buf, size_t count) 
void RevProc::ECALL_read(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: read called\n");
  int fd = RegFile->RV64[10];
  uint64_t BufAddr = RegFile->RV64[11];
  size_t BufSize = RegFile->RV64[12];

  /* Check if Current Ctx has access to the fd */
  std::shared_ptr<RevThread> CurrCtx = HartToExecCtx();

  if( !CurrCtx->FindFD(fd) ){
    output->fatal(CALL_INFO, -1,
                  "Core %d; Hart %d; ThreadID %" PRIu32 " tried to read from file descriptor: %d but did not have access to it\n",
                  id, HartToExec, HartToExecThreadID(), fd);
    return;
  }
  /*
   * This buffer is an intermediate buffer for storing the data read from host 
   * for later use in writing to RevMem
  */
  char TmpBuf[BufSize];

  /*
   * Read nbytes of fd from host
   *
   * NOTE: Because the fd is in the Ctx's fildes vector, we can reasonably
   *       assume the file is already open on the host system because we 
   *       try to maintain parity between those
   */

  /* Do the read on the host */
  uint64_t rc = read(fd, &TmpBuf, BufSize);

  /* Write that data to the buffer inside of Rev */
  mem->WriteMem(feature->GetHart(), BufAddr, BufSize, &TmpBuf);

  RegFile->RV64[10] = rc;
  return;
}                  

// 64, rev_write(unsigned int fd, const char  *buf, size_t count) 
void RevProc::ECALL_write(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: write called\n");
  int fildes = RegFile->RV64[10];
  std::size_t nbytes = RegFile->RV64[12];

  char buf[nbytes];
  char bufchar;
  for (unsigned i=0; i<nbytes; i++){
    // mem->ReadVal(RegFile->RV64[11] + sizeof(char)*i, sizeof(char), &bufchar, REVMEM_FLAGS(0));
  }
  // mem->ReadVal(RegFile->RV64[11], sizeof(buf), &buf[0], REVMEM_FLAGS(0));

  /* Perform the write on the host system */
  const int rc = write(fildes, buf, nbytes);

  /* write returns the number of bytes written */
  RegFile->RV64[10] = rc;
  return;
}                 

// 65, rev_readv(unsigned long fd, const struct iovec  *vec, unsigned long vlen) 
void RevProc::ECALL_readv(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: readv called\n");
  return;
}                 

// 66, rev_writev(unsigned long fd, const struct iovec  *vec, unsigned long vlen) 
void RevProc::ECALL_writev(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: writev called\n");
  return;
}                

// 67, rev_pread64(unsigned int fd, char  *buf, size_t count, loff_t pos) 
void RevProc::ECALL_pread64(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: pread64 called\n");
  return;
}               

// 68, rev_pwrite64(unsigned int fd, const char  *buf, size_t count, loff_t pos) 
void RevProc::ECALL_pwrite64(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: pwrite64 called\n");
  return;
}              

// 69, rev_preadv(unsigned long fd, const struct iovec  *vec, unsigned long vlen, unsigned long pos_l, unsigned long pos_h) 
void RevProc::ECALL_preadv(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: preadv called\n");
  return;
}                

// 70, rev_pwritev(unsigned long fd, const struct iovec  *vec, unsigned long vlen, unsigned long pos_l, unsigned long pos_h) 
void RevProc::ECALL_pwritev(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: pwritev called\n");
  return;
}               

// 71, rev_sendfile64(int out_fd, int in_fd, loff_t  *offset, size_t count) 
void RevProc::ECALL_sendfile64(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: sendfile64 called\n");
  return;
}            

// 72, rev_pselect6_time32(int, fd_set  *, fd_set  *, fd_set  *, struct old_timespec32  *, void  *) 
void RevProc::ECALL_pselect6_time32(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: pselect6_time32 called\n");
  return;
}       

// 73, rev_ppoll_time32(struct pollfd  *, unsigned int, struct old_timespec32  *, const sigset_t  *, size_t) 
void RevProc::ECALL_ppoll_time32(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: ppoll_time32 called\n");
  return;
}          

// 74, rev_signalfd4(int ufd, sigset_t  *user_mask, size_t sizemask, int flags) 
void RevProc::ECALL_signalfd4(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: signalfd4 called\n");
  return;
}             

// 75, rev_vmsplice(int fd, const struct iovec  *iov, unsigned long nr_segs, unsigned int flags) 
void RevProc::ECALL_vmsplice(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: vmsplice called\n");
  return;
}              

// 76, rev_vmsplice(int fd, const struct iovec  *iov, unsigned long nr_segs, unsigned int flags) 
void RevProc::ECALL_splice(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: splice called\n");
  return;
}                

// 77, rev_tee(int fdin, int fdout, size_t len, unsigned int flags) 
void RevProc::ECALL_tee(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: tee called\n");
  return;
}                   

// 78, rev_readlinkat(int dfd, const char  *path, char  *buf, int bufsiz) 
void RevProc::ECALL_readlinkat(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: readlinkat called\n");
  return;
}            

// 79, rev_newfstatat(int dfd, const char  *filename, struct stat  *statbuf, int flag) 
void RevProc::ECALL_newfstatat(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: newfstatat called\n");
  return;
}            

// 80, rev_newfstat(unsigned int fd, struct stat  *statbuf) 
void RevProc::ECALL_newfstat(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: newfstat called\n");
  return;
}              

// 81, rev_sync(void) 
void RevProc::ECALL_sync(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: sync called\n");
  return;
}                  

// 82, rev_fsync(unsigned int fd) 
void RevProc::ECALL_fsync(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fsync called\n");
  return;
}                 

// 83, rev_fdatasync(unsigned int fd) 
void RevProc::ECALL_fdatasync(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fdatasync called\n");
  return;
}             

// 84, rev_sync_file_range2(int fd, unsigned int flags, loff_t offset, loff_t nbytes) 
void RevProc::ECALL_sync_file_range2(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: sync_file_range2 called\n");
  return;
}      

// 84, rev_sync_file_range(int fd, loff_t offset, loff_t nbytes, unsigned int flags) 
void RevProc::ECALL_sync_file_range(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: sync_file_range called\n");
  return;
}       

// 85, rev_timerfd_create(int clockid, int flags) 
void RevProc::ECALL_timerfd_create(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: timerfd_create called\n");
  return;
}        

// 86, rev_timerfd_settime(int ufd, int flags, const struct __kernel_itimerspec  *utmr, struct __kernel_itimerspec  *otmr) 
void RevProc::ECALL_timerfd_settime(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: timerfd_settime called\n");
  return;
}       

// 87, rev_timerfd_gettime(int ufd, struct __kernel_itimerspec  *otmr) 
void RevProc::ECALL_timerfd_gettime(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: timerfd_gettime called\n");
  return;
}       

// 88, rev_utimensat(int dfd, const char  *filename, struct __kernel_timespec  *utimes, int flags) 
void RevProc::ECALL_utimensat(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: utimensat called\n");
  return;
}             

// 89, rev_acct(const char  *name) 
void RevProc::ECALL_acct(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: acct called\n");
  return;
}                  

// 90, rev_capget(cap_user_header_t header, cap_user_data_t dataptr) 
void RevProc::ECALL_capget(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: capget called\n");
  return;
}                

// 91, rev_capset(cap_user_header_t header, const cap_user_data_t data) 
void RevProc::ECALL_capset(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: capset called\n");
  return;
}                

// 92, rev_personality(unsigned int personality) 
void RevProc::ECALL_personality(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: personality called\n");
  return;
}           

// TODO: Add ThreadManager Logic
// 93, rev_exit(int error_code) 
void RevProc::ECALL_exit(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: exit called\n");
  std::shared_ptr<RevThread> CurrCtx = HartToExecCtx();
  const uint64_t status = RegFile->RV64[10];

  /* If the current ctx has ParentThreadID = 0,
     it has no parent and we should terminate the sim */
  // if( CurrCtx->GetParentThreadID() == 0 ){
  //   output->verbose(CALL_INFO, 0, 0,
  //                   "Process %u exiting with status %lu\n",
  //                   CurrCtx->GetThreadID(), status );
  //   exit(status);
  // } else {
  //   /* Parent exists & Child is exiting... switch back to parent */
  //   CtxSwitchAlert(CurrCtx->GetParentThreadID());
  //   output->verbose(CALL_INFO, 0, 0,
  //                   "Process %u exiting with status %lu\n",
  //                   CurrCtx->GetThreadID(), status );
  // }
  return;
}                  

// 94, rev_exit_group(int error_code) 
void RevProc::ECALL_exit_group(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: exit_group called\n");
  return;
}            

// 95, rev_waitid(int which, pid_t pid, struct siginfo  *infop, int options, struct rusage  *ru) 
void RevProc::ECALL_waitid(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: waitid called\n");
  return;
}                

// 96, rev_set_tid_address(int  *tidptr) 
void RevProc::ECALL_set_tid_address(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: set_tid_address called\n");
  return;
}       

// 97, rev_unshare(unsigned long unshare_flags) 
void RevProc::ECALL_unshare(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: unshare called\n");
  return;
}               

// 98, rev_futex(u32  *uaddr, int op, u32 val, struct __kernel_timespec  *utime, u32  *uaddr2, u32 val3) 
void RevProc::ECALL_futex(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: futex called\n");
  return;
}                 

// 99, rev_set_robust_list(struct robust_list_head  *head, size_t len) 
void RevProc::ECALL_set_robust_list(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: set_robust_list called\n");
  return;
}       

// 100, rev_get_robust_list(int pid, struct robust_list_head  *  *head_ptr, size_t  *len_ptr) 
void RevProc::ECALL_get_robust_list(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: get_robust_list called\n");
  return;
}       

// 101, rev_nanosleep(struct __kernel_timespec  *rqtp, struct __kernel_timespec  *rmtp) 
void RevProc::ECALL_nanosleep(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: nanosleep called\n");
  return;
}             

// 102, rev_getitimer(int which, struct __kernel_old_itimerval  *value) 
void RevProc::ECALL_getitimer(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: getitimer called\n");
  return;
}             

// 103, rev_setitimer(int which, struct __kernel_old_itimerval  *value, struct __kernel_old_itimerval  *ovalue) 
void RevProc::ECALL_setitimer(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: setitimer called\n");
  return;
}             

// 104, rev_kexec_load(unsigned long entry, unsigned long nr_segments, struct kexec_segment  *segments, unsigned long flags) 
void RevProc::ECALL_kexec_load(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: kexec_load called\n");
  return;
}            

// 105, rev_init_module(void  *umod, unsigned long len, const char  *uargs) 
void RevProc::ECALL_init_module(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: init_module called\n");
  return;
}           

// 106, rev_delete_module(const char  *name_user, unsigned int flags) 
void RevProc::ECALL_delete_module(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: delete_module called\n");
  return;
}         

// 107, rev_timer_create(clockid_t which_clock, struct sigevent  *timer_event_spec, timer_t  * created_timer_id) 
void RevProc::ECALL_timer_create(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: timer_create called\n");
  return;
}          

// 108, rev_timer_gettime(timer_t timer_id, struct __kernel_itimerspec  *setting) 
void RevProc::ECALL_timer_gettime(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: timer_gettime called\n");
  return;
}         

// 109, rev_timer_getoverrun(timer_t timer_id) 
void RevProc::ECALL_timer_getoverrun(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: timer_getoverrun called\n");
  return;
}      

// 110, rev_timer_settime(timer_t timer_id, int flags, const struct __kernel_itimerspec  *new_setting, struct __kernel_itimerspec  *old_setting) 
void RevProc::ECALL_timer_settime(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: timer_settime called\n");
  return;
}         

// 111, rev_timer_delete(timer_t timer_id) 
void RevProc::ECALL_timer_delete(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: timer_delete called\n");
  return;
}          

// 112, rev_clock_settime(clockid_t which_clock, const struct __kernel_timespec  *tp) 
void RevProc::ECALL_clock_settime(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: clock_settime called\n");
  return;
}         

// 113, rev_clock_gettime(clockid_t which_clock, struct __kernel_timespec  *tp) 
void RevProc::ECALL_clock_gettime(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: clock_gettime called\n");
  return;
}         

// 114, rev_clock_getres(clockid_t which_clock, struct __kernel_timespec  *tp) 
void RevProc::ECALL_clock_getres(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: clock_getres called\n");
  return;
}          

// 115, rev_clock_nanosleep(clockid_t which_clock, int flags, const struct __kernel_timespec  *rqtp, struct __kernel_timespec  *rmtp) 
void RevProc::ECALL_clock_nanosleep(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: clock_nanosleep called\n");
  return;
}       

// 116, rev_syslog(int type, char  *buf, int len) 
void RevProc::ECALL_syslog(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: syslog called\n");
  return;
}                

// 117, rev_ptrace(long request, long pid, unsigned long addr, unsigned long data) 
void RevProc::ECALL_ptrace(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: ptrace called\n");
  return;
}                

// 118, rev_sched_setparam(pid_t pid, struct sched_param  *param) 
void RevProc::ECALL_sched_setparam(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: sched_setparam called\n");
  return;
}        

// 119, rev_sched_setscheduler(pid_t pid, int policy, struct sched_param  *param) 
void RevProc::ECALL_sched_setscheduler(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: sched_setscheduler called\n");
  return;
}    

// 120, rev_sched_getscheduler(pid_t pid) 
void RevProc::ECALL_sched_getscheduler(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: sched_getscheduler called\n");
  return;
}    

// 121, rev_sched_getparam(pid_t pid, struct sched_param  *param) 
void RevProc::ECALL_sched_getparam(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: sched_getparam called\n");
  return;
}        

// 122, rev_sched_setaffinity(pid_t pid, unsigned int len, unsigned long  *user_mask_ptr) 
void RevProc::ECALL_sched_setaffinity(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: sched_setaffinity called\n");
  return;
}     

// 123, rev_sched_getaffinity(pid_t pid, unsigned int len, unsigned long  *user_mask_ptr) 
void RevProc::ECALL_sched_getaffinity(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: sched_getaffinity called\n");
  return;
}     

// 124, rev_sched_yield(void) 
void RevProc::ECALL_sched_yield(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: sched_yield called\n");
  return;
}           

// 125, rev_sched_get_priority_max(int policy) 
void RevProc::ECALL_sched_get_priority_max(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: sched_get_priority_max called\n");
  return;
}

// 126, rev_sched_get_priority_min(int policy) 
void RevProc::ECALL_sched_get_priority_min(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: sched_get_priority_min called\n");
  return;
}

// 127, rev_sched_rr_get_interval(pid_t pid, struct __kernel_timespec  *interval) 
void RevProc::ECALL_sched_rr_get_interval(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: sched_rr_get_interval called\n");
  return;
} 

// 128, rev_restart_syscall(void) 
void RevProc::ECALL_restart_syscall(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: restart_syscall called\n");
  return;
}       

// 129, rev_kill(pid_t pid, int sig) 
void RevProc::ECALL_kill(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: kill called\n");
  return;
}                  

// 130, rev_tkill(pid_t pid, int sig) 
void RevProc::ECALL_tkill(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: tkill called\n");
  return;
}                 

// 131, rev_tgkill(pid_t tgid, pid_t pid, int sig) 
void RevProc::ECALL_tgkill(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: tgkill called\n");
  return;
}                

// 132, rev_sigaltstack(const struct sigaltstack  *uss, struct sigaltstack  *uoss) 
void RevProc::ECALL_sigaltstack(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: sigaltstack called\n");
  return;
}           

// 133, rev_rt_sigsuspend(sigset_t  *unewset, size_t sigsetsize) 
void RevProc::ECALL_rt_sigsuspend(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: rt_sigsuspend called\n");
  return;
}         

// 134, rev_rt_sigaction(int, const struct sigaction  *, struct sigaction  *, size_t) 
void RevProc::ECALL_rt_sigaction(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: rt_sigaction called\n");
  return;
}          

// 135, rev_rt_sigprocmask(int how, sigset_t  *set, sigset_t  *oset, size_t sigsetsize) 
void RevProc::ECALL_rt_sigprocmask(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: rt_sigprocmask called\n");
  return;
}        

// 136, rev_rt_sigpending(sigset_t  *set, size_t sigsetsize) 
void RevProc::ECALL_rt_sigpending(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: rt_sigpending called\n");
  return;
}         

// 137, rev_rt_sigtimedwait_time32(const sigset_t  *uthese, siginfo_t  *uinfo, const struct old_timespec32  *uts, size_t sigsetsize) 
void RevProc::ECALL_rt_sigtimedwait_time32(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: rt_sigtimedwait_time32 called\n");
  return;
}

// 138, rev_rt_sigqueueinfo(pid_t pid, int sig, siginfo_t  *uinfo) 
void RevProc::ECALL_rt_sigqueueinfo(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: rt_sigqueueinfo called\n");
  return;
}       

// 140, rev_setpriority(int which, int who, int niceval) 
void RevProc::ECALL_setpriority(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: setpriority called\n");
  return;
}           

// 141, rev_getpriority(int which, int who) 
void RevProc::ECALL_getpriority(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: getpriority called\n");
  return;
}           

// 142, rev_reboot(int magic1, int magic2, unsigned int cmd, void  *arg) 
void RevProc::ECALL_reboot(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: reboot called\n");
  return;
}                

// 143, rev_setregid(gid_t rgid, gid_t egid) 
void RevProc::ECALL_setregid(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: setregid called\n");
  return;
}              

// 144, rev_setgid(gid_t gid) 
void RevProc::ECALL_setgid(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: setgid called\n");
  return;
}                

// 145, rev_setreuid(uid_t ruid, uid_t euid) 
void RevProc::ECALL_setreuid(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: setreuid called\n");
  return;
}              

// 146, rev_setuid(uid_t uid) 
void RevProc::ECALL_setuid(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: setuid called\n");
  return;
}                

// 147, rev_setresuid(uid_t ruid, uid_t euid, uid_t suid) 
void RevProc::ECALL_setresuid(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: setresuid called\n");
  return;
}             

// 148, rev_getresuid(uid_t  *ruid, uid_t  *euid, uid_t  *suid) 
void RevProc::ECALL_getresuid(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: getresuid called\n");
  return;
}             

// 149, rev_setresgid(gid_t rgid, gid_t egid, gid_t sgid) 
void RevProc::ECALL_setresgid(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: setresgid called\n");
  return;
}             

// 150, rev_getresgid(gid_t  *rgid, gid_t  *egid, gid_t  *sgid) 
void RevProc::ECALL_getresgid(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: getresgid called\n");
  return;
}             

// 151, rev_setfsuid(uid_t uid) 
void RevProc::ECALL_setfsuid(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: setfsuid called\n");
  return;
}              

// 152, rev_setfsgid(gid_t gid) 
void RevProc::ECALL_setfsgid(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: setfsgid called\n");
  return;
}              

// 153, rev_times(struct tms  *tbuf) 
void RevProc::ECALL_times(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: times called\n");
  return;
}                 

// 154, rev_setpgid(pid_t pid, pid_t pgid) 
void RevProc::ECALL_setpgid(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: setpgid called\n");
  return;
}               

// 155, rev_getpgid(pid_t pid) 
void RevProc::ECALL_getpgid(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: getpgid called\n");
  return;
}               

// 156, rev_getsid(pid_t pid) 
void RevProc::ECALL_getsid(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: getsid called\n");
  return;
}                

// 157, rev_setsid(void) 
void RevProc::ECALL_setsid(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: setsid called\n");
  return;
}                

// 158, rev_getgroups(int gidsetsize, gid_t  *grouplist) 
void RevProc::ECALL_getgroups(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: getgroups called\n");
  return;
}             

// 159, rev_setgroups(int gidsetsize, gid_t  *grouplist) 
void RevProc::ECALL_setgroups(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: setgroups called\n");
  return;
}             

// 160, rev_newuname(struct new_utsname  *name) 
void RevProc::ECALL_newuname(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: newuname called\n");
  return;
}              

// 161, rev_sethostname(char  *name, int len) 
void RevProc::ECALL_sethostname(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: sethostname called\n");
  return;
}           

// 162, rev_setdomainname(char  *name, int len) 
void RevProc::ECALL_setdomainname(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: setdomainname called\n");
  return;
}         

// 163, rev_getrlimit(unsigned int resource, struct rlimit  *rlim) 
void RevProc::ECALL_getrlimit(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: getrlimit called\n");
  return;
}             

// 164, rev_setrlimit(unsigned int resource, struct rlimit  *rlim) 
void RevProc::ECALL_setrlimit(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: setrlimit called\n");
  return;
}             

// 165, rev_getrusage(int who, struct rusage  *ru) 
void RevProc::ECALL_getrusage(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: getrusage called\n");
  return;
}             

// 166, rev_umask(int mask) 
void RevProc::ECALL_umask(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: umask called\n");
  return;
}                 

// 167, rev_prctl(int option, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5) 
void RevProc::ECALL_prctl(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: prctl called\n");
  return;
}                 

// 168, rev_getcpu(unsigned  *cpu, unsigned  *node, struct getcpu_cache  *cache) 
void RevProc::ECALL_getcpu(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: getcpu called\n");
  return;
}                

// 169, rev_gettimeofday(struct __kernel_old_timeval  *tv, struct timezone  *tz) 
void RevProc::ECALL_gettimeofday(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: gettimeofday called\n");
  return;
}          

// 170, rev_settimeofday(struct __kernel_old_timeval  *tv, struct timezone  *tz) 
void RevProc::ECALL_settimeofday(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: settimeofday called\n");
  return;
}          

// 171, rev_adjtimex(struct __kernel_timex  *txc_p) 
void RevProc::ECALL_adjtimex(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: adjtimex called\n");
  return;
}              

// 172, rev_getpid(void) 
void RevProc::ECALL_getpid(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: getpid called\n");
  // There is only ever a single PID in Rev (for now)
  // output->verbose(CALL_INFO, 2, 0, "ECALL_getpid called\n");
  // uint32_t CurrentThreadID = ActiveThreadIDs.at(HartToExec);
  // // auto CurrentCtx = ThreadTable.at(CurrentThreadID);
  // RegFile->RV64[10] = ActiveThreadIDs.at(HartToExec);
  return;
}                

// TODO: Add ThreadManager Logic
// 173, rev_getppid(void) 
void RevProc::ECALL_getppid(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: getppid called\n");
  /* TODO: Implement error handling */
  output->verbose(CALL_INFO, 2, 0, "ECALL_getppid called\n");
  // uint32_t CurrentThreadID = ActiveThreadIDs.at(HartToExec);
  // auto CurrentCtx = ThreadTable.at(CurrentThreadID);
  // uint32_t ParentThreadID = CurrentCtx->GetParentThreadID();
  // RegFile->RV64[10] = ParentThreadID;
  return;
}               

// 174, rev_getuid(void) 
void RevProc::ECALL_getuid(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: getuid called\n");
  return;
}                

// 175, rev_geteuid(void) 
void RevProc::ECALL_geteuid(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: geteuid called\n");
  return;
}               

// 176, rev_getgid(void) 
void RevProc::ECALL_getgid(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: getgid called\n");
  return;
}                

// 177, rev_getegid(void) 
void RevProc::ECALL_getegid(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: getegid called\n");
  return;
}               

// 178, rev_gettid(void) 
void RevProc::ECALL_gettid(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: gettid called\n");
  RevRegFile* regFile = RegFile;

  /* rc = Currently Executing Hart */
  regFile->RV64[10] = AssignedThreads.at(HartToExec)->GetThreadID();
  return;
}                

// 179, rev_sysinfo(struct sysinfo  *info) 
void RevProc::ECALL_sysinfo(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: sysinfo called\n");
  return;
}               

// 180, rev_mq_open(const char  *name, int oflag, umode_t mode, struct mq_attr  *attr) 
void RevProc::ECALL_mq_open(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: mq_open called\n");
  return;
}               

// 181, rev_mq_unlink(const char  *name) 
void RevProc::ECALL_mq_unlink(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: mq_unlink called\n");
  return;
}             

// 182, rev_mq_timedsend(mqd_t mqdes, const char  *msg_ptr, size_t msg_len, unsigned int msg_prio, const struct __kernel_timespec  *abs_timeout) 
void RevProc::ECALL_mq_timedsend(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: mq_timedsend called\n");
  return;
}          

// 183, rev_mq_timedreceive(mqd_t mqdes, char  *msg_ptr, size_t msg_len, unsigned int  *msg_prio, const struct __kernel_timespec  *abs_timeout) 
void RevProc::ECALL_mq_timedreceive(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: mq_timedreceive called\n");
  return;
}       

// 184, rev_mq_notify(mqd_t mqdes, const struct sigevent  *notification) 
void RevProc::ECALL_mq_notify(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: mq_notify called\n");
  return;
}             

// 185, rev_mq_getsetattr(mqd_t mqdes, const struct mq_attr  *mqstat, struct mq_attr  *omqstat) 
void RevProc::ECALL_mq_getsetattr(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: mq_getsetattr called\n");
  return;
}         

// 186, rev_msgget(key_t key, int msgflg) 
void RevProc::ECALL_msgget(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: msgget called\n");
  return;
}                

// 187, rev_old_msgctl(int msqid, int cmd, struct msqid_ds  *buf) 
void RevProc::ECALL_msgctl(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: msgctl called\n");
  return;
}                

// 188, rev_msgrcv(int msqid, struct msgbuf  *msgp, size_t msgsz, long msgtyp, int msgflg) 
void RevProc::ECALL_msgrcv(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: msgrcv called\n");
  return;
}                

// 189, rev_msgsnd(int msqid, struct msgbuf  *msgp, size_t msgsz, int msgflg) 
void RevProc::ECALL_msgsnd(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: msgsnd called\n");
  return;
}                

// 190, rev_semget(key_t key, int nsems, int semflg) 
void RevProc::ECALL_semget(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: semget called\n");
  return;
}                

// 191, rev_semctl(int semid, int semnum, int cmd, unsigned long arg) 
void RevProc::ECALL_semctl(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: semctl called\n");
  return;
}                

// 192, rev_semtimedop(int semid, struct sembuf  *sops, unsigned nsops, const struct __kernel_timespec  *timeout) 
void RevProc::ECALL_semtimedop(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: semtimedop called\n");
  return;
}            

// 193, rev_semop(int semid, struct sembuf  *sops, unsigned nsops) 
void RevProc::ECALL_semop(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: semop called\n");
  return;
}                 

// 194, rev_shmget(key_t key, size_t size, int flag) 
void RevProc::ECALL_shmget(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: shmget called\n");
  return;
}                

// 195, rev_old_shmctl(int shmid, int cmd, struct shmid_ds  *buf) 
void RevProc::ECALL_shmctl(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: shmctl called\n");
  return;
}                

// 196, rev_shmat(int shmid, char  *shmaddr, int shmflg) 
void RevProc::ECALL_shmat(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: shmat called\n");
  return;
}                 

// 197, rev_shmdt(char  *shmaddr) 
void RevProc::ECALL_shmdt(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: shmdt called\n");
  return;
}                 

// 198, rev_socket(int, int, int) 
void RevProc::ECALL_socket(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: socket called\n");
  return;
}                

// 199, rev_socketpair(int, int, int, int  *) 
void RevProc::ECALL_socketpair(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: socketpair called\n");
  return;
}            

// 200, rev_bind(int, struct sockaddr  *, int) 
void RevProc::ECALL_bind(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: bind called\n");
  return;
}                  

// 201, rev_listen(int, int) 
void RevProc::ECALL_listen(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: listen called\n");
  return;
}                

// 202, rev_accept(int, struct sockaddr  *, int  *) 
void RevProc::ECALL_accept(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: accept called\n");
  return;
}                

// 203, rev_connect(int, struct sockaddr  *, int) 
void RevProc::ECALL_connect(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: connect called\n");
  return;
}               

// 204, rev_getsockname(int, struct sockaddr  *, int  *) 
void RevProc::ECALL_getsockname(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: getsockname called\n");
  return;
}           

// 205, rev_getpeername(int, struct sockaddr  *, int  *) 
void RevProc::ECALL_getpeername(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: getpeername called\n");
  return;
}           

// 206, rev_sendto(int, void  *, size_t, unsigned, struct sockaddr  *, int) 
void RevProc::ECALL_sendto(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: sendto called\n");
  return;
}                

// 207, rev_recvfrom(int, void  *, size_t, unsigned, struct sockaddr  *, int  *) 
void RevProc::ECALL_recvfrom(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: recvfrom called\n");
  return;
}              

// 208, rev_setsockopt(int fd, int level, int optname, char  *optval, int optlen) 
void RevProc::ECALL_setsockopt(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: setsockopt called\n");
  return;
}            

// 209, rev_getsockopt(int fd, int level, int optname, char  *optval, int  *optlen) 
void RevProc::ECALL_getsockopt(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: getsockopt called\n");
  return;
}            

// 210, rev_shutdown(int, int) 
void RevProc::ECALL_shutdown(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: shutdown called\n");
  return;
}              

// 211, rev_sendmsg(int fd, struct user_msghdr  *msg, unsigned flags) 
void RevProc::ECALL_sendmsg(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: sendmsg called\n");
  return;
}               

// 212, rev_recvmsg(int fd, struct user_msghdr  *msg, unsigned flags) 
void RevProc::ECALL_recvmsg(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: recvmsg called\n");
  return;
}               

// 213, rev_readahead(int fd, loff_t offset, size_t count) 
void RevProc::ECALL_readahead(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: readahead called\n");
  return;
}             

// 214, rev_brk(unsigned long brk) 
void RevProc::ECALL_brk(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: brk called\n");
  uint64_t Addr = RegFile->RV64[10];

  const uint64_t heapend = mem->GetHeapEnd();
  if( Addr > 0 && Addr > heapend ){
    uint64_t Size = Addr - heapend;
    mem->ExpandHeap(Size); 
  } else {
    output->fatal(CALL_INFO, 11, "Out of memory / Unable to expand system break (brk) to Addr = 0x%lx", Addr);
  }
  return;
}                   

// 215, rev_munmap(unsigned long addr, size_t len) 
void RevProc::ECALL_munmap(){

  output->verbose(CALL_INFO, 2, 0, "ECALL: munmap called\n"); 
  uint64_t Addr = RegFile->RV64[10];
  uint64_t Size = RegFile->RV64[11];

  int rc =  (int)mem->DeallocMem(Addr, Size) == -1;
  if(rc == -1){
    output->fatal(CALL_INFO, 11, 
                  "Failed to perform munmap(Addr = 0x%lx, Size = 0x%lx)"
                  "likely because the memory was not allocated to begin with" , 
                  Addr, Size);
  }
  RegFile->RV64[10] = rc;
  return;
}


// 216, rev_mremap(unsigned long addr, unsigned long old_len, unsigned long new_len, unsigned long flags, unsigned long new_addr) 
void RevProc::ECALL_mremap(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: mremap called\n");
  return;
}                

// 217, rev_add_key(const char  *_type, const char  *_description, const void  *_payload, size_t plen, key_serial_t destringid) 
void RevProc::ECALL_add_key(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: add_key called\n");
  return;
}               

// 218, rev_request_key(const char  *_type, const char  *_description, const char  *_callout_info, key_serial_t destringid) 
void RevProc::ECALL_request_key(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: request_key called\n");
  return;
}           

// 219, rev_keyctl(int cmd, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5) 
void RevProc::ECALL_keyctl(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: keyctl called\n");
  return;
}                

// TODO: Add ThreadManager Logic
// TODO: Figure out the difference between this and clone3
// 220, rev_clone(unsigned long, unsigned long, int  *, unsigned long, int  *)
void RevProc::ECALL_clone(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: clone called\n");
  uint64_t CloneArgsAddr = RegFile->RV64[10];
  // size_t SizeOfCloneArgs = RegFile()->RV64[11];

  // Fetch the clone_args 
  struct clone_args args;
  bool *RequestDone;

  // Parse clone flags 
  // - Only kept around the relevant flags
  // for( uint64_t bit=1; bit != 0; bit <<= 1 ){
  //   switch (args.flags & bit) {
  //     case CLONE_VM: /* For Rev this is set when you want to duplicate the parents stack */
  //       std::cout << "CLONE_VM is true" << std::endl;
  //       break;
  //     case CLONE_FILES: /* Set if open files shared between processes */
  //       std::cout << "CLONE_FILES is true" << std::endl;
  //       break;
  //     case CLONE_SIGHAND: /* Set if signal handlers shared */
  //       std::cout << "CLONE_SIGHAND is true" << std::endl;
  //       break;
  //     case CLONE_VFORK: /* Set if the parent wants the child to wake it up on mm_release */
  //       std::cout << "CLONE_VFORK is true" << std::endl;
  //       break;
  //     case CLONE_PARENT: /* Set if we want to have the same parent as the cloner */
  //       std::cout << "CLONE_PARENT is true" << std::endl;
  //       break;
  //     case CLONE_THREAD: /* Set to add to same thread group */
  //       std::cout << "CLONE_THREAD is true" << std::endl;
  //       break;
  //     case CLONE_NEWNS: /* Set to create new namespace */
  //       std::cout << "CLONE_NEWNS is true" << std::endl;
  //       break;
  //     case CLONE_SETTLS: /* Set TLS info */
  //       std::cout << "CLONE_SETTLS is true" << std::endl;
  //       break;
  //     case CLONE_PARENT_SETTID: /* Store TID in userlevel buffer before MM copy */
  //       std::cout << "CLONE_PARENT_SETTID is true" << std::endl;
  //       break;
  //     case CLONE_CHILD_CLEARTID: /* Register exit futex and memory location to clear */
  //       std::cout << "CLONE_CHILD_CLEARTID is true" << std::endl;
  //       break;
  //     case CLONE_DETACHED: /* Create clone detached */
  //       std::cout << "CLONE_DETACHED is true" << std::endl;
  //       break;
  //     case CLONE_CHILD_SETTID: /* New cgroup namespace */
  //       std::cout << "CLONE_CHILD_SETTID is true" << std::endl;
  //       break;
  //     case CLONE_NEWCGROUP: /* New cgroup namespace */
  //       std::cout << "CLONE_NEWCGROUP is true" << std::endl;
  //       break;
  //     case CLONE_IO: /* Clone I/O Context */
  //       std::cout << "CLONE_IO is true" << std::endl;
  //       break;
  //     default:
  //       break;
  //   }
  // }

  // Get the parent ctx (Current active, executing ThreadID)
  // std::shared_ptr<RevThread> ParentCtx = ThreadTable.at(ActiveThreadIDs.at(HartToExec));

  // Create the child ctx 
  uint32_t ChildThreadID = SpawnThread();
  // std::shared_ptr<RevThread> ChildCtx = ThreadTable.at(ChildThreadID);

  // TODO: Create a copy of Parents Memory Space 
  // std::pair<uint64_t, uint64_t> ParentThreadMemPtrs = ParentCtx->GetThreadMemInfo();
  // std::pair<uint64_t, uint64_t> ChildThreadMemPtrs = ChildCtx->GetThreadMemInfo();

  // Read the parents stack and copy it to the Child's stack

  // std::cout << "Parent Stack Starts at: 0x" << std::hex << ParentThreadMemPtrs.first << std::endl;
  // std::cout << "Child Stack Starts at: 0x" << std::hex << ChildThreadMemPtrs.first << std::endl;
  
  // uint64_t p_stack_size, p_stack_baseaddr;
  // // Find the ThreadMemSeg that starts at ParentThreadMemPtrs.first
  // for( auto ThreadSeg : mem->GetThreadMemSegs() ){
  //   // FIXME: Currently the addresses get a little messed up somehow
  //   if( ThreadSeg->contains(ParentThreadMemPtrs.first)){
  //     std::cout << "Found ThreadMemSeg that starts at ParentThreadMemPtrs.first" << std::endl;
  //     p_stack_size = ThreadSeg->getSize();
  //     p_stack_baseaddr = ThreadSeg->getBaseAddr();
  //   }
  // }


  // Create a variable of size = p_stack_size
  // uint8_t StackData[p_stack_size];
  


  // Output the range that is going to be read 
  // mem->ReadVal(HartToDecode, ParentThreadMemPtrs.first - _STACK_SIZE_, StackData, REVMEM_FLAGS(0));
  // mem->ReadMem(HartToDecode, ParentThreadMemPtrs.first - _STACK_SIZE_, _STACK_SIZE_ - 1, &StackData, REVMEM_FLAGS(0));
  

  std::cout << "MADE IT" << std::endl;

  // mem->WriteMem(HartToDecode, ChildThreadMemPtrs.first - _STACK_SIZE_, _STACK_SIZE_ - 1, &StackData);
    
  std::cout << "WROTE IT" << std::endl;

  // mem->WriteMem(HartToExec, ChildThreadMemPtrs.first - _STACK_SIZE_, _STACK_SIZE_, StackData);


  /*
   * ===========================================================================================
   * Register File
   * ===========================================================================================
   * We need to duplicate the parent's RegFile to to the Childs
   * - NOTE: when we return from this function, the return value will
   *         be automatically stored in the Proc.RegFile[HartToExec]'s a0
   *         register. In a traditional fork code this looks like:
   *
   *         pid_t pid = fork()
   *         if pid < 0: // Error
   *         else if pid = 0: // New Child Process
   *         else: // Parent Process
   *
   *         In this case, the value of pid is the value thats returned to a0
   *         It follows that
   *         - The child's regfile MUST have 0 in its a0 (despite its pid != 0 to the RevProc)
   *         - The Parent's a0 register MUST have its ThreadID in it
   * ===========================================================================================
   */

  /*
   Alert the Proc there needs to be a Ctx switch
   Pass the ThreadID that will be switched to once the 
   current pipeline is executed until completion
  */
  // CtxSwitchAlert(ChildThreadID);

  /* Parent's return value is the child's ThreadID */
  // RegFile->RV64[10] = ChildThreadID;

  /* Child's return value is 0 */
  // ChildCtx->GetRegFile()->RV64[10] = 0;

  return;
}                 

// 221, rev_execve(const char  *filename, const char  *const  *argv, const char  *const  *envp) 
void RevProc::ECALL_execve(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: execve called\n");
  return;
}                

// 222, rev_old_mmap(struct mmap_arg_struct  *arg) 
void RevProc::ECALL_mmap(){

  uint64_t Addr = RegFile->RV64[10];
  uint64_t Size = RegFile->RV64[11];
  // uint64_t Prot = RegFile->RV64[12];
  // uint64_t Flags = RegFile->RV64[13];
  // uint64_t fd = RegFile->RV64[14];
  // uint64_t offset = RegFile->RV64[15];

  if( !Addr ){
    // If address is NULL... We add it to MemSegs.end()->getTopAddr()+1
    Addr = mem->AllocMem(Size+1);
    // Addr = mem->AddMemSeg(Size); 
  } else {
    // We were passed an address... try to put a segment there.
    // Currently there is no handling of getting it 'close' to the 
    // suggested address... instead if it can't allocate a new segment 
    // there it fails.
    if( !mem->AllocMemAt(Addr, Size) ){
      output->fatal(CALL_INFO, 11, "Failed to add mem segment\n");
    }
  }
  RegFile->RV64[10] = Addr;
  return;
}


// 223, rev_fadvise64_64(int fd, loff_t offset, loff_t len, int advice) 
void RevProc::ECALL_fadvise64_64(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fadvise64_64 called\n");
  return;
}          

// 224, rev_swapon(const char  *specialfile, int swap_flags) 
void RevProc::ECALL_swapon(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: swapon called\n");
  return;
}                

// 225, rev_swapoff(const char  *specialfile) 
void RevProc::ECALL_swapoff(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: swapoff called\n");
  return;
}               

// 226, rev_mprotect(unsigned long start, size_t len, unsigned long prot) 
void RevProc::ECALL_mprotect(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: mprotect called\n");
  return;
}              

// 227, rev_msync(unsigned long start, size_t len, int flags) 
void RevProc::ECALL_msync(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: msync called\n");
  return;
}                 

// 228, rev_mlock(unsigned long start, size_t len) 
void RevProc::ECALL_mlock(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: mlock called\n");
  return;
}                 

// 229, rev_munlock(unsigned long start, size_t len) 
void RevProc::ECALL_munlock(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: munlock called\n");
  return;
}               

// 230, rev_mlockall(int flags) 
void RevProc::ECALL_mlockall(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: mlockall called\n");
  return;
}              

// 231, rev_munlockall(void) 
void RevProc::ECALL_munlockall(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: munlockall called\n");
  return;
}            

// 232, rev_mincore(unsigned long start, size_t len, unsigned char  * vec) 
void RevProc::ECALL_mincore(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: mincore called\n");
  return;
}               

// 233, rev_madvise(unsigned long start, size_t len, int behavior) 
void RevProc::ECALL_madvise(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: madvise called\n");
  return;
}               

// 234, rev_remap_file_pages(unsigned long start, unsigned long size, unsigned long prot, unsigned long pgoff, unsigned long flags) 
void RevProc::ECALL_remap_file_pages(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: remap_file_pages called\n");
  return;
}      

// 235, rev_mbind(unsigned long start, unsigned long len, unsigned long mode, const unsigned long  *nmask, unsigned long maxnode, unsigned flags) 
void RevProc::ECALL_mbind(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: mbind called\n");
  return;
}                 

// 236, rev_get_mempolicy(int  *policy, unsigned long  *nmask, unsigned long maxnode, unsigned long addr, unsigned long flags) 
void RevProc::ECALL_get_mempolicy(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: get_mempolicy called\n");
  return;
}         

// 237, rev_set_mempolicy(int mode, const unsigned long  *nmask, unsigned long maxnode) 
void RevProc::ECALL_set_mempolicy(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: set_mempolicy called\n");
  return;
}         

// 238, rev_migrate_pages(pid_t pid, unsigned long maxnode, const unsigned long  *from, const unsigned long  *to) 
void RevProc::ECALL_migrate_pages(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: migrate_pages called\n");
  return;
}         

// 239, rev_move_pages(pid_t pid, unsigned long nr_pages, const void  *  *pages, const int  *nodes, int  *status, int flags) 
void RevProc::ECALL_move_pages(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: move_pages called\n");
  return;
}            

// 240, rev_rt_tgsigqueueinfo(pid_t tgid, pid_t pid, int sig, siginfo_t  *uinfo) 
void RevProc::ECALL_rt_tgsigqueueinfo(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: rt_tgsigqueueinfo called\n");
  return;
}     

// 241, rev_perf_event_open(") 
void RevProc::ECALL_perf_event_open(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: perf_event_open called\n");
  return;
}       

// 242, rev_accept4(int, struct sockaddr  *, int  *, int) 
void RevProc::ECALL_accept4(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: accept4 called\n");
  return;
}               

// 243, rev_recvmmsg_time32(int fd, struct mmsghdr  *msg, unsigned int vlen, unsigned flags, struct old_timespec32  *timeout) 
void RevProc::ECALL_recvmmsg_time32(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: recvmmsg_time32 called\n");
  return;
}       

// 260, rev_wait4(pid_t pid, int  *stat_addr, int options, struct rusage  *ru) 
void RevProc::ECALL_wait4(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: wait4 called\n");
  return;
}                 

// 261, rev_prlimit64(pid_t pid, unsigned int resource, const struct rlimit64  *new_rlim, struct rlimit64  *old_rlim) 
void RevProc::ECALL_prlimit64(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: prlimit64 called\n");
  return;
}             

// 262, rev_fanotify_init(unsigned int flags, unsigned int event_f_flags) 
void RevProc::ECALL_fanotify_init(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fanotify_init called\n");
  return;
}         

// 263, rev_fanotify_mark(int fanotify_fd, unsigned int flags, u64 mask, int fd, const char  *pathname) 
void RevProc::ECALL_fanotify_mark(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fanotify_mark called\n");
  return;
}         

// 264, rev_name_to_handle_at(int dfd, const char  *name, struct file_handle  *handle, int  *mnt_id, int flag) 
void RevProc::ECALL_name_to_handle_at(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: name_to_handle_at called\n");
  return;
}     

// 265, rev_open_by_handle_at(int mountdirfd, struct file_handle  *handle, int flags) 
void RevProc::ECALL_open_by_handle_at(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: open_by_handle_at called\n");
  return;
}     

// 266, rev_clock_adjtime(clockid_t which_clock, struct __kernel_timex  *tx) 
void RevProc::ECALL_clock_adjtime(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: clock_adjtime called\n");
  return;
}         

// 267, rev_syncfs(int fd) 
void RevProc::ECALL_syncfs(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: syncfs called\n");
  return;
}                

// 268, rev_setns(int fd, int nstype) 
void RevProc::ECALL_setns(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: setns called\n");
  return;
}                 

// 269, rev_sendmmsg(int fd, struct mmsghdr  *msg, unsigned int vlen, unsigned flags) 
void RevProc::ECALL_sendmmsg(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: sendmmsg called\n");
  return;
}              

// 270, rev_process_vm_readv(pid_t pid, const struct iovec  *lvec, unsigned long liovcnt, const struct iovec  *rvec, unsigned long riovcnt, unsigned long flags) 
void RevProc::ECALL_process_vm_readv(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: process_vm_readv called\n");
  return;
}      

// 271, rev_process_vm_writev(pid_t pid, const struct iovec  *lvec, unsigned long liovcnt, const struct iovec  *rvec, unsigned long riovcnt, unsigned long flags) 
void RevProc::ECALL_process_vm_writev(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: process_vm_writev called\n");

  return;
}     

// 272, rev_kcmp(pid_t pid1, pid_t pid2, int type, unsigned long idx1, unsigned long idx2) 
void RevProc::ECALL_kcmp(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: kcmp called\n");
  return;
}                  

// 273, rev_finit_module(int fd, const char  *uargs, int flags) 
void RevProc::ECALL_finit_module(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: finit_module called\n");
  return;
}          

// 274, rev_sched_setattr(pid_t pid, struct sched_attr  *attr, unsigned int flags) 
void RevProc::ECALL_sched_setattr(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: sched_setattr called\n");
  return;
}         

// 275, rev_sched_getattr(pid_t pid, struct sched_attr  *attr, unsigned int size, unsigned int flags) 
void RevProc::ECALL_sched_getattr(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: sched_getattr called\n");
  return;
}         

// 276, rev_renameat2(int olddfd, const char  *oldname, int newdfd, const char  *newname, unsigned int flags) 
void RevProc::ECALL_renameat2(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: renameat2 called\n");
  return;
}             

// 277, rev_seccomp(unsigned int op, unsigned int flags, void  *uargs) 
void RevProc::ECALL_seccomp(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: seccomp called\n");
  return;
}               

// 278, rev_getrandom(char  *buf, size_t count, unsigned int flags) 
void RevProc::ECALL_getrandom(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: getrandom called\n");
  return;
}             

// 279, rev_memfd_create(const char  *uname_ptr, unsigned int flags) 
void RevProc::ECALL_memfd_create(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: memfd_create called\n");
  return;
}          

// 280, rev_bpf(int cmd, union bpf_attr *attr, unsigned int size) 
void RevProc::ECALL_bpf(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: bpf called\n");
  return;
}                   

// 281, rev_execveat(int dfd, const char  *filename, const char  *const  *argv, const char  *const  *envp, int flags) 
void RevProc::ECALL_execveat(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: execveat called\n");
  return;
}              

// 282, rev_userfaultfd(int flags) 
void RevProc::ECALL_userfaultfd(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: userfaultfd called\n");
  return;
}           

// 283, rev_membarrier(int cmd, unsigned int flags, int cpu_id) 
void RevProc::ECALL_membarrier(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: membarrier called\n");
  return;
}            

// 284, rev_mlock2(unsigned long start, size_t len, int flags) 
void RevProc::ECALL_mlock2(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: mlock2 called\n");
  return;
}                

// 285, rev_copy_file_range(int fd_in, loff_t  *off_in, int fd_out, loff_t  *off_out, size_t len, unsigned int flags) 
void RevProc::ECALL_copy_file_range(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: copy_file_range called\n");
  return;
}       

// 286, rev_preadv2(unsigned long fd, const struct iovec  *vec, unsigned long vlen, unsigned long pos_l, unsigned long pos_h, rwf_t flags) 
void RevProc::ECALL_preadv2(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: preadv2 called\n");
  return;
}               

// 287, rev_pwritev2(unsigned long fd, const struct iovec  *vec, unsigned long vlen, unsigned long pos_l, unsigned long pos_h, rwf_t flags) 
void RevProc::ECALL_pwritev2(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: pwritev2 called\n");
  return;
}              

// 288, rev_pkey_mprotect(unsigned long start, size_t len, unsigned long prot, int pkey) 
void RevProc::ECALL_pkey_mprotect(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: pkey_mprotect called\n");
  return;
}         

// 289, rev_pkey_alloc(unsigned long flags, unsigned long init_val) 
void RevProc::ECALL_pkey_alloc(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: pkey_alloc called\n");
  return;
}            

// 290, rev_pkey_free(int pkey) 
void RevProc::ECALL_pkey_free(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: pkey_free called\n");
  return;
}             

// 291, rev_statx(int dfd, const char  *path, unsigned flags, unsigned mask, struct statx  *buffer) 
void RevProc::ECALL_statx(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: statx called\n");
  return;
}                 

// 292, rev_io_pgetevents(aio_context_t ctx_id, long min_nr, long nr, struct io_event  *events, struct __kernel_timespec  *timeout, const struct __aio_sigset *sig) 
void RevProc::ECALL_io_pgetevents(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: io_pgetevents called\n");
  return;
}         

// 293, rev_rseq(struct rseq  *rseq, uint32_t rseq_len, int flags, uint32_t sig) 
void RevProc::ECALL_rseq(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: rseq called\n");
  return;
}                  

// 294, rev_kexec_file_load(int kernel_fd, int initrd_fd, unsigned long cmdline_len, const char  *cmdline_ptr, unsigned long flags) 
void RevProc::ECALL_kexec_file_load(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: kexec_file_load called\n");
  return;
}       

// // 403, rev_clock_gettime(clockid_t which_clock, struct __kernel_timespec  *tp) 
// void RevProc::ECALL_clock_gettime(){
//   output->verbose(CALL_INFO, 2, 0, "ECALL: clock_gettime called\n");
//   return;
// }         

// // 404, rev_clock_settime(clockid_t which_clock, const struct __kernel_timespec  *tp) 
// void RevProc::ECALL_clock_settime(){
//   output->verbose(CALL_INFO, 2, 0, "ECALL: clock_settime called\n");
//   return;
// }         

// // 405, rev_clock_adjtime(clockid_t which_clock, struct __kernel_timex  *tx) 
// void RevProc::ECALL_clock_adjtime(){
//   output->verbose(CALL_INFO, 2, 0, "ECALL: clock_adjtime called\n");
//   return;
// }         

// // 406, rev_clock_getres(clockid_t which_clock, struct __kernel_timespec  *tp) 
// void RevProc::ECALL_clock_getres(){
//   output->verbose(CALL_INFO, 2, 0, "ECALL: clock_getres called\n");
//   return;
// }          

// // 407, rev_clock_nanosleep(clockid_t which_clock, int flags, const struct __kernel_timespec  *rqtp, struct __kernel_timespec  *rmtp) 
// void RevProc::ECALL_clock_nanosleep(){
//   output->verbose(CALL_INFO, 2, 0, "ECALL: clock_nanosleep called\n");
//   return;
// }       

// // 408, rev_timer_gettime(timer_t timer_id, struct __kernel_itimerspec  *setting) 
// void RevProc::ECALL_timer_gettime(){
//   output->verbose(CALL_INFO, 2, 0, "ECALL: timer_gettime called\n");
//   return;
// }         

// // 409, rev_timer_settime(timer_t timer_id, int flags, const struct __kernel_itimerspec  *new_setting, struct __kernel_itimerspec  *old_setting) 
// void RevProc::ECALL_timer_settime(){
//   output->verbose(CALL_INFO, 2, 0, "ECALL: timer_settime called\n");
//   return;
// }         

// // 410, rev_timerfd_gettime(int ufd, struct __kernel_itimerspec  *otmr) 
// void RevProc::ECALL_timerfd_gettime(){
//   output->verbose(CALL_INFO, 2, 0, "ECALL: timerfd_gettime called\n");
//   return;
// }       

// // 411, rev_timerfd_settime(int ufd, int flags, const struct __kernel_itimerspec  *utmr, struct __kernel_itimerspec  *otmr) 
// void RevProc::ECALL_timerfd_settime(){
//   output->verbose(CALL_INFO, 2, 0, "ECALL: timerfd_settime called\n");
//   return;
// }       

// // 412, rev_utimensat(int dfd, const char  *filename, struct __kernel_timespec  *utimes, int flags) 
// void RevProc::ECALL_utimensat(){
//   output->verbose(CALL_INFO, 2, 0, "ECALL: utimensat called\n");
//   return;
// }             

// // 416, rev_io_pgetevents(aio_context_t ctx_id, long min_nr, long nr, struct io_event  *events, struct __kernel_timespec  *timeout, const struct __aio_sigset *sig) 
// void RevProc::ECALL_io_pgetevents(){
//   output->verbose(CALL_INFO, 2, 0, "ECALL: io_pgetevents called\n");
//   return;
// }         

// // 418, rev_mq_timedsend(mqd_t mqdes, const char  *msg_ptr, size_t msg_len, unsigned int msg_prio, const struct __kernel_timespec  *abs_timeout) 
// void RevProc::ECALL_mq_timedsend(){
//   output->verbose(CALL_INFO, 2, 0, "ECALL: mq_timedsend called\n");
//   return;
// }          

// // 419, rev_mq_timedreceive(mqd_t mqdes, char  *msg_ptr, size_t msg_len, unsigned int  *msg_prio, const struct __kernel_timespec  *abs_timeout) 
// void RevProc::ECALL_mq_timedreceive(){
//   output->verbose(CALL_INFO, 2, 0, "ECALL: mq_timedreceive called\n");
//   return;
// }       

// // 420, rev_semtimedop(int semid, struct sembuf  *sops, unsigned nsops, const struct __kernel_timespec  *timeout) 
// void RevProc::ECALL_semtimedop(){
//   output->verbose(CALL_INFO, 2, 0, "ECALL: semtimedop called\n");
//   return;
// }            

// // 422, rev_futex(u32  *uaddr, int op, u32 val, struct __kernel_timespec  *utime, u32  *uaddr2, u32 val3) 
// void RevProc::ECALL_futex(){
//   output->verbose(CALL_INFO, 2, 0, "ECALL: futex called\n");
//   return;
// }                 

// // 423, rev_sched_rr_get_interval(pid_t pid, struct __kernel_timespec  *interval) 
// void RevProc::ECALL_sched_rr_get_interval(){
//   output->verbose(CALL_INFO, 2, 0, "ECALL: sched_rr_get_interval called\n");
//   return;
// } 
//

// 424, rev_pidfd_send_signal(int pidfd, int sig, siginfo_t  *info, unsigned int flags) 
void RevProc::ECALL_pidfd_send_signal(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: pidfd_send_signal called\n");
  return;
}     

// 425, rev_io_uring_setup(u32 entries, struct io_uring_params  *p) 
void RevProc::ECALL_io_uring_setup(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: io_uring_setup called\n");
  return;
}        

// 426, rev_io_uring_enter(unsigned int fd, u32 to_submit, u32 min_complete, u32 flags, const sigset_t  *sig, size_t sigsz) 
void RevProc::ECALL_io_uring_enter(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: io_uring_enter called\n");
  return;
}        

// 427, rev_io_uring_register(unsigned int fd, unsigned int op, void  *arg, unsigned int nr_args) 
void RevProc::ECALL_io_uring_register(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: io_uring_register called\n");
  return;
}     

// 428, rev_open_tree(int dfd, const char  *path, unsigned flags) 
void RevProc::ECALL_open_tree(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: open_tree called\n");
  return;
}             

// 429, rev_move_mount(int from_dfd, const char  *from_path, int to_dfd, const char  *to_path, unsigned int ms_flags) 
void RevProc::ECALL_move_mount(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: move_mount called\n");
  return;
}            

// 430, rev_fsopen(const char  *fs_name, unsigned int flags) 
void RevProc::ECALL_fsopen(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fsopen called\n");
  return;
}                

// 431, rev_fsconfig(int fs_fd, unsigned int cmd, const char  *key, const void  *value, int aux) 
void RevProc::ECALL_fsconfig(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fsconfig called\n");
  return;
}              

// 432, rev_fsmount(int fs_fd, unsigned int flags, unsigned int ms_flags) 
void RevProc::ECALL_fsmount(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fsmount called\n");
  return;
}               

// 433, rev_fspick(int dfd, const char  *path, unsigned int flags) 
void RevProc::ECALL_fspick(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fspick called\n");
  return;
}                

// 434, rev_pidfd_open(pid_t pid, unsigned int flags) 
void RevProc::ECALL_pidfd_open(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: pidfd_open called\n");
  return;
}            

// 435, rev_clone3(struct clone_args  *uargs, size_t size) 
void RevProc::ECALL_clone3(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: clone3 called\n");
  uint64_t CloneArgsAddr = RegFile->RV64[10];
  // size_t SizeOfCloneArgs = RegFile()->RV64[11];

  /* Fetch the clone_args */
  struct clone_args args;
  // mem->ReadVal(CloneArgsAddr, sizeof(uint64_t), &args, REVMEM_FLAGS(0));

  /*
   * Parse clone flags 
   * NOTE: if no flags are set, we get fork() like behavior
   */
  for( uint64_t bit=1; bit != 0; bit <<= 1 ){
    switch (args.flags & bit) {
      case CLONE_VM:
        // std::cout << "CLONE_VM is true" << std::endl;
        break;
      case CLONE_FS: /* Set if fs info shared between processes */
        // std::cout << "CLONE_FS is true" << std::endl;
        break;
      case CLONE_FILES: /* Set if open files shared between processes */
        // std::cout << "CLONE_FILES is true" << std::endl;
        break;
      case CLONE_SIGHAND: /* Set if signal handlers shared */
        // std::cout << "CLONE_SIGHAND is true" << std::endl;
        break;
      case CLONE_PIDFD: /* Set if a pidfd should be placed in the parent */
        // std::cout << "CLONE_PIDFD is true" << std::endl;
        break;
      case CLONE_PTRACE: /* Set if tracing continues on the child */
        // std::cout << "CLONE_PTRACE is true" << std::endl;
        break;
      case CLONE_VFORK: /* Set if the parent wants the child to wake it up on mm_release */
        // std::cout << "CLONE_VFORK is true" << std::endl;
        break;
      case CLONE_PARENT: /* Set if we want to have the same parent as the cloner */
        // std::cout << "CLONE_PARENT is true" << std::endl;
        break;
      case CLONE_THREAD: /* Set to add to same thread group */
        // std::cout << "CLONE_THREAD is true" << std::endl;
        break;
      case CLONE_NEWNS: /* Set to create new namespace */
        // std::cout << "CLONE_NEWNS is true" << std::endl;
        break;
      case CLONE_SYSVSEM: /* Set to shared SVID SEM_UNDO semantics */
        // std::cout << "CLONE_SYSVSEM is true" << std::endl;
        break;
      case CLONE_SETTLS: /* Set TLS info */
        // std::cout << "CLONE_SETTLS is true" << std::endl;
        break;
      case CLONE_PARENT_SETTID: /* Store TID in userlevel buffer before MM copy */
        // std::cout << "CLONE_PARENT_SETTID is true" << std::endl;
        break;
      case CLONE_CHILD_CLEARTID: /* Register exit futex and memory location to clear */
        // std::cout << "CLONE_CHILD_CLEARTID is true" << std::endl;
        break;
      case CLONE_DETACHED: /* Create clone detached */
        // std::cout << "CLONE_DETACHED is true" << std::endl;
        break;
      case CLONE_UNTRACED: /* Set if the tracing process can't force CLONE_PTRACE on this clone */
        // std::cout << "CLONE_UNTRACED is true" << std::endl;
        break;
      case CLONE_CHILD_SETTID: /* New cgroup namespace */
        // std::cout << "CLONE_CHILD_SETTID is true" << std::endl;
        break;
      case CLONE_NEWCGROUP: /* New cgroup namespace */
        // std::cout << "CLONE_NEWCGROUP is true" << std::endl;
        break;
      case CLONE_NEWUTS: /* New utsname group */
        // std::cout << "CLONE_NEWUTS is true" << std::endl;
        break;
      case CLONE_NEWIPC: /* New ipcs */
        // std::cout << "CLONE_NEWIPC is true" << std::endl;
        break;
      case CLONE_NEWUSER: /* New user namespace */
        // std::cout << "CLONE_NEWUSER is true" << std::endl;
        break;
      case CLONE_NEWPID: /* New pid namespace */
        // std::cout << "CLONE_NEWThreadID is true" << std::endl;
        break;
      case CLONE_NEWNET: /* New network namespace */
        // std::cout << "CLONE_NEWNET is true" << std::endl;
        break;
      case CLONE_IO: /* Clone I/O Context */
        // std::cout << "CLONE_IO is true" << std::endl;
        break;
      default:
        break;
    }
  }

  /* Get the parent ctx (Current active, executing ThreadID) */
  // std::shared_ptr<RevThread> ParentCtx = ThreadTable.at(ActiveThreadIDs.at(HartToExec));

  /* TODO: Create a copy of Parents Memory Space (need Demand Paging first) */

  /*
   * ===========================================================================================
   * Register File
   * ===========================================================================================
   * We need to duplicate the parent's RegFile to to the Childs
   * - NOTE: when we return from this function, the return value will
   *         be automatically stored in the Proc.RegFile[HartToExec]'s a0
   *         register. In a traditional fork code this looks like:
   *
   *         pid_t pid = fork()
   *         if pid < 0: // Error
   *         else if pid = 0: // New Child Process
   *         else: // Parent Process
   *
   *         In this case, the value of pid is the value thats returned to a0
   *         It follows that
   *         - The child's regfile MUST have 0 in its a0 (despite its pid != 0 to the RevProc)
   *         - The Parent's a0 register MUST have its ThreadID in it
   * ===========================================================================================
   */

  /*
   Alert the Proc there needs to be a Ctx switch
   Pass the ThreadID that will be switched to once the 
   current pipeline is executed until completion
  */
  // CtxSwitchAlert(ChildThreadID);

  /* Parent's return value is the child's ThreadID */
  // RegFile->RV64[10] = ChildThreadID;

  /* Child's return value is 0 */
  // ChildCtx->GetRegFile()->RV64[10] = 0;

  return;
}                

// 436, rev_close_range(unsigned int fd, unsigned int max_fd, unsigned int flags) 
void RevProc::ECALL_close_range(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: close_range called\n");
  return;
}           

// 437, rev_openat2(int dfd, const char  *filename, struct open_how *how, size_t size) 
void RevProc::ECALL_openat2(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: openat2 called\n");
  return;
}               

// 438, rev_pidfd_getfd(int pidfd, int fd, unsigned int flags) 
void RevProc::ECALL_pidfd_getfd(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: pidfd_getfd called\n");
  return;
}           


// 439, rev_faccessat2(int dfd, const char  *filename, int mode, int flags) 
void RevProc::ECALL_faccessat2(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: faccessat2 called\n");
  return;
}            


// 440, rev_process_madvise(int pidfd, const struct iovec  *vec, size_t vlen, int behavior, unsigned int flags) 
void RevProc::ECALL_process_madvise(){
  output->verbose(CALL_INFO, 2, 0, "ECALL: process_madvise called\n");
  return;
}       










