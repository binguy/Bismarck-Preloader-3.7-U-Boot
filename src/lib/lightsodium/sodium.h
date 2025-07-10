#ifndef SODIUM_H
#define SODIUM_H

#ifdef SODIUM_UTIL
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#define COMPILER_ASSERT(expr) assert(expr)
extern void dbg_print(unsigned char *p, unsigned int size, const char *c);
#define DBGPRINT(p)    dbg_print((unsigned char*)p, sizeof(p), ""#p)
#define CONFIG_ENABLE_ENCRYPT
#define NATIVE_LITTLE_ENDIAN

#if 0
typedef struct {
    // make sure the above model is correct
    unsigned int            signature;
    // version = [31:24]: major version, [23:8]: regular rlz version, [7:0]: reserved
    unsigned int            version;
    // some pointer here (soc.h)
    unsigned int            four_pointer_here[4];

} soc_stru_header_t;

typedef struct {
    // --------------
    //      This structure is used to Communication between lpreloader and preloader only, not intent to change.
    //      expection: the export_func_list may be used by u-boot.
    // --------------
    soc_stru_header_t       header;

    // The irq entry point, if necessary
    // Saving context and preparing stack should be done in this isr
    // CAUTION: the position from the beginning should be exactly 4x6 = __24__ bytes (or 0x38 from 9fc0_0000)
    unsigned int            one_pointer_here;

    unsigned int            size_of_plr_load_firstly;   // used only for lplr+plr and plr tells lplr how many bytes to loader firstly. (in bytes)

    unsigned int            seven_pointer_here[7];
} basic_io_t;
#define SIGNATURE_PLR_FL    0x27317223
#endif

#else
#include <soc.h>
#define COMPILER_ASSERT(expr) ({ if (!(expr)) while(1); })
#define NULL    ((void *)0)
#define NATIVE_BIG_ENDIAN
typedef long long int int64_t;
typedef int int32_t;
typedef short int16_t;
typedef char int8_t;
typedef unsigned int * uintptr_t;
typedef unsigned long long uint64_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
#ifdef __SIZE_TYPE__
typedef __SIZE_TYPE__ size_t;
#else
typedef unsigned long long size_t;
#endif
#endif

#define inline_memcpy(d, s, l) ({char *__d=(char*)d;const char *__s=(const char *)s;unsigned __l=l;while (__l-- >0) *(__d++)=*(__s++);})
#define inline_memset(d, v, l) ({char *__d=(char*)d; char __v=(char)v; unsigned __l=l; while (__l-- >0) *(__d++)=__v;})

#ifndef bzero
#define inline_bzero(p, n) ({char *q=(char*)p; unsigned m=n; while (m-- >0) *(q++)=0;})
#define bzero   inline_bzero
#endif
/* #define memset  inline_memset
#define memcpy  inline_memcpy*/


#define ADDITIONAL_DATA     (const unsigned char *)"SODIUM"
#define ADDITIONAL_DATA_LEN 6//(sizeof(ADDITIONAL_DATA))

#define sodium_misuse(...)

#define crypto_SIZE_MAX                                         (0x80000)
#define crypto_box_NONCEBYTES                                   24U
#define crypto_box_MACBYTES                                     16U
#define crypto_box_PUBLICKEYBYTES                               32U
#define crypto_box_SECRETKEYBYTES                               32U
#define crypto_box_SEALBYTES                                    (crypto_box_PUBLICKEYBYTES + crypto_box_MACBYTES)
#define crypto_box_BEFORENMBYTES                                32U
#define crypto_secretbox_KEYBYTES                               32U
#define crypto_secretbox_ZEROBYTES                              (16U + 16U)
#define crypto_aead_chacha20poly1305_KEYBYTES                   32U
#define crypto_aead_chacha20poly1305_NPUBBYTES                  8U
#define crypto_aead_chacha20poly1305_ABYTES                     16U
#define crypto_aead_chacha20poly1305_MESSAGEBYTES_MAX           (crypto_SIZE_MAX - crypto_aead_chacha20poly1305_ABYTES)
#define crypto_sign_BYTES                                       64U
#define crypto_sign_PUBLICKEYBYTES                              32U
#define crypto_sign_SECRETKEYBYTES                              (32U + 32U)
#define crypto_hash_sha512_BYTES                                64U
#define crypto_scalarmult_curve25519_BYTES                      32U
#define crypto_stream_salsa20_KEYBYTES                          32U
#define crypto_stream_chacha20_KEYBYTES                         32U
#define crypto_stream_chacha20_MESSAGEBYTES_MAX                 crypto_SIZE_MAX
#define crypto_box_curve25519xsalsa20poly1305_MACBYTES          16U
#define crypto_box_curve25519xsalsa20poly1305_MESSAGEBYTES_MAX  (crypto_SIZE_MAX - crypto_box_curve25519xsalsa20poly1305_MACBYTES)
#define crypto_box_MESSAGEBYTES_MAX                             crypto_box_curve25519xsalsa20poly1305_MESSAGEBYTES_MAX
#define crypto_onetimeauth_poly1305_KEYBYTES                    32U

