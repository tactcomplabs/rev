#include "../../common/syscalls/syscalls.h"

#include <stdlib.h>

// used to derive logical id of other core
long cantor_pairing(long x, long y) {
  return (0.5 * (x + y) * (x + y + 1)) + y;
}

int main(int argc, char **argv) {
  const char msg[14] = "Hello World!\n";

  // Get my network ID
  uint64_t my_cpu_id = rev_get_cpu_id();
  uint64_t myid = rev_get_logical_noc_id();
  uint64_t other_noc_id = 0;

  // Find logical id of other core
  if (my_cpu_id == 0) {
    // NOTE: the reason we add 1 is because when we assign the noc id
    // using cantor pairing (0, 0) will give use 0.5 which is not a valid noc
    // so if you check RevCPU where we enable the NOC and SetLogicalID, we add 1
    // to the id
    other_noc_id = cantor_pairing(my_cpu_id, 0);
  } else if (my_cpu_id == 1) {
    other_noc_id = cantor_pairing(my_cpu_id, 0);
  }
  rev_send_noc_msg(other_noc_id, &msg[0], sizeof(msg));

  return 0;
}
