// aarch64_xor_self.S
// as -o aarch.o aarch64_xor_self.S && ld -o aarch aarch.o
    .text
    .global _start
    .section .rodata
encrypted:
    .byte 0x1f,0x3b,0x2c,0x00
enc_len = . - encrypted

    .section .data
key:
    .byte 0x5A

    .text
_start:
    adr x0, encrypted      // x0 -> data
    ldrb w1, [x0]          // not needed but example
    ldrb w2, =key
    ldrb w3, [w2]          // key into w3 (low 8 bits)
    mov x1, enc_len
loop:
    cbz x1, done
    ldrb w4, [x0]
    eor w4, w4, w3
    strb w4, [x0]
    add x0, x0, #1
    sub x1, x1, #1
    b loop
done:
    // syscall write(1, encrypted, enc_len)
    mov x8, #64
    mov x0, #1
    adr x1, encrypted
    mov x2, enc_len
    svc #0
    // exit
    mov x8, #93
    mov x0, #0
    svc #0
