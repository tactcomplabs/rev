#include <atomic>
#define assert(x) if (!(x)) { asm(".byte 0x00"); asm(".byte 0x00"); asm(".byte 0x00"); asm(".byte 0x00"); }

int main() {
        std::atomic<int> a;
        a = 1;  //amoswap
        assert(a == 1);
        a++;    //amoadd
        assert(a == 2);

        std::atomic<unsigned long long> b;
        b = 1;  //amoswap
        assert(b == 1);
        b++;    //amoadd
        assert(b == 2);
}
