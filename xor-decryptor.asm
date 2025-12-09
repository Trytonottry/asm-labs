section .text
global _start

_start:
    ; ESI = адрес зашифрованных данных
    ; ECX = длина
    ; DL  = ключ XOR

    mov esi, encrypted_data
    mov ecx, enc_len
    mov dl, 0x55              ; тот же ключ

decode:
    mov al, [esi]
    xor al, dl
    mov [esi], al
    inc esi
    loop decode

    ; После этого ESI указывает на расшифрованные данные
    ; Далее можно вызвать их как код или использовать как строку.

    ; Вывод результата
    mov eax, 4
    mov ebx, 1
    mov ecx, encrypted_data
    mov edx, enc_len
    int 0x80

    ; exit(0)
    mov eax, 1
    xor ebx, ebx
    int 0x80

section .data
    encrypted_data db 0x19,0x30,0x39,0x39,0x32,0x71,0x69,0x39,0x20,0x39,0x30,0x21 ; "Hello, world!" ^ 0x55
    enc_len equ $ - encrypted_data
