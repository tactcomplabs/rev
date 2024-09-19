//
// _SYSCALLS_H_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
//
#ifndef _SYSCALLS_H_
#define _SYSCALLS_H_

#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include <time.h>

// The following is required to build on MacOS
// because sigval & siginfo_t are already defined
// in the two headers that get pulled in when building
// rev. If you encounter any future issues where rev
// does not build because of conflicting definitions
// try adding another type from this file to the
// ifdef
#ifdef __APPLE__
#include <sys/signal.h>
#include <sys/unistd.h>
#else
union sigval {
  int   sival_int;
  void* sival_ptr;
};

typedef struct {
  int          si_signo;
  int          si_code;
  union sigval si_value;
  int          si_errno;
  pid_t        si_pid;
  uid_t        si_uid;
  void*        si_addr;
  int          si_status;
  int          si_band;
} siginfo_t;
#endif

// Clone Flags
// clang-format off
#define CSIGNAL              0x000000ff /* Signal mask to be sent at exit */
#define CLONE_VM             0x00000100 /* Set if VM shared between processes */
#define CLONE_FS             0x00000200 /* Set if fs info shared between processes */
#define CLONE_FILES          0x00000400 /* Set if open files shared between processes */
#define CLONE_SIGHAND        0x00000800 /* Set if signal handlers shared */
#define CLONE_PIDFD          0x00001000 /* Set if a pidfd should be placed in the parent */
#define CLONE_PTRACE         0x00002000 /* Set if tracing continues on the child */
#define CLONE_VFORK          0x00004000 /* Set if the parent wants the child to wake it up on mm_release */
#define CLONE_PARENT         0x00008000 /* Set if we want to have the same parent as the cloner */
#define CLONE_THREAD         0x00010000 /* Set to add to same thread group */
#define CLONE_NEWNS          0x00020000 /* Set to create new namespace */
#define CLONE_SYSVSEM        0x00040000 /* Set to shared SVID SEM_UNDO semantics */
#define CLONE_SETTLS         0x00080000 /* Set TLS info */
#define CLONE_PARENT_SETTID  0x00100000 /* Store TID in userlevel buffer before MM copy */
#define CLONE_CHILD_CLEARTID 0x00200000 /* Register exit futex and memory location to clear */
#define CLONE_DETACHED       0x00400000 /* Create clone detached */
#define CLONE_UNTRACED       0x00800000 /* Set if the tracing process can't force CLONE_PTRACE on this clone */
#define CLONE_CHILD_SETTID   0x01000000 /* New cgroup namespace */
#define CLONE_NEWCGROUP      0x02000000 /* New cgroup namespace */
#define CLONE_NEWUTS         0x04000000 /* New utsname group */
#define CLONE_NEWIPC         0x08000000 /* New ipcs */
#define CLONE_NEWUSER        0x10000000 /* New user namespace */
#define CLONE_NEWPID         0x20000000 /* New pid namespace */
#define CLONE_NEWNET         0x40000000 /* New network namespace */
#define CLONE_IO             0x80000000 /* Clone I/O Context */

/* AIO Flags */
#define IOCB_CMD_PREAD 0
#define IOCB_CMD_PWRITE 1
#define IOCB_CMD_FSYNC 2
#define IOCB_CMD_FDSYNC 3
/* 4 was the experimental IOCB_CMD_PREADX */
#define IOCB_CMD_POLL 5
#define IOCB_CMD_NOOP 6
#define IOCB_CMD_PREADV 7
#define IOCB_CMD_PWRITEV 8

/*
 * Valid flags for the "aio_flags" member of the "struct iocb".
 *
 * IOCB_FLAG_RESFD - Set if the "aio_resfd" member of the "struct iocb"
 *                   is valid.
 * IOCB_FLAG_IOPRIO - Set if the "aio_reqprio" member of the "struct iocb"
 *                    is valid.
 */
#define IOCB_FLAG_RESFD         (1 << 0)
#define IOCB_FLAG_IOPRIO        (1 << 1)


/* Flags from sys/mman.h */

/* Protections are chosen from these bits, OR'd together.  The
   implementation does not necessarily support PROT_EXEC or PROT_WRITE
   without PROT_READ.  The only guarantees are that no writing will be
   allowed without PROT_WRITE and no access will be allowed for PROT_NONE. */

#define PROT_READ       0x1             /* Page can be read.  */
#define PROT_WRITE      0x2             /* Page can be written.  */
#define PROT_EXEC       0x4             /* Page can be executed.  */
#define PROT_NONE       0x0             /* Page can not be accessed.  */
#define PROT_GROWSDOWN  0x01000000      /* Extend change to start of growsdown vma (mprotect only).  */
#define PROT_GROWSUP    0x02000000      /* Extend change to start of growsup vma (mprotect only).  */

/* Sharing types (must choose one and only one of these).  */
#define MAP_SHARED      0x01            /* Share changes.  */
#define MAP_PRIVATE     0x02            /* Changes are private.  */
#define MAP_SHARED_VALIDATE     0x03    /* Share changes and validate extension flags.  */
#define MAP_TYPE        0x0f            /* Mask for type of mapping.  */

/* Other flags.  */
#define MAP_FIXED       0x10            /* Interpret addr exactly.  */
#define MAP_FILE        0
# define MAP_ANONYMOUS  0x20            /* Don't use a file.  */
#define MAP_ANON        MAP_ANONYMOUS
/* When MAP_HUGETLB is set bits [26:31] encode the log2 of the huge page size.  */
#define MAP_HUGE_SHIFT  26
#define MAP_HUGE_MASK   0x3f

/* Flags to `msync'.  */
#define MS_ASYNC        1               /* Sync memory asynchronously.  */
#define MS_SYNC         4               /* Synchronous memory sync.  */
#define MS_INVALIDATE   2               /* Invalidate the caches.  */

