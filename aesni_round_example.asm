; aesni_round_example.asm  (фрагмент, учебный)
; Требуется nasm, линковка с C/обёрткой или ручная подготовка стека
; Здесь — просто демонстрация инструкции aesenc (SSE registers: xmm)

section .text
global do_one_round    ; экспортируемую функцию можно вызывать из C

; void do_one_round(uint8_t *state, uint8_t *roundkey);
do_one_round:
    ; Linux x86_64 calling convention:
    ; rdi -> state(16 bytes), rsi -> roundkey(16 bytes)
    movdqu xmm0, [rdi]        ; загрузить state в xmm0
    movdqu xmm1, [rsi]        ; загрузить roundkey в xmm1
    aesenc xmm0, xmm1         ; применить один раунд AES
    movdqu [rdi], xmm0        ; сохранить результат обратно
    ret
