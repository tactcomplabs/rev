#include <stdio.h>
#include <string.h>

#define assert(x)                                                              \
  do                                                                           \
    if (!(x)) {                                                                \
      asm(".dword 0x00000000");                                                \
    }                                                                          \
  while (0)

const char *const text =
    "Hello I am a normal text with some pattern hidden inside.";

int main() {
  assert(strstr(text, "pattern") == text + 35);
  return 0;
}
