#include "../include/RevProc.h"

namespace SST::RevCPU{
/* ======================================================= */
/* int rev_setxattr(const char *path, const char *name,    */
/*              const void *value, size_t size, int flags) */
/*======================================================== */
RevProc::ECALL_status_t RevProc::ECALL_setxattr(RevInst& inst){
#if 0
  // TODO: Need to load the data from (value, size bytes) into
  // hostValue vector before it can be passed to setxattr() on host.

  auto path = RegFile->GetX<uint64_t>(feature, 10);
  auto name = RegFile->GetX<uint64_t>(feature, 11);
  auto value = RegFile->GetX<uint64_t>(feature, 12);
  auto size = RegFile->GetX<size_t>(feature, 13);
  auto flags = RegFile->GetX<uint64_t>(feature, 14);

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
      RegFile->SetX(feature, 10, rc);
    };

    // Parse the name string, then call setxattr() using path and name
    return ECALL_LoadAndParseString(inst, name, action);
  }
#else
  return ECALL_status_t::SUCCESS;
#endif
}

/* Increments program break by n bytes  */
RevProc::ECALL_status_t RevProc::ECALL_brk(RevInst& inst){
  auto Addr = RegFile->GetX<uint64_t>(feature, 10);

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

/* ======================================================= */
/* rev_clone3(struct clone_args*, size_t args_size)        */
/* ======================================================= */
RevProc::ECALL_status_t RevProc::ECALL_clone(RevInst& inst){
  auto rtval = ECALL_status_t::ERROR;
  auto CloneArgsAddr = RegFile->GetX<uint64_t>(feature, 10);
  // auto SizeOfCloneArgs = RegFile()->GetX<size_t>(feature, 11);

 if(0 == ECALL.bytesRead){
    // First time through the function...
    /* Fetch the clone_args */
    // struct clone_args args;  // So while clone_args is a whole struct, we appear to be only
                                // using the 1st uint64, so that's all we're going to fetch
   uint64_t* args = reinterpret_cast<uint64_t*>(ECALL.buf.data());
   mem->ReadVal<uint64_t>(HartToExec, CloneArgsAddr, args, inst.hazard, REVMEM_FLAGS(0x00));
   ECALL.bytesRead = sizeof(*args);
   rtval = ECALL_status_t::CONTINUE;
 }else{
    /*
    * Parse clone flags
    * NOTE: if no flags are set, we get fork() like behavior
    */
   uint64_t* args = reinterpret_cast<uint64_t*>(ECALL.buf.data());
    for( uint64_t bit=1; bit != 0; bit <<= 1 ){
      switch (*args & bit) {
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
          // std::cout << "CLONE_NEWPID is true" << std::endl;
          break;
        case CLONE_NEWNET: /* New network namespace */
          // std::cout << "CLONE_NEWNET is true" << std::endl;
          break;
        case CLONE_IO: /* Clone I/O Context */
          // std::cout << "CLONE_IO is true" << std::endl;
          break;
        default:
          break;
      } // switch
    } // for

    /* Get the parent ctx (Current active, executing PID) */
    std::shared_ptr<RevThreadCtx> ParentCtx = ThreadTable.at(ActivePIDs.at(HartToExec));

    /* Create the child ctx */
    uint32_t ChildPID = CreateChildCtx();
    std::shared_ptr<RevThreadCtx> ChildCtx = ThreadTable.at(ChildPID);

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
    *         - The Parent's a0 register MUST have its PID in it
    * ===========================================================================================
    */

    /*
    Alert the Proc there needs to be a Ctx switch
    Pass the PID that will be switched to once the
    current pipeline is executed until completion
    */
    CtxSwitchAlert(ChildPID);

    // Parent's return value is the child's PID
    RegFile->SetX(feature, 10, ChildPID);

    // Child's return value is 0
    ChildCtx->GetRegFile()->SetX(feature, 10, 0);

    // clean up ecall state
    rtval = RevProc::ECALL_status_t::SUCCESS;
    ECALL.bytesRead = 0;

  } //else
  return rtval;
}


/* =============================== */
/* rev_chdir(const char *filename) */
/* =============================== */
RevProc::ECALL_status_t RevProc::ECALL_chdir(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL_chdir called\n");
  auto path = RegFile->GetX<uint64_t>(feature, 10);
  auto action = [&]{
    int rc = chdir(ECALL.string.c_str());
    RegFile->SetX(feature, 10, rc);
  };
  return ECALL_LoadAndParseString(inst, path, action);
}

/* ============================================================ */
/* rev_mkdirat(int dfd, const char * path, unsigned short mode) */
/* ============================================================ */
// RevProc::ECALL_status_t RevProc::ECALL_mkdir(){
//   output->verbose(CALL_INFO, 2, 0, "ECALL_mkdir called\n");
//   std::string path = "";
//   unsigned i=0;
//
//   const int rc = chdir(path.data());
//   RegFile->SetX(feature, 10, rc);
// }



/* ======================== */
/* rev_exit(int error_code) */
/* ======================== */
RevProc::ECALL_status_t RevProc::ECALL_exit(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL_exit called\n");
  auto CurrCtx = HartToExecCtx();
  auto status = RegFile->GetX<uint64_t>(feature, 10);

  // If the current ctx has ParentPID = 0,
  // it has no parent and we should terminate the sim
  if( CurrCtx->GetParentPID() == 0 ){
    output->verbose(CALL_INFO, 0, 0,
                    "Process %u exiting with status %lu\n",
                    CurrCtx->GetPID(), status );
    exit(status);
  } else {
    // Parent exists & Child is exiting... switch back to parent
    CtxSwitchAlert(CurrCtx->GetParentPID());
    output->verbose(CALL_INFO, 0, 0,
                    "Process %u exiting with status %lu\n",
                    CurrCtx->GetPID(), status );
  }
  return ECALL_status_t::SUCCESS;
}

/* ========================================= */
/* rev_getcwd(char *buf, unsigned long size) */
/* ========================================= */
RevProc::ECALL_status_t RevProc::ECALL_getcwd(RevInst& inst){
  auto BufAddr = RegFile->GetX<uint64_t>(feature, 10);
  auto size = RegFile->GetX<uint64_t>(feature, 11);
  auto CWD = std::filesystem::current_path();
  mem->WriteMem(feature->GetHart(), BufAddr, size, CWD.c_str());

  // Returns null-terminated string in buf
  // (no need to set x10 since it's already got BufAddr)
  // RegFile->SetX(feature, 10, BufAddr);

  return ECALL_status_t::SUCCESS;
}

/* ================ */
/* rev_getpid(void) */
/* ================ */
RevProc::ECALL_status_t RevProc::ECALL_getpid(RevInst& inst){
  // TODO: Implement error handling
  output->verbose(CALL_INFO, 2, 0, "ECALL_getpid called\n");
  uint32_t CurrentPID = ActivePIDs.at(HartToExec);
  auto CurrentCtx = ThreadTable.at(CurrentPID);
  RegFile->SetX(feature, 10, ActivePIDs.at(HartToExec));
  return ECALL_status_t::SUCCESS;
}

/* ================= */
/* rev_getppid(void) */
/* ================= */
RevProc::ECALL_status_t RevProc::ECALL_getppid(RevInst& inst){
  // TODO: Implement error handling
  output->verbose(CALL_INFO, 2, 0, "ECALL_getppid called\n");
  uint32_t CurrentPID = ActivePIDs.at(HartToExec);
  auto CurrentCtx = ThreadTable.at(CurrentPID);
  uint32_t ParentPID = CurrentCtx->GetParentPID();
  RegFile->SetX(feature, 10, ParentPID);
  return ECALL_status_t::SUCCESS;
}

/* ========================================================== */
/* rev_write(int fd, const char *buf, size_t nbytes)          */
/* ========================================================== */
RevProc::ECALL_status_t RevProc::ECALL_write(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL_write called\n");
  auto fd = RegFile->GetX<int>(feature, 10);
  auto addr = RegFile->GetX<uint64_t>(feature, 11);
  auto nbytes = RegFile->GetX<uint64_t>(feature, 12);
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
    RegFile->SetX(feature, 10, rc);

    // Reset our tracking state
    ECALL.clear();

    DependencyClear(HartToExec, 10, false);
    rtv = ECALL_status_t::SUCCESS;
  }else {
    auto readfunc = [&](auto* buf){
      mem->ReadVal(HartToExec,
                   addr + ECALL.string.size(),
                   buf,
                   inst.hazard,
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
  }
  return rtv;
}

/* ========================================================================== */
/* rev_timer_settime(timer_t timer_id, int flags, */
/*   const struct __kernel_itimerspec  *new_setting, */
/*   struct __kernel_itimerspec  *old_setting) */
/* ========================================================================== */
RevProc::ECALL_status_t RevProc::ECALL_timer_settime(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: timer_settime called\n");
  return ECALL_status_t::SUCCESS;
}

/* ======================================================================== */
/* rev_timer_gettime(timer_t timer_id, struct __kernel_itimerspec *setting) */
/* ======================================================================== */
RevProc::ECALL_status_t RevProc::ECALL_timer_gettime(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: timer_gettime called\n");
  return ECALL_status_t::SUCCESS;
}

/* ========================================================================== */
/* rev_clock_settime(clockid_t which_clock, */
/* const struct __kernel_timespec *tp) */
/* ========================================================================== */
RevProc::ECALL_status_t RevProc::ECALL_clock_settime(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: clock_settime called\n");
  return ECALL_status_t::SUCCESS;
}

/* ============================================================ */
/* rev_clock_gettime(clockid_t which_clock, struct timeval *tp) */
/* ============================================================ */
RevProc::ECALL_status_t RevProc::ECALL_clock_gettime(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: clock_gettime called\n");
  return ECALL_status_t::SUCCESS;
}

/* ====================================== */
/* rev_mmap(struct mmap_arg_struct *args) */
/* ====================================== */
// void *mmap(void *addr, size_t length, int prot, int flags,
//          int fd, off_t offset);
RevProc::ECALL_status_t RevProc::ECALL_mmap(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: mmap called\n");

  auto addr = RegFile->GetX<uint64_t>(feature, 10);
  auto size = RegFile->GetX<uint64_t>(feature, 11);
  // auto prot = RegFile->GetX<int>(feature, 12);
  // auto Flags = RegFile->GetX<int>(feature, 13);
  // auto fd = RegFile->GetX<int>(feature, 14);
  // auto offset = RegFile->GetX<off_t>(feature, 15);

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
  RegFile->SetX(feature, 10, addr);
  return ECALL_status_t::SUCCESS;
}

/* ================================== */
/* munmap(void *addr, size_t length); */
/* ================================== */
RevProc::ECALL_status_t RevProc::ECALL_munmap(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: munmap called\n");
  auto Addr = RegFile->GetX<uint64_t>(feature, 10);
  auto Size = RegFile->GetX<uint64_t>(feature, 11);

  int rc =  mem->DeallocMem(Addr, Size) == uint64_t(-1);
  if(rc == -1){
    output->fatal(CALL_INFO, 11,
                  "Failed to perform munmap(Addr = 0x%lx, Size = 0x%lx)"
                  "likely because the memory was not allocated to begin with" ,
                  Addr, Size);
  }

  RegFile->SetX(feature, 10, rc);
  return ECALL_status_t::SUCCESS;
}


/* ================ */
/* rev_gettid(void) */
/* ================ */
RevProc::ECALL_status_t RevProc::ECALL_gettid(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: gettid called\n");
  RevRegFile* regFile = RegFile;

  /* rc = Currently Executing Hart */
  regFile->SetX(feature, 10, HartToExec);
  return ECALL_status_t::SUCCESS;
}

/* ========================================================= */
/* rev_settimeofday(struct timeval *tv, struct timezone *tz) */
/* ========================================================= */
RevProc::ECALL_status_t RevProc::ECALL_settimeofday(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: settimeofday called\n");
  return ECALL_status_t::SUCCESS;
}

/* ============================================================= */
/* int rev_gettimeofday(struct timeval *tv, struct timezone *tz) */
/* ============================================================= */
RevProc::ECALL_status_t RevProc::ECALL_gettimeofday(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: gettimeofday called\n");
  return ECALL_status_t::SUCCESS;
}


/* ========================================================================== */
/* rev_rt_sigprocmask(int how, sigset_t *set, sigset_t *oset, */
/* size_t sigsetsize) */
/* ========================================================================== */
RevProc::ECALL_status_t RevProc::ECALL_rt_sigprocmask(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: rt_sigprocmask called\n");
  return ECALL_status_t::SUCCESS;
}

/* ================================== */
/* rev_timer_delete(timer_t timer_id) */
/* ================================== */
RevProc::ECALL_status_t RevProc::ECALL_timer_delete(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: timer_delete called\n");
  return ECALL_status_t::SUCCESS;
}

/* ===========================================================================*/
/* rev_timer_create(clockid_t which_clock, struct sigevent *timer_event_spec, */
/*   timer_t *created_timer_id) */
/* ===========================================================================*/
RevProc::ECALL_status_t RevProc::ECALL_timer_create(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: timer_create called\n");
  return ECALL_status_t::SUCCESS;
}


/* ========================================================================== */
/* rev_nanosleep(struct __kernel_timespec *rqtp, */
/* struct __kernel_timespec *rmtp) */
/* ========================================================================== */
RevProc::ECALL_status_t RevProc::ECALL_nanosleep(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: nanosleep called\n");
  return ECALL_status_t::SUCCESS;
}

/* ========================================================================== */
/* rev_get_robust_list(int pid, struct robust_list_head *head_ptr, */
/*   size_t *len_ptr) */
/* ========================================================================== */
RevProc::ECALL_status_t RevProc::ECALL_get_robust_list(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: get_robust_list called\n");
  return ECALL_status_t::SUCCESS;
}

/* ============================================================== */
/* rev_set_robust_list(struct robust_list_head *head, size_t len) */
/* ============================================================== */
RevProc::ECALL_status_t RevProc::ECALL_set_robust_list(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: set_robust_list called\n");
  return ECALL_status_t::SUCCESS;
}

/* ========================================================================== */
/* rev_waitid(int which, pid_t pid, struct siginfo  *infop, int options, */
/*   struct rusage  *ru) */
/* ==========================================================================*/
RevProc::ECALL_status_t RevProc::ECALL_waitid(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: waitid called\n");
  return ECALL_status_t::SUCCESS;
}

/* ============================== */
/* rev_exit_group(int error_code) */
/* ============================== */
RevProc::ECALL_status_t RevProc::ECALL_exit_group(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: exit_group called\n");
  return ECALL_status_t::SUCCESS;
}

/* ============================== */
/* rev_fdatasync(unsigned int fd) */
/* ============================== */
RevProc::ECALL_status_t RevProc::ECALL_fdatasync(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fdatasync called\n");
  return ECALL_status_t::SUCCESS;
}

/* ========================== */
/* rev_fsync(unsigned int fd) */
/* ========================== */
RevProc::ECALL_status_t RevProc::ECALL_fsync(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fsync called\n");
  return ECALL_status_t::SUCCESS;
}

/* ============== */
/* rev_sync(void) */
/* ============== */
RevProc::ECALL_status_t RevProc::ECALL_sync(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: sync called\n");
  return ECALL_status_t::SUCCESS;
}

/* =================================================================== */
/*  ssize_t tee(int fd_in, int fd_out, size_t len, unsigned int flags) */
/* =================================================================== */
RevProc::ECALL_status_t RevProc::ECALL_tee(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: tee called\n");
#if 0
  // commented out to remove warnings
  auto fd_in  = RegFile->GetX<int>(feature, 10);
  auto fd_out = RegFile->GetX<int>(feature, 11);
  auto len    = RegFile->GetX<uint64_t>(feature, 12);
  auto flags  = RegFile->GetX<uint32_t>(feature, 13);
#endif
  return ECALL_status_t::SUCCESS;
}


/* =================================================================== */
/* int openat(int dirfd, const char *pathname, int flags, mode_t mode) */
/* =================================================================== */
RevProc::ECALL_status_t RevProc::ECALL_openat(RevInst& inst){
  auto dirfd = RegFile->GetX<int>(feature, 10);
  auto pathname = RegFile->GetX<uint64_t>(feature, 11);

  // commented out to remove warnings
  // auto flags = RegFile->GetX<int>(feature, 12);
  // auto mode = RegFile->GetX<int>(feature, 13);

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

    HartToExecCtx()->AddFD(fd);

    // openat returns the file descriptor of the opened file
    RegFile->SetX(feature, 10, fd);
  };

  return ECALL_LoadAndParseString(inst, pathname, action);
}

/* =================================================== */
/* rev_read(unsigned int fd, char *buf, size_t nbytes) */
/* =================================================== */
RevProc::ECALL_status_t RevProc::ECALL_read(RevInst& inst){
  auto fd = RegFile->GetX<int>(feature, 10);
  auto BufAddr = RegFile->GetX<uint64_t>(feature, 11);
  auto BufSize = RegFile->GetX<uint64_t>(feature, 12);

  /* Check if Current Ctx has access to the fd */
  auto CurrCtx = HartToExecCtx();

  if( !CurrCtx->FindFD(fd) ){
    output->fatal(CALL_INFO, -1,
                  "Core %u; Hart %" PRIu16 "; PID %" PRIu32
                  " tried to read from file descriptor: %d, but did not have access to it\n",
                  id, HartToExec, HartToExecPID(), fd);
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
  mem->WriteMem(feature->GetHart(), BufAddr, BufSize, &TmpBuf[0]);

  RegFile->SetX(feature, 10, rc);
  return ECALL_status_t::SUCCESS;
}


/* ========================== */
/* rev_close(unsigned int fd) */
/* ========================== */
RevProc::ECALL_status_t RevProc::ECALL_close(RevInst& inst){
  auto fd = RegFile->GetX<int>(feature, 10);
  auto CurrCtx = HartToExecCtx();

  // Check if CurrCtx has fd in fildes vector
  if( !CurrCtx->FindFD(fd) ){
    output->fatal(CALL_INFO, -1,
                  "Core %u; Hart %d; PID %" PRIu32 " tried to close file descriptor %d but did not have access to it\n",
                  id, HartToExec, HartToExecPID(), fd);
    return ECALL_status_t::SUCCESS;
  }
  // Close file on host
  int rc = close(fd);

  // Remove from Ctx's fildes
  CurrCtx->RemoveFD(fd);

  // rc is propogated to rev from host
  RegFile->SetX(feature, 10, rc);

  return ECALL_status_t::SUCCESS;
}

/* ====================================================================== */
/* rev_fchown(unsigned int fd, unsigned short user, unsigned short group) */
/* ====================================================================== */
RevProc::ECALL_status_t RevProc::ECALL_fchown(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fchown called");
  return ECALL_status_t::SUCCESS;
}

/* ==================================================================================== */
/* rev_fchownat(int dfd, const char *filename, unsigned user, unsigned group, int flag) */
/* ==================================================================================== */
RevProc::ECALL_status_t RevProc::ECALL_fchownat(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: fchownat called");
  return ECALL_status_t::SUCCESS;
}

/* ======================================================================*/
/* rev_mkdirat(int dirfd, const char * path, unsigned short mode)        */
/* ======================================================================*/
RevProc::ECALL_status_t RevProc::ECALL_mkdirat(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL_mkdirat called");
  auto dirfd = RegFile->GetX<int>(feature, 10);
  auto path = RegFile->GetX<uint64_t>(feature, 11);
  auto mode = RegFile->GetX<unsigned short>(feature, 12);

  auto action = [&]{
    // Do the mkdirat on the host
    int rc = mkdirat(dirfd, ECALL.string.c_str(), mode);
    RegFile->SetX(feature, 10, rc);
  };
  return ECALL_LoadAndParseString(inst, path, action);
}

/* =========================================================== */
/* rev_dup3(unsigned int oldfd, unsigned int newfd, int flags) */
/* =========================================================== */
RevProc::ECALL_status_t RevProc::ECALL_dup3(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: dup3 called");
  return ECALL_status_t::SUCCESS;
}


/* =========================================================== */
/* rev_dup(unsigned int fildes)                                */
/* =========================================================== */
RevProc::ECALL_status_t RevProc::ECALL_dup(RevInst& inst){
  output->verbose(CALL_INFO, 2, 0, "ECALL: dup called");
  return ECALL_status_t::SUCCESS;
}

