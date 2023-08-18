#include <atomic>
#define assert(x) if (!(x)) { asm(".byte 0x00"); asm(".byte 0x00"); asm(".byte 0x00"); asm(".byte 0x00"); }

int main() {
        std::atomic<int> a;
        a = 0;
        a++;
        assert(a == 1);
}
