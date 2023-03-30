#include "../include/syscalls/RevChdir.h"
#include "../include/RevSysCalls.h"
std::unordered_map<int, systemcall_t> SystemCalls::jump_table32 = {
  { RevChdir::value, RevChdir::ECall<Riscv32> },
};
std::unordered_map<int, systemcall_t> SystemCalls::jump_table64 = {
  { RevChdir::value, RevChdir::ECall<Riscv64> },
};
