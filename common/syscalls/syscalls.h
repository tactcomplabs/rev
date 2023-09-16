//
// _SYSCALLS_H_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//
//

#include <stdint.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

/* Clone Flags */
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
#define	IOCB_CMD_PREAD 0
#define	IOCB_CMD_PWRITE 1
#define	IOCB_CMD_FSYNC 2
#define	IOCB_CMD_FDSYNC 3
/* 4 was the experimental IOCB_CMD_PREADX */
#define	IOCB_CMD_POLL 5
#define	IOCB_CMD_NOOP 6
#define	IOCB_CMD_PREADV 7
#define	IOCB_CMD_PWRITEV 8

/*
 * Valid flags for the "aio_flags" member of the "struct iocb".
 *
 * IOCB_FLAG_RESFD - Set if the "aio_resfd" member of the "struct iocb"
 *                   is valid.
 * IOCB_FLAG_IOPRIO - Set if the "aio_reqprio" member of the "struct iocb"
 *                    is valid.
 */
#define IOCB_FLAG_RESFD		(1 << 0)
#define IOCB_FLAG_IOPRIO	(1 << 1)


/* Flags from sys/mman.h */

/* Protections are chosen from these bits, OR'd together.  The
   implementation does not necessarily support PROT_EXEC or PROT_WRITE
   without PROT_READ.  The only guarantees are that no writing will be
   allowed without PROT_WRITE and no access will be allowed for PROT_NONE. */

#define PROT_READ	0x1		/* Page can be read.  */
#define PROT_WRITE	0x2		/* Page can be written.  */
#define PROT_EXEC	0x4		/* Page can be executed.  */
#define PROT_NONE	0x0		/* Page can not be accessed.  */
#define PROT_GROWSDOWN	0x01000000	/* Extend change to start of growsdown vma (mprotect only).  */
#define PROT_GROWSUP	0x02000000	/* Extend change to start of growsup vma (mprotect only).  */

/* Sharing types (must choose one and only one of these).  */
#define MAP_SHARED	0x01		/* Share changes.  */
#define MAP_PRIVATE	0x02		/* Changes are private.  */
#define MAP_SHARED_VALIDATE	0x03	/* Share changes and validate extension flags.  */
#define MAP_TYPE	0x0f		/* Mask for type of mapping.  */

/* Other flags.  */
#define MAP_FIXED	0x10		/* Interpret addr exactly.  */
#define MAP_FILE	0
# define MAP_ANONYMOUS	0x20		/* Don't use a file.  */
#define MAP_ANON	MAP_ANONYMOUS
/* When MAP_HUGETLB is set bits [26:31] encode the log2 of the huge page size.  */
#define MAP_HUGE_SHIFT	26
#define MAP_HUGE_MASK	0x3f

/* Flags to `msync'.  */
#define MS_ASYNC	1		/* Sync memory asynchronously.  */
#define MS_SYNC		4		/* Synchronous memory sync.  */
#define MS_INVALIDATE	2		/* Invalidate the caches.  */

/* Advice to `madvise'.  */
# define MADV_NORMAL	  0	/* No further special treatment.  */
# define MADV_RANDOM	  1	/* Expect random page references.  */
# define MADV_SEQUENTIAL  2	/* Expect sequential page references.  */
# define MADV_WILLNEED	  3	/* Will need these pages.  */
# define MADV_DONTNEED	  4	/* Don't need these pages.  */
# define MADV_FREE	  8	/* Free pages only if memory pressure.  */
# define MADV_REMOVE	  9	/* Remove these pages and resources.  */
# define MADV_DONTFORK	  10	/* Do not inherit across fork.  */
# define MADV_DOFORK	  11	/* Do inherit across fork.  */
# define MADV_MERGEABLE	  12	/* KSM may merge identical pages.  */
# define MADV_UNMERGEABLE 13	/* KSM may not merge identical pages.  */
# define MADV_HUGEPAGE	  14	/* Worth backing with hugepages.  */
# define MADV_NOHUGEPAGE  15	/* Not worth backing with hugepages.  */
# define MADV_DONTDUMP	  16    /* Explicity exclude from the core dump, overrides the coredump filter bits.  */
# define MADV_DODUMP	  17	/* Clear the MADV_DONTDUMP flag.  */
# define MADV_WIPEONFORK  18	/* Zero memory on fork, child only.  */
# define MADV_KEEPONFORK  19	/* Undo MADV_WIPEONFORK.  */
# define MADV_COLD        20	/* Deactivate these pages.  */
# define MADV_PAGEOUT     21	/* Reclaim these pages.  */
# define MADV_POPULATE_READ 22	/* Populate (prefault) page tables readable.  */
# define MADV_POPULATE_WRITE 23	/* Populate (prefault) page tables writable.  */
# define MADV_DONTNEED_LOCKED 24 /* Like MADV_DONTNEED, but drop locked pages too.  */
# define MADV_COLLAPSE    25	/* Synchronous hugepage collapse.  */
# define MADV_HWPOISON	  100	/* Poison a page for testing.  */

// /* The POSIX people had to invent similar names for the same things.  */
// #ifdef __USE_XOPEN2K
// # define POSIX_MADV_NORMAL	0 /* No further special treatment.  */
// # define POSIX_MADV_RANDOM	1 /* Expect random page references.  */
// # define POSIX_MADV_SEQUENTIAL	2 /* Expect sequential page references.  */
// # define POSIX_MADV_WILLNEED	3 /* Will need these pages.  */
// # define POSIX_MADV_DONTNEED	4 /* Don't need these pages.  */
// #endif

/* Flags for `mlockall'.  */
#define MCL_CURRENT	1		/* Lock all currently mapped pages.  */
#define MCL_FUTURE	2		/* Lock all additions to address space.  */
#define MCL_ONFAULT	4		/* Lock all pages that are faulted in.  */

#define	F_OK	0
#define	R_OK	4
#define	W_OK	2
#define	X_OK	1

#define	SEEK_SET	0
#define	SEEK_CUR	1
#define	SEEK_END	2

#define STDIN_FILENO    0       /* standard input file descriptor */
#define STDOUT_FILENO   1       /* standard output file descriptor */
#define STDERR_FILENO   2       /* standard error file descriptor */



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


typedef uint32_t rwf_t;
typedef unsigned long aio_context_t;
typedef uint16_t aio_key;
typedef uint16_t aio_rw_flags;
typedef int32_t key_serial_t; /* key handle serial number */
typedef uint32_t key_perm_t; /* key handle permissions mask */
typedef struct __user_cap_header_struct {
  uint32_t version;
  int pid;
} *cap_user_header_t;


union sigval {
	int sival_int;
	void *sival_ptr;
};

typedef struct{
	int si_signo;
	int si_code;
	union sigval si_value;
	int si_errno;
	pid_t si_pid;
	uid_t si_uid;
	void *si_addr;
	int si_status;
	int si_band;
}siginfo_t;

typedef struct __user_cap_data_struct {
  uint32_t effective;
  uint32_t permitted;
  uint32_t inheritable;
} *cap_user_data_t;



/* To optimize the implementation one can use the following struct.  */
struct aioinit
  {
    int aio_threads;		/* Maximum number of threads.  */
    int aio_num;		/* Number of expected simultaneous requests.  */
    int aio_locks;		/* Not used.  */
    int aio_usedba;		/* Not used.  */
    int aio_debug;		/* Not used.  */
    int aio_numusers;		/* Not used.  */
    int aio_idle_time;		/* Number of seconds before idle thread terminates.  */
    int aio_reserved;
  };

typedef unsigned short umode_t;
typedef unsigned short qid_t;
typedef off_t loff_t;
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
	uint64_t	data;		/* the data field from the iocb */
	uint64_t	obj;		/* what iocb this event came from */
	int64_t		res;		/* result code for this event */
	int64_t		res2;		/* secondary result */
};


struct iocb {
   uint64_t   aio_data;
   uint16_t   aio_key;
   uint16_t   aio_rw_flags;
   uint16_t   aio_lio_opcode;
   int16_t   aio_reqprio;
   uint32_t   aio_fildes;
   uint64_t   aio_buf;
   uint64_t   aio_nbytes;
   int64_t   aio_offset;
   uint64_t   aio_reserved2;
   uint32_t   aio_flags;
   uint32_t   aio_resfd;
};


