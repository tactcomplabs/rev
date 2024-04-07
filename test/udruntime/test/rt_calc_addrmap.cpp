#include "updown.h"
#include <iostream>

int main() {
  // Default configurations runtime
  UpDown::UDRuntime_t< UpDown::RevRTAllocPolicy< UpDown::ud_mapped_memory_t > >
                       test_rt = UpDown::UDRuntime_t< UpDown::RevRTAllocPolicy<
      UpDown::ud_mapped_memory_t > >();  // = new UpDown::UDRuntime_t();

  //  test_rt->dumpBaseAddrs();
  //  test_rt->dumpMachineConfig();

  //  delete test_rt;

  UpDown::ud_machine_t machine;

  machine.NumUDs   = 64;
  machine.NumLanes = 64;

  test_rt          = UpDown::UDRuntime_t<
    UpDown::RevRTAllocPolicy< UpDown::ud_mapped_memory_t > >(
    machine );  // new UpDown::UDRuntime_t(machine);

  //  test_rt.dumpBaseAddrs();
  // test_rt.dumpMachineConfig();


  return 0;
}