#define ENC_KEY_BYTES   (crypto_box_SEALBYTES + crypto_aead_chacha20poly1305_KEYBYTES)
#define SIGN_BYTES      (crypto_sign_BYTES)
#define PUK_KEY_BYTES   (crypto_sign_PUBLICKEYBYTES)
#define SEK_KEY_BYTES   (crypto_sign_SECRETKEYBYTES-crypto_sign_PUBLICKEYBYTES)
#define NONCEBYTES      24U

#define BLK_SIZE        (4096)
#define CIPHER_BLK_SIZE (BLK_SIZE + crypto_aead_chacha20poly1305_ABYTES)

#define CRYPTO_ALIGN(x) __attribute__ ((aligned(x)))
#define CRYPTO_MIPS16   __attribute__((mips16))

typedef int32_t fe25519[10];

typedef struct {
    fe25519 X;
    fe25519 Y;
    fe25519 Z;
} ge25519_p2;

typedef struct {
    fe25519 X;
    fe25519 Y;
    fe25519 Z;
    fe25519 T;
} ge25519_p3;

typedef struct {
    fe25519 X;
    fe25519 Y;
    fe25519 Z;
    fe25519 T;
} ge25519_p1p1;

typedef struct {
    fe25519 yplusx;
    fe25519 yminusx;
    fe25519 xy2d;
} ge25519_precomp;

typedef struct {
    fe25519 YplusX;
    fe25519 YminusX;
    fe25519 Z;
    fe25519 T2d;
} ge25519_cached;

typedef struct crypto_sign_state {
    uint64_t state[8];
    uint64_t count[2];
    uint8_t  buf[128];
} crypto_sign_state;

typedef struct CRYPTO_ALIGN(64) crypto_generichash_state {
    uint64_t h[8];
    uint64_t t[2];
    uint64_t f[2];
    uint8_t  buf[2 * 128];
    size_t   buflen;
    uint8_t  last_node;
} crypto_generichash_state;

typedef struct CRYPTO_ALIGN(16) crypto_onetimeauth_poly1305_state {
    unsigned char opaque[256];
} crypto_onetimeauth_poly1305_state;

typedef struct chacha_ctx {
    uint32_t input[16];
}chacha_ctx;

#ifdef AUTH_UBT_SIGN_ONLY
typedef struct CRYPTO_ALIGN(32) crypto_header {
    uint8_t signu[SIGN_BYTES];  // u-boot
    uint8_t sign_pk[crypto_sign_PUBLICKEYBYTES];
    uint8_t type;
    uint8_t dev;
} CRYPTO_HEADER_T;
#else
typedef struct CRYPTO_ALIGN(4) crypto_header {
    uint8_t nonce[NONCEBYTES];
    uint8_t key_in_box[ENC_KEY_BYTES];
    uint8_t signf[SIGN_BYTES];  // first load
    uint8_t signp[SIGN_BYTES];  // preloader
    uint8_t signu[SIGN_BYTES];  // u-boot
    uint8_t sign_pk[crypto_sign_PUBLICKEYBYTES];
    uint8_t recpnt_sk[crypto_box_SECRETKEYBYTES];
    uint8_t type;
    uint8_t dev;
} CRYPTO_HEADER_T;
#endif

enum
{
    CRYPTO_NOR_DEVICE,
    CRYPTO_NAND_DEVICE,
};

enum
{
    CRYOPT_NONE,     // do nothing
    CRYOPT_SIG,      // signature only
    CRYOPT_SIG_ENC,  // signature with encryped
    CRYOPT_VERIFY,   // verify
};

int memcmp(const void * cs,const void * ct,size_t count);
void * memmove(void * dest,const void *src,size_t count);
void sodium_memzero(void *const pnt, const size_t len);
int sodium_is_zero(const unsigned char *n, const size_t nlen);
uint64_t load_3(const unsigned char *in);
uint64_t load_4(const unsigned char *in);
int randombytes (uint8_t *, uint64_t);

