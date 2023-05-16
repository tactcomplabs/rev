#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

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


uint64_t rev_getcwd(char *buf, unsigned long size){          
    asm volatile (
      "li a7, 17\n\t"
      "ecall \n\t"
    );
    return 0;
}

uint64_t rev_dup(unsigned int fildes){             
    asm volatile (
      "li a7, 23\n\t"
      "ecall \n\t"
    );
    return 0;
}

uint64_t rev_dup3(unsigned int oldfd, unsigned int newfd, int flags){            
    asm volatile (
      "li a7, 24\n\t"
      "ecall \n\t"
    );
    return 0;
}

uint64_t rev_mkdirat(int dfd, const char * path, unsigned short mode){         
    asm volatile (
      "li a7, 34\n\t"
      "ecall \n\t"
    );
    return 0;
}

uint64_t rev_chdir(const char *filename){
  
  asm volatile (
    "li a7, 49\n\t"
    "ecall\n\t"
  );
  return 0;
}

uint64_t rev_fchownat(int dfd, const char *filename, unsigned user, unsigned group, int flag){        
    asm volatile (
      "li a7, 54\n\t"
      "ecall \n\t"
    );
    return 0;
}

uint64_t rev_fchown(unsigned int fd, unsigned short user, unsigned short group){          
    asm volatile (
      "li a7, 55\n\t"
      "ecall \n\t"
    );
    return 0;
}

uint64_t rev_close(unsigned int fd){
    asm volatile (
      "li a7, 57\n\t"
      "ecall \n\t"
    );
    return 0;
}

uint64_t rev_read(unsigned int fd, char *buf, size_t nbytes){
    asm volatile (
      "li a7, 63\n\t"
      "ecall \n\t"
    );
    return 0;
}

uint64_t rev_write(unsigned int fd, const char *buf, size_t nbytes){          
    asm volatile (
      "li a7, 64\n\t"
      "ecall \n\t"
    );
    return 0;
}

uint64_t rev_openat(int dfd, const char *filename, int flags, unsigned short mode){          
    asm volatile (
      "li a7, 65\n\t"
      "ecall \n\t"
    );
    return 0;
}

uint64_t rev_tee(int fdin, int fdout, size_t len, unsigned int flags){             
    asm volatile (
      "li a7, 77\n\t"
      "ecall \n\t"
    );
    return 0;
}

uint64_t rev_sync(void){            
    asm volatile (
      "li a7, 81\n\t"
      "ecall \n\t"
    );
    return 0;
}

uint64_t rev_fsync(unsigned int fd){           
    asm volatile (
      "li a7, 82\n\t"
      "ecall \n\t"
    );
    return 0;
}

uint64_t rev_fdatasync(unsigned int fd){       
    asm volatile (
      "li a7, 83\n\t"
      "ecall \n\t"
    );
    return 0;
}

uint64_t rev_exit(int error_code){           
    asm volatile (
      "li a7, 93\n\t"
      "ecall \n\t"
    );
    return 0;
}

uint64_t rev_exit_group(int error_code){      
    asm volatile (
      "li a7, 94\n\t"
      "ecall \n\t"
    );
    return 0;
}

uint64_t rev_waitid(int which, pid_t pid, struct siginfo  *infop, int options, struct rusage  *ru){          
    asm volatile (
      "li a7, 95\n\t"
      "ecall \n\t"
    );
    return 0;
}

uint64_t rev_set_robust_list(struct robust_list_head *head, size_t len){ 
    asm volatile (
      "li a7, 99\n\t"
      "ecall \n\t"
    );
    return 0;
}

uint64_t rev_get_robust_list(int pid, struct robust_list_head *head_ptr, size_t *len_ptr){ 
    asm volatile (
      "li a7, 100\n\t"
      "ecall \n\t"
    );
    return 0;
}

uint64_t rev_nanosleep(struct __kernel_timespec *rqtp, struct __kernel_timespec *rmtp){
    asm volatile (
      "li a7, 101\n\t"
      "ecall \n\t"
    );
    return 0;
}

uint64_t rev_timer_create(clockid_t which_clock, struct sigevent *timer_event_spec, timer_t *created_timer_id){
    asm volatile (
      "li a7, 107\n\t"
      "ecall \n\t"
    );
    return 0;
}

uint64_t rev_timer_delete(timer_t timer_id){    
    asm volatile (
      "li a7, 110\n\t"
      "ecall \n\t"
    );
    return 0;
}

uint64_t rev_rt_sigprocmask(int how, sigset_t *set,
                            sigset_t *oset, size_t sigsetsize){  
    asm volatile (
      "li a7, 135\n\t"
      "ecall \n\t"
    );
    return 0;
}

uint64_t rev_gettimeofday(struct timeval *tv, struct timezone *tz){    
    asm volatile (
      "li a7, 169\n\t"
      "ecall \n\t"
    );
    return 0;
}

uint64_t rev_settimeofday(struct timeval *tv, struct timezone *tz){    
    asm volatile (
      "li a7, 170\n\t"
      "ecall \n\t"
    );
    return 0;
}
uint64_t rev_getpid(void){         
    asm volatile (
      "li a7, 172\n\t"
      "ecall \n\t"
    );
    return 0;
}
uint64_t rev_getppid(void){        
    asm volatile (
      "li a7, 173\n\t"
      "ecall \n\t"
    );
    return 0;
}

uint64_t rev_gettid(void){          
    asm volatile (
      "li a7, 178\n\t"
      "ecall \n\t"
    );
    return 0;
}

// uint64_t rev_fork() {

//     asm volatile (
//         "li a7, 220\n\t"     // load the value 220 into a7
//         "ecall\n\t"       // execute the ecall instruction
//     );
//     return 0;
// }

uint64_t rev_clone3(struct clone_args* args, size_t args_size) {

    asm volatile (
        "li a7, 220\n\t"     // load the value 220 into a7
        "ecall\n\t"       // execute the ecall instruction
    );
    return 0;
}

uint64_t rev_mmap(struct mmap_arg_struct *args){            
    asm volatile (
      "li a7, 222\n\t"
        "ecall \n\t"
      );
    return 0;
}

uint64_t rev_clock_gettime(clockid_t which_clock, struct timeval *tp){
    asm volatile (
      "li a7, 403\n\t"
        "ecall \n\t"
      );
    return 0;
}

uint64_t rev_clock_settime(clockid_t which_clock, const struct __kernel_timespec *tp){   
    asm volatile (
      "li a7, 404\n\t"
        "ecall \n\t"
      );
    return 0;
}

uint64_t rev_timer_gettime(timer_t timer_id, struct __kernel_itimerspec *setting){   
    asm volatile (
      "li a7, 408\n\t"
        "ecall \n\t"
      );
    return 0;
}

uint64_t rev_timer_settime(timer_t timer_id, int flags, const struct __kernel_itimerspec  *new_setting, struct __kernel_itimerspec  *old_setting){   
    asm volatile (
      "li a7, 409\n\t"
        "ecall \n\t"
      );
    return 0;
}

