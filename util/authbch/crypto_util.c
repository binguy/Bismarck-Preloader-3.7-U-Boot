#include <sodium.h>
#include <util.h>
#include <assert.h>

int randombytes(uint8_t *x, uint64_t size)
{
    FILE *f = fopen("/dev/urandom", "rb");
    assert(f!=NULL);

    fread(x, 1, size, f);
    fclose(f);
    return 0;
}

void key_print(uint8_t *p, uint32_t size, const char *c)
{
    int i=size, cnt=0;
    uint32_t sum=0;
    printf("  %s:", c);
    while(i>0) {
        if(0==cnt%16) { printf("\n  "); }
        printf("%02X ", p[(size-i)]);
        sum+=p[(size)-i];
        i--; cnt++;
    }
    printf("\n");
}

void dbg_print(uint8_t *p, uint32_t size, const char *c) {
    int i=size, cnt=0;
    uint32_t sum=0;
    printf("%s:", c);
    while(i>0) {
        if(0==cnt%16) { printf("\n%08x:\t", cnt); }
        else if(0==(cnt&0x7)) { printf("  "); }
        printf("%02X ", p[(size-i)]);
        sum+=p[(size)-i];
        i--; cnt++;
    }
    printf("\n");
}

void crypto_keygen(CRYPTO_INFO_T *info)
{
    crypto_sign_ed25519_keypair(info->sign_pk, info->sign_sk);
}

void crypto_sign_gen(uint8_t *dst, const uint8_t *src, uint32_t size, uint8_t *key)
{
    crypto_sign_state state;
    crypto_sign_init(&state);
    assert(src!=NULL);
    crypto_sign_update(&state, src, size);
    crypto_sign_final_create(&state, dst, NULL, key);
    //key_print(dst, SIGN_BYTES, "Signature");
}
