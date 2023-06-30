#define assert(x)                                                              \
  if (!(x)) {                                                                  \
    asm(".byte 0x00");                                                         \
    asm(".byte 0x00");                                                         \
    asm(".byte 0x00");                                                         \
    asm(".byte 0x00");                                                         \
  }

int main() {
  // test for infinite loops
  for (int i=0;i<2;i++){
    int zz = (i+1)/i;
  }

  // test the corner cases
  int _zero = 0x00l;
  int divisor = 7;
  int zzx = divisor / _zero;
  assert(zzx == 0xFFFFFFFFFFFFFFFF);

  return 0;
}
