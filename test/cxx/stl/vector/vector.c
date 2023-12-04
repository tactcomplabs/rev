#include "revalloc.hpp"
#include <vector>
#include "rev-macros.h"

#define assert(x) if (!(x)) { asm(".byte 0x00"); asm(".byte 0x00"); asm(".byte 0x00"); asm(".byte 0x00"); }

int main() {
  std::vector<int, Allocator<int> > v;
  v.push_back(0xbeef);
  int a = v.back();
  assert(a == 0xbeef);
  assert(v[0] == 0xbeef);
  v.push_back(0xdead);
  a = v.back();
  assert(a == 0xdead);
  assert(v[0] == 0xbeef);
  assert(v[1] == 0xdead);
  assert(v.size() == 2);
  v.pop_back();
  assert(v.size() == 1);
  assert(v[0] == 0xbeef);
  v.clear();
  for(int i = 0; i < 100; i++){
    v.push_back(i);
  }
  assert(v.size() == 100);
  for(int i = 0; i < 100; i++){
    assert(v[i] == i);
  }
  return 0;
}
