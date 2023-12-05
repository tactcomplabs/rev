#include "syscalls.h"
#include <memory>
#define assert(x)                                                              \
  do                                                                           \
    if (!(x)) {                                                                \
      asm(".dword 0x00000000");                                                \
    }                                                                          \
  while (0)


//Fine to overload new at global scope, could also be done per class
void* operator new(std::size_t t){
     void* p = reinterpret_cast<void*>(rev_mmap(0,
              t,
              PROT_READ | PROT_WRITE | PROT_EXEC,
              MAP_PRIVATE | MAP_ANONYMOUS,
              -1,
              0));
    return p;
}

//Not necessary to do this as a template, but this makes it easy to overload per class
  template<typename T>
  void revDel(void* p){
        std::size_t addr = reinterpret_cast<std::size_t>(p);
        rev_munmap(addr, sizeof(T));
  }


class rec {
  public:
  unsigned c;
  rec(unsigned a, unsigned b) {
    c = a + b;
  }

  //Must overload delete per class as we need to know how big the thing is we are freeing unless someone has
  // a better idea w/ some c++ magic?
  void operator delete(void* p){ revDel<rec>(p); }
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
