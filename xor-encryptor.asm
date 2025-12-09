section .data
    text db "Hello, world!", 0
    key  db 0x55

section .bss
    enc_buffer resb 64

section .text
global _start

_start:
    ; esi = адрес исходной строки
    mov esi, text
    mov edi, enc_buffer
    mov ecx, 0               ; счетчик

encrypt_loop:
    mov al, [esi + ecx]      ; читаем байт
    cmp al, 0                ; конец строки?
    je done_encrypt
    xor al, [key]            ; шифруем XOR'ом
    mov [edi + ecx], al      ; пишем результат
    inc ecx
    jmp encrypt_loop

done_encrypt:
    ; Вывод зашифрованного текста
    mov eax, 4               ; sys_write
    mov ebx, 1               ; stdout
    mov ecx, enc_buffer
    mov edx, ecx
.count:
    cmp byte [edx], 0
    je .write
    inc edx
    jmp .count
.write:
    sub edx, ecx
    int 0x80

    ; exit(0)
    mov eax, 1
    xor ebx, ebx
    int 0x80