/* Advice to `madvise'.  */
# define MADV_NORMAL      0     /* No further special treatment.  */
# define MADV_RANDOM      1     /* Expect random page references.  */
# define MADV_SEQUENTIAL  2     /* Expect sequential page references.  */
# define MADV_WILLNEED    3     /* Will need these pages.  */
# define MADV_DONTNEED    4     /* Don't need these pages.  */
# define MADV_FREE        8     /* Free pages only if memory pressure.  */
# define MADV_REMOVE      9     /* Remove these pages and resources.  */
# define MADV_DONTFORK    10    /* Do not inherit across fork.  */
# define MADV_DOFORK      11    /* Do inherit across fork.  */
# define MADV_MERGEABLE   12    /* KSM may merge identical pages.  */
# define MADV_UNMERGEABLE 13    /* KSM may not merge identical pages.  */
# define MADV_HUGEPAGE    14    /* Worth backing with hugepages.  */
# define MADV_NOHUGEPAGE  15    /* Not worth backing with hugepages.  */
# define MADV_DONTDUMP    16    /* Explicity exclude from the core dump, overrides the coredump filter bits.  */
# define MADV_DODUMP      17    /* Clear the MADV_DONTDUMP flag.  */
# define MADV_WIPEONFORK  18    /* Zero memory on fork, child only.  */
# define MADV_KEEPONFORK  19    /* Undo MADV_WIPEONFORK.  */
# define MADV_COLD        20    /* Deactivate these pages.  */
# define MADV_PAGEOUT     21    /* Reclaim these pages.  */
# define MADV_POPULATE_READ 22  /* Populate (prefault) page tables readable.  */
# define MADV_POPULATE_WRITE 23 /* Populate (prefault) page tables writable.  */
# define MADV_DONTNEED_LOCKED 24 /* Like MADV_DONTNEED, but drop locked pages too.  */
# define MADV_COLLAPSE    25    /* Synchronous hugepage collapse.  */
# define MADV_HWPOISON    100   /* Poison a page for testing.  */

// /* The POSIX people had to invent similar names for the same things.  */
// #ifdef __USE_XOPEN2K
// # define POSIX_MADV_NORMAL   0 /* No further special treatment.  */
// # define POSIX_MADV_RANDOM   1 /* Expect random page references.  */
// # define POSIX_MADV_SEQUENTIAL       2 /* Expect sequential page references.  */
// # define POSIX_MADV_WILLNEED 3 /* Will need these pages.  */
// # define POSIX_MADV_DONTNEED 4 /* Don't need these pages.  */
// #endif

/* Flags for `mlockall'.  */
#define MCL_CURRENT     1               /* Lock all currently mapped pages.  */
#define MCL_FUTURE      2               /* Lock all additions to address space.  */
#define MCL_ONFAULT     4               /* Lock all pages that are faulted in.  */

#define F_OK    0
#define R_OK    4
#define W_OK    2
#define X_OK    1

#define SEEK_SET        0
#define SEEK_CUR        1
#define SEEK_END        2

#define STDIN_FILENO    0       /* standard input file descriptor */
#define STDOUT_FILENO   1       /* standard output file descriptor */
#define STDERR_FILENO   2       /* standard error file descriptor */

// clang-format on

struct __aio_sigset;
struct epoll_event;
struct iattr;
struct inode;
struct iocb;
struct io_event;
struct iovec;
struct __kernel_old_itimerval;
struct kexec_segment;
struct linux_dirent;
struct linux_dirent64;
struct list_head;
struct mmap_arg_struct;
struct msgbuf;
struct user_msghdr;
struct mmsghdr;
struct msqid_ds;
struct new_utsname;
struct nfsctl_arg;
struct __old_kernel_stat;
struct oldold_utsname;
struct old_utsname;
struct pollfd;
struct rlimit;
struct rlimit64;
struct rusage;
struct sched_param;
struct sched_attr;
struct sel_arg_struct;
struct semaphore;
struct sembuf;
struct shmid_ds;
struct sockaddr;
struct stat;
struct stat64;
struct statfs;
struct statfs64;
struct statx;
struct sysinfo;
struct timespec;
struct __kernel_old_timeval;
struct __kernel_timex;
struct timezone;
struct tms;
struct utimbuf;
struct mq_attr;
struct compat_stat;
struct old_timeval32;
struct robust_list_head;
struct futex_waitv;
struct getcpu_cache;
struct old_linux_dirent;
struct perf_event_attr;
struct file_handle;
struct sigaltstack;
struct rseq;
union bpf_attr;
struct io_uring_params;
struct clone_args;
struct open_how;
struct mount_attr;
struct landlock_ruleset_attr;
struct old_timespec32;
struct __kernel_itimerspec;
struct sigevent;
struct sigaction;
struct siginfo;

typedef uint32_t      rwf_t;
typedef unsigned long aio_context_t;
typedef uint16_t      aio_key;
typedef uint16_t      aio_rw_flags;
typedef int32_t       key_serial_t; /* key handle serial number */
typedef uint32_t      key_perm_t;   /* key handle permissions mask */

typedef struct __user_cap_header_struct {
  uint32_t version;
  int      pid;
}* cap_user_header_t;

#define __kernel_timespec timespec

typedef struct __user_cap_data_struct {
  uint32_t effective;
  uint32_t permitted;
  uint32_t inheritable;
}* cap_user_data_t;

/* To optimize the implementation one can use the following struct.  */
struct aioinit {
  int aio_threads;   /* Maximum number of threads.  */
  int aio_num;       /* Number of expected simultaneous requests.  */
  int aio_locks;     /* Not used.  */
  int aio_usedba;    /* Not used.  */
  int aio_debug;     /* Not used.  */
  int aio_numusers;  /* Not used.  */
  int aio_idle_time; /* Number of seconds before idle thread terminates.  */
  int aio_reserved;
};

typedef unsigned short umode_t;
typedef unsigned short qid_t;
typedef off_t          loff_t;

struct clone_args {
  int flags;        /* Flags bit mask */
  int pidfd;        /* Where to store PID file descriptor (int *) */
  int child_tid;    /* Where to store child TID, in child's memory (pid_t *) */
  int parent_tid;   /* Where to store child TID, in parent's memory (pid_t *) */
  int exit_signal;  /* Signal to deliver to parent on child termination */
  int stack;        /* Pointer to lowest byte of stack */
  int stack_size;   /* Size of stack */
  int tls;          /* Location of new TLS */
  int set_tid;      /* Pointer to a pid_t array (since Linux 5.5) */
  int set_tid_size; /* Number of elements in set_tid (since Linux 5.5) */
  int cgroup;       /* File descriptor for target cgroup of child (since Linux 5.7) */
};

