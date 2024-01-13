#include "../../common/syscalls/syscalls.h"

#include <stdlib.h>

int main(int argc, char **argv) {
  const char msg[14] = "Hello World!\n";

  // Get my network ID
  uint64_t myid = rev_get_logical_network_id();

  if (myid == 0) {
    rev_send_network_msg(1, &msg[0], sizeof(msg));
  } else {
    rev_send_network_msg(0, &msg[0], sizeof(msg));
  }
  return 0;
}
