/* from libsodium */
#include <sodium.h>

int
crypto_sign_init(crypto_sign_state *state)
{
    return crypto_hash_sha512_init(state);
}

int
crypto_sign_update(crypto_sign_state *state, const unsigned char *m,
                   unsigned long long mlen)
{
    return crypto_hash_sha512_update(state, m, mlen);
}

#ifdef CONFIG_ENABLE_ENCRYPT
int
crypto_sign_final_create(crypto_sign_state *state, unsigned char *sig,
                         unsigned long long *siglen_p, const unsigned char *sk)
{
    unsigned char ph[crypto_hash_sha512_BYTES];

    crypto_hash_sha512_final(state, ph);
    return _crypto_sign_ed25519_detached(sig, siglen_p, ph, sizeof ph, sk, 1);
}
#endif

int
crypto_sign_final_verify(crypto_sign_state *state, unsigned char *sig,
                         const unsigned char *pk)
{
    unsigned char ph[crypto_hash_sha512_BYTES];

    crypto_hash_sha512_final(state, ph);
    return _crypto_sign_ed25519_verify_detached(sig, ph, sizeof ph, pk, 1);
}