/* read() from /dev/aio returns these structures. */
struct io_event {
  uint64_t data; /* the data field from the iocb */
  uint64_t obj;  /* what iocb this event came from */
  int64_t  res;  /* result code for this event */
  int64_t  res2; /* secondary result */
};

struct iocb {
  uint64_t aio_data;
  uint16_t aio_key;
  uint16_t aio_rw_flags;
  uint16_t aio_lio_opcode;
  int16_t  aio_reqprio;
  uint32_t aio_fildes;
  uint64_t aio_buf;
  uint64_t aio_nbytes;
  int64_t  aio_offset;
  uint64_t aio_reserved2;
  uint32_t aio_flags;
  uint32_t aio_resfd;
};

struct rev_cpuinfo {
  uint32_t cores;
  uint32_t harts_per_core;
};

struct rev_stats {
  uint64_t cycles;
  uint64_t instructions;
};

#ifndef SYSCALL_TYPES_ONLY

//-----------------------------------------------------------------------------
// Create a Rev wrapper to a System Call
// __attribute__((naked)) prevents GCC/Clang from modifying any registers in
// the prologue or epilogue, and prevents optimizing away the ECALL at -O3
//-----------------------------------------------------------------------------
// clang-format off
#define REV_SYSCALL(NUM, PROTO) __attribute__((naked)) \
  static PROTO{ asm(" li a7," #NUM "; ecall; ret"); }

REV_SYSCALL(    0, int rev_io_setup(unsigned nr_reqs, aio_context_t *ctx) );
REV_SYSCALL(    1, int rev_io_destroy(aio_context_t ctx) );
REV_SYSCALL(    2, int rev_io_submit(aio_context_t ctx_id, long nr, struct iocb **iocbpp) );
REV_SYSCALL(    3, int rev_io_cancel(aio_context_t ctx_id, struct iocb *iocb, struct io_event *result) );
REV_SYSCALL(    4, int rev_io_getevents(aio_context_t ctx_id, long min_nr, long nr, struct io_event *events, struct __kernel_timespec *timeout) );
REV_SYSCALL(    5, int rev_setxattr(const char *path, const char *name, const void *value, size_t size, int flags) );
REV_SYSCALL(    6, int rev_lsetxattr(const char *path, const char *name, const void *value, size_t size, int flags) );
REV_SYSCALL(    7, int rev_fsetxattr(int fd, const char *name, const void *value, size_t size, int flags) );
REV_SYSCALL(    8, ssize_t rev_getxattr(const char *path, const char *name, void *value, size_t size) );
REV_SYSCALL(    9, ssize_t rev_lgetxattr(const char *path, const char *name, void *value, size_t size) );
REV_SYSCALL(   10, ssize_t rev_fgetxattr(int fd, const char *name, void *value, size_t size) );
REV_SYSCALL(   11, ssize_t rev_listxattr(const char *path, char *list, size_t size) );
REV_SYSCALL(   12, ssize_t rev_llistxattr(const char *path, char *list, size_t size) );
REV_SYSCALL(   13, ssize_t rev_flistxattr(int fd, char *list, size_t size) );
REV_SYSCALL(   14, int rev_removexattr(const char *path, const char *name) );
REV_SYSCALL(   15, int rev_lremovexattr(const char *path, const char *name) );
REV_SYSCALL(   16, int rev_fremovexattr(int fd, const char *name) );
REV_SYSCALL(   17, int rev_getcwd(char *buf, unsigned long size) );
REV_SYSCALL(   18, int rev_lookup_dcookie(uint64_t cookie64, char *buf, size_t len) );
REV_SYSCALL(   19, int rev_eventfd2(unsigned int count, int flags) );
REV_SYSCALL(   20, int rev_epoll_create1(int flags) );
REV_SYSCALL(   21, int rev_epoll_ctl(int epfd, int op, int fd, struct epoll_event *event) );
REV_SYSCALL(   22, int rev_epoll_pwait(int epfd, struct epoll_event *events, int maxevents, int timeout, const sigset_t *sigmask, size_t sigsetsize) );
REV_SYSCALL(   23, int rev_dup(unsigned int fildes) );
REV_SYSCALL(   24, int rev_dup3(unsigned int oldfd, unsigned int newfd, int flags) );
REV_SYSCALL(   25, int rev_fcntl64(int fd, unsigned int cmd, unsigned long arg) );
REV_SYSCALL(   26, int rev_inotify_init1(int flags) );
REV_SYSCALL(   27, int rev_inotify_add_watch(int fd, const char *path, uint32_t mask) );
REV_SYSCALL(   28, int rev_inotify_rm_watch(int fd, int32_t wd) );
REV_SYSCALL(   29, int rev_ioctl(int fd, unsigned int cmd, unsigned long arg) );
REV_SYSCALL(   30, int rev_ioprio_set(int which, int who, int ioprio) );
REV_SYSCALL(   31, int rev_ioprio_get(int which, int who) );
REV_SYSCALL(   32, int rev_flock(int fd, unsigned int cmd) );
REV_SYSCALL(   33, int rev_mknodat(int dfd, const char * filename, umode_t mode, unsigned dev) );
REV_SYSCALL(   34, int rev_mkdirat(int dfd, const char * pathname, umode_t mode) );
REV_SYSCALL(   35, int rev_unlinkat(int dfd, const char * pathname, int flag) );
REV_SYSCALL(   36, int rev_symlinkat(const char * oldname, int newdfd, const char * newname) );
REV_SYSCALL(   37, int rev_linkat(int dfd, const char * pathname, int flag) );
REV_SYSCALL(   38, int rev_renameat(int olddfd, const char * oldname, int newdfd, const char * newname) );
REV_SYSCALL(   39, int rev_umount(char *name, int flags) );
REV_SYSCALL(   40, int rev_mount(char *name, int flags) );
REV_SYSCALL(   41, int rev_pivot_root(const char *new_root, const char *put_old) );
REV_SYSCALL(   42, int rev_ni_syscall(void) );
REV_SYSCALL(   43, int rev_statfs64(const char *path, size_t sz, struct statfs64 *buf) );
REV_SYSCALL(   44, int rev_fstatfs64(int fd, size_t sz, struct statfs64 *buf) );
REV_SYSCALL(   45, int rev_truncate64(const char *path, loff_t length) );
REV_SYSCALL(   46, int rev_ftruncate64(int fd, loff_t length) );
REV_SYSCALL(   47, int rev_fallocate(int fd, int mode, loff_t offset, loff_t len) );
REV_SYSCALL(   48, int rev_faccessat(int dfd, const char *filename, int mode) );
REV_SYSCALL(   49, int rev_chdir(const char *filename) );
REV_SYSCALL(   50, int rev_fchdir(int fd) );
REV_SYSCALL(   51, int rev_chroot(const char *filename) );
REV_SYSCALL(   52, int rev_fchmod(int fd, umode_t mode) );
REV_SYSCALL(   53, int rev_fchmodat(int dfd, const char * filename, umode_t mode) );
REV_SYSCALL(   54, int rev_fchownat(int dfd, const char *filename, uid_t user, gid_t group, int flag) );
REV_SYSCALL(   55, int rev_fchown(int fd, uid_t user, gid_t group) );
REV_SYSCALL(   56, int rev_openat(int dfd, const char *filename, int flags, umode_t mode) );
REV_SYSCALL(   57, int rev_close(int fd) );
REV_SYSCALL(   58, int rev_vhangup(void) );
REV_SYSCALL(   59, int rev_pipe2(int *fildes, int flags) );
REV_SYSCALL(   60, int rev_quotactl(unsigned int cmd, const char *special, qid_t id, void *addr) );
REV_SYSCALL(   61, ssize_t rev_getdents64(unsigned int fd, struct linux_dirent64 *dirent, unsigned int count) );
REV_SYSCALL(   62, int rev_llseek(unsigned int fd, unsigned long offset_high, unsigned long offset_low, loff_t *result, unsigned int whence) );
REV_SYSCALL(   63, ssize_t rev_read(unsigned int fd, char *buf, size_t count) );
REV_SYSCALL(   64, ssize_t rev_write(unsigned int fd, const char *buf, size_t count) );
REV_SYSCALL(   65, ssize_t rev_readv(unsigned long fd, const struct iovec *vec, unsigned long vlen) );
REV_SYSCALL(   66, ssize_t rev_writev(unsigned long fd, const struct iovec *vec, unsigned long vlen) );
REV_SYSCALL(   67, ssize_t rev_pread64(unsigned int fd, char *buf, size_t count, loff_t pos) );
REV_SYSCALL(   68, ssize_t rev_pwrite64(unsigned int fd, const char *buf, size_t count, loff_t pos) );
REV_SYSCALL(   69, ssize_t rev_preadv(unsigned long fd, const struct iovec *vec, unsigned long vlen, unsigned long pos_l, unsigned long pos_h) );
REV_SYSCALL(   70, ssize_t rev_pwritev(unsigned long fd, const struct iovec *vec, unsigned long vlen, unsigned long pos_l, unsigned long pos_h) );
REV_SYSCALL(   71, ssize_t rev_sendfile64(int out_fd, int in_fd, loff_t *offset, size_t count) );
REV_SYSCALL(   72, int rev_pselect6_time32(int n, fd_set *inp, fd_set *outp, fd_set *exp, struct old_timespec32 *tsp, void *sig) );
REV_SYSCALL(   73, int rev_ppoll_time32(struct pollfd *ufds, unsigned int nfds, struct old_timespec32 *tsp, const sigset_t *sigmask, size_t sigsetsize) );
REV_SYSCALL(   74, int rev_signalfd4(int ufd, sigset_t *user_mask, size_t sizemask, int flags) );
REV_SYSCALL(   75, ssize_t rev_vmsplice(int fd, const struct iovec *iov, unsigned long nr_segs, unsigned int flags) );
REV_SYSCALL(   76, ssize_t rev_splice(int fd, const struct iovec *iov, unsigned long nr_segs, unsigned int flags) );
REV_SYSCALL(   77, ssize_t rev_tee(int fdin, int fdout, size_t len, unsigned int flags) );
REV_SYSCALL(   78, ssize_t rev_readlinkat(int dfd, const char *path, char *buf, int bufsiz) );
REV_SYSCALL(   79, int rev_newfstatat(int dfd, const char *filename, struct stat *statbuf, int flag) );
REV_SYSCALL(   80, int rev_newfstat(int fd, struct stat *statbuf) );
REV_SYSCALL(   81, int rev_sync(void) );
REV_SYSCALL(   82, int rev_fsync(int fd) );
REV_SYSCALL(   83, int rev_fdatasync(int fd) );
REV_SYSCALL(   84, int rev_sync_file_range2(int fd, unsigned int flags, loff_t offset, loff_t nbytes) );
REV_SYSCALL(   84, int rev_sync_file_range(int fd, loff_t offset, loff_t nbytes, unsigned int flags) );
REV_SYSCALL(   85, int rev_timerfd_create(int clockid, int flags) );
REV_SYSCALL(   86, int rev_timerfd_settime(int ufd, int flags, const struct __kernel_itimerspec *utmr, struct __kernel_itimerspec *otmr) );
REV_SYSCALL(   87, int rev_timerfd_gettime(int ufd, struct __kernel_itimerspec *otmr) );
REV_SYSCALL(   88, int rev_utimensat(int dfd, const char *filename, struct __kernel_timespec *utimes, int flags) );
REV_SYSCALL(   89, int rev_acct(const char *name) );
REV_SYSCALL(   90, int rev_capget(cap_user_header_t header, cap_user_data_t dataptr) );
REV_SYSCALL(   91, int rev_capset(cap_user_header_t header, const cap_user_data_t data) );
REV_SYSCALL(   92, int rev_personality(unsigned int personality) );
REV_SYSCALL(   93, int rev_exit(int error_code) );
REV_SYSCALL(   94, int rev_exit_group(int error_code) );
REV_SYSCALL(   95, int rev_waitid(int which, pid_t pid, struct siginfo *infop, int options, struct rusage *ru) );
REV_SYSCALL(   96, pid_t rev_set_tid_address(int *tidptr) );
REV_SYSCALL(   97, int rev_unshare(int unshare_flags) );
REV_SYSCALL(   98, int rev_futex(uint32_t *uaddr, int op, uint32_t val, struct __kernel_timespec *utime, uint32_t *uaddr2, uint32_t val3) );
REV_SYSCALL(   99, int rev_set_robust_list(struct robust_list_head *head, size_t len) );
REV_SYSCALL(  100, int rev_get_robust_list(int pid, struct robust_list_head * *head_ptr, size_t *len_ptr) );
REV_SYSCALL(  101, int rev_nanosleep(struct __kernel_timespec *rqtp, struct __kernel_timespec *rmtp) );
REV_SYSCALL(  102, int rev_getitimer(int which, struct __kernel_old_itimerval *value) );
REV_SYSCALL(  103, int rev_setitimer(int which, struct __kernel_old_itimerval *value, struct __kernel_old_itimerval *ovalue) );
REV_SYSCALL(  104, int rev_kexec_load(unsigned long entry, unsigned long nr_segments, struct kexec_segment *segments, unsigned long flags) );
REV_SYSCALL(  105, int rev_init_module(void *umod, unsigned long len, const char *uargs) );
REV_SYSCALL(  106, int rev_delete_module(const char *name_user, unsigned int flags) );
REV_SYSCALL(  107, int rev_timer_create(clockid_t which_clock, struct sigevent *timer_event_spec, timer_t * created_timer_id) );
REV_SYSCALL(  108, int rev_timer_gettime(timer_t timer_id, struct __kernel_itimerspec *setting) );
REV_SYSCALL(  109, int rev_timer_getoverrun(timer_t timer_id) );
REV_SYSCALL(  110, int rev_timer_settime(timer_t timer_id, int flags, const struct __kernel_itimerspec *new_setting, struct __kernel_itimerspec *old_setting) );
REV_SYSCALL(  111, int rev_timer_delete(timer_t timer_id) );
REV_SYSCALL(  112, int rev_clock_settime(clockid_t which_clock, const struct __kernel_timespec *tp) );
REV_SYSCALL(  113, int rev_clock_gettime(clockid_t which_clock, struct __kernel_timespec *tp) );
REV_SYSCALL(  114, int rev_clock_getres(clockid_t which_clock, struct __kernel_timespec *tp) );
REV_SYSCALL(  115, int rev_clock_nanosleep(clockid_t which_clock, int flags, const struct __kernel_timespec *rqtp, struct __kernel_timespec *rmtp) );
REV_SYSCALL(  116, int rev_syslog(int type, char *buf, int len) );
REV_SYSCALL(  117, int rev_ptrace(long request, long pid, unsigned long addr, unsigned long data) );
REV_SYSCALL(  118, int rev_sched_setparam(pid_t pid, struct sched_param *param) );
REV_SYSCALL(  119, int rev_sched_setscheduler(pid_t pid, int policy, struct sched_param *param) );
REV_SYSCALL(  120, int rev_sched_getscheduler(pid_t pid) );
REV_SYSCALL(  121, int rev_sched_getparam(pid_t pid, struct sched_param *param) );
REV_SYSCALL(  122, int rev_sched_setaffinity(pid_t pid, unsigned int len, unsigned long *user_mask_ptr) );
REV_SYSCALL(  123, int rev_sched_getaffinity(pid_t pid, unsigned int len, unsigned long *user_mask_ptr) );
REV_SYSCALL(  124, int rev_sched_yield(void) );
REV_SYSCALL(  125, int rev_sched_get_priority_max(int policy) );
REV_SYSCALL(  126, int rev_sched_get_priority_min(int policy) );
REV_SYSCALL(  127, int rev_sched_rr_get_interval(pid_t pid, struct __kernel_timespec *interval) );
REV_SYSCALL(  128, long rev_restart_syscall(void) );
REV_SYSCALL(  129, int rev_kill(pid_t pid, int sig) );
REV_SYSCALL(  130, int rev_tkill(pid_t pid, int sig) );
REV_SYSCALL(  131, int rev_tgkill(pid_t tgid, pid_t pid, int sig) );
REV_SYSCALL(  132, int rev_sigaltstack(const struct sigaltstack *uss, struct sigaltstack *uoss) );
REV_SYSCALL(  133, int rev_rt_sigsuspend(sigset_t *unewset, size_t sigsetsize) );
REV_SYSCALL(  134, int rev_rt_sigaction(int signum, const struct sigaction *act, struct sigaction *oldact, size_t sigsetsize) );
REV_SYSCALL(  135, int rev_rt_sigprocmask(int how, sigset_t *set, sigset_t *oset, size_t sigsetsize) );
REV_SYSCALL(  136, int rev_rt_sigpending(sigset_t *set, size_t sigsetsize) );
REV_SYSCALL(  137, int rev_rt_sigtimedwait_time32(const sigset_t *uthese, siginfo_t *uinfo, const struct old_timespec32 *uts, size_t sigsetsize) );
REV_SYSCALL(  138, int rev_rt_sigqueueinfo(pid_t pid, int sig, siginfo_t *uinfo) );
REV_SYSCALL(  140, int rev_setpriority(int which, int who, int niceval) );
REV_SYSCALL(  141, int rev_getpriority(int which, int who) );
REV_SYSCALL(  142, int rev_reboot(int magic1, int magic2, unsigned int cmd, void *arg) );
REV_SYSCALL(  143, int rev_setregid(gid_t rgid, gid_t egid) );
REV_SYSCALL(  144, int rev_setgid(gid_t gid) );
REV_SYSCALL(  145, int rev_setreuid(uid_t ruid, uid_t euid) );
REV_SYSCALL(  146, int rev_setuid(uid_t uid) );
REV_SYSCALL(  147, int rev_setresuid(uid_t ruid, uid_t euid, uid_t suid) );
REV_SYSCALL(  148, int rev_getresuid(uid_t *ruid, uid_t *euid, uid_t *suid) );
REV_SYSCALL(  149, int rev_setresgid(gid_t rgid, gid_t egid, gid_t sgid) );
REV_SYSCALL(  150, int rev_getresgid(gid_t *rgid, gid_t *egid, gid_t *sgid) );
REV_SYSCALL(  151, int rev_setfsuid(uid_t uid) );
REV_SYSCALL(  152, int rev_setfsgid(gid_t gid) );
REV_SYSCALL(  153, clock_t rev_times(struct tms *tbuf) );
REV_SYSCALL(  154, int rev_setpgid(pid_t pid, pid_t pgid) );
REV_SYSCALL(  155, int rev_getpgid(pid_t pid) );
REV_SYSCALL(  156, int rev_getsid(pid_t pid) );
REV_SYSCALL(  157, int rev_setsid(void) );
REV_SYSCALL(  158, int rev_getgroups(int gidsetsize, gid_t *grouplist) );
REV_SYSCALL(  159, int rev_setgroups(int gidsetsize, gid_t *grouplist) );
REV_SYSCALL(  160, int rev_newuname(struct new_utsname *name) );
REV_SYSCALL(  161, int rev_sethostname(char *name, int len) );
REV_SYSCALL(  162, int rev_setdomainname(char *name, int len) );
REV_SYSCALL(  163, int rev_getrlimit(unsigned int resource, struct rlimit *rlim) );
REV_SYSCALL(  164, int rev_setrlimit(unsigned int resource, struct rlimit *rlim) );
REV_SYSCALL(  165, int rev_getrusage(int who, struct rusage *ru) );
REV_SYSCALL(  166, int rev_umask(int mask) );
REV_SYSCALL(  167, int rev_prctl(int option, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5) );
REV_SYSCALL(  168, int rev_getcpu(unsigned *cpu, unsigned *node, struct getcpu_cache *cache) );
REV_SYSCALL(  169, int rev_gettimeofday(struct __kernel_old_timeval *tv, struct timezone *tz) );
REV_SYSCALL(  170, int rev_settimeofday(struct __kernel_old_timeval *tv, struct timezone *tz) );
REV_SYSCALL(  171, int rev_adjtimex(struct __kernel_timex *txc_p) );
REV_SYSCALL(  172, int rev_getpid(void) );
REV_SYSCALL(  173, int rev_getppid(void) );
REV_SYSCALL(  174, int rev_getuid(void) );
REV_SYSCALL(  175, int rev_geteuid(void) );
REV_SYSCALL(  176, int rev_getgid(void) );
REV_SYSCALL(  177, int rev_getegid(void) );
REV_SYSCALL(  178, int rev_gettid(void) );
REV_SYSCALL(  179, int rev_sysinfo(struct sysinfo *info) );
REV_SYSCALL(  180, int rev_mq_open(const char *name, int oflag, umode_t mode, struct mq_attr *attr) );
REV_SYSCALL(  181, int rev_mq_unlink(const char *name) );
#if 0   // whenever mqd_t is defined -- it lives in <sys/msg.h>
REV_SYSCALL(  182, int rev_mq_timedsend(mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned int msg_prio, const struct __kernel_timespec *abs_timeout) );
REV_SYSCALL(  183, int rev_mq_timedreceive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned int *msg_prio, const struct __kernel_timespec *abs_timeout) );
REV_SYSCALL(  184, int rev_mq_notify(mqd_t mqdes, const struct sigevent *notification) );
REV_SYSCALL(  185, int rev_mq_getsetattr(mqd_t mqdes, const struct mq_attr *mqstat, struct mq_attr *omqstat) );
#endif
REV_SYSCALL(  186, int rev_msgget(key_t key, int msgflg) );
REV_SYSCALL(  187, int rev_old_msgctl(int msqid, int cmd, struct msqid_ds *buf) );
REV_SYSCALL(  188, ssize_t rev_msgrcv(int msqid, struct msgbuf *msgp, size_t msgsz, long msgtyp, int msgflg) );
REV_SYSCALL(  189, int rev_msgsnd(int msqid, struct msgbuf *msgp, size_t msgsz, int msgflg) );
REV_SYSCALL(  190, int rev_semget(key_t key, int nsems, int semflg) );
REV_SYSCALL(  191, int rev_semctl(int semid, int semnum, int cmd, unsigned long arg) );
REV_SYSCALL(  192, int rev_semtimedop(int semid, struct sembuf *sops, unsigned nsops, const struct __kernel_timespec *timeout) );
REV_SYSCALL(  193, int rev_semop(int semid, struct sembuf *sops, unsigned nsops) );
REV_SYSCALL(  194, int rev_shmget(key_t key, size_t size, int flag) );
REV_SYSCALL(  195, int rev_old_shmctl(int shmid, int cmd, struct shmid_ds *buf) );
REV_SYSCALL(  196, int rev_shmat(int shmid, char *shmaddr, int shmflg) );
REV_SYSCALL(  197, int rev_shmdt(char *shmaddr) );
REV_SYSCALL(  198, int rev_socket(int domain, int type, int proocol) );
REV_SYSCALL(  199, int rev_socketpair(int domain, int type, int protocol, int *sv) );
REV_SYSCALL(  200, int rev_bind(int sockfd, struct sockaddr *addr, int addrlen) );
REV_SYSCALL(  201, int rev_listen(int sockfd, int backlog) );
REV_SYSCALL(  202, int rev_accept(int sockfd, struct sockaddr *addr, int *addrlen) );
REV_SYSCALL(  203, int rev_connect(int sockfd, struct sockaddr *addr, int addrlen) );
REV_SYSCALL(  204, int rev_getsockname(int sockfd, struct sockaddr *addr, int *addrlen) );
REV_SYSCALL(  205, int rev_getpeername(int sockfd, struct sockaddr *addr, int *addrlen) );
REV_SYSCALL(  206, ssize_t rev_sendto(int sockfd, void *buf, size_t len, int flags, struct sockaddr *dest_addr, int addrlen) );
REV_SYSCALL(  207, ssize_t rev_recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, int *addrlen) );
REV_SYSCALL(  208, int rev_setsockopt(int sockfd, int level, int optname, char *optval, int optlen) );
REV_SYSCALL(  209, int rev_getsockopt(int sockfd, int level, int optname, char *optval, int *optlen) );
REV_SYSCALL(  210, int rev_shutdown(int sockfd, int how) );
REV_SYSCALL(  211, ssize_t rev_sendmsg(int sockfd, struct user_msghdr *msg, unsigned flags) );
REV_SYSCALL(  212, ssize_t rev_recvmsg(int sockfd, struct user_msghdr *msg, unsigned flags) );
REV_SYSCALL(  213, ssize_t rev_readahead(int sockfd, loff_t offset, size_t count) );
REV_SYSCALL(  214, int rev_brk(unsigned long brk) );
REV_SYSCALL(  215, int rev_munmap(unsigned long addr, size_t len) );
REV_SYSCALL(  216, int rev_mremap(unsigned long addr, unsigned long old_len, unsigned long new_len, unsigned long flags, unsigned long new_addr) );
REV_SYSCALL(  217, int rev_add_key(const char *_type, const char *_description, const void *_payload, size_t plen, key_serial_t destringid) );
REV_SYSCALL(  218, int rev_request_key(const char *_type, const char *_description, const char *_callout_info, key_serial_t destringid) );
REV_SYSCALL(  219, int rev_keyctl(int cmd, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5) );
REV_SYSCALL(  220, int rev_fork(void) );
REV_SYSCALL(  220, int rev_clone(unsigned long flags, void *stack, int *parent_tid, void *tls, int *child_tid) );
REV_SYSCALL(  221, int rev_execve(const char *filename, const char *const *argv, const char *const *envp) );
REV_SYSCALL(  222, uint64_t rev_mmap(uint64_t addr, size_t length, int prot, int flags, int fd, off_t offset) );
REV_SYSCALL(  223, int rev_fadvise64_64(int fd, loff_t offset, loff_t len, int advice) );
REV_SYSCALL(  224, int rev_swapon(const char *specialfile, int swap_flags) );
REV_SYSCALL(  225, int rev_swapoff(const char *specialfile) );
REV_SYSCALL(  226, int rev_mprotect(unsigned long start, size_t len, unsigned long prot) );
REV_SYSCALL(  227, int rev_msync(unsigned long start, size_t len, int flags) );
REV_SYSCALL(  228, int rev_mlock(unsigned long start, size_t len) );
REV_SYSCALL(  229, int rev_munlock(unsigned long start, size_t len) );
REV_SYSCALL(  230, int rev_mlockall(int flags) );
REV_SYSCALL(  231, int rev_munlockall(void) );
REV_SYSCALL(  232, int rev_mincore(unsigned long start, size_t len, unsigned char * vec) );
REV_SYSCALL(  233, int rev_madvise(unsigned long start, size_t len, int behavior) );
REV_SYSCALL(  234, long rev_remap_file_pages(unsigned long start, unsigned long size, unsigned long prot, unsigned long pgoff, unsigned long flags) );
REV_SYSCALL(  235, long rev_mbind(unsigned long start, unsigned long len, unsigned long mode, const unsigned long *nmask, unsigned long maxnode, unsigned flags) );
REV_SYSCALL(  236, int rev_get_mempolicy(int *policy, unsigned long *nmask, unsigned long maxnode, unsigned long addr, unsigned long flags) );
REV_SYSCALL(  237, int rev_set_mempolicy(int mode, const unsigned long *nmask, unsigned long maxnode) );
REV_SYSCALL(  238, int rev_migrate_pages(pid_t pid, unsigned long maxnode, const unsigned long *from, const unsigned long *to) );
REV_SYSCALL(  239, int rev_move_pages(pid_t pid, unsigned long nr_pages, const void * *pages, const int *nodes, int *status, int flags) );
REV_SYSCALL(  240, int rev_rt_tgsigqueueinfo(pid_t tgid, pid_t pid, int sig, siginfo_t *uinfo) );
REV_SYSCALL(  241, int rev_perf_event_open(void) );
REV_SYSCALL(  242, int rev_accept4(int sockfd, struct sockaddr *addr, int *addrlen, int flags) );
REV_SYSCALL(  243, int rev_recvmmsg_time32(int fd, struct mmsghdr *msg, unsigned int vlen, unsigned flags, struct old_timespec32* timeout) );
REV_SYSCALL(  260, int rev_wait4(pid_t pid, int *stat_addr, int options, struct rusage *ru) );
REV_SYSCALL(  261, int rev_prlimit64(pid_t pid, unsigned int resource, const struct rlimit64 *new_rlim, struct rlimit64 *old_rlim) );
REV_SYSCALL(  262, int rev_fanotify_init(unsigned int flags, unsigned int event_int) );
REV_SYSCALL(  263, int rev_fanotify_mark(int fanotify_fd, unsigned int flags, uint64_t mask, int fd, const char *p) );
REV_SYSCALL(  264, int rev_name_to_handle_at(int dfd, const char *name, struct file_handle *handle, int *mnt_id, int flag) );
REV_SYSCALL(  265, int rev_open_by_handle_at(int mountdirfd, struct file_handle *handle, int flags) );
REV_SYSCALL(  266, int rev_clock_adjtime(clockid_t which_clock, struct __kernel_timex *tx) );
REV_SYSCALL(  267, int rev_syncfs(int fd) );
REV_SYSCALL(  268, int rev_setns(int fd, int nstype ) );
REV_SYSCALL(  269, int rev_sendmmsg(int fd, struct mmsghdr *msg, unsigned int vlen, unsigned flags) );
REV_SYSCALL(  270, int rev_process_vm_readv(pid_t pid, const struct iovec *lvec, unsigned long liovcnt, const struct iovec *rvec, unsigned long riovcnt, unsigned long flags) );
REV_SYSCALL(  271, int rev_process_vm_writev(pid_t pid, const struct iovec *lvec, unsigned long liovcnt, const struct iovec *rvec, unsigned long riovcnt, unsigned long flags) );
REV_SYSCALL(  272, int rev_kcmp(pid_t pid1, pid_t pid2, int type, unsigned long idx1, unsigned long idx2) );
REV_SYSCALL(  273, int rev_finit_module(int fd, const char *uargs, int flags) );
REV_SYSCALL(  274, int rev_sched_setattr(pid_t pid, struct sched_attr *attr, unsigned int size, unsigned int flags) );
REV_SYSCALL(  275, int rev_sched_getattr(pid_t pid, struct sched_attr *attr, unsigned int size, unsigned int flags) );
REV_SYSCALL(  276, int rev_renameat2(int olddfd, const char *oldname, int newdfd, const char *newname, unsigned int flags) );
REV_SYSCALL(  277, int rev_seccomp(unsigned int op, unsigned int flags, void *uargs) );
REV_SYSCALL(  278, int rev_getrandom(char *buf, size_t count, unsigned int flags) );
REV_SYSCALL(  279, int rev_memfd_create(const char *uname_ptr, unsigned int flags) );
REV_SYSCALL(  280, int rev_bpf(int cmd, union bpf_attr *attr, unsigned int size) );
REV_SYSCALL(  281, int rev_execveat(int dfd, const char *filename, const char *const *argv, const char *const *envp, int flags) );
REV_SYSCALL(  282, int rev_userfaultfd(int flags) );
REV_SYSCALL(  283, int rev_membarrier(int cmd, unsigned int flags, int cpu_id) );
REV_SYSCALL(  284, int rev_mlock2(unsigned long start, size_t len, int flags) );
REV_SYSCALL(  285, int rev_copy_file_range(int fd_in, loff_t *off_in, int fd_out, loff_t *off_out, size_t len, unsigned int flags) );
REV_SYSCALL(  286, int rev_preadv2(int fd, const struct iovec *vec, unsigned long vlen, unsigned long pos_l, unsigned long pos_h, rwf_t flags) );
REV_SYSCALL(  287, int rev_pwritev2(int fd, const struct iovec *vec, unsigned long vlen, unsigned long pos_l, unsigned long pos_h, rwf_t flags) );
REV_SYSCALL(  288, int rev_pkey_mprotect(unsigned long start, size_t len, unsigned long prot) );
REV_SYSCALL(  289, int rev_pkey_alloc(unsigned long flags, unsigned long init_val) );
REV_SYSCALL(  290, int rev_pkey_free(int pkey) );
REV_SYSCALL(  291, int rev_statx(int dfd, const char *path, unsigned flags, unsigned mask, struct statx *buffer) );
REV_SYSCALL(  292, int rev_io_pgetevents(aio_context_t ctx_id, long min_nr, long nr, struct io_event *events, struct __kernel_timespec *timeout, const struct __aio_sigset *sig) );
REV_SYSCALL(  293, int rev_rseq(struct rseq *rseq, uint32_t rseq_len, int flags, uint32_t sig) );
REV_SYSCALL(  294, int rev_kexec_file_load(int kernel_fd, int initrd_fd, unsigned long cmdline_len, const char *cmdline_ptr, unsigned long flags) );

