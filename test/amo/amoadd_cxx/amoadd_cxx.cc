#include <atomic>
#define assert(x) if (!(x)) { asm(".byte 0x00"); asm(".byte 0x00"); asm(".byte 0x00"); asm(".byte 0x00"); }
#include "../../include/rev-macros.h"

int main() {
        TRACE_ON;
        std::atomic<int> a;
//        a = 1;  //amoswap
//        assert(a == 1);
//        a++;    //amoadd
//        assert(a == 2);

        std::atomic<unsigned long long> b;
        b = 0xDEADBEEF;  //amoswap
        assert(b == 0xDEADBEEF);
        b++;    //amoadd
        assert(b == 0xDEADBEF0);
        TRACE_OFF;
}
