#include <atomic>
#define assert(x) if (!(x)) { asm(".byte 0x00"); asm(".byte 0x00"); asm(".byte 0x00"); asm(".byte 0x00"); }

int main() {
        std::atomic<int> a;
        a = 1;  //amoswap
        assert(a == 1);
        a++;    //amoadd
        assert(a == 2);
}
