#define assert(x) do if (!(x)) { asm(".dword 0x00000000"); } while(0)

int main() {
        volatile double da = 0.199;
        volatile double db = 0.25;

        assert(da < db);
        assert(da <= db);
        assert(db > da);
        assert(db >= da);
        da = 0.25;
        assert(da == db);
}