REV_SYSCALL(  424, int rev_pidfd_send_signal(int pidfd, int sig, siginfo_t *info, unsigned int flags) );
REV_SYSCALL(  425, int rev_io_uring_setup(uint32_t entries, struct io_uring_params *p) );
REV_SYSCALL(  426, int rev_io_uring_enter(int fd, uint32_t to_submit, uint32_t min_complete, uint32_t flags, const sigset_t *sig, size_t sigsz) );
REV_SYSCALL(  427, int rev_io_uring_register(int fd, unsigned int op, void *arg, unsigned int nr_args) );
REV_SYSCALL(  428, int rev_open_tree(int dfd, const char *path, unsigned flags) );
REV_SYSCALL(  429, int rev_move_mount(int from_dfd, const char *from_path, int to_dfd, const char *to_path, unsigned int ms_flags) );
REV_SYSCALL(  430, int rev_fsopen(const char *fs_name, unsigned int flags) );
REV_SYSCALL(  431, int rev_fsconfig(int fs_fd, unsigned int cmd, const char *key, const void *value, int aux) );
REV_SYSCALL(  432, int rev_fsmount(int fs_fd, unsigned int flags, unsigned int ms_flags) );
REV_SYSCALL(  433, int rev_fspick(int dfd, const char *path, unsigned int flags) );
REV_SYSCALL(  434, int rev_pidfd_open(pid_t pid, unsigned int flags) );
REV_SYSCALL(  435, int rev_clone3(struct clone_args *uargs, size_t size) );
REV_SYSCALL(  436, int rev_close_range(int fd, unsigned int max_fd, unsigned int flags) );
REV_SYSCALL(  437, int rev_openat2(int dfd, const char *filename, struct open_how *how, size_t size) );
REV_SYSCALL(  438, int rev_pidfd_getfd(int pidfd, int fd, unsigned int flags) );
REV_SYSCALL(  439, int rev_faccessat2(int dfd, const char *filename, int mode, int flags) );
REV_SYSCALL(  440, int rev_process_madvise(int pidfd, const struct iovec *vec, size_t vlen, int behavior, unsigned int flags) );

