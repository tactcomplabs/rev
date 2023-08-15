#include <stdlib.h>
#include <stdint.h>

uint64_t atom64 = 0;
int main() {
    __atomic_fetch_add(&atom64, 2, __ATOMIC_RELAXED);
    __atomic_fetch_add(&atom64, 2, __ATOMIC_CONSUME);
    __atomic_fetch_add(&atom64, 2, __ATOMIC_ACQUIRE);
    __atomic_fetch_add(&atom64, 2, __ATOMIC_RELEASE);
    __atomic_fetch_add(&atom64, 2, __ATOMIC_ACQ_REL);
    __atomic_fetch_add(&atom64, 2, __ATOMIC_SEQ_CST);

    return 0;
}