int crypto_verify_32(const unsigned char *x, const unsigned char *y);
int crypto_hash_sha512_init(crypto_sign_state *state);
int crypto_hash_sha512_update(crypto_sign_state *state, const unsigned char *m, unsigned long long mlen);
int crypto_hash_sha512_final(crypto_sign_state *state, unsigned char *out);
int crypto_hash_sha512(unsigned char *out, const unsigned char *in, unsigned long long inlen);
int crypto_sign_init(crypto_sign_state *state);
int crypto_sign_update(crypto_sign_state *state, const unsigned char *m, unsigned long long mlen);
int crypto_sign_final_create(crypto_sign_state *state, unsigned char *sig, unsigned long long *siglen_p, const unsigned char *sk);
int crypto_sign_final_verify(crypto_sign_state *state, unsigned char *sig, const unsigned char *pk);
int crypto_sign_ed25519_keypair(uint8_t *pk, uint8_t *sk);
int _crypto_sign_ed25519_detached(unsigned char *sig, unsigned long long *siglen_p, const unsigned char *m, unsigned long long mlen, const unsigned char *sk, int prehashed);
int _crypto_sign_ed25519_verify_detached(const unsigned char *sig, const unsigned char *m, unsigned long long   mlen, const unsigned char *pk, int prehashed);
int crypto_stream_salsa20_xor_ic(unsigned char *c, const unsigned char *m, unsigned long long mlen, const unsigned char *n, uint64_t ic, const unsigned char *k);
int crypto_stream_salsa20_xor(unsigned char *c, const unsigned char *m, unsigned long long mlen, const unsigned char *n, const unsigned char *k);
int crypto_box_seal(unsigned char *c, const unsigned char *m, unsigned long long mlen, const unsigned char *pk);
int crypto_box_seal_open(unsigned char *m, const unsigned char *c, unsigned long long clen, const unsigned char *pk, const unsigned char *sk);
int crypto_sign_ed25519_pk_to_curve25519(unsigned char *curve25519_pk, const unsigned char *ed25519_pk);
int crypto_sign_ed25519_sk_to_curve25519(unsigned char *curve25519_sk, const unsigned char *ed25519_sk);
void crypto_aead_chacha20poly1305_keygen(unsigned char k[crypto_aead_chacha20poly1305_KEYBYTES]);
int crypto_aead_chacha20poly1305_encrypt(unsigned char *c, unsigned long long *clen_p, const unsigned char *m, unsigned long long mlen, const unsigned char *ad,
                                         unsigned long long adlen, const unsigned char *nsec, const unsigned char *npub, const unsigned char *k);
int crypto_aead_chacha20poly1305_decrypt(unsigned char *m, unsigned long long *mlen_p, unsigned char *nsec, const unsigned char *c, unsigned long long clen,
                                         const unsigned char *ad, unsigned long long adlen, const unsigned char *npub, const unsigned char *k);
int crypto_verify_16(const unsigned char *x, const unsigned char *y);
int crypto_verify_32(const unsigned char *x, const unsigned char *y);
#define U32C(v) (v##U)
#define U32V(v) ((uint32_t)(v) &U32C(0xFFFFFFFF))

#define ROTATE(v, c) (ROTL32(v, c))
#define XOR(v, w) ((v) ^ (w))
#define PLUS(v, w) (U32V((v) + (w)))
#define PLUSONE(v) (PLUS((v), 1))

#define QUARTERROUND(a, b, c, d) \
    a = PLUS(a, b);              \
    d = ROTATE(XOR(d, a), 16);   \
    c = PLUS(c, d);              \
    b = ROTATE(XOR(b, c), 12);   \
    a = PLUS(a, b);              \
    d = ROTATE(XOR(d, a), 8);    \
    c = PLUS(c, d);              \
    b = ROTATE(XOR(b, c), 7);

#define ROTL32(X, B) rotl32((X), (B))
static inline uint32_t
rotl32(const uint32_t x, const int b)
{
    return (x << b) | (x >> (32 - b));
}

#define ROTR32(X, B) rotr32((X), (B))
static inline uint32_t
rotr32(const uint32_t x, const int b)
{
    return (x >> b) | (x << (32 - b));
}

#define ROTR64(X, B) rotr64((X), (B))
static inline uint64_t
rotr64(const uint64_t x, const int b)
{
    return (x >> b) | (x << (64 - b));
}

#define LOAD32_LE(SRC) load32_le(SRC)
static inline uint32_t
load32_le(const uint8_t src[4])
{
#ifdef NATIVE_LITTLE_ENDIAN
    uint32_t w;
    inline_memcpy(&w, src, sizeof w);
    return w;
#else // BIG ENDIAN
    uint32_t w = (uint32_t) src[0];
    w |= (uint32_t) src[1] <<  8;
    w |= (uint32_t) src[2] << 16;
    w |= (uint32_t) src[3] << 24;
    return w;
#endif
}


