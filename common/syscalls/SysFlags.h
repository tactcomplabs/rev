//
// _PanAddr_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include <cstdint>
#ifndef _CLONEFLAGS_H_
#define _CLONEFLAGS_H_

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

#define AT_FDCWD                -100    /* Special value used to indicate the *at functions should use the current working directory. */
#define AT_SYMLINK_NOFOLLOW     0x100   /* Do not follow symbolic links.  */
#define AT_REMOVEDIR            0x200   /* Remove directory instead of unlinking file.  */
#define AT_SYMLINK_FOLLOW       0x400   /* Follow symbolic links.  */
#define AT_NO_AUTOMOUNT 0x800   /* Suppress terminal automount traversal.  */
#define AT_EMPTY_PATH           0x1000  /* Allow empty relative pathname.  */
#define AT_STATX_SYNC_TYPE      0x6000
#define AT_STATX_SYNC_AS_STAT   0x0000
#define AT_STATX_FORCE_SYNC     0x2000
#define AT_STATX_DONT_SYNC      0x4000
#define AT_RECURSIVE            0x8000  /* Apply to the entire subtree.  */

struct clone_args {
    uint64_t flags;        /* Flags bit mask */
    uint64_t pidfd;        /* Where to store PID file descriptor (int *) */
    uint64_t child_tid;    /* Where to store child TID, in child's memory (pid_t *) */
    uint64_t parent_tid;   /* Where to store child TID, in parent's memory (pid_t *) */
    uint64_t exit_signal;  /* Signal to deliver to parent on child termination */
    uint64_t stack;        /* Pointer to lowest byte of stack */
    uint64_t stack_size;   /* Size of stack */
    uint64_t tls;          /* Location of new TLS */
    uint64_t set_tid;      /* Pointer to a pid_t array (since Linux 5.5) */
    uint64_t set_tid_size; /* Number of elements in set_tid (since Linux 5.5) */
    uint64_t cgroup;       /* File descriptor for target cgroup of child (since Linux 5.7) */
};

#endif

// EOF
