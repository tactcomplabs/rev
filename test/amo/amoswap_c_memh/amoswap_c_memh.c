#include <limits.h>
#include <stdint.h>
#include <stdio.h>

#define assert(x)                                                              \
  if (!(x)) {                                                                  \
    asm(".byte 0x00");                                                         \
    asm(".byte 0x00");                                                         \
    asm(".byte 0x00");                                                         \
    asm(".byte 0x00");                                                         \
  }

uint64_t swap_dest = 0xfee1dead;
uint64_t swap_src = (((uint64_t)0xdeadbeef) << 32);
uint64_t swap_prev = 0;

uint32_t swap_dest32 = 0xfee1dead;
uint32_t swap_src32 = (((uint32_t)0xdeadbeef) << 16);
uint32_t swap_prev32 = 0;

void test_single_thread32() {
  // swap_prev should become swap_dest
  // swap_dest should become swap_src
  __atomic_exchange(&swap_dest32, &swap_src32, &swap_prev32, __ATOMIC_SEQ_CST);
  assert(swap_dest32 == (((uint32_t)0xdeadbeef) << 16));
  assert(swap_prev32 == 0xfee1dead);
}

void test_single_thread64() {
  // swap_prev should become swap_dest
  // swap_dest should become swap_src
  __atomic_exchange(&swap_dest, &swap_src, &swap_prev, __ATOMIC_SEQ_CST);
  assert(swap_dest == (((uint64_t)0xdeadbeef) << 32));
  assert(swap_prev == 0xfee1dead);
}

int main() {
  // single thread test
  test_single_thread32();
  test_single_thread64();
  return 0;
}
