global _start

extern aes128_key_expand
extern aes128_encrypt

section .data
    key db "1234567890ABCDEF"      ; 16 bytes
    block db "HELLO_AES_DEMO!!"    ; 16 bytes

    roundkeys times 176 db 0

section .text
_start:
    ; key expansion
    mov rdi, key
    mov rsi, roundkeys
    call aes128_key_expand

    ; encrypt block
    mov rdi, block
    mov rsi, roundkeys
    call aes128_encrypt

    ; вывести результат
    mov rax, 1
    mov rdi, 1
    mov rsi, block
    mov rdx, 16
    syscall

    ; exit
    mov rax, 60
    xor rdi, rdi
    syscall
