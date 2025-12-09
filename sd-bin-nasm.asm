; build:
; nasm -f elf64 decrypt.asm -o decrypt.o
; ld decrypt.o -o decrypt

global _start

section .data
    key     db 0x55
    msg_enc db 0x11,0x10,0x19,0x19,0x1a,0x75,0x06,0x1e,0x18,0x00
    ; это "HELLO ASM!" XOR 0x55

section .text

_start:
    mov rsi, msg_enc     ; rsi -> encrypted message
    mov rcx, 10          ; длина строки
    mov al, [key]        ; al = XOR key

decrypt_loop:
    xor byte [rsi], al   ; дешифруем байт
    inc rsi
    loop decrypt_loop

    ; теперь msg_enc содержит нормальную строку "HELLO ASM!\0"

    ; пишем в stdout
    mov rax, 1           ; write
    mov rdi, 1           ; stdout
    mov rsi, msg_enc     ; pointer to string
    mov rdx, 10          ; length
    syscall

    ; exit
    mov rax, 60
    xor rdi, rdi
    syscall
