#include "../include/syscalls/RevChdir.h"
#include "../include/syscalls/RevMkdirat.h"
#include "../include/RevSysCalls.h"

#define _SYSCALL_DEBUG_ 1 

std::unordered_map<int, systemcall_t> SystemCalls::jump_table32 = {
  { RevChdir::value, RevChdir::ECall<Riscv32> },
  { RevMkdirat::value, RevMkdirat::ECall<Riscv32> },
};
std::unordered_map<int, systemcall_t> SystemCalls::jump_table64 = {
  { RevChdir::value, RevChdir::ECall<Riscv64> },
  { RevMkdirat::value, RevMkdirat::ECall<Riscv64> },
};