// Actual System Calls
// int rev_io_setup(unsigned nr_reqs, aio_context_t  *ctx);
// int rev_io_destroy(aio_context_t ctx);
// int rev_io_submit(aio_context_t, long, struct iocb  *  *);
// int rev_io_cancel(aio_context_t ctx_id, struct iocb  *iocb, struct io_event  *result);
// int rev_io_getevents(aio_context_t ctx_id, long min_nr, long nr, struct io_event  *events, struct __kernel_timespec  *timeout);
// int rev_setxattr(const char  *path, const char  *name, const void  *value, size_t size, int flags);
// int rev_lsetxattr(const char  *path, const char  *name, const void  *value, size_t size, int flags);
// int rev_fsetxattr(int fd, const char  *name, const void  *value, size_t size, int flags);
// int rev_getxattr(const char  *path, const char  *name, void  *value, size_t size);
// int rev_lgetxattr(const char  *path, const char  *name, void  *value, size_t size);
// int rev_fgetxattr(int fd, const char  *name, void  *value, size_t size);
// int rev_listxattr(const char  *path, char  *list, size_t size);
// int rev_llistxattr(const char  *path, char  *list, size_t size);
// int rev_flistxattr(int fd, char  *list, size_t size);
// int rev_removexattr(const char  *path, const char  *name);
// int rev_lremovexattr(const char  *path, const char  *name);
// int rev_fremovexattr(int fd, const char  *name);
// int rev_getcwd(char  *buf, unsigned long size);
// int rev_lookup_dcookie(uint64_t cookie64, char  *buf, size_t len);
// int rev_eventfd2(unsigned int count, int flags);
// int rev_epoll_create1(int flags);
// int rev_epoll_ctl(int epfd, int op, int fd, struct epoll_event  *event);
// int rev_epoll_pwait(int epfd, struct epoll_event  *events, int maxevents, int timeout, const sigset_t  *sigmask, size_t sigsetsize);
// int rev_dup(unsigned int fildes);
// int rev_dup3(unsigned int oldfd, unsigned int newfd, int flags);
// int rev_fcntl64(unsigned int fd, unsigned int cmd, unsigned long arg);
// int rev_inotify_init1(int flags);
// int rev_inotify_add_watch(int fd, const char  *path, uint32_t mask);
// int rev_inotify_rm_watch(int fd, int32_t wd);
// int rev_ioctl(unsigned int fd, unsigned int cmd, unsigned long arg);
// int rev_ioprio_set(int which, int who, int ioprio);
// int rev_ioprio_get(int which, int who);
// int rev_flock(unsigned int fd, unsigned int cmd);
// int rev_mknodat(int dfd, const char  * filename, umode_t mode, unsigned dev);
// int rev_mkdirat(int dfd, const char  * pathname, umode_t mode);
// int rev_unlinkat(int dfd, const char  * pathname, int flag);
// int rev_symlinkat(const char  * oldname, int newdfd, const char  * newname);
// int rev_unlinkat(int dfd, const char  * pathname, int flag);
// int rev_renameat(int olddfd, const char  * oldname, int newdfd, const char  * newname);
// int rev_umount(char  *name, int flags);
// int rev_umount(char  *name, int flags);
// int rev_pivot_root(const char  *new_root, const char  *put_old);
// int rev_ni_syscall(void);
// int rev_statfs64(const char  *path, size_t sz, struct statfs64  *buf);
// int rev_fstatfs64(unsigned int fd, size_t sz, struct statfs64  *buf);
// int rev_truncate64(const char  *path, loff_t length);
// int rev_ftruncate64(unsigned int fd, loff_t length);
// int rev_fallocate(int fd, int mode, loff_t offset, loff_t len);
// int rev_faccessat(int dfd, const char  *filename, int mode);
// int rev_chdir(const char  *filename);
// int rev_fchdir(unsigned int fd);
// int rev_chroot(const char  *filename);
// int rev_fchmod(unsigned int fd, umode_t mode);
// int rev_fchmodat(int dfd, const char  * filename, umode_t mode);
// int rev_fchownat(int dfd, const char  *filename, uid_t user, gid_t group, int flag);
// int rev_fchown(unsigned int fd, uid_t user, gid_t group);
// int rev_openat(int dfd, const char  *filename, int flags, umode_t mode);
// int rev_close(unsigned int fd);
// int rev_vhangup(void);
// int rev_pipe2(int  *fildes, int flags);
// int rev_quotactl(unsigned int cmd, const char  *special, qid_t id, void  *addr);
// int rev_getdents64(unsigned int fd, struct linux_dirent64  *dirent, unsigned int count);
// int rev_llseek(unsigned int fd, unsigned long offset_high, unsigned long offset_low, loff_t  *result, unsigned int whence);
// int rev_read(unsigned int fd, char  *buf, size_t count);
// int rev_write(unsigned int fd, const char  *buf, size_t count);
// int rev_readv(unsigned long fd, const struct iovec  *vec, unsigned long vlen);
// int rev_writev(unsigned long fd, const struct iovec  *vec, unsigned long vlen);
// int rev_pread64(unsigned int fd, char  *buf, size_t count, loff_t pos);
// int rev_pwrite64(unsigned int fd, const char  *buf, size_t count, loff_t pos);
// int rev_preadv(unsigned long fd, const struct iovec  *vec, unsigned long vlen, unsigned long pos_l, unsigned long pos_h);
// int rev_pwritev(unsigned long fd, const struct iovec  *vec, unsigned long vlen, unsigned long pos_l, unsigned long pos_h);
// int rev_sendfile64(int out_fd, int in_fd, loff_t  *offset, size_t count);
// int rev_pselect6_time32(int, fd_set  *, fd_set  *, fd_set  *, struct old_timespec32  *, void  *);
// int rev_ppoll_time32(struct pollfd  *, unsigned int, struct old_timespec32  *, const sigset_t  *, size_t);
// int rev_signalfd4(int ufd, sigset_t  *user_mask, size_t sizemask, int flags);
// int rev_vmsplice(int fd, const struct iovec  *iov, unsigned long nr_segs, unsigned int flags);
// int rev_vmsplice(int fd, const struct iovec  *iov, unsigned long nr_segs, unsigned int flags);
// int rev_tee(int fdin, int fdout, size_t len, unsigned int flags);
// int rev_readlinkat(int dfd, const char  *path, char  *buf, int bufsiz);
// int rev_newfstatat(int dfd, const char  *filename, struct stat  *statbuf, int flag);
// int rev_newfstat(unsigned int fd, struct stat  *statbuf);
// int rev_sync(void);
// int rev_fsync(unsigned int fd);
// int rev_fdatasync(unsigned int fd);
// int rev_sync_file_range2(int fd, unsigned int flags, loff_t offset, loff_t nbytes);
// int rev_sync_file_range(int fd, loff_t offset, loff_t nbytes, unsigned int flags);
// int rev_timerfd_create(int clockid, int flags);
// int rev_timerfd_settime(int ufd, int flags, const struct __kernel_itimerspec  *utmr, struct __kernel_itimerspec  *otmr);
// int rev_timerfd_gettime(int ufd, struct __kernel_itimerspec  *otmr);
// int rev_utimensat(int dfd, const char  *filename, struct __kernel_timespec  *utimes, int flags);
// int rev_acct(const char  *name);
// int rev_capget(cap_user_header_t header, cap_user_data_t dataptr);
// int rev_capset(cap_user_header_t header, const cap_user_data_t data);
// int rev_personality(unsigned int personality);
// int rev_exit(int error_code);
// int rev_exit_group(int error_code);
// int rev_waitid(int which, pid_t pid, siginfo_t *infop, int options, struct rusage  *ru);
// int rev_set_tid_address(int  *tidptr);
// int rev_unshare(unsigned long unshare_flags);
// int rev_futex(uint32_t  *uaddr, int op, uint32_t val, struct __kernel_timespec  *utime, uint32_t  *uaddr2, uint32_t val3);
// int rev_set_robust_list(struct robust_list_head  *head, size_t len);
// int rev_get_robust_list(int pid, struct robust_list_head  *  *head_ptr, size_t  *len_ptr);
// int rev_nanosleep(struct __kernel_timespec  *rqtp, struct __kernel_timespec  *rmtp);
// int rev_getitimer(int which, struct __kernel_old_itimerval  *value);
// int rev_setitimer(int which, struct __kernel_old_itimerval  *value, struct __kernel_old_itimerval  *ovalue);
// int rev_kexec_load(unsigned long entry, unsigned long nr_segments, struct kexec_segment  *segments, unsigned long flags);
// int rev_init_module(void  *umod, unsigned long len, const char  *uargs);
// int rev_delete_module(const char  *name_user, unsigned int flags);
// int rev_timer_create(clockid_t which_clock, struct sigevent  *timer_event_spec, timer_t  * created_timer_id);
// int rev_timer_gettime(timer_t timer_id, struct __kernel_itimerspec  *setting);
// int rev_timer_getoverrun(timer_t timer_id);
// int rev_timer_settime(timer_t timer_id, int flags, const struct __kernel_itimerspec  *new_setting, struct __kernel_itimerspec  *old_setting);
// int rev_timer_delete(timer_t timer_id);
// // int rev_clock_settime(clockid_t which_clock, const struct __kernel_timespec  *tp);
// int rev_clock_nanosleep(clockid_t which_clock, int flags, const struct __kernel_timespec  *rqtp, struct __kernel_timespec  *rmtp);
// int rev_syslog(int type, char  *buf, int len);
// int rev_ptrace(long request, long pid, unsigned long addr, unsigned long data);
// int rev_sched_setparam(pid_t pid, struct sched_param  *param);
// int rev_sched_setscheduler(pid_t pid, int policy, struct sched_param  *param);
// int rev_sched_getscheduler(pid_t pid);
// int rev_sched_getparam(pid_t pid, struct sched_param  *param);
// int rev_sched_setaffinity(pid_t pid, unsigned int len, unsigned long  *user_mask_ptr);
// int rev_sched_getaffinity(pid_t pid, unsigned int len, unsigned long  *user_mask_ptr);
// int rev_sched_yield(void);
// int rev_sched_get_priority_max(int policy);
// int rev_sched_get_priority_min(int policy);
// int rev_sched_rr_get_interval(pid_t pid, struct __kernel_timespec  *interval);
// int rev_restart_syscall(void);
// int rev_kill(pid_t pid, int sig);
// int rev_tkill(pid_t pid, int sig);
// int rev_tgkill(pid_t tgid, pid_t pid, int sig);
// int rev_sigaltstack(const struct sigaltstack  *uss, struct sigaltstack  *uoss);
// int rev_rt_sigsuspend(sigset_t  *unewset, size_t sigsetsize);
// int rev_rt_sigaction(int, const struct sigaction  *, struct sigaction  *, size_t);
// int rev_rt_sigprocmask(int how, sigset_t  *set, sigset_t  *oset, size_t sigsetsize);
// int rev_rt_sigpending(sigset_t  *set, size_t sigsetsize);
// int rev_rt_sigtimedwait_time32(const sigset_t  *uthese, siginfo_t  *uinfo, const struct old_timespec32  *uts, size_t sigsetsize);
// int rev_rt_sigqueueinfo(pid_t pid, int sig, siginfo_t *uinfo);
// int rev_setpriority(int which, int who, int niceval);
// int rev_getpriority(int which, int who);
// int rev_reboot(int magic1, int magic2, unsigned int cmd, void  *arg);
// int rev_setregid(gid_t rgid, gid_t egid);
// int rev_setgid(gid_t gid);
// int rev_setreuid(uid_t ruid, uid_t euid);
// int rev_setuid(uid_t uid);
// int rev_setresuid(uid_t ruid, uid_t euid, uid_t suid);
// int rev_getresuid(uid_t  *ruid, uid_t  *euid, uid_t  *suid);
// int rev_setresgid(gid_t rgid, gid_t egid, gid_t sgid);
// int rev_getresgid(gid_t  *rgid, gid_t  *egid, gid_t  *sgid);
// int rev_setfsuid(uid_t uid);
// int rev_setfsgid(gid_t gid);
// int rev_times(struct tms  *tbuf);
// int rev_setpgid(pid_t pid, pid_t pgid);
// int rev_getpgid(pid_t pid);
// int rev_getsid(pid_t pid);
// int rev_setsid(void);
// int rev_getgroups(int gidsetsize, gid_t  *grouplist);
// int rev_setgroups(int gidsetsize, gid_t  *grouplist);
// int rev_newuname(struct new_utsname  *name);
// int rev_sethostname(char  *name, int len);
// int rev_setdomainname(char  *name, int len);
// int rev_getrlimit(unsigned int resource, struct rlimit  *rlim);
// int rev_setrlimit(unsigned int resource, struct rlimit  *rlim);
// int rev_getrusage(int who, struct rusage  *ru);
// int rev_umask(int mask);
// int rev_prctl(int option, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5);
// int rev_getcpu(unsigned  *cpu, unsigned  *node, struct getcpu_cache  *cache);
// int rev_gettimeofday(struct __kernel_old_timeval  *tv, struct timezone  *tz);
// int rev_settimeofday(struct __kernel_old_timeval  *tv, struct timezone  *tz);
// int rev_adjtimex(struct __kernel_timex  *txc_p);
// int rev_getpid(void);
// int rev_getppid(void);
// int rev_getuid(void);
// int rev_geteuid(void);
// int rev_getgid(void);
// int rev_getegid(void);
// int rev_gettid(void);
// int rev_sysinfo(struct sysinfo  *info);
// int rev_mq_open(const char  *name, int oflag, umode_t mode, struct mq_attr  *attr);
// int rev_mq_unlink(const char  *name);
// int rev_mq_timedsend(uint64_t mqdes, const char  *msg_ptr, size_t msg_len, unsigned int msg_prio, const struct __kernel_timespec  *abs_timeout);
// int rev_mq_timedreceive(uint64_t mqdes, char  *msg_ptr, size_t msg_len, unsigned int  *msg_prio, const struct __kernel_timespec  *abs_timeout);
// int rev_mq_notify(uint64_t mqdes, const struct sigevent  *notification);
// int rev_mq_getsetattr(uint64_t mqdes, const struct mq_attr  *mqstat, struct mq_attr  *omqstat);
// int rev_msgget(key_t key, int msgflg);
// int rev_old_msgctl(int msqid, int cmd, struct msqid_ds  *buf);
// int rev_msgrcv(int msqid, struct msgbuf  *msgp, size_t msgsz, long msgtyp, int msgflg);
// int rev_msgsnd(int msqid, struct msgbuf  *msgp, size_t msgsz, int msgflg);
// int rev_semget(key_t key, int nsems, int semflg);
// int rev_semctl(int semid, int semnum, int cmd, unsigned long arg);
// int rev_semtimedop(int semid, struct sembuf  *sops, unsigned nsops, const struct __kernel_timespec  *timeout);
// int rev_semop(int semid, struct sembuf  *sops, unsigned nsops);
// int rev_shmget(key_t key, size_t size, int flag);
// int rev_old_shmctl(int shmid, int cmd, struct shmid_ds  *buf);
// int rev_shmat(int shmid, char  *shmaddr, int shmflg);
// int rev_shmdt(char  *shmaddr);
// int rev_socket(int, int, int);
// int rev_socketpair(int, int, int, int  *);
// int rev_bind(int, struct sockaddr  *, int);
// int rev_listen(int, int);
// int rev_accept(int, struct sockaddr  *, int  *);
// int rev_connect(int, struct sockaddr  *, int);
// int rev_getsockname(int, struct sockaddr  *, int  *);
// int rev_getpeername(int, struct sockaddr  *, int  *);
// int rev_sendto(int, void  *, size_t, unsigned, struct sockaddr  *, int);
// int rev_recvfrom(int, void  *, size_t, unsigned, struct sockaddr  *, int  *);
// int rev_setsockopt(int fd, int level, int optname, char  *optval, int optlen);
// int rev_getsockopt(int fd, int level, int optname, char  *optval, int  *optlen);
// int rev_shutdown(int, int);
// int rev_sendmsg(int fd, struct user_msghdr  *msg, unsigned flags);
// int rev_recvmsg(int fd, struct user_msghdr  *msg, unsigned flags);
// int rev_readahead(int fd, loff_t offset, size_t count);
// int rev_brk(unsigned long brk);
// int rev_munmap(unsigned long addr, size_t len);
// int rev_mremap(unsigned long addr, unsigned long old_len, unsigned long new_len, unsigned long flags, unsigned long new_addr);
// int rev_add_key(const char  *_type, const char  *_description, const void  *_payload, size_t plen, key_serial_t destringid);
// int rev_request_key(const char  *_type, const char  *_description, const char  *_callout_info, key_serial_t destringid);
// int rev_keyctl(int cmd, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5);
// int rev_fork(); // Actually uses clone under the hood
// int rev_clone(unsigned long, unsigned long, int  *, unsigned long, int  *);
// int rev_execve(const char  *filename, const char  *const  *argv, const char  *const  *envp);
// int rev_old_mmap(struct mmap_arg_struct  *arg);
// int rev_fadvise64_64(int fd, loff_t offset, loff_t len, int advice);
// int rev_swapon(const char  *specialfile, int swap_flags);
// int rev_swapoff(const char  *specialfile);
// int rev_mprotect(unsigned long start, size_t len, unsigned long prot);
// int rev_msync(unsigned long start, size_t len, int flags);
// int rev_mlock(unsigned long start, size_t len);
// int rev_munlock(unsigned long start, size_t len);
// int rev_mlockall(int flags);
// int rev_munlockall(void);
// int rev_mincore(unsigned long start, size_t len, unsigned char  * vec);
// int rev_madvise(unsigned long start, size_t len, int behavior);
// int rev_remap_file_pages(unsigned long start, unsigned long size, unsigned long prot, unsigned long pgoff, unsigned long flags);
// int rev_mbind(unsigned long start, unsigned long len, unsigned long mode, const unsigned long  *nmask, unsigned long maxnode, unsigned flags);
// int rev_get_mempolicy(int  *policy, unsigned long  *nmask, unsigned long maxnode, unsigned long addr, unsigned long flags);
// int rev_set_mempolicy(int mode, const unsigned long  *nmask, unsigned long maxnode);
// int rev_migrate_pages(pid_t pid, unsigned long maxnode, const unsigned long  *from, const unsigned long  *to);
// int rev_move_pages(pid_t pid, unsigned long nr_pages, const void  *  *pages, const int  *nodes, int  *status, int flags);
// int rev_rt_tgsigqueueinfo(pid_t tgid, pid_t pid, int sig, siginfo_t  *uinfo);
// int rev_perf_event_open();
// int rev_accept4(int, struct sockaddr  *, int  *, int);
// int rev_recvmmsg_time32(int fd, struct mmsghdr  *msg, unsigned int vlen, unsigned flags, struct old_timespec32  *timeout);
// int rev_wait4(pid_t pid, int  *stat_addr, int options, struct rusage  *ru);
// int rev_prlimit64(pid_t pid, unsigned int resource, const struct rlimit64  *new_rlim, struct rlimit64  *old_rlim);
// int rev_fanotify_init(unsigned int flags, unsigned int event_f_flags);
// int rev_fanotify_mark(int fanotify_fd, unsigned int flags, uint64_t mask, int fd, const char  *pathname);
// int rev_name_to_handle_at(int dfd, const char  *name, struct file_handle  *handle, int  *mnt_id, int flag);
// int rev_open_by_handle_at(int mountdirfd, struct file_handle  *handle, int flags);
// int rev_clock_adjtime(clockid_t which_clock, struct __kernel_timex  *tx);
// int rev_syncfs(int fd);
// int rev_setns(int fd, int nstype);
// int rev_sendmmsg(int fd, struct mmsghdr  *msg, unsigned int vlen, unsigned flags);
// int rev_process_vm_readv(pid_t pid, const struct iovec  *lvec, unsigned long liovcnt, const struct iovec  *rvec, unsigned long riovcnt, unsigned long flags);
// int rev_process_vm_writev(pid_t pid, const struct iovec  *lvec, unsigned long liovcnt, const struct iovec  *rvec, unsigned long riovcnt, unsigned long flags);
// int rev_kcmp(pid_t pid1, pid_t pid2, int type, unsigned long idx1, unsigned long idx2);
// int rev_finit_module(int fd, const char  *uargs, int flags);
// int rev_sched_setattr(pid_t pid, struct sched_attr  *attr, unsigned int flags);
// int rev_sched_getattr(pid_t pid, struct sched_attr  *attr, unsigned int size, unsigned int flags);
// int rev_renameat2(int olddfd, const char  *oldname, int newdfd, const char  *newname, unsigned int flags);
// int rev_seccomp(unsigned int op, unsigned int flags, void  *uargs);
// int rev_getrandom(char  *buf, size_t count, unsigned int flags);
// int rev_memfd_create(const char  *uname_ptr, unsigned int flags);
// int rev_bpf(int cmd, union bpf_attr *attr, unsigned int size);
// int rev_execveat(int dfd, const char  *filename, const char  *const  *argv, const char  *const  *envp, int flags);
// int rev_userfaultfd(int flags);
// int rev_membarrier(int cmd, unsigned int flags, int cpu_id);
// int rev_mlock2(unsigned long start, size_t len, int flags);
// int rev_copy_file_range(int fd_in, loff_t  *off_in, int fd_out, loff_t  *off_out, size_t len, unsigned int flags);
// int rev_preadv2(unsigned long fd, const struct iovec  *vec, unsigned long vlen, unsigned long pos_l, unsigned long pos_h, rwf_t flags);
// int rev_pwritev2(unsigned long fd, const struct iovec  *vec, unsigned long vlen, unsigned long pos_l, unsigned long pos_h, rwf_t flags);
// int rev_pkey_mprotect(unsigned long start, size_t len, unsigned long prot, int pkey);
// int rev_pkey_alloc(unsigned long flags, unsigned long init_val);
// int rev_pkey_free(int pkey);
// int rev_statx(int dfd, const char  *path, unsigned flags, unsigned mask, struct statx  *buffer);
// // int rev_io_pgetevents(aio_context_t ctx_id, long min_nr, long nr, struct io_event  *events, struct __kernel_timespec  *timeout, const struct __aio_sigset *sig);
// int rev_rseq(struct rseq  *rseq, uint32_t rseq_len, int flags, uint32_t sig);
// int rev_kexec_file_load(int kernel_fd, int initrd_fd, unsigned long cmdline_len, const char  *cmdline_ptr, unsigned long flags);
// int rev_clock_gettime(clockid_t which_clock, struct __kernel_timespec  *tp);
// int rev_clock_settime(clockid_t which_clock, const struct __kernel_timespec  *tp);
// int rev_clock_adjtime(clockid_t which_clock, struct __kernel_timex  *tx);
// int rev_clock_getres(clockid_t which_clock, struct __kernel_timespec  *tp);
// // int rev_clock_nanosleep(clockid_t which_clock, int flags, const struct __kernel_timespec  *rqtp, struct __kernel_timespec  *rmtp);
// // int rev_timer_gettime(timer_t timer_id, struct __kernel_itimerspec  *setting);
// // int rev_timer_settime(timer_t timer_id, int flags, const struct __kernel_itimerspec  *new_setting, struct __kernel_itimerspec  *old_setting);
// // int rev_timerfd_gettime(int ufd, struct __kernel_itimerspec  *otmr);
// // int rev_timerfd_settime(int ufd, int flags, const struct __kernel_itimerspec  *utmr, struct __kernel_itimerspec  *otmr);
// // int rev_utimensat(int dfd, const char  *filename, struct __kernel_timespec  *utimes, int flags);
// int rev_io_pgetevents(aio_context_t ctx_id, long min_nr, long nr, struct io_event  *events, struct __kernel_timespec  *timeout, const struct __aio_sigset *sig);
// // int rev_mq_timedsend(uint64_t mqdes, const char  *msg_ptr, size_t msg_len, unsigned int msg_prio, const struct __kernel_timespec  *abs_timeout);
// // int rev_mq_timedreceive(uint64_t mqdes, char  *msg_ptr, size_t msg_len, unsigned int  *msg_prio, const struct __kernel_timespec  *abs_timeout);
// int rev_pidfd_send_signal(int pidfd, int sig, siginfo_t  *info, unsigned int flags);
// int rev_io_uring_setup(uint32_t entries, struct io_uring_params  *p);
// int rev_io_uring_enter(unsigned int fd, uint32_t to_submit, uint32_t min_complete, uint32_t flags, const sigset_t  *sig, size_t sigsz);
// int rev_io_uring_register(unsigned int fd, unsigned int op, void  *arg, unsigned int nr_args);
// int rev_open_tree(int dfd, const char  *path, unsigned flags);
// int rev_move_mount(int from_dfd, const char  *from_path, int to_dfd, const char  *to_path, unsigned int ms_flags);
// int rev_fsopen(const char  *fs_name, unsigned int flags);
// int rev_fsconfig(int fs_fd, unsigned int cmd, const char  *key, const void  *value, int aux);
// int rev_fsmount(int fs_fd, unsigned int flags, unsigned int ms_flags);
// int rev_fspick(int dfd, const char  *path, unsigned int flags);
// int rev_pidfd_open(pid_t pid, unsigned int flags);
// int rev_clone3(struct clone_args  *uargs, size_t size);
// int rev_close_range(unsigned int fd, unsigned int max_fd, unsigned int flags);
// int rev_openat2(int dfd, const char  *filename, struct open_how *how, size_t size);
// int rev_pidfd_getfd(int pidfd, int fd, unsigned int flags);
// int rev_faccessat2(int dfd, const char  *filename, int mode, int flags);
// int rev_process_madvise(int pidfd, const struct iovec  *vec, size_t vlen, int behavior, unsigned int flags);

