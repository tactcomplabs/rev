#include "updown.h"
#include <iostream>

int main() {
  // Default configurations runtime
  UpDown::UDRuntime_t< UpDown::RevRTAllocPolicy< UpDown::ud_mapped_memory_t > >
                test_rt = UpDown::UDRuntime_t< UpDown::RevRTAllocPolicy<
      UpDown::ud_mapped_memory_t > >();  // = new UpDown::UDRuntime_t();
  //UpDown::UDRuntime_t *test_rt = new UpDown::UDRuntime_t();

  // printf("=== Base Addresses ===\n");
  //  test_rt->dumpBaseAddrs();
  // printf("\n=== Machine Config ===\n");
  // test_rt->dumpMachineConfig();

  UpDown::ptr_t a_ptr, a_ptr2, a_ptr3, a_ptr4, a_ptr5, a_ptr6, a_ptr7, a_ptr8;

  a_ptr  = (UpDown::ptr_t) test_rt.mm_malloc( 100 * sizeof( UpDown::word_t ) );
  a_ptr2 = (UpDown::ptr_t) test_rt.mm_malloc( 200 * sizeof( UpDown::word_t ) );
  a_ptr3 = (UpDown::ptr_t) test_rt.mm_malloc( 300 * sizeof( UpDown::word_t ) );
  a_ptr4 = (UpDown::ptr_t) test_rt.mm_malloc( 400 * sizeof( UpDown::word_t ) );
  a_ptr5 =
    (UpDown::ptr_t) test_rt.mm_malloc_global( 100 * sizeof( UpDown::word_t ) );
  a_ptr6 =
    (UpDown::ptr_t) test_rt.mm_malloc_global( 200 * sizeof( UpDown::word_t ) );
  a_ptr7 =
    (UpDown::ptr_t) test_rt.mm_malloc_global( 300 * sizeof( UpDown::word_t ) );
  a_ptr8 =
    (UpDown::ptr_t) test_rt.mm_malloc_global( 400 * sizeof( UpDown::word_t ) );

  /*
  test_rt->mm_free(a_ptr2);
  test_rt->mm_free(a_ptr3);
  a_ptr5 = (UpDown::ptr_t)test_rt->mm_malloc(50*sizeof(UpDown::word_t));
  printf("a_ptr5 = 0x%lX\n", reinterpret_cast<uint64_t>(a_ptr5));
  test_rt->mm_free(a_ptr4);
  test_rt->mm_free(a_ptr);
  test_rt->mm_free(a_ptr5);
  test_rt->mm_free_global(a_ptr6);
  test_rt->mm_free_global(a_ptr7);
  test_rt->mm_free_global(a_ptr8);*/

  return 0;
}
