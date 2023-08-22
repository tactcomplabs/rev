#define assert(x) if (!(x)) { asm(".byte 0x00"); asm(".byte 0x00"); asm(".byte 0x00"); asm(".byte 0x00"); }
int main(){
        int d = -20;
        int x = -3;
        int div = d/x;
        assert(div == 6);
        return 0;
}
