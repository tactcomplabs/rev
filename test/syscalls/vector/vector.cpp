#include <vector>

#define assert(x)                                                              \
  if (!(x)) {                                                                  \
    asm(".byte 0x00");                                                         \
    asm(".byte 0x00");                                                         \
    asm(".byte 0x00");                                                         \
    asm(".byte 0x00");                                                         \
  }

int main() {
  std::vector<int> v;
  for(int i = 0; i < 1000; i++){
    v.push_back(i);
  }
  for(int i = 0; i < 1000; i++){
    assert(v[i] == i);
  }
  return 0;
}
