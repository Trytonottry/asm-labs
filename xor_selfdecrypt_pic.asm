; xor_selfdecrypt_pic.asm
; nasm -f elf64 xor_selfdecrypt_pic.asm -o xor.o
; ld xor.o -o xor

section .data
    key db 0x5A                ; ключ — для демо (лучше хранить в register)
section .rodata
    encrypted: db 0x3b,0x3f,0x22,0x32,0x20,0x2f,0x1f,0x0a ; пример: "Secret\n" ^ 0x5A
    enc_len:  equ $ - encrypted

section .text
global _start
_start:
    ; RIP-relative access (PIC)
    lea rsi, [rel encrypted]   ; rsi -> данные
    mov rcx, enc_len           ; длина
    mov bl, byte [rel key]     ; загрузили ключ

.decrypt_loop:
    test rcx, rcx
    jz .done
    mov al, [rsi]
    xor al, bl
    mov [rsi], al
    inc rsi
    dec rcx
    jmp .decrypt_loop

.done:
    ; write(1, encrypted, enc_len)
    mov rax, 1
    mov rdi, 1
    lea rsi, [rel encrypted]
    mov rdx, enc_len
    syscall

    ; scrub key in memory/reg
    xor ebx, ebx
    mov byte [rel key], 0

    ; exit(0)
    mov rax, 60
    xor rdi, rdi
    syscall
