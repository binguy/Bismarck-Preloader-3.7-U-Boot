#include <sodium.h>

#define offsetof(st, m) __builtin_offsetof(st, m)

int crypto_core_hsalsa20(unsigned char *out, const unsigned char *in, const unsigned char *k, const unsigned char *c);
int crypto_stream_salsa20(unsigned char *c, unsigned long long clen, const unsigned char *n, const unsigned char *k);
int crypto_onetimeauth_poly1305_init(crypto_onetimeauth_poly1305_state *state,
                                     const unsigned char *key);
int crypto_onetimeauth_poly1305_update(crypto_onetimeauth_poly1305_state *state,
                                       const unsigned char *in,
                                       unsigned long long inlen);
int crypto_onetimeauth_poly1305_final(crypto_onetimeauth_poly1305_state *state,
                                      unsigned char *out);

static const uint64_t blake2b_IV[8] = {
    0x6a09e667f3bcc908ULL, 0xbb67ae8584caa73bULL, 0x3c6ef372fe94f82bULL,
    0xa54ff53a5f1d36f1ULL, 0x510e527fade682d1ULL, 0x9b05688c2b3e6c1fULL,
    0x1f83d9abfb41bd6bULL, 0x5be0cd19137e2179ULL
};

static const uint8_t blake2b_sigma[12][16] = {
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
    { 14, 10, 4, 8, 9, 15, 13, 6, 1, 12, 0, 2, 11, 7, 5, 3 },
    { 11, 8, 12, 0, 5, 2, 15, 13, 10, 14, 3, 6, 7, 1, 9, 4 },
    { 7, 9, 3, 1, 13, 12, 11, 14, 2, 6, 5, 10, 4, 0, 15, 8 },
    { 9, 0, 5, 7, 2, 4, 10, 15, 14, 1, 11, 12, 6, 8, 3, 13 },
    { 2, 12, 6, 10, 0, 11, 8, 3, 4, 13, 7, 5, 15, 14, 1, 9 },
    { 12, 5, 1, 15, 14, 13, 4, 10, 0, 7, 6, 3, 9, 2, 8, 11 },
    { 13, 11, 7, 14, 12, 1, 3, 9, 5, 0, 15, 4, 8, 6, 2, 10 },
    { 6, 15, 14, 9, 11, 3, 0, 8, 12, 2, 13, 7, 1, 4, 10, 5 },
    { 10, 2, 8, 4, 7, 6, 1, 5, 15, 11, 9, 14, 3, 12, 13, 0 },
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
    { 14, 10, 4, 8, 9, 15, 13, 6, 1, 12, 0, 2, 11, 7, 5, 3 }
};

enum blake2b_constant {
    BLAKE2B_BLOCKBYTES    = 128,
    BLAKE2B_OUTBYTES      = 64,
    BLAKE2B_KEYBYTES      = 64,
    BLAKE2B_SALTBYTES     = 16,
    BLAKE2B_PERSONALBYTES = 16
};

typedef struct blake2b_param_ {
    uint8_t digest_length;                   /*  1 */
    uint8_t key_length;                      /*  2 */
    uint8_t fanout;                          /*  3 */
    uint8_t depth;                           /*  4 */
    uint8_t leaf_length[4];                  /*  8 */
    uint8_t node_offset[8];                  /* 16 */
    uint8_t node_depth;                      /* 17 */
    uint8_t inner_length;                    /* 18 */
    uint8_t reserved[14];                    /* 32 */
    uint8_t salt[BLAKE2B_SALTBYTES];         /* 48 */
    uint8_t personal[BLAKE2B_PERSONALBYTES]; /* 64 */
} blake2b_param;

#define crypto_generichash_blake2b_state  crypto_generichash_state
#define blake2b_state crypto_generichash_state

