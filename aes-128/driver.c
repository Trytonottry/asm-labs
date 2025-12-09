// driver.c
// gcc -O2 -maes -o aes_demo driver.c aesni_round.s
#include <stdint.h>
#include <stdio.h>
#include <string.h>

extern void aes_round_encrypt(uint8_t *state, uint8_t *roundkey);

int main(){
    uint8_t state[16] = { /* 16 bytes plaintext */ 0 };
    uint8_t roundkey[16] = { /* 16 bytes key round */ 0 };
    // Load test vectors or set sample
    for(int i=0;i<16;i++){ state[i]=i; roundkey[i]=0x0f; }
    aes_round_encrypt(state, roundkey);
    for(int i=0;i<16;i++) printf("%02x", state[i]);
    printf("\n");
    return 0;
}
