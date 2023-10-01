#include <stdio.h>
#include <string.h>
#define u16 unsigned short
#define u8 unsigned char
#define assert(x) if (!(x)) { asm(".byte 0x00"); asm(".byte 0x00"); asm(".byte 0x00"); asm(".byte 0x00"); }

/*
 * Write 0 to a memory, hammer down ones around it,
 * check that it is still 0 at the end.
 */

#define hammer(type) int hammer_##type(type *addr, size_t size) { \
        int ret = 0; \
        type ff = (type)0xffffffffffff; \
        for (size_t i = 2; i < size - 2; i++) { \
                addr[i] = 0; \
                for (size_t j = 0; j < 10; j++) { \
                        addr[i - 1] = ff; \
                        addr[i - 2] = ff; \
                        addr[i + 1] = ff; \
                        addr[i + 2] = ff; \
                } \
                ret |= (addr[i] != 0);  \
        } \
        return ret; \
}

hammer(u16)

int test_3(void *addr, size_t size) {
        int ret = 0;

        ret |= hammer_u16(addr, size / 2);
        return ret;
}

/* Memory to test */
#define SIZE 1000
u8 mem[SIZE];

int main(){
        assert(!test_3(mem, SIZE));
}
