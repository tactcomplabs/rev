#define assert(x)                                                              \
  do                                                                           \
    if (!(x)) {                                                                \
      asm(".dword 0x00000000");                                                \
    }                                                                          \
  while (0)

int main() {
  int d = -20;
  int x = -3;
  int div = d / x;
  assert(div == 6);
  return 0;
}
