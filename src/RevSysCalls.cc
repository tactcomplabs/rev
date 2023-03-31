#include "../include/syscalls/RevChdir.h"
#include "../include/syscalls/RevMkdirat.h"
#include "../include/syscalls/RevWrite.h"
#include "../include/syscalls/RevPwrite.h"
#include "../include/RevSysCalls.h"

#define _SYSCALL_DEBUG_ 1 

std::unordered_map<int, systemcall_t> SystemCalls::jump_table32 = {
  { RevChdir::value, RevChdir::ECall<Riscv32> },
  { RevMkdirat::value, RevMkdirat::ECall<Riscv32> },
  { RevWrite::value, RevWrite::ECall<Riscv32> },
  { RevPwrite::value, RevPwrite::ECall<Riscv32> },
};
std::unordered_map<int, systemcall_t> SystemCalls::jump_table64 = {
  { RevChdir::value, RevChdir::ECall<Riscv64> },
  { RevMkdirat::value, RevMkdirat::ECall<Riscv64> },
  { RevWrite::value, RevWrite::ECall<Riscv64> },
  { RevPwrite::value, RevPwrite::ECall<Riscv64> },
};