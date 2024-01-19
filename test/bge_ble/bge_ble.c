int main() {
        asm("li a2, 2; li a3, 2; bge a2, a3, 0x8; .byte 0x00; .byte 0x00; .byte 0x00; .byte 0x00;");
        asm("li a2, 2; li a3, 2; ble a2, a3, 0x8; .byte 0x00; .byte 0x00; .byte 0x00; .byte 0x00;");

        return 0;
}
