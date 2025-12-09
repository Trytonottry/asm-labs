; xor_selfdecrypt_x86_64.asm
; nasm -f elf64 xor_selfdecrypt_x86_64.asm && ld -o xor_self xor_selfdecrypt_x86_64.o

section .text
global _start

_start:
    ; Получим текущий адрес (RIP-relative)
    lea rsi, [rel encrypted]    ; rsi -> зашифрованные данные
    mov rcx, enc_len            ; длина
    mov bl, 0x5A                ; ключ (пример). В реальном коде — не хранить в .data

decrypt_loop:
    test rcx, rcx
    jz done
    mov al, [rsi]
    xor al, bl
    mov [rsi], al
    inc rsi
    dec rcx
    jmp decrypt_loop

done:
    ; write(1, encrypted, enc_len)
    mov rax, 1
    mov rdi, 1
    lea rsi, [rel encrypted]
    mov rdx, enc_len
    syscall

    ; scrub key (байтово)
    xor ebx, ebx
    mov bl, 0

    ; exit(0)
    mov rax, 60
    xor rdi, rdi
    syscall

section .data
encrypted: db  'U','\x0F','\x1A',0x30,0x7C,0x2B,0x00  ; пример: "Secret\n" ^ 0x5A
enc_len  equ $ - encrypted
