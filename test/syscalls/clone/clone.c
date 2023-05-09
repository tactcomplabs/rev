#include <unistd.h>
#include <sys/syscall.h>
#include <linux/sched.h>
#include <stdint.h>
#include <errno.h>
#include <signal.h>

int child_func(void) {
    _exit(0);
}

int main(void) {
    // Allocate memory for the child stack
    uintptr_t stack_size = 16 * 1024;
    uintptr_t stack[stack_size];

    // Set up the clone_args structure
    struct clone_args args;
    args.flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD | CLONE_SYSVSEM | CLONE_DETACHED;
    args.pidfd = 0;
    args.child_tid = 0;
    args.parent_tid = 0;
    args.exit_signal = 0;
    args.stack = (uintptr_t)(stack + stack_size / sizeof(uintptr_t)); // Pass the stack top address
    args.stack_size = stack_size;
    args.tls = 0;
    args.set_tid_size = 0;
    args.cgroup = 0;

    // Make the clone3 system call
    int ret = syscall(__NR_clone3, &args, sizeof(args));

    if (ret == -1) {
        return errno;
    }

    return 0;
}

