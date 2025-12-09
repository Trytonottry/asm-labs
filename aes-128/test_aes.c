// test_aes.c
// Build either portable or AES-NI variants. If building AES-NI, compile with -DUSE_AESNI -maes -msse4.1

#include <stdio.h>
#include <stdint.h>
#include <string.h>

void aes128_key_expansion(const uint8_t key[16], uint8_t round_keys[176]);
void aes128_encrypt_block_portable(const uint8_t in[16], uint8_t out[16], const uint8_t round_keys[176]);

#ifdef USE_AESNI
#include <wmmintrin.h>
void aes128_key_expansion_aesni(const uint8_t *userKey, __m128i roundKeys[11]);
void aes128_encrypt_block_aesni(const uint8_t in[16], uint8_t out[16], const __m128i roundKeys[11]);
#endif

static void print_hex(const uint8_t *b, size_t n){
    for(size_t i=0;i<n;i++) printf("%02x", b[i]);
    printf("\n");
}

int main(void){
    const uint8_t key[16] = {
        0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
        0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f
    };
    const uint8_t pt[16] = {
        0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,
        0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff
    };
    const uint8_t expected[16] = {
        0x69,0xc4,0xe0,0xd8,0x6a,0x7b,0x04,0x30,
        0xd8,0xcd,0xb7,0x80,0x70,0xb4,0xc5,0x5a
    };

    printf("Plaintext: ");
    print_hex(pt,16);
    printf("Key:       ");
    print_hex(key,16);

    // portable
    uint8_t round_keys[176];
    aes128_key_expansion(key, round_keys);
    uint8_t ct1[16];
    aes128_encrypt_block_portable(pt, ct1, round_keys);
    printf("Portable AES-128: ");
    print_hex(ct1,16);
    printf("Expected:         ");
    print_hex(expected,16);
    printf("Result: %s\n", memcmp(ct1, expected, 16) == 0 ? "OK" : "FAIL");

#ifdef USE_AESNI
    // AES-NI
    __m128i rkni[11];
    aes128_key_expansion_aesni(key, rkni);
    uint8_t ct2[16];
    aes128_encrypt_block_aesni(pt, ct2, rkni);
    printf("AES-NI AES-128:   ");
    print_hex(ct2,16);
    printf("Result(AES-NI): %s\n", memcmp(ct2, expected, 16) == 0 ? "OK" : "FAIL");
#endif

    return 0;
}
