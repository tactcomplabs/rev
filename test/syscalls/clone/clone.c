#include <sched.h>
#include <stdint.h>
#include <errno.h>
#include <signal.h>
#include "../../../common/syscalls.h"

int child_func(void) {
    _exit(0);
}

int main(void) {
    // Allocate memory for the child stack
    struct clone_args args;

    args.stack = 10;
    args.tls = 20;
    args.set_tid = 1234;
    args.pidfd = 123447;
    args.cgroup = 6789;
    args.flags = CLONE_VM | CLONE_FS;

    // Make the clone3 system call
    // int ret = syscall(SYS_clone3, &args, sizeof(args));// syscall(__NR_clone3, &args, sizeof(args));
    int ret = rev_clone3(&args, sizeof(struct clone_args));// syscall(__NR_clone3, &args, sizeof(args));

    if (ret == -1) {
        return errno;
    }

    return 0;
}

