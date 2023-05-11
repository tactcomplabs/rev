#include <sched.h>
#include <stdint.h>
#include <errno.h>
#include <signal.h>

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
int rev_clone3(struct clone_args* args, size_t args_size) {
    uintptr_t stack_size = 16 * 1024;
    uintptr_t stack[stack_size];

    args->flags = 10;
    args->pidfd = 20;
    args->child_tid = 30;
    args->parent_tid = 40;
    args->exit_signal = 50;
    args->stack = (uintptr_t)(stack + stack_size / sizeof(uintptr_t)); // Pass the stack top address
    args->stack_size = stack_size;
    args->tls = 60;
    args->set_tid_size = 90;
    args->cgroup = 99;

    asm volatile (
        "li a7, 220\n\t"     // load the value 220 into a7
        "ecall\n\t"       // execute the ecall instruction
    );
    return 0;
}

int child_func(void) {
    _exit(0);
}

int main(void) {
    // Allocate memory for the child stack
    struct clone_args args;

    // Make the clone3 system call
    // int ret = syscall(SYS_clone3, &args, sizeof(args));// syscall(__NR_clone3, &args, sizeof(args));
    int ret = rev_clone3(&args, sizeof(struct clone_args));// syscall(__NR_clone3, &args, sizeof(args));

    if (ret == -1) {
        return errno;
    }

    return 0;
}

