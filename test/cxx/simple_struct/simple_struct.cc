#define assert(x)                                                              \
  do                                                                           \
    if (!(x)) {                                                                \
      asm(".dword 0x00000000");                                                \
    }                                                                          \
  while (0)

struct rec {
  unsigned c;
  rec(unsigned a, unsigned b) {
    c = a + b;
  }
};

int main() {
  rec testrec(1,2);
  assert(testrec.c==3);

  #if 1
  rec* pRec = new rec(2,3);
  assert(pRec->c==5);
  delete pRec;
  #endif
}
