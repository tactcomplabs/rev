#define assert(x)                                                              \
  do                                                                           \
    if (!(x)) {                                                                \
      asm(".word 0x00");                                                       \
    }                                                                          \
  while (0)

#define FMA_TEST(T, INST)                                                      \
  do {                                                                         \
    volatile T ad = 2.0;                                                       \
    volatile T bd = 3.0;                                                       \
    volatile T cd = 5.0;                                                       \
    volatile T rd;                                                             \
    asm volatile(INST " %0, %1, %2, %3"                                        \
                 : "=f"(rd)                                                    \
                 : "f"(ad), "f"(bd), "f"(cd));                                 \
    assert(rd >= 10.99);                                                       \
    assert(rd <= 11.01);                                                       \
  } while (0)

int main() {
  FMA_TEST(float, "fmadd.s");
  FMA_TEST(double, "fmadd.d");
  return 0;
}
