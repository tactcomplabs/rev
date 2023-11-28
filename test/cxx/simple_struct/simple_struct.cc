#define assert(x) if (!(x)) { asm(".byte 0x00"); asm(".byte 0x00"); asm(".byte 0x00"); asm(".byte 0x00"); }

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