REV_SYSCALL(  500, int rev_cpuinfo(struct rev_cpuinfo *info) );
REV_SYSCALL(  501, int rev_perf_stats(struct rev_stats *stats) );

// ==================== REV PTHREADS
typedef unsigned long int rev_pthread_t;
// pthread_t *restrict thread
// const pthread_attr_t *restrict attr - NOT USED RIGHT NOW
// void *(*start_routine)(void *)
// void *restrict arg);
REV_SYSCALL( 1000, int rev_pthread_create( rev_pthread_t* thread, void* attr, void* fn, void* arg ) );
REV_SYSCALL( 1001, int rev_pthread_join( rev_pthread_t thread ) );

// ==================== REV MEMDUMP
REV_SYSCALL( 9000, void dump_mem_range( uint64_t addr, uint64_t size ) );
REV_SYSCALL( 9001, void dump_mem_range_to_file( const char* outputFile, uint64_t addr, uint64_t size ) );
REV_SYSCALL( 9002, void dump_stack( ) );
REV_SYSCALL( 9003, void dump_stack_to_file( const char* outputFile ) );
REV_SYSCALL( 9004, void dump_valid_mem( ) );
REV_SYSCALL( 9005, void dump_valid_mem_to_file( const char* outputFile ) );
REV_SYSCALL( 9006, void dump_thread_mem( ) );
REV_SYSCALL( 9007, void dump_thread_mem_to_file( const char* outputFile ) );

// ==================== REV PRINT UTILITIES
REV_SYSCALL( 9110, __attribute__(( format( printf, 1, 2 ) )) int rev_fast_printf( const char* format, ... ) );

// clang-format on

#endif  //SYSCALL_TYPES_ONLY

#endif
