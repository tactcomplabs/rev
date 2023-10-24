#include <string.h>
#define assert(x) if (!(x)) { asm(".byte 0x00"); asm(".byte 0x00"); asm(".byte 0x00"); asm(".byte 0x00"); }
#define N (1024)
char mem[N];

int main() {
  memset(mem, 42, N);
  for(int i = 0; i < N; i++){
    assert(mem[i] == 42);
  }
  memset(mem, 0, N);
  for(int i = 0; i < N; i++){
    assert(mem[i] == 0);
  }
  return 0;
}
