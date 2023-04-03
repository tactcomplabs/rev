#include "unistd.h"

int main(){
  pid_t pid = getpid();
  if( pid<0){
    return 1;
  }
  return 0;
}
