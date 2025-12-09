    .text
    .globl aes_round_encrypt
// void aes_round_encrypt(uint8_t *state, uint8_t *roundkey);
aes_round_encrypt:
    movdqu (%rdi), %xmm0
    movdqu (%rsi), %xmm1
    aesenc %xmm1, %xmm0
    movdqu %xmm0, (%rdi)
    ret
