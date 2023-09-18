
#include "../../../common/syscalls/syscalls.h"
#include "unistd.h"

int main(){
  pid_t pid = rev_getpid();
  if( pid < 0){
    return 1;
  }
  return 0;
}
