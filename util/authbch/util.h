#ifndef FIT_BCH_CRYPTO_UTIL_H
#define FIT_BCH_CRYPTO_UTIL_H

#include <sodium.h>
#include <assert.h>

void key_print(uint8_t *p, uint32_t size, const char *c);
int fdtbch_malloc(uint8_t **buf, uint32_t size);

/*******************************************/
/* FDT: for FIT/FDT                        */
/*******************************************/
#define FIT_IMAGES_PATH         "/images"
#define FIT_DATA_PROP           "data"
#define FIT_DATA_POSITION_PROP  "data-position"
#define FIT_DATA_OFFSET_PROP    "data-offset"
#define FIT_DATA_SIZE_PROP      "data-size"
#define FIT_TIMESTAMP_PROP      "timestamp"
#define FIT_DESC_PROP           "description"
#define FIT_ARCH_PROP           "arch"
#define FIT_TYPE_PROP           "type"
#define FIT_OS_PROP             "os"
#define FIT_COMP_PROP           "compression"
#define FIT_ENTRY_PROP          "entry"
#define FIT_LOAD_PROP           "load"
#define FIT_SIGN_INFO           "sign"

#define FIT_PLR_IMAGES_PATH     "/images/preloader"
#define FIT_UBT_IMAGES_PATH     "/images/uboot"

#ifndef FDT_MAGIC
#define FDT_MAGIC               (0xd00dfeed)
#endif

#define FDT_ALIGN_SIZE(sz)      (((sz)+0x3)&~(0x3))

int fdt_setprop_path_val(void *fdt, const char *path, const char *name, const void *val, int len);
int fdt_setprop_path_u32(void *fdt, const char *path, const char *name, const uint32_t *val);
int fdt_getprop_path_val(const void *fdt, const char *path, const char *name, void **out, int *len);
int fdt_getprop_path_u32(const void *fdt, const char *path, const char *name, uint32_t *out);

#define fdt_get_plr_datasize(f, v)      fdt_getprop_path_u32(f, FIT_PLR_IMAGES_PATH, FIT_DATA_SIZE_PROP, v)
#define fdt_get_plr_dataoff(f, v)       fdt_getprop_path_u32(f, FIT_PLR_IMAGES_PATH, FIT_DATA_OFFSET_PROP, v)
#define fdt_set_plr_datasize(f, v)      fdt_setprop_path_u32(f, FIT_PLR_IMAGES_PATH, FIT_DATA_SIZE_PROP, v)
#define fdt_set_plr_dataoff(f, v)       fdt_setprop_path_u32(f, FIT_PLR_IMAGES_PATH, FIT_DATA_OFFSET_PROP, v)

#define fdt_get_ubt_datasize(f, v)      fdt_getprop_path_u32(f, FIT_UBT_IMAGES_PATH, FIT_DATA_SIZE_PROP, v)
#define fdt_get_ubt_dataoff(f, v)       fdt_getprop_path_u32(f, FIT_UBT_IMAGES_PATH, FIT_DATA_OFFSET_PROP, v)
#define fdt_set_ubt_datasize(f, v)      fdt_setprop_path_u32(f, FIT_UBT_IMAGES_PATH, FIT_DATA_SIZE_PROP, v)
#define fdt_set_ubt_dataoff(f, v)       fdt_setprop_path_u32(f, FIT_UBT_IMAGES_PATH, FIT_DATA_OFFSET_PROP, v)

#define fdt_get_ubt_sign(f, v)          fdt_getprop_path_val(f, FIT_UBT_IMAGES_PATH, FIT_SIGN_INFO, (void**)v, NULL)
#define fdt_set_ubt_sign(f, v, l)       fdt_setprop_path_val(f, FIT_UBT_IMAGES_PATH, FIT_SIGN_INFO, v, l)
//#define fdt_set_plr_timestamp(f, v, t)  fdt_setprop_path_u32(f, FIT_PLR_IMAGES_PATH, FIT_TIMESTAMP_PROP, v)

/*******************************************/
/* CRYPTO: for CRYPTO information          */
/*******************************************/

typedef struct CRYPTO {
    uint8_t opt;
    uint8_t dt;
    uint8_t psize;
    uint8_t copies;
    uint8_t chapok[crypto_aead_chacha20poly1305_KEYBYTES];
    uint8_t sign_pk[crypto_sign_PUBLICKEYBYTES];
    uint8_t sign_sk[crypto_sign_SECRETKEYBYTES];
    uint8_t recpnt_pk[crypto_box_PUBLICKEYBYTES];
    uint8_t recpnt_sk[crypto_box_SECRETKEYBYTES];
    uint8_t nonce[NONCEBYTES];
    uint8_t sign[SIGN_BYTES];
    uint8_t key_in_box[ENC_KEY_BYTES];
    char *keys;        // key input file
    char *op_key_pair; // generate key pair file
    char *input;       // fdt input file
    char *output;      // fdt output file
    int ifd;           // input file descriptor
    int ofd;           // input file descriptor
} CRYPTO_INFO_T;

void crypto_keygen(CRYPTO_INFO_T *crypto);
void crypto_sign_gen(uint8_t *dst, const uint8_t *src, uint32_t size, uint8_t *key);

/*******************************************/
/* BCH: for BCH information                */
/*******************************************/
typedef struct BCH_INFO {
    void *img;
    void *encode_img;
    uint32_t img_size;
    uint32_t encode_img_size;
    uint32_t img_off;
    uint8_t img_copies;
} BCH_INFO_T;

#endif // FIT_BCH_CRYPTO_UTIL_H
