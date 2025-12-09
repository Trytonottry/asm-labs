// aes128_aesni.c
// AES-NI accelerated AES-128 (key expansion + encrypt block).
// Compile with -maes -msse4.1

#include <stdint.h>
#include <wmmintrin.h>
#include <emmintrin.h>
#include <string.h>

// Expand 128-bit key into 11 __m128i round keys
void aes128_key_expansion_aesni(const uint8_t *userKey, __m128i roundKeys[11]) {
    roundKeys[0] = _mm_loadu_si128((const __m128i*)userKey);
    __m128i temp = roundKeys[0];
    __m128i temp2;

    #define EXPAND_ASSIST(x, i, rcon) \
        temp2 = _mm_aeskeygenassist_si128(temp, rcon); \
        temp2 = _mm_shuffle_epi32(temp2, _MM_SHUFFLE(3,3,3,3)); \
        temp = _mm_xor_si128(temp, _mm_slli_si128(temp, 4)); \
        temp = _mm_xor_si128(temp, _mm_slli_si128(temp, 4)); \
        temp = _mm_xor_si128(temp, _mm_slli_si128(temp, 4)); \
        roundKeys[i] = _mm_xor_si128(temp, temp2);

    EXPAND_ASSIST(roundKeys, 1, 0x01)
    temp = roundKeys[1];
    EXPAND_ASSIST(roundKeys, 2, 0x02)
    temp = roundKeys[2];
    EXPAND_ASSIST(roundKeys, 3, 0x04)
    temp = roundKeys[3];
    EXPAND_ASSIST(roundKeys, 4, 0x08)
    temp = roundKeys[4];
    EXPAND_ASSIST(roundKeys, 5, 0x10)
    temp = roundKeys[5];
    EXPAND_ASSIST(roundKeys, 6, 0x20)
    temp = roundKeys[6];
    EXPAND_ASSIST(roundKeys, 7, 0x40)
    temp = roundKeys[7];
    EXPAND_ASSIST(roundKeys, 8, 0x80)
    temp = roundKeys[8];
    EXPAND_ASSIST(roundKeys, 9, 0x1B)
    temp = roundKeys[9];
    EXPAND_ASSIST(roundKeys,10, 0x36)
    #undef EXPAND_ASSIST
}

// Encrypt single block with precomputed roundKeys[11]
void aes128_encrypt_block_aesni(const uint8_t in[16], uint8_t out[16], const __m128i roundKeys[11]) {
    __m128i m = _mm_loadu_si128((const __m128i*)in);
    m = _mm_xor_si128(m, roundKeys[0]);
    for(int r=1; r<10; ++r){
        m = _mm_aesenc_si128(m, roundKeys[r]);
    }
    m = _mm_aesenclast_si128(m, roundKeys[10]);
    _mm_storeu_si128((__m128i*)out, m);
}