int rev_io_setup(unsigned nr_reqs, aio_context_t  *ctx){
  int rc;
  asm volatile (
    "li a7, 0 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_io_destroy(aio_context_t ctx){
  int rc;
  asm volatile (
    "li a7, 1 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_io_submit(aio_context_t, long, struct iocb  *  *){
  int rc;
  asm volatile (
    "li a7, 2 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_io_cancel(aio_context_t ctx_id, struct iocb  *iocb, struct io_event  *result){
  int rc;
  asm volatile (
    "li a7, 3 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_io_getevents(aio_context_t ctx_id, long min_nr, long nr, struct io_event  *events, struct __kernel_timespec  *timeout){
  int rc;
  asm volatile (
    "li a7, 4 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_setxattr(const char  *path, const char  *name, const void  *value, size_t size, int flags){
  int rc;
  asm volatile (
    "li a7, 5 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_lsetxattr(const char  *path, const char  *name, const void  *value, size_t size, int flags){
  int rc;
  asm volatile (
    "li a7, 6 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_fsetxattr(int fd, const char  *name, const void  *value, size_t size, int flags){
  int rc;
  asm volatile (
    "li a7, 7 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_getxattr(const char  *path, const char  *name, void  *value, size_t size){
  int rc;
  asm volatile (
    "li a7, 8 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_lgetxattr(const char  *path, const char  *name, void  *value, size_t size){
  int rc;
  asm volatile (
    "li a7, 9 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_fgetxattr(int fd, const char  *name, void  *value, size_t size){
  int rc;
  asm volatile (
    "li a7, 10 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_listxattr(const char  *path, char  *list, size_t size){
  int rc;
  asm volatile (
    "li a7, 11 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_llistxattr(const char  *path, char  *list, size_t size){
  int rc;
  asm volatile (
    "li a7, 12 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_flistxattr(int fd, char  *list, size_t size){
  int rc;
  asm volatile (
    "li a7, 13 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_removexattr(const char  *path, const char  *name){
  int rc;
  asm volatile (
    "li a7, 14 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_lremovexattr(const char  *path, const char  *name){
  int rc;
  asm volatile (
    "li a7, 15 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_fremovexattr(int fd, const char  *name){
  int rc;
  asm volatile (
    "li a7, 16 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_getcwd(char  *buf, unsigned long size){
  int rc;
  asm volatile (
    "li a7, 17 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_lookup_dcookie(uint64_t cookie64, char  *buf, size_t len){
  int rc;
  asm volatile (
    "li a7, 18 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_eventfd2(unsigned int count, int flags){
  int rc;
  asm volatile (
    "li a7, 19 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_epoll_create1(int flags){
  int rc;
  asm volatile (
    "li a7, 20 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_epoll_ctl(int epfd, int op, int fd, struct epoll_event  *event){
  int rc;
  asm volatile (
    "li a7, 21 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_epoll_pwait(int epfd, struct epoll_event  *events, int maxevents, int timeout, const sigset_t  *sigmask, size_t sigsetsize){
  int rc;
  asm volatile (
    "li a7, 22 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_dup(unsigned int fildes){
  int rc;
  asm volatile (
    "li a7, 23 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_dup3(unsigned int oldfd, unsigned int newfd, int flags){
  int rc;
  asm volatile (
    "li a7, 24 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_fcntl64(unsigned int fd, unsigned int cmd, unsigned long arg){
  int rc;
  asm volatile (
    "li a7, 25 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_inotify_init1(int flags){
  int rc;
  asm volatile (
    "li a7, 26 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_inotify_add_watch(int fd, const char  *path, uint32_t mask){
  int rc;
  asm volatile (
    "li a7, 27 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_inotify_rm_watch(int fd, int32_t wd){
  int rc;
  asm volatile (
    "li a7, 28 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_ioctl(unsigned int fd, unsigned int cmd, unsigned long arg){
  int rc;
  asm volatile (
    "li a7, 29 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_ioprio_set(int which, int who, int ioprio){
  int rc;
  asm volatile (
    "li a7, 30 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_ioprio_get(int which, int who){
  int rc;
  asm volatile (
    "li a7, 31 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_flock(unsigned int fd, unsigned int cmd){
  int rc;
  asm volatile (
    "li a7, 32 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_mknodat(int dfd, const char  * filename, umode_t mode, unsigned dev){
  int rc;
  asm volatile (
    "li a7, 33 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_mkdirat(int dfd, const char  * pathname, umode_t mode){
  int rc;
  asm volatile (
    "li a7, 34 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_unlinkat(int dfd, const char  * pathname, int flag){
  int rc;
  asm volatile (
    "li a7, 35 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_symlinkat(const char  * oldname, int newdfd, const char  * newname){
  int rc;
  asm volatile (
    "li a7, 36 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

// int rev_unlinkat(int dfd, const char  * pathname, int flag){
//   int rc;
//   asm volatile (
//     "li a7, 37 \n\t"
//     "ecall \n\t"
//     "mv %0, a0" : "=r" (rc)
//     );
//   return rc;
// }

int rev_renameat(int olddfd, const char  * oldname, int newdfd, const char  * newname){
  int rc;
  asm volatile (
    "li a7, 38 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_umount(char  *name, int flags){
  int rc;
  asm volatile (
    "li a7, 39 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

// int rev_umount(char  *name, int flags){
//   int rc;
//   asm volatile (
//     "li a7, 40 \n\t"
//     "ecall \n\t"
//     "mv %0, a0" : "=r" (rc)
//     );
//   return rc;
// }

int rev_pivot_root(const char  *new_root, const char  *put_old){
  int rc;
  asm volatile (
    "li a7, 41 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_ni_syscall(void){
  int rc;
  asm volatile (
    "li a7, 42 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_statfs64(const char  *path, size_t sz, struct statfs64  *buf){
  int rc;
  asm volatile (
    "li a7, 43 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_fstatfs64(unsigned int fd, size_t sz, struct statfs64  *buf){
  int rc;
  asm volatile (
    "li a7, 44 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_truncate64(const char  *path, loff_t length){
  int rc;
  asm volatile (
    "li a7, 45 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_ftruncate64(unsigned int fd, loff_t length){
  int rc;
  asm volatile (
    "li a7, 46 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_fallocate(int fd, int mode, loff_t offset, loff_t len){
  int rc;
  asm volatile (
    "li a7, 47 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_faccessat(int dfd, const char  *filename, int mode){
  int rc;
  asm volatile (
    "li a7, 48 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_chdir(const char  *filename){
  int rc;
  asm volatile (
    "li a7, 49 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_fchdir(unsigned int fd){
  int rc;
  asm volatile (
    "li a7, 50 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_chroot(const char  *filename){
  int rc;
  asm volatile (
    "li a7, 51 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_fchmod(unsigned int fd, umode_t mode){
  int rc;
  asm volatile (
    "li a7, 52 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_fchmodat(int dfd, const char  * filename, umode_t mode){
  int rc;
  asm volatile (
    "li a7, 53 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_fchownat(int dfd, const char  *filename, uid_t user, gid_t group, int flag){
  int rc;
  asm volatile (
    "li a7, 54 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_fchown(unsigned int fd, uid_t user, gid_t group){
  int rc;
  asm volatile (
    "li a7, 55 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_openat(int dfd, const char  *filename, int flags, umode_t mode){
  int rc;
  asm volatile (
    "li a7, 56 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_close(unsigned int fd){
  int rc;
  asm volatile (
    "li a7, 57 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_vhangup(void){
  int rc;
  asm volatile (
    "li a7, 58 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_pipe2(int  *fildes, int flags){
  int rc;
  asm volatile (
    "li a7, 59 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_quotactl(unsigned int cmd, const char  *special, qid_t id, void  *addr){
  int rc;
  asm volatile (
    "li a7, 60 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_getdents64(unsigned int fd, struct linux_dirent64  *dirent, unsigned int count){
  int rc;
  asm volatile (
    "li a7, 61 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_llseek(unsigned int fd, unsigned long offset_high, unsigned long offset_low, loff_t  *result, unsigned int whence){
  int rc;
  asm volatile (
    "li a7, 62 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_read(unsigned int fd, char  *buf, size_t count){
  int rc;
  asm volatile (
    "li a7, 63 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_write(unsigned int fd, const char  *buf, size_t count){
  int rc;
  asm volatile (
    "li a7, 64 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_readv(unsigned long fd, const struct iovec  *vec, unsigned long vlen){
  int rc;
  asm volatile (
    "li a7, 65 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_writev(unsigned long fd, const struct iovec  *vec, unsigned long vlen){
  int rc;
  asm volatile (
    "li a7, 66 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_pread64(unsigned int fd, char  *buf, size_t count, loff_t pos){
  int rc;
  asm volatile (
    "li a7, 67 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_pwrite64(unsigned int fd, const char  *buf, size_t count, loff_t pos){
  int rc;
  asm volatile (
    "li a7, 68 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_preadv(unsigned long fd, const struct iovec  *vec, unsigned long vlen, unsigned long pos_l, unsigned long pos_h){
  int rc;
  asm volatile (
    "li a7, 69 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_pwritev(unsigned long fd, const struct iovec  *vec, unsigned long vlen, unsigned long pos_l, unsigned long pos_h){
  int rc;
  asm volatile (
    "li a7, 70 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_sendfile64(int out_fd, int in_fd, loff_t  *offset, size_t count){
  int rc;
  asm volatile (
    "li a7, 71 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_pselect6_time32(int, fd_set  *, fd_set  *, fd_set  *, struct old_timespec32  *, void  *){
  int rc;
  asm volatile (
    "li a7, 72 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_ppoll_time32(struct pollfd  *, unsigned int, struct old_timespec32  *, const sigset_t  *, size_t){
  int rc;
  asm volatile (
    "li a7, 73 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_signalfd4(int ufd, sigset_t  *user_mask, size_t sizemask, int flags){
  int rc;
  asm volatile (
    "li a7, 74 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_vmsplice(int fd, const struct iovec  *iov, unsigned long nr_segs, unsigned int flags){
  int rc;
  asm volatile (
    "li a7, 75 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

// int rev_vmsplice(int fd, const struct iovec  *iov, unsigned long nr_segs, unsigned int flags){
//   int rc;
//   asm volatile (
//     "li a7, 76 \n\t"
//     "ecall \n\t"
//     "mv %0, a0" : "=r" (rc)
//     );
//   return rc;
// }

int rev_tee(int fdin, int fdout, size_t len, unsigned int flags){
  int rc;
  asm volatile (
    "li a7, 77 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_readlinkat(int dfd, const char  *path, char  *buf, int bufsiz){
  int rc;
  asm volatile (
    "li a7, 78 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_newfstatat(int dfd, const char  *filename, struct stat  *statbuf, int flag){
  int rc;
  asm volatile (
    "li a7, 79 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_newfstat(unsigned int fd, struct stat  *statbuf){
  int rc;
  asm volatile (
    "li a7, 80 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_sync(void){
  int rc;
  asm volatile (
    "li a7, 81 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_fsync(unsigned int fd){
  int rc;
  asm volatile (
    "li a7, 82 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_fdatasync(unsigned int fd){
  int rc;
  asm volatile (
    "li a7, 83 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_sync_file_range2(int fd, unsigned int flags, loff_t offset, loff_t nbytes){
  int rc;
  asm volatile (
    "li a7, 84 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_sync_file_range(int fd, loff_t offset, loff_t nbytes, unsigned int flags){
  int rc;
  asm volatile (
    "li a7, 84 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_timerfd_create(int clockid, int flags){
  int rc;
  asm volatile (
    "li a7, 85 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_timerfd_settime(int ufd, int flags, const struct __kernel_itimerspec  *utmr, struct __kernel_itimerspec  *otmr){
  int rc;
  asm volatile (
    "li a7, 86 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_timerfd_gettime(int ufd, struct __kernel_itimerspec  *otmr){
  int rc;
  asm volatile (
    "li a7, 87 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_utimensat(int dfd, const char  *filename, struct __kernel_timespec  *utimes, int flags){
  int rc;
  asm volatile (
    "li a7, 88 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_acct(const char  *name){
  int rc;
  asm volatile (
    "li a7, 89 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_capget(cap_user_header_t header, cap_user_data_t dataptr){
  int rc;
  asm volatile (
    "li a7, 90 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_capset(cap_user_header_t header, const cap_user_data_t data){
  int rc;
  asm volatile (
    "li a7, 91 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_personality(unsigned int personality){
  int rc;
  asm volatile (
    "li a7, 92 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_exit(int error_code){
  int rc;
  asm volatile (
    "li a7, 93 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_exit_group(int error_code){
  int rc;
  asm volatile (
    "li a7, 94 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_waitid(int which, pid_t pid, struct siginfo  *infop, int options, struct rusage  *ru){
  int rc;
  asm volatile (
    "li a7, 95 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_set_tid_address(int  *tidptr){
  int rc;
  asm volatile (
    "li a7, 96 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_unshare(unsigned long unshare_flags){
  int rc;
  asm volatile (
    "li a7, 97 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

// int rev_futex(uint32_t  *uaddr, int op, uint32_t val, struct __kernel_timespec  *utime, uint32_t  *uaddr2, uint32_t val3){
//   int rc;
//   asm volatile (
//     "li a7, 98 \n\t"
//     "ecall \n\t"
//     "mv %0, a0" : "=r" (rc)
//     );
//   return rc;
// }

int rev_set_robust_list(struct robust_list_head  *head, size_t len){
  int rc;
  asm volatile (
    "li a7, 99 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_get_robust_list(int pid, struct robust_list_head  *  *head_ptr, size_t  *len_ptr){
  int rc;
  asm volatile (
    "li a7, 100 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_nanosleep(struct __kernel_timespec  *rqtp, struct __kernel_timespec  *rmtp){
  int rc;
  asm volatile (
    "li a7, 101 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_getitimer(int which, struct __kernel_old_itimerval  *value){
  int rc;
  asm volatile (
    "li a7, 102 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_setitimer(int which, struct __kernel_old_itimerval  *value, struct __kernel_old_itimerval  *ovalue){
  int rc;
  asm volatile (
    "li a7, 103 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_kexec_load(unsigned long entry, unsigned long nr_segments, struct kexec_segment  *segments, unsigned long flags){
  int rc;
  asm volatile (
    "li a7, 104 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_init_module(void  *umod, unsigned long len, const char  *uargs){
  int rc;
  asm volatile (
    "li a7, 105 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_delete_module(const char  *name_user, unsigned int flags){
  int rc;
  asm volatile (
    "li a7, 106 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_timer_create(clockid_t which_clock, struct sigevent  *timer_event_spec, timer_t  * created_timer_id){
  int rc;
  asm volatile (
    "li a7, 107 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_timer_gettime(timer_t timer_id, struct __kernel_itimerspec  *setting){
  int rc;
  asm volatile (
    "li a7, 108 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_timer_getoverrun(timer_t timer_id){
  int rc;
  asm volatile (
    "li a7, 109 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_timer_settime(timer_t timer_id, int flags, const struct __kernel_itimerspec  *new_setting, struct __kernel_itimerspec  *old_setting){
  int rc;
  asm volatile (
    "li a7, 110 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_timer_delete(timer_t timer_id){
  int rc;
  asm volatile (
    "li a7, 111 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_clock_settime(clockid_t which_clock, const struct __kernel_timespec  *tp){
  int rc;
  asm volatile (
    "li a7, 112 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_clock_gettime(clockid_t which_clock, struct __kernel_timespec  *tp){
  int rc;
  asm volatile (
    "li a7, 113 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_clock_getres(clockid_t which_clock, struct __kernel_timespec  *tp){
  int rc;
  asm volatile (
    "li a7, 114 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_clock_nanosleep(clockid_t which_clock, int flags, const struct __kernel_timespec  *rqtp, struct __kernel_timespec  *rmtp){
  int rc;
  asm volatile (
    "li a7, 115 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_syslog(int type, char  *buf, int len){
  int rc;
  asm volatile (
    "li a7, 116 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_ptrace(long request, long pid, unsigned long addr, unsigned long data){
  int rc;
  asm volatile (
    "li a7, 117 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_sched_setparam(pid_t pid, struct sched_param  *param){
  int rc;
  asm volatile (
    "li a7, 118 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_sched_setscheduler(pid_t pid, int policy, struct sched_param  *param){
  int rc;
  asm volatile (
    "li a7, 119 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_sched_getscheduler(pid_t pid){
  int rc;
  asm volatile (
    "li a7, 120 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_sched_getparam(pid_t pid, struct sched_param  *param){
  int rc;
  asm volatile (
    "li a7, 121 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_sched_setaffinity(pid_t pid, unsigned int len, unsigned long  *user_mask_ptr){
  int rc;
  asm volatile (
    "li a7, 122 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_sched_getaffinity(pid_t pid, unsigned int len, unsigned long  *user_mask_ptr){
  int rc;
  asm volatile (
    "li a7, 123 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_sched_yield(void){
  int rc;
  asm volatile (
    "li a7, 124 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_sched_get_priority_max(int policy){
  int rc;
  asm volatile (
    "li a7, 125 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_sched_get_priority_min(int policy){
  int rc;
  asm volatile (
    "li a7, 126 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_sched_rr_get_interval(pid_t pid, struct __kernel_timespec  *interval){
  int rc;
  asm volatile (
    "li a7, 127 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_restart_syscall(void){
  int rc;
  asm volatile (
    "li a7, 128 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_kill(pid_t pid, int sig){
  int rc;
  asm volatile (
    "li a7, 129 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_tkill(pid_t pid, int sig){
  int rc;
  asm volatile (
    "li a7, 130 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_tgkill(pid_t tgid, pid_t pid, int sig){
  int rc;
  asm volatile (
    "li a7, 131 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_sigaltstack(const struct sigaltstack  *uss, struct sigaltstack  *uoss){
  int rc;
  asm volatile (
    "li a7, 132 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_rt_sigsuspend(sigset_t  *unewset, size_t sigsetsize){
  int rc;
  asm volatile (
    "li a7, 133 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_rt_sigaction(int, const struct sigaction  *, struct sigaction  *, size_t){
  int rc;
  asm volatile (
    "li a7, 134 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_rt_sigprocmask(int how, sigset_t  *set, sigset_t  *oset, size_t sigsetsize){
  int rc;
  asm volatile (
    "li a7, 135 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_rt_sigpending(sigset_t  *set, size_t sigsetsize){
  int rc;
  asm volatile (
    "li a7, 136 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_rt_sigtimedwait_time32(const sigset_t  *uthese, siginfo_t  *uinfo, const struct old_timespec32  *uts, size_t sigsetsize){
  int rc;
  asm volatile (
    "li a7, 137 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_rt_sigqueueinfo(pid_t pid, int sig, siginfo_t  *uinfo){
  int rc;
  asm volatile (
    "li a7, 138 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_setpriority(int which, int who, int niceval){
  int rc;
  asm volatile (
    "li a7, 140 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_getpriority(int which, int who){
  int rc;
  asm volatile (
    "li a7, 141 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_reboot(int magic1, int magic2, unsigned int cmd, void  *arg){
  int rc;
  asm volatile (
    "li a7, 142 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_setregid(gid_t rgid, gid_t egid){
  int rc;
  asm volatile (
    "li a7, 143 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_setgid(gid_t gid){
  int rc;
  asm volatile (
    "li a7, 144 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_setreuid(uid_t ruid, uid_t euid){
  int rc;
  asm volatile (
    "li a7, 145 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_setuid(uid_t uid){
  int rc;
  asm volatile (
    "li a7, 146 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_setresuid(uid_t ruid, uid_t euid, uid_t suid){
  int rc;
  asm volatile (
    "li a7, 147 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_getresuid(uid_t  *ruid, uid_t  *euid, uid_t  *suid){
  int rc;
  asm volatile (
    "li a7, 148 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_setresgid(gid_t rgid, gid_t egid, gid_t sgid){
  int rc;
  asm volatile (
    "li a7, 149 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_getresgid(gid_t  *rgid, gid_t  *egid, gid_t  *sgid){
  int rc;
  asm volatile (
    "li a7, 150 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_setfsuid(uid_t uid){
  int rc;
  asm volatile (
    "li a7, 151 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_setfsgid(gid_t gid){
  int rc;
  asm volatile (
    "li a7, 152 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_times(struct tms  *tbuf){
  int rc;
  asm volatile (
    "li a7, 153 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_setpgid(pid_t pid, pid_t pgid){
  int rc;
  asm volatile (
    "li a7, 154 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_getpgid(pid_t pid){
  int rc;
  asm volatile (
    "li a7, 155 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_getsid(pid_t pid){
  int rc;
  asm volatile (
    "li a7, 156 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_setsid(void){
  int rc;
  asm volatile (
    "li a7, 157 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_getgroups(int gidsetsize, gid_t  *grouplist){
  int rc;
  asm volatile (
    "li a7, 158 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_setgroups(int gidsetsize, gid_t  *grouplist){
  int rc;
  asm volatile (
    "li a7, 159 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_newuname(struct new_utsname  *name){
  int rc;
  asm volatile (
    "li a7, 160 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_sethostname(char  *name, int len){
  int rc;
  asm volatile (
    "li a7, 161 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_setdomainname(char  *name, int len){
  int rc;
  asm volatile (
    "li a7, 162 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_getrlimit(unsigned int resource, struct rlimit  *rlim){
  int rc;
  asm volatile (
    "li a7, 163 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_setrlimit(unsigned int resource, struct rlimit  *rlim){
  int rc;
  asm volatile (
    "li a7, 164 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_getrusage(int who, struct rusage  *ru){
  int rc;
  asm volatile (
    "li a7, 165 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_umask(int mask){
  int rc;
  asm volatile (
    "li a7, 166 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_prctl(int option, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5){
  int rc;
  asm volatile (
    "li a7, 167 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_getcpu(unsigned  *cpu, unsigned  *node, struct getcpu_cache  *cache){
  int rc;
  asm volatile (
    "li a7, 168 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_gettimeofday(struct __kernel_old_timeval  *tv, struct timezone  *tz){
  int rc;
  asm volatile (
    "li a7, 169 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_settimeofday(struct __kernel_old_timeval  *tv, struct timezone  *tz){
  int rc;
  asm volatile (
    "li a7, 170 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_adjtimex(struct __kernel_timex  *txc_p){
  int rc;
  asm volatile (
    "li a7, 171 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_getpid(void){
  int rc;
  asm volatile (
    "li a7, 172 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_getppid(void){
  int rc;
  asm volatile (
    "li a7, 173 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_getuid(void){
  int rc;
  asm volatile (
    "li a7, 174 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_geteuid(void){
  int rc;
  asm volatile (
    "li a7, 175 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_getgid(void){
  int rc;
  asm volatile (
    "li a7, 176 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_getegid(void){
  int rc;
  asm volatile (
    "li a7, 177 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_gettid(void){
  int rc;
  asm volatile (
    "li a7, 178 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_sysinfo(struct sysinfo  *info){
  int rc;
  asm volatile (
    "li a7, 179 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_mq_open(const char  *name, int oflag, umode_t mode, struct mq_attr  *attr){
  int rc;
  asm volatile (
    "li a7, 180 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_mq_unlink(const char  *name){
  int rc;
  asm volatile (
    "li a7, 181 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

// int rev_mq_timedsend(mqd_t mqdes, const char  *msg_ptr, size_t msg_len, unsigned int msg_prio, const struct __kernel_timespec  *abs_timeout){
//   int rc;
//   asm volatile (
//     "li a7, 182 \n\t"
//     "ecall \n\t"
//     "mv %0, a0" : "=r" (rc)
//     );
//   return rc;
// }

// int rev_mq_timedreceive(mqd_t mqdes, char  *msg_ptr, size_t msg_len, unsigned int  *msg_prio, const struct __kernel_timespec  *abs_timeout){
//   int rc;
//   asm volatile (
//     "li a7, 183 \n\t"
//     "ecall \n\t"
//     "mv %0, a0" : "=r" (rc)
//     );
//   return rc;
// }

// int rev_mq_notify(mqd_t mqdes, const struct sigevent  *notification){
//   int rc;
//   asm volatile (
//     "li a7, 184 \n\t"
//     "ecall \n\t"
//     "mv %0, a0" : "=r" (rc)
//     );
//   return rc;
// }

// int rev_mq_getsetattr(mqd_t mqdes, const struct mq_attr  *mqstat, struct mq_attr  *omqstat){
//   int rc;
//   asm volatile (
//     "li a7, 185 \n\t"
//     "ecall \n\t"
//     "mv %0, a0" : "=r" (rc)
//     );
//   return rc;
// }

int rev_msgget(key_t key, int msgflg){
  int rc;
  asm volatile (
    "li a7, 186 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_old_msgctl(int msqid, int cmd, struct msqid_ds  *buf){
  int rc;
  asm volatile (
    "li a7, 187 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_msgrcv(int msqid, struct msgbuf  *msgp, size_t msgsz, long msgtyp, int msgflg){
  int rc;
  asm volatile (
    "li a7, 188 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_msgsnd(int msqid, struct msgbuf  *msgp, size_t msgsz, int msgflg){
  int rc;
  asm volatile (
    "li a7, 189 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_semget(key_t key, int nsems, int semflg){
  int rc;
  asm volatile (
    "li a7, 190 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_semctl(int semid, int semnum, int cmd, unsigned long arg){
  int rc;
  asm volatile (
    "li a7, 191 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_semtimedop(int semid, struct sembuf  *sops, unsigned nsops, const struct __kernel_timespec  *timeout){
  int rc;
  asm volatile (
    "li a7, 192 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_semop(int semid, struct sembuf  *sops, unsigned nsops){
  int rc;
  asm volatile (
    "li a7, 193 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_shmget(key_t key, size_t size, int flag){
  int rc;
  asm volatile (
    "li a7, 194 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_old_shmctl(int shmid, int cmd, struct shmid_ds  *buf){
  int rc;
  asm volatile (
    "li a7, 195 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_shmat(int shmid, char  *shmaddr, int shmflg){
  int rc;
  asm volatile (
    "li a7, 196 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_shmdt(char  *shmaddr){
  int rc;
  asm volatile (
    "li a7, 197 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_socket(int, int, int){
  int rc;
  asm volatile (
    "li a7, 198 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_socketpair(int, int, int, int  *){
  int rc;
  asm volatile (
    "li a7, 199 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_bind(int, struct sockaddr  *, int){
  int rc;
  asm volatile (
    "li a7, 200 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_listen(int, int){
  int rc;
  asm volatile (
    "li a7, 201 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_accept(int, struct sockaddr  *, int  *){
  int rc;
  asm volatile (
    "li a7, 202 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_connect(int, struct sockaddr  *, int){
  int rc;
  asm volatile (
    "li a7, 203 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_getsockname(int, struct sockaddr  *, int  *){
  int rc;
  asm volatile (
    "li a7, 204 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_getpeername(int, struct sockaddr  *, int  *){
  int rc;
  asm volatile (
    "li a7, 205 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_sendto(int, void  *, size_t, unsigned, struct sockaddr  *, int){
  int rc;
  asm volatile (
    "li a7, 206 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_recvfrom(int, void  *, size_t, unsigned, struct sockaddr  *, int  *){
  int rc;
  asm volatile (
    "li a7, 207 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_setsockopt(int fd, int level, int optname, char  *optval, int optlen){
  int rc;
  asm volatile (
    "li a7, 208 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_getsockopt(int fd, int level, int optname, char  *optval, int  *optlen){
  int rc;
  asm volatile (
    "li a7, 209 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_shutdown(int, int){
  int rc;
  asm volatile (
    "li a7, 210 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_sendmsg(int fd, struct user_msghdr  *msg, unsigned flags){
  int rc;
  asm volatile (
    "li a7, 211 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_recvmsg(int fd, struct user_msghdr  *msg, unsigned flags){
  int rc;
  asm volatile (
    "li a7, 212 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_readahead(int fd, loff_t offset, size_t count){
  int rc;
  asm volatile (
    "li a7, 213 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_brk(unsigned long brk){
  int rc;
  asm volatile (
    "li a7, 214 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_munmap(unsigned long addr, size_t len){
  int rc;
  asm volatile (
    "li a7, 215 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_mremap(unsigned long addr, unsigned long old_len, unsigned long new_len, unsigned long flags, unsigned long new_addr){
  int rc;
  asm volatile (
    "li a7, 216 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_add_key(const char  *_type, const char  *_description, const void  *_payload, size_t plen, key_serial_t destringid){
  int rc;
  asm volatile (
    "li a7, 217 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_request_key(const char  *_type, const char  *_description, const char  *_callout_info, key_serial_t destringid){
  int rc;
  asm volatile (
    "li a7, 218 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_keyctl(int cmd, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5){
  int rc;
  asm volatile (
    "li a7, 219 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_fork(){
  int rc;
  asm volatile (
    "li a7, 220 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_clone(unsigned long, unsigned long, int  *, unsigned long, int  *){
  int rc;
  asm volatile (
    "li a7, 220 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_execve(const char  *filename, const char  *const  *argv, const char  *const  *envp){
  int rc;
  asm volatile (
    "li a7, 221 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_mmap(uint64_t addr, size_t length, int prot, int flags, int fd, off_t offset){
  int rc;
  asm volatile (
    "li a7, 222 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_fadvise64_64(int fd, loff_t offset, loff_t len, int advice){
  int rc;
  asm volatile (
    "li a7, 223 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_swapon(const char  *specialfile, int swap_flags){
  int rc;
  asm volatile (
    "li a7, 224 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_swapoff(const char  *specialfile){
  int rc;
  asm volatile (
    "li a7, 225 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_mprotect(unsigned long start, size_t len, unsigned long prot){
  int rc;
  asm volatile (
    "li a7, 226 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_msync(unsigned long start, size_t len, int flags){
  int rc;
  asm volatile (
    "li a7, 227 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_mlock(unsigned long start, size_t len){
  int rc;
  asm volatile (
    "li a7, 228 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_munlock(unsigned long start, size_t len){
  int rc;
  asm volatile (
    "li a7, 229 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_mlockall(int flags){
  int rc;
  asm volatile (
    "li a7, 230 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_munlockall(void){
  int rc;
  asm volatile (
    "li a7, 231 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_mincore(unsigned long start, size_t len, unsigned char  * vec){
  int rc;
  asm volatile (
    "li a7, 232 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_madvise(unsigned long start, size_t len, int behavior){
  int rc;
  asm volatile (
    "li a7, 233 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_remap_file_pages(unsigned long start, unsigned long size, unsigned long prot, unsigned long pgoff, unsigned long flags){
  int rc;
  asm volatile (
    "li a7, 234 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_mbind(unsigned long start, unsigned long len, unsigned long mode, const unsigned long  *nmask, unsigned long maxnode, unsigned flags){
  int rc;
  asm volatile (
    "li a7, 235 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_get_mempolicy(int  *policy, unsigned long  *nmask, unsigned long maxnode, unsigned long addr, unsigned long flags){
  int rc;
  asm volatile (
    "li a7, 236 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_set_mempolicy(int mode, const unsigned long  *nmask, unsigned long maxnode){
  int rc;
  asm volatile (
    "li a7, 237 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_migrate_pages(pid_t pid, unsigned long maxnode, const unsigned long  *from, const unsigned long  *to){
  int rc;
  asm volatile (
    "li a7, 238 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_move_pages(pid_t pid, unsigned long nr_pages, const void  *  *pages, const int  *nodes, int  *status, int flags){
  int rc;
  asm volatile (
    "li a7, 239 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_rt_tgsigqueueinfo(pid_t tgid, pid_t pid, int sig, siginfo_t  *uinfo){
  int rc;
  asm volatile (
    "li a7, 240 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}

int rev_perf_event_open(){
  int rc;
  asm volatile (
    "li a7, 241 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
  );
  return rc;
}

int rev_accept4(int, struct sockaddr  *, int *, int){
  int rc;
  asm volatile (
    "li a7, 242 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
);
return rc;
}

int rev_recvmmsg_time32(int fd, struct mmsghdr  *msg, unsigned int vlen, unsigned flags, struct old_timespec32* timeout){
  int rc;
  asm volatile (
    "li a7, 243 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
  );
  return rc;
}

int rev_wait4(pid_t pid, int  *stat_addr, int options, struct rusage *ru){
  int rc;
  asm volatile (
    "li a7, 260 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
  );
  return rc;
}

int rev_prlimit64(pid_t pid, unsigned int resource, const struct rlimit64  *new_rlim, struct rlimit64  *old_rlim){;
  int rc;
  asm volatile (
    "li a7, 261 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
  );
  return rc;
}

int rev_fanotify_init(unsigned int flags, unsigned int event_int){
  int rc;
  asm volatile (
    "li a7, 262 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
);
return rc;
}

int rev_fanotify_mark(int fanotify_fd, unsigned int flags, uint64_t mask, int fd, const char  *p){
  int rc;
  asm volatile (
    "li a7, 263 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
  );
  return rc;
}

int rev_name_to_handle_at(int dfd, const char  *name, struct file_handle  *handle, int  *mnt_id, int flag){
  int rc;
  asm volatile (
    "li a7, 264 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
  );
  return rc;
}

int rev_open_by_handle_at(int mountdirfd, struct file_handle  *handle, int flags){
  int rc;
  asm volatile (
    "li a7, 265 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
);
return rc;
}

int rev_clock_adjtime(clockid_t which_clock, struct __kernel_timex *tx){
  int rc;
  asm volatile (
    "li a7, 266 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
  );
  return rc;
}

int rev_syncfs(int fd){
  int rc;
  asm volatile (
    "li a7, 267 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
  );
  return rc;
}

int rev_setns(int fd, int nstype ){
  int rc;
  asm volatile (
    "li a7, 268 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
  );
return rc;
}

int rev_sendmmsg(int fd, struct mmsghdr  *msg, unsigned int vlen, unsigned flags){
  int rc;
  asm volatile (
    "li a7, 269 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
  );
  return rc;
}

int rev_process_vm_readv(pid_t pid, const struct iovec  *lvec, unsigned long liovcnt, const struct iovec  *rvec, unsigned long riovcnt, unsigned long flags){
  int rc;
  asm volatile (
    "li a7, 270 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
  );
  return rc;
}

int rev_process_vm_writev(pid_t pid, const struct iovec  *lvec, unsigned long liovcnt, const struct iovec  *rvec, unsigned long riovcnt, unsigned long flags){
  int rc;
  asm volatile (
    "li a7, 271 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
  );
  return rc;
}

int rev_kcmp(pid_t pid1, pid_t pid2, int type, unsigned long idx1, unsigned long idx2){
  int rc;
  asm volatile (
    "li a7, 272 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
);
return rc;
}

int rev_finit_module(int fd, const char  *uargs, int flags){
  int rc;
  asm volatile (
    "li a7, 273 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
  );
return rc;
}

int rev_sched_setattr(pid_t pid, struct sched_attr  *attr, unsigned int size, unsigned int flags){
  int rc;
  asm volatile (
    "li a7, 274 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
);
return rc;
}

int rev_sched_getattr(pid_t pid, struct sched_attr  *attr, unsigned int size, unsigned int flags){
  int rc;
  asm volatile (
    "li a7, 275 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
  );
  return rc;
}

int rev_renameat2(int olddfd, const char  *oldname, int newdfd, const char  *newname, unsigned int flags){
  int rc;
  asm volatile (
    "li a7, 276 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
  );
  return rc;
}

int rev_seccomp(unsigned int op, unsigned int flags, void *uargs){
  int rc;
  asm volatile (
    "li a7, 277 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
  );
  return rc;
}

int rev_getrandom(char  *buf, size_t count, unsigned int flags){
  int rc;
  asm volatile (
    "li a7, 278 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
  );
  return rc;
}

int rev_memfd_create(const char  *uname_ptr, unsigned int flags){
  int rc;
  asm volatile (
    "li a7, 279 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
  );
  return rc;
}

int rev_bpf(int cmd, union bpf_attr *attr, unsigned int size){
  int rc;
  asm volatile (
    "li a7, 280 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
  );
  return rc;
}

int rev_execveat(int dfd, const char  *filename, const char  *const  *argv, const char  *const  *envp, int flags){
  int rc;
  asm volatile (
    "li a7, 281 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
  );
  return rc;
}

int rev_userfaultfd(int flags){
  int rc;
  asm volatile (
    "li a7, 282 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
  );
  return rc;
}

int rev_membarrier(int cmd, unsigned int flags, int cpu_id){
  int rc;
  asm volatile (
    "li a7, 283 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
  );
  return rc;
}

int rev_mlock2(unsigned long start, size_t len, int flags){
  int rc;
  asm volatile (
    "li a7, 284 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
  );
  return rc;
}

int rev_copy_file_range(int fd_in, loff_t  *off_in, int fd_out, loff_t  *off_out, size_t len, unsigned int flags){
  int rc;
  asm volatile (
    "li a7, 285 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
  );
  return rc;
}

int rev_preadv2(unsigned long fd, const struct iovec  *vec, unsigned long vlen, unsigned long pos_l, unsigned long pos_h, rwf_t flags){
  int rc;
  asm volatile (
    "li a7, 286 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
);
return rc;
}

int rev_pwritev2(unsigned long fd, const struct iovec  *vec, unsigned long vlen, unsigned long pos_l, unsigned long pos_h, rwf_t flags){
  int rc;
  asm volatile (
    "li a7, 287 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
);
return rc;
}

int rev_pkey_mprotect(unsigned long start, size_t len, unsigned long prot){
  int rc;
  asm volatile (
    "li a7, 288 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
);
return rc;
}

int rev_pkey_alloc(unsigned long flags, unsigned long init_val){
  int rc;
  asm volatile (
    "li a7, 289 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
  );
  return rc;
}

int rev_pkey_free(int pkey){
  int rc;
  asm volatile (
    "li a7, 290 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
  );
  return rc;
}

int rev_statx(int dfd, const char  *path, unsigned flags, unsigned mask, struct statx *buffer){
  int rc;
  asm volatile (
    "li a7, 291 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
  );
  return rc;
}

int rev_io_pgetevents(aio_context_t ctx_id, long min_nr, long nr, struct io_event  *events, struct __kernel_timespec  *timeout, const struct __aio_sigset *sig){
  int rc;
  asm volatile (
    "li a7, 292 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
  );
  return rc;
}

int rev_rseq(struct rseq  *rseq, uint32_t rseq_len, int flags, uint32_t sig){
  int rc;
  asm volatile (
    "li a7, 293 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
  );
  return rc;
}

int rev_kexec_file_load(int kernel_fd, int initrd_fd, unsigned long cmdline_len, const char  *cmdline_ptr, unsigned long flags){
  int rc;
  asm volatile (
    "li a7, 294 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
  );
  return rc;
}

// int rev_clock_gettime(clockid_t which_clock, struct __kernel_timespec *tp){
//   int rc;
//   asm volatile (
//     "li a7, 403 \n\t"
//     "ecall \n\t"
//     "mv %0, a0" : "=r" (rc)
// );
// return rc;
// }

// int rev_clock_settime(clockid_t which_clock, const struct __kernel_timespec *tp){
//   int rc;
//   asm volatile (
//     "li a7, 404 \n\t"
//     "ecall \n\t"
//     "mv %0, a0" : "=r" (rc)
// );
// return rc;
// }

// int rev_clock_adjtime(clockid_t which_clock, struct __kernel_timint rc;
//   asm volatile (
//     "li a7, 405 \n\t"
//     "ecall \n\t"
//     "mv %0, a0" : "=r" (rc)
// );
// return rc;
// }

// int rev_clock_getres(clockid_t which_clock, struct __kernel_timespint rc;
//   asm volatile (
//     "li a7, 406 \n\t"
//     "ecall \n\t"
//     "mv %0, a0" : "=r" (rc)
// );
// return rc;
// }

// int rev_clock_nanosleep(clockid_t which_clock, int flags, const struct __kernel_timespec  *rqtp, struct __kernel_timespecint rc;
//   asm volatile (
//     "li a7, 407 \n\t"
//     "ecall \n\t"
//     "mv %0, a0" : "=r" (rc)
// );
// return rc;
// }

// int rev_timer_gettime(timer_t timer_id, struct __kernel_itimerspec *tp){
//   int rc;
//   asm volatile (
//     "li a7, 408 \n\t"
//     "ecall \n\t"
//     "mv %0, a0" : "=r" (rc)
// );
// return rc;
// }

// int rev_timer_settime(timer_t timer_id, int flags, const struct __kernel_itimerspec  *new_setting, struct __kernel_itimerspec  *old_setting){
//   int rc;
//   asm volatile (
//     "li a7, 409 \n\t"
//     "ecall \n\t"
//     "mv %0, a0" : "=r" (rc)
// );
// return rc;
// }

// int rev_timerfd_gettime(int ufd, struct __kernel_itimerspecint rc;
//   asm volatile (
//     "li a7, 410 \n\t"
//     "ecall \n\t"
//     "mv %0, a0" : "=r" (rc)
// );
// return rc;
// }

// int rev_timerfd_settime(int ufd, int flags, const struct __kernel_itimerspec  *utmr, struct __kernel_itimerspecint rc;
//   asm volatile (
//     "li a7, 411 \n\t"
//     "ecall \n\t"
//     "mv %0, a0" : "=r" (rc)
// );
// return rc;
// }

// int rev_utimensat(int dfd, const char  *filename, struct __kernel_timespec  *utimes, int flags){
//   asm volatile (
//     "li a7, 412 \n\t"
//     "ecall \n\t"
//     "mv %0, a0" : "=r" (rc)
// );
// return rc;
// }

// int rev_io_pgetevents(aio_context_t ctx_id, long min_nr, long nr, struct io_event  *events, struct __kernel_timespec  *timeout, const struct __aio_sigsint rc;
//   asm volatile (
//     "li a7, 416 \n\t"
//     "ecall \n\t"
//     "mv %0, a0" : "=r" (rc)
// );
// return rc;
// }

// int rev_mq_timedsend(mqd_t mqdes, const char  *msg_ptr, size_t msg_len, unsigned int msg_prio, const struct __kernel_timespec  *abs_
//   asm volatile (
//     "li a7, 418 \n\t"
//     "ecall \n\t"
//     "mv %0, a0" : "=r" (rc)
// );
// return rc;
// }

// int rev_mq_timedreceive(mqd_t mqdes, char  *msg_ptr, size_t msg_len, unsigned int  *msg_prio, const struct __kernel_timespec  *abs_int rc;
//   asm volatile (
//     "li a7, 419 \n\t"
//     "ecall \n\t"
//     "mv %0, a0" : "=r" (rc)
// );
// return rc;
// }

// int rev_semtimedop(int semid, struct sembuf  *sops, unsigned nsops, const struct __kernel_timespec  *int rc;
//   asm volatile (
//     "li a7, 420 \n\t"
//     "ecall \n\t"
//     "mv %0, a0" : "=r" (rc)
// );
// return rc;
// }

// int rev_futex(uint32_t  *uaddr, int op, uint32_t val, struct __kernel_timespec  *utime, uint32_t  *uaddr2, uint rc;
//   asm volatile (
//     "li a7, 422 \n\t"
//     "ecall \n\t"
//     "mv %0, a0" : "=r" (rc)
// );
// return rc;
// }

// int rev_sched_rr_get_interval(pid_t pid, struct __kernel_timespec  *iint rc;
//   asm volatile (
//     "li a7, 423 \n\t"
//     "ecall \n\t"
//     "mv %0, a0" : "=r" (rc)
// );
// return rc;
// }

int rev_pidfd_send_signal(int pidfd, int sig, siginfo_t  *info, unsigned int flags){
  int rc;
  asm volatile (
    "li a7, 424 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
);
return rc;
}

int rev_io_uring_setup(uint32_t entries, struct io_uring_params *p){
  int rc;
  asm volatile (
    "li a7, 425 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
  );
  return rc;
}

int rev_io_uring_enter(unsigned int fd, uint32_t to_submit, uint32_t min_complete, uint32_t flags, const sigset_t  *sig, size_t sigsz){
  int rc;
  asm volatile (
    "li a7, 426 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
  );
  return rc;
}

int rev_io_uring_register(unsigned int fd, unsigned int op, void  *arg, unsigned int nr_args){
  int rc;
  asm volatile (
    "li a7, 427 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
  );
  return rc;
}

int rev_open_tree(int dfd, const char  *path, unsigned flags){
  int rc;
  asm volatile (
    "li a7, 428 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
  );
  return rc;
}

int rev_move_mount(int from_dfd, const char  *from_path, int to_dfd, const char  *to_path, unsigned int ms_flags){
  int rc;
  asm volatile (
    "li a7, 429 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
  );
  return rc;
}

int rev_fsopen(const char  *fs_name, unsigned int flags){
  int rc;
  asm volatile (
    "li a7, 430 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
  );
  return rc;
}

int rev_fsconfig(int fs_fd, unsigned int cmd, const char  *key, const void  *value, int aux){
  int rc;
  asm volatile (
    "li a7, 431 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
  );
  return rc;
}

int rev_fsmount(int fs_fd, unsigned int flags, unsigned int ms_flags){
  int rc;
  asm volatile (
    "li a7, 432 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
);
return rc;
}

int rev_fspick(int dfd, const char  *path, unsigned int flags){
  int rc;
  asm volatile (
    "li a7, 433 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
  );
  return rc;
}

int rev_pidfd_open(pid_t pid, unsigned int flags){
  int rc;
  asm volatile (
    "li a7, 434 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
  );
  return rc;
}

int rev_clone3(struct clone_args  *uargs, size_t size){
  int rc;
  asm volatile (
    "li a7, 435 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
  );
  return rc;
}

int rev_close_range(unsigned int fd, unsigned int max_fd, unsigned int flags){
  int rc;
  asm volatile (
    "li a7, 436 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
  );
  return rc;
}

int rev_openat2(int dfd, const char  *filename, struct open_how *how, size_t size){
  int rc;
  asm volatile (
    "li a7, 437 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
  );
  return rc;
}

int rev_pidfd_getfd(int pidfd, int fd, unsigned int flags){
  int rc;
  asm volatile (
    "li a7, 438 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
  );
  return rc;
}

int rev_faccessat2(int dfd, const char  *filename, int mode, int flags){
  int rc;
  asm volatile (
    "li a7, 439 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
  );
  return rc;
}

int rev_process_madvise(int pidfd, const struct iovec  *vec, size_t vlen, int behavior, unsigned int flags){
  int rc;
  asm volatile (
    "li a7, 440 \n\t"
    "ecall \n\t"
    "mv %0, a0" : "=r" (rc)
    );
  return rc;
}