#define STORE32_LE(DST, W) store32_le((DST), (W))
static inline void
store32_le(uint8_t dst[4], uint32_t w)
{
#ifdef NATIVE_LITTLE_ENDIAN
    inline_memcpy(dst, &w, sizeof w);
#else // BIG ENDIAN
    dst[0] = (uint8_t) w; w >>= 8;
    dst[1] = (uint8_t) w; w >>= 8;
    dst[2] = (uint8_t) w; w >>= 8;
    dst[3] = (uint8_t) w;
#endif
}

#define LOAD32_BE(SRC) load32_be(SRC)
static inline uint32_t
load32_be(const uint8_t src[4])
{
#ifdef NATIVE_LITTLE_ENDIAN
    uint32_t w = (uint32_t) src[3];
    w |= (uint32_t) src[2] <<  8;
    w |= (uint32_t) src[1] << 16;
    w |= (uint32_t) src[0] << 24;
    return w;
#else
    uint32_t w;
    inline_memcpy(&w, src, sizeof w);
    return w;

#endif
}

#define STORE32_BE(DST, W) store32_be((DST), (W))
static inline void
store32_be(uint8_t dst[4], uint32_t w)
{
#ifdef NATIVE_LITTLE_ENDIAN
    dst[3] = (uint8_t) w; w >>= 8;
    dst[2] = (uint8_t) w; w >>= 8;
    dst[1] = (uint8_t) w; w >>= 8;
    dst[0] = (uint8_t) w;
#else
    inline_memcpy(dst, &w, sizeof w);
#endif
}
#define LOAD64_BE(SRC) load64_be(SRC)
static inline uint64_t
load64_be(const uint8_t src[8])
{
#ifdef NATIVE_LITTLE_ENDIAN
    uint64_t w = (uint64_t) src[7];
    w |= (uint64_t) src[6] <<  8;
    w |= (uint64_t) src[5] << 16;
    w |= (uint64_t) src[4] << 24;
    w |= (uint64_t) src[3] << 32;
    w |= (uint64_t) src[2] << 40;
    w |= (uint64_t) src[1] << 48;
    w |= (uint64_t) src[0] << 56;
    return w;
#else // BIG ENDIAN
    uint64_t w;
    inline_memcpy(&w, src, sizeof w);
    return w;
#endif
}

#define LOAD64_LE(SRC) load64_le(SRC)
static inline uint64_t
load64_le(const uint8_t src[8])
{
#ifdef NATIVE_LITTLE_ENDIAN
    uint64_t w;
    inline_memcpy(&w, src, sizeof w);
    return w;
#else // BIG ENDIAN
    uint64_t w = (uint64_t) src[0];
    w |= (uint64_t) src[1] <<  8;
    w |= (uint64_t) src[2] << 16;
    w |= (uint64_t) src[3] << 24;
    w |= (uint64_t) src[4] << 32;
    w |= (uint64_t) src[5] << 40;
    w |= (uint64_t) src[6] << 48;
    w |= (uint64_t) src[7] << 56;
    return w;
#endif
}

#define STORE64_BE(DST, W) store64_be((DST), (W))
static inline void
store64_be(uint8_t dst[8], uint64_t w)
{
#ifdef NATIVE_LITTLE_ENDIAN
    dst[7] = (uint8_t) w; w >>= 8;
    dst[6] = (uint8_t) w; w >>= 8;
    dst[5] = (uint8_t) w; w >>= 8;
    dst[4] = (uint8_t) w; w >>= 8;
    dst[3] = (uint8_t) w; w >>= 8;
    dst[2] = (uint8_t) w; w >>= 8;
    dst[1] = (uint8_t) w; w >>= 8;
    dst[0] = (uint8_t) w;
#else // BIG ENDIAN
    inline_memcpy(dst, &w, sizeof w);
#endif
}

#define STORE64_LE(DST, W) store64_le((DST), (W))
static inline void
store64_le(uint8_t dst[8], uint64_t w)
{
#ifdef NATIVE_LITTLE_ENDIAN
    inline_memcpy(dst, &w, sizeof w);
#else // BIG ENDIAN
    dst[0] = (uint8_t) w; w >>= 8;
    dst[1] = (uint8_t) w; w >>= 8;
    dst[2] = (uint8_t) w; w >>= 8;
    dst[3] = (uint8_t) w; w >>= 8;
    dst[4] = (uint8_t) w; w >>= 8;
    dst[5] = (uint8_t) w; w >>= 8;
    dst[6] = (uint8_t) w; w >>= 8;
    dst[7] = (uint8_t) w;
#endif
}
#endif //SODIUM_H