int
blake2b_compress(blake2b_state *S, const uint8_t block[BLAKE2B_BLOCKBYTES])
{
    uint64_t m[16];
    uint64_t v[16];
    int      i;

    for (i = 0; i < 16; ++i) {
        m[i] = LOAD64_LE(block + i * sizeof(m[i]));
    }
    for (i = 0; i < 8; ++i) {
        v[i] = S->h[i];
    }
    v[8]  = blake2b_IV[0];
    v[9]  = blake2b_IV[1];
    v[10] = blake2b_IV[2];
    v[11] = blake2b_IV[3];
    v[12] = S->t[0] ^ blake2b_IV[4];
    v[13] = S->t[1] ^ blake2b_IV[5];
    v[14] = S->f[0] ^ blake2b_IV[6];
    v[15] = S->f[1] ^ blake2b_IV[7];
#define G(r, i, a, b, c, d)                         \
    do {                                            \
        a = a + b + m[blake2b_sigma[r][2 * i + 0]]; \
        d = ROTR64(d ^ a, 32);                      \
        c = c + d;                                  \
        b = ROTR64(b ^ c, 24);                      \
        a = a + b + m[blake2b_sigma[r][2 * i + 1]]; \
        d = ROTR64(d ^ a, 16);                      \
        c = c + d;                                  \
        b = ROTR64(b ^ c, 63);                      \
    } while (0)
#define ROUND(r)                           \
    do {                                   \
        G(r, 0, v[0], v[4], v[8], v[12]);  \
        G(r, 1, v[1], v[5], v[9], v[13]);  \
        G(r, 2, v[2], v[6], v[10], v[14]); \
        G(r, 3, v[3], v[7], v[11], v[15]); \
        G(r, 4, v[0], v[5], v[10], v[15]); \
        G(r, 5, v[1], v[6], v[11], v[12]); \
        G(r, 6, v[2], v[7], v[8], v[13]);  \
        G(r, 7, v[3], v[4], v[9], v[14]);  \
    } while (0)
    ROUND(0);
    ROUND(1);
    ROUND(2);
    ROUND(3);
    ROUND(4);
    ROUND(5);
    ROUND(6);
    ROUND(7);
    ROUND(8);
    ROUND(9);
    ROUND(10);
    ROUND(11);

    for (i = 0; i < 8; ++i) {
        S->h[i] = S->h[i] ^ v[i] ^ v[i + 8];
    }

#undef G
#undef ROUND
    return 0;
}

/* LCOV_EXCL_START */
static inline int
blake2b_set_lastnode(blake2b_state *S)
{
    S->f[1] = -1;
    return 0;
}
/* LCOV_EXCL_STOP */

static inline int
blake2b_is_lastblock(const blake2b_state *S)
{
    return S->f[0] != 0;
}

static inline int
blake2b_set_lastblock(blake2b_state *S)
{
    if (S->last_node) {
        blake2b_set_lastnode(S);
    }
    S->f[0] = -1;
    return 0;
}


static inline int
blake2b_init0(blake2b_state *S)
{
    int i;

    for (i  = 0; i < 8; i++) {
        S->h[i] = blake2b_IV[i];
    }
    inline_memset(S->t, 0, offsetof(blake2b_state, last_node) + sizeof(S->last_node)
           - offsetof(blake2b_state, t));
    return 0;
}

static inline int
blake2b_increment_counter(blake2b_state *S, const uint64_t inc)
{
#ifdef HAVE_TI_MODE
    uint128_t t = ((uint128_t) S->t[1] << 64) | S->t[0];
    t += inc;
    S->t[0] = (uint64_t)(t >> 0);
    S->t[1] = (uint64_t)(t >> 64);
#else
    S->t[0] += inc;
    S->t[1] += (S->t[0] < inc);
#endif
    return 0;
}

/* inlen now in bytes */
int
blake2b_update(blake2b_state *S, const uint8_t *in, uint64_t inlen)
{
    while (inlen > 0) {
        size_t left = S->buflen;
        size_t fill = 2 * BLAKE2B_BLOCKBYTES - left;

        if (inlen > fill) {
            inline_memcpy(S->buf + left, in, fill); /* Fill buffer */
            S->buflen += fill;
            blake2b_increment_counter(S, BLAKE2B_BLOCKBYTES);
            blake2b_compress(S, S->buf); /* Compress */
            inline_memcpy(S->buf, S->buf + BLAKE2B_BLOCKBYTES,
                   BLAKE2B_BLOCKBYTES); /* Shift buffer left */
            S->buflen -= BLAKE2B_BLOCKBYTES;
            in += fill;
            inlen -= fill;
        } else /* inlen <= fill */
        {
            inline_memcpy(S->buf + left, in, inlen);
            S->buflen += inlen; /* Be lazy, do not compress */
            in += inlen;
            inlen -= inlen;
        }
    }

    return 0;
}

int
blake2b_final(blake2b_state *S, uint8_t *out, uint8_t outlen)
{
    unsigned char buffer[BLAKE2B_OUTBYTES];

    if (!outlen || outlen > BLAKE2B_OUTBYTES) {
        //sodium_misuse();
    }
    if (blake2b_is_lastblock(S)) {
        return -1;
    }
    if (S->buflen > BLAKE2B_BLOCKBYTES) {
        blake2b_increment_counter(S, BLAKE2B_BLOCKBYTES);
        blake2b_compress(S, S->buf);
        S->buflen -= BLAKE2B_BLOCKBYTES;
        COMPILER_ASSERT(S->buflen <= BLAKE2B_BLOCKBYTES);
        inline_memcpy(S->buf, S->buf + BLAKE2B_BLOCKBYTES, S->buflen);
    }

    blake2b_increment_counter(S, S->buflen);
    blake2b_set_lastblock(S);
    inline_memset(S->buf + S->buflen, 0,
           2 * BLAKE2B_BLOCKBYTES - S->buflen); /* Padding */
    blake2b_compress(S, S->buf);

    COMPILER_ASSERT(sizeof buffer == 64U);
    STORE64_LE(buffer + 8 * 0, S->h[0]);
    STORE64_LE(buffer + 8 * 1, S->h[1]);
    STORE64_LE(buffer + 8 * 2, S->h[2]);
    STORE64_LE(buffer + 8 * 3, S->h[3]);
    STORE64_LE(buffer + 8 * 4, S->h[4]);
    STORE64_LE(buffer + 8 * 5, S->h[5]);
    STORE64_LE(buffer + 8 * 6, S->h[6]);
    STORE64_LE(buffer + 8 * 7, S->h[7]);
    inline_memcpy(out, buffer, outlen); /* outlen <= BLAKE2B_OUTBYTES (64) */

    sodium_memzero(S->h, sizeof S->h);
    sodium_memzero(S->buf, sizeof S->buf);

    return 0;
}

/* init xors IV with input parameter block */
int
blake2b_init_param(blake2b_state *S, const blake2b_param *P)
{
    size_t         i;
    const uint8_t *p;

    //COMPILER_ASSERT(sizeof *P == 64);
    blake2b_init0(S);
    p = (const uint8_t *) (P);

    /* IV XOR ParamBlock */
    for (i = 0; i < 8; i++) {
        S->h[i] ^= LOAD64_LE(p + sizeof(S->h[i]) * i);
    }
    return 0;
}

int
blake2b_init_key(blake2b_state *S, const uint8_t outlen, const void *key,
                 const uint8_t keylen)
{
    blake2b_param P[1];

    if ((!outlen) || (outlen > BLAKE2B_OUTBYTES)) {
        //sodium_misuse();
    }
    if (!key || !keylen || keylen > BLAKE2B_KEYBYTES) {
        //sodium_misuse();
    }
    P->digest_length = outlen;
    P->key_length    = keylen;
    P->fanout        = 1;
    P->depth         = 1;
    STORE32_LE(P->leaf_length, 0);
    STORE64_LE(P->node_offset, 0);
    P->node_depth   = 0;
    P->inner_length = 0;
    inline_memset(P->reserved, 0, sizeof(P->reserved));
    inline_memset(P->salt, 0, sizeof(P->salt));
    inline_memset(P->personal, 0, sizeof(P->personal));

    if (blake2b_init_param(S, P) < 0) {
        //sodium_misuse();
    }
    {
        uint8_t block[BLAKE2B_BLOCKBYTES];
        inline_memset(block, 0, BLAKE2B_BLOCKBYTES);
        inline_memcpy(block, key, keylen); /* keylen cannot be 0 */
        blake2b_update(S, block, BLAKE2B_BLOCKBYTES);
        bzero(block, BLAKE2B_BLOCKBYTES); /* Burn the key from stack */
    }
    return 0;
}


int
blake2b_init(blake2b_state *S, const uint8_t outlen)
{
    blake2b_param P[1];

    if ((!outlen) || (outlen > BLAKE2B_OUTBYTES)) {
        //sodium_misuse();
        while(1);
    }
    P->digest_length = outlen;
    P->key_length    = 0;
    P->fanout        = 1;
    P->depth         = 1;
    STORE32_LE(P->leaf_length, 0);
    STORE64_LE(P->node_offset, 0);
    P->node_depth   = 0;
    P->inner_length = 0;
    inline_memset(P->reserved, 0, sizeof(P->reserved));
    inline_memset(P->salt, 0, sizeof(P->salt));
    inline_memset(P->personal, 0, sizeof(P->personal));
    return blake2b_init_param(S, P);
}

#ifdef CONFIG_ENABLE_ENCRYPT
int crypto_scalarmult_curve25519_ref10_base(unsigned char *q, const unsigned char *n);

static int
crypto_box_keypair(unsigned char *pk, unsigned char *sk)
{
    randombytes(sk, 32);

    return crypto_scalarmult_curve25519_ref10_base(pk, sk);
}

int
crypto_secretbox_detached(unsigned char *c, unsigned char *mac,
                          const unsigned char *m,
                          unsigned long long mlen, const unsigned char *n,
                          const unsigned char *k)
{
    crypto_onetimeauth_poly1305_state state;
    unsigned char                     block0[64U];
    unsigned char                     subkey[crypto_stream_salsa20_KEYBYTES];
    unsigned long long                i;
    unsigned long long                mlen0;

    crypto_core_hsalsa20(subkey, n, k, NULL);

    if (((uintptr_t) c > (uintptr_t) m &&
         (unsigned long long)((uintptr_t) c - (uintptr_t) m) < mlen) ||
        ((uintptr_t) m > (uintptr_t) c &&
         (unsigned long long)((uintptr_t) m - (uintptr_t) c) < mlen)) { /* LCOV_EXCL_LINE */
        memmove(c, m, mlen);
        m = c;
    }
    inline_memset(block0, 0U, crypto_secretbox_ZEROBYTES);
    COMPILER_ASSERT(64U >= crypto_secretbox_ZEROBYTES);
    mlen0 = mlen;
    if (mlen0 > 64U - crypto_secretbox_ZEROBYTES) {
        mlen0 = 64U - crypto_secretbox_ZEROBYTES;
    }
    for (i = 0U; i < mlen0; i++) {
        block0[i + crypto_secretbox_ZEROBYTES] = m[i];
    }
    crypto_stream_salsa20_xor(block0, block0,
                              mlen0 + crypto_secretbox_ZEROBYTES,
                              n + 16, subkey);
    COMPILER_ASSERT(crypto_secretbox_ZEROBYTES >=
                    crypto_onetimeauth_poly1305_KEYBYTES);
    crypto_onetimeauth_poly1305_init(&state, block0);

    for (i = 0U; i < mlen0; i++) {
        c[i] = block0[crypto_secretbox_ZEROBYTES + i];
    }
    sodium_memzero(block0, sizeof block0);
    if (mlen > mlen0) {
        crypto_stream_salsa20_xor_ic(c + mlen0, m + mlen0, mlen - mlen0,
                                     n + 16, 1U, subkey);
    }
    sodium_memzero(subkey, sizeof subkey);

    crypto_onetimeauth_poly1305_update(&state, c, mlen);
    crypto_onetimeauth_poly1305_final(&state, mac);
    sodium_memzero(&state, sizeof state);

    return 0;
}
#endif

#ifndef UINT8_MAX
#define UINT8_MAX 0xff /* 255U */
#endif

int
crypto_generichash_blake2b_init(crypto_generichash_blake2b_state *state,
                                const unsigned char *key, const size_t keylen,
                                const size_t outlen)
{
    if (outlen <= 0U || outlen > BLAKE2B_OUTBYTES ||
        keylen > BLAKE2B_KEYBYTES) {
        return -1;
    }
    COMPILER_ASSERT(outlen <= UINT8_MAX);
    COMPILER_ASSERT(keylen <= UINT8_MAX);
    if (key == NULL || keylen <= 0U) {
        if (blake2b_init(state, (uint8_t) outlen) != 0) {
            return -1; /* LCOV_EXCL_LINE */
        }
    } else if (blake2b_init_key(state, (uint8_t) outlen, key,
                                (uint8_t) keylen) != 0) {
        return -1; /* LCOV_EXCL_LINE */
    }
    return 0;
}

int
crypto_generichash_init(crypto_generichash_state *state,
                        const unsigned char *key,
                        const size_t keylen, const size_t outlen)
{
    return crypto_generichash_blake2b_init
        ((crypto_generichash_blake2b_state *) state, key, keylen, outlen);
}

int
crypto_generichash_update(crypto_generichash_state *state,
                          const unsigned char *in,
                          unsigned long long inlen)
{
    return blake2b_update(state, in, inlen);
}

int
crypto_generichash_final(crypto_generichash_state *state,
                         unsigned char *out, const size_t outlen)
{
    return blake2b_final(state, out, outlen);
}

static int
_crypto_box_seal_nonce(unsigned char *nonce,
                       const unsigned char *pk1, const unsigned char *pk2)
{
    crypto_generichash_state st;

    crypto_generichash_init(&st, NULL, 0U, crypto_box_NONCEBYTES);
    crypto_generichash_update(&st, pk1, crypto_box_PUBLICKEYBYTES);
    crypto_generichash_update(&st, pk2, crypto_box_PUBLICKEYBYTES);
    crypto_generichash_final(&st, nonce, crypto_box_NONCEBYTES);

    return 0;
}

int crypto_onetimeauth_poly1305_verify(const unsigned char *h,
                                       const unsigned char *in,
                                       unsigned long long   inlen,
                                       const unsigned char *k);
int
crypto_secretbox_open_detached(unsigned char *m, const unsigned char *c,
                               const unsigned char *mac,
                               unsigned long long clen,
                               const unsigned char *n,
                               const unsigned char *k)
{
    unsigned char      block0[64U];
    unsigned char      subkey[crypto_stream_salsa20_KEYBYTES];
    unsigned long long i;
    unsigned long long mlen0;

    crypto_core_hsalsa20(subkey, n, k, NULL);
    crypto_stream_salsa20(block0, crypto_stream_salsa20_KEYBYTES,
                          n + 16, subkey);

    if (crypto_onetimeauth_poly1305_verify(mac, c, clen, block0) != 0) {
        sodium_memzero(subkey, sizeof subkey);
        return -1;
    }

    if (m == NULL) {
        return 0;
    }
    if (((uintptr_t) c >= (uintptr_t) m &&
         (unsigned long long)((uintptr_t) c - (uintptr_t) m) < clen) ||
        ((uintptr_t) m >= (uintptr_t) c &&
         (unsigned long long)((uintptr_t) m - (uintptr_t) c) < clen)) { /* LCOV_EXCL_LINE */
        memmove(m, c, clen);
        c = m;
    }
    mlen0 = clen;
    if (mlen0 > 64U - crypto_secretbox_ZEROBYTES) {
        mlen0 = 64U - crypto_secretbox_ZEROBYTES;
    }
    for (i = 0U; i < mlen0; i++) {
        block0[crypto_secretbox_ZEROBYTES + i] = c[i];
    }
    crypto_stream_salsa20_xor(block0, block0,
                              crypto_secretbox_ZEROBYTES + mlen0,
                              n + 16, subkey);
    for (i = 0U; i < mlen0; i++) {
        m[i] = block0[i + crypto_secretbox_ZEROBYTES];
    }
    if (clen > mlen0) {
        crypto_stream_salsa20_xor_ic(m + mlen0, c + mlen0, clen - mlen0,
                                     n + 16, 1U, subkey);
    }
    sodium_memzero(subkey, sizeof subkey);

    return 0;
}

int crypto_scalarmult_curve25519_ref10(unsigned char *q,
                                       const unsigned char *n,
                                       const unsigned char *p);
int
crypto_scalarmult_curve25519(unsigned char *q, const unsigned char *n,
                             const unsigned char *p)
{
    size_t                 i;
    volatile unsigned char d = 0;

    if (crypto_scalarmult_curve25519_ref10(q, n, p) != 0) {
        return -1; /* LCOV_EXCL_LINE */
    }

    for (i = 0; i < crypto_scalarmult_curve25519_BYTES; i++) {
        d |= q[i];
    }
    return -(1 & ((d - 1) >> 8));
}

int
crypto_box_curve25519xsalsa20poly1305_beforenm(unsigned char *k,
                                               const unsigned char *pk,
                                               const unsigned char *sk)
{
    static const unsigned char zero[16] = { 0 };
    unsigned char s[32];

    if (crypto_scalarmult_curve25519(s, sk, pk) != 0) {
        return -1;
    }
    return crypto_core_hsalsa20(k, zero, s, NULL);
}

int
crypto_box_beforenm(unsigned char *k, const unsigned char *pk,
                    const unsigned char *sk)
{
    return crypto_box_curve25519xsalsa20poly1305_beforenm(k, pk, sk);
}

#ifdef CONFIG_ENABLE_ENCRYPT
int
crypto_box_detached_afternm(unsigned char *c, unsigned char *mac,
                            const unsigned char *m, unsigned long long mlen,
                            const unsigned char *n, const unsigned char *k)
{
    return crypto_secretbox_detached(c, mac, m, mlen, n, k);
}

int
crypto_box_detached(unsigned char *c, unsigned char *mac,
                    const unsigned char *m, unsigned long long mlen,
                    const unsigned char *n, const unsigned char *pk,
                    const unsigned char *sk)
{
    unsigned char k[crypto_box_BEFORENMBYTES];
    int           ret;

    COMPILER_ASSERT(crypto_box_BEFORENMBYTES >= crypto_secretbox_KEYBYTES);
    if (crypto_box_beforenm(k, pk, sk) != 0) {
        return -1;
    }
    ret = crypto_box_detached_afternm(c, mac, m, mlen, n, k);
    sodium_memzero(k, sizeof k);

    return ret;
}
#endif

int
crypto_box_open_detached_afternm(unsigned char *m, const unsigned char *c,
                                 const unsigned char *mac,
                                 unsigned long long clen,
                                 const unsigned char *n,
                                 const unsigned char *k)
{
    return crypto_secretbox_open_detached(m, c, mac, clen, n, k);
}


int
crypto_box_open_detached(unsigned char *m, const unsigned char *c,
                         const unsigned char *mac,
                         unsigned long long clen, const unsigned char *n,
                         const unsigned char *pk, const unsigned char *sk)
{
    unsigned char k[crypto_box_BEFORENMBYTES];
    int           ret;

    if (crypto_box_beforenm(k, pk, sk) != 0) {
        return -1;
    }
    ret = crypto_box_open_detached_afternm(m, c, mac, clen, n, k);
    sodium_memzero(k, sizeof k);

    return ret;
}

#ifdef CONFIG_ENABLE_ENCRYPT
int
crypto_box_easy(unsigned char *c, const unsigned char *m,
                unsigned long long mlen, const unsigned char *n,
                const unsigned char *pk, const unsigned char *sk)
{
    if (mlen > crypto_box_MESSAGEBYTES_MAX) {
        sodium_misuse();
    }
    return crypto_box_detached(c + crypto_box_MACBYTES, c, m, mlen, n,
                               pk, sk);
}
#endif

int
crypto_box_open_easy(unsigned char *m, const unsigned char *c,
                     unsigned long long clen, const unsigned char *n,
                     const unsigned char *pk, const unsigned char *sk)
{
    if (clen < crypto_box_MACBYTES) {
        return -2;
    }
    return crypto_box_open_detached(m, c + crypto_box_MACBYTES, c,
                                    clen - crypto_box_MACBYTES,
                                    n, pk, sk);
}

#ifdef CONFIG_ENABLE_ENCRYPT
int
crypto_box_seal(unsigned char *c, const unsigned char *m,
                unsigned long long mlen, const unsigned char *pk)
{
    unsigned char nonce[crypto_box_NONCEBYTES];
    unsigned char epk[crypto_box_PUBLICKEYBYTES];
    unsigned char esk[crypto_box_SECRETKEYBYTES];
    int           ret;

    if (crypto_box_keypair(epk, esk) != 0) {
       return -1; /* LCOV_EXCL_LINE */
    }

    inline_memcpy(c, epk, crypto_box_PUBLICKEYBYTES);
    _crypto_box_seal_nonce(nonce, epk, pk);
    ret = crypto_box_easy(c + crypto_box_PUBLICKEYBYTES, m, mlen,
                          nonce, pk, esk);
    sodium_memzero(esk, sizeof esk);
    sodium_memzero(epk, sizeof epk);
    sodium_memzero(nonce, sizeof nonce);

    return ret;
}
#endif

int
crypto_box_seal_open(unsigned char *m, const unsigned char *c,
                     unsigned long long clen,
                     const unsigned char *pk, const unsigned char *sk)
{
    unsigned char nonce[crypto_box_NONCEBYTES];

    if (clen < crypto_box_SEALBYTES) {
        return -2;
    }
    _crypto_box_seal_nonce(nonce, c, pk);

    COMPILER_ASSERT(crypto_box_PUBLICKEYBYTES < crypto_box_SEALBYTES);
    return crypto_box_open_easy(m, c + crypto_box_PUBLICKEYBYTES,
                                clen - crypto_box_PUBLICKEYBYTES,
                                nonce, c, sk);
}


