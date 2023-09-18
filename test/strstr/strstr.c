#include <stdio.h>
#include <string.h>
#define assert(x)                                                              \
  if (!(x)) {                                                                  \
    asm(".byte 0x00");                                                         \
    asm(".byte 0x00");                                                         \
    asm(".byte 0x00");                                                         \
    asm(".byte 0x00");                                                         \
  }

const char * const text = "Hello I am a normal text with some pattern hidden inside.";

int main() {
        assert(strstr(text, "pattern") == text + 35);
        return 0;
}
