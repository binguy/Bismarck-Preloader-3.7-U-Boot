#include <argp.h>
#include <libfdt_env.h>
#include <fdt.h>
#include <libfdt.h>
#include <libfdt_internal.h>
#include <util.h>

extern int bch_gen_fdt(int fd, BCH_INFO_T *info);
extern int bch_gen_plr(int fd, BCH_INFO_T *info);
extern int bch_gen_ubt(int fd, BCH_INFO_T *info);
extern void bch_init_parameters(uint32_t psz);
extern int fdt_get_node(const void *fdt, const char *path);
extern uint32_t get_first_load_bytes(void *addr);

int fdtbch_malloc(uint8_t **buf, uint32_t size)
{
    *buf = malloc(size);
    if (NULL==*buf) {
        printf("EE: malloc failed\n");
        return 1;
    }
    memset(*buf, 0xff, size);
    return 0;
}

const char pk[] = "pk: ";
const char sk[] = "sk: ";

void fdtbch_scan_key(char *string, uint8_t *key)
{
    uint32_t len = strlen(string);
    uint32_t i;
    char b[2];
    for(i=0; i<len; i+=2) {
        memcpy(b, string+i, 2);
        sscanf(b, "%02hhX", key+(i>>1));
    }
}

int fdtbch_import_keyfile(CRYPTO_INFO_T *info)
{
    char item[32];
    char keys[128];
    char line[256];
    FILE *fp = fopen(info->keys, "r");
    if(fp == NULL) {
        printf("EE: unable to open input file '%s'\n", info->keys);
        return 1;
    }
    while(fgets(line, sizeof(line), fp) != NULL) {
        sscanf(line, "%s %s\n", item, keys);
        if(0==strncmp(pk, item, strlen(pk)-2)) {
            fdtbch_scan_key(keys, info->sign_pk);
        } else if(0==strncmp(sk, item, strlen(sk)-2)) {
            fdtbch_scan_key(keys, info->sign_sk);
        } else { continue; }
    }
    return 0;
}

void fdtbch_generate_keyfile(CRYPTO_INFO_T *info)
{
    uint32_t i, j=0;
    char o[512];
    int fd;

    sprintf(o+j, pk);
    j+=sizeof(pk)-1;
    for(i=0; i<crypto_sign_PUBLICKEYBYTES; i++, j+=2) {
        sprintf(o+j, "%02hhX", info->sign_pk[i]);
    }
    sprintf(o+(j++), "\n");
    sprintf(o+j, sk);
    j+=sizeof(sk)-1;
    for(i=0; i<crypto_sign_SECRETKEYBYTES; i++, j+=2) {
        sprintf(o+j, "%02X", info->sign_sk[i]);
    }
    if (info->op_key_pair != NULL) {
        fd = open(info->op_key_pair, O_RDWR|O_TRUNC|O_CREAT, 0666);
    } else {
        fd = open("keys", O_RDWR|O_TRUNC|O_CREAT, 0666);
    }
    if(fd<0) {
        printf("EE: unable to open output file 'keys'\n");
    }
    write(fd, o, strlen(o));
    close(fd);
}

#define DEV_TYPE    'd'
#define INPUT_IMG   'i'
#define OUTPUT_IMG  'o'
#define IMPORT_KEY  'k'
#define GEN_KEY     'g'
#define PAGE_SIZE   'p'
#define IMG_COPIES  'c'

static int parse_opts(int key, char *arg, struct argp_state *state)
{
    CRYPTO_INFO_T *info = (CRYPTO_INFO_T *)state->input;
    switch (key)
    {
        case DEV_TYPE:
            info->dt = atoi(arg);
            break;
        case IMPORT_KEY:
            info->keys = arg;
            break;
        case GEN_KEY:
            info->op_key_pair = arg;
            break;
        case INPUT_IMG:
            info->input = arg;
            break;
        case OUTPUT_IMG:
            info->output = arg;
            break;
        case PAGE_SIZE:
            info->psize = atoi(arg);
            break;
        case IMG_COPIES:
            info->copies = atoi(arg);
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

#define offset_fdt  0

int do_img_process(CRYPTO_INFO_T *info)
{
    uint8_t *fdt=NULL, *pimg=NULL;
    int fdt_size;
    uint32_t size, off;
    CRYPTO_HEADER_T hdr;
    BCH_INFO_T fdt_info, plr_info, ubt_info;
    memset(&hdr, 0, sizeof(CRYPTO_HEADER_T));

    // process FDT information: get FDT header first, and then full FDT
    struct fdt_header fdt_hdr;
    read(info->ifd, (void *)&fdt_hdr, sizeof(struct fdt_header));
    if(fdt_check_header(&fdt_hdr)) {
        printf("EE: Invalid FDT format\n");
        return 0;
    }

    fdt_size = fdt_totalsize(&fdt_hdr);
    if(0!=fdtbch_malloc(&fdt, fdt_size)) { goto done; }
    pread(info->ifd, fdt, fdt_size, offset_fdt);

    // gen_hdr_bch first time, for offset
    fdt_info.img = fdt_info.encode_img = fdt;
    fdt_info.img_size = fdt_info.encode_img_size = fdt_size;
    fdt_info.img_off = 0;
    if(info->dt == CRYPTO_NAND_DEVICE) {
        fdt_info.img_copies = info->copies;
        printf("II: Generate BCH info for FDT (%d copies)\n", info->copies);
        bch_gen_fdt(info->ofd, &fdt_info);
    }
    // process Preloader
    fdt_get_plr_datasize(fdt, &size);
    fdt_get_plr_dataoff(fdt, &off);
    printf("II: Preloader data size = 0x%x @ 0x%x (fdt_size=0x%x)\n", size, off, fdt_size);

    if(0!=fdtbch_malloc(&pimg, size)) { goto done; }
    pread(info->ifd, pimg, size, FDT_ALIGN_SIZE(fdt_size));

    plr_info.img = plr_info.encode_img = pimg;
    plr_info.img_size = plr_info.encode_img_size = size;
    plr_info.img_off = FDT_ALIGN_SIZE(fdt_info.encode_img_size);
    fdt_set_plr_datasize(fdt, &plr_info.encode_img_size);
    if(info->dt == CRYPTO_NAND_DEVICE) {
        plr_info.img_copies = info->copies;
        printf("II: Generate BCH info for Preloader (%d copies)\n", info->copies);
        bch_gen_plr(info->ofd, &plr_info);
    } else {
        plr_info.img_size = size;
        pwrite(info->ofd, plr_info.img, plr_info.img_size, plr_info.img_off);
    }

    // retrieve CRYPTO_HEADER from fdt
    fdt_get_ubt_sign(fdt, &hdr);

    // clone key information
    hdr.type = info->opt;
    memcpy(hdr.sign_pk, info->sign_pk, sizeof(info->sign_sk));
    //key_print(hdr.sign_pk, sizeof(hdr.sign_pk), "SIGN_PK");

    // update u-Boot hdr
    if(fdt_get_node(fdt, FIT_UBT_IMAGES_PATH)>0) {
        fdt_get_ubt_datasize(fdt, &size);
        fdt_get_ubt_dataoff(fdt, &off);
        printf("II: U-Boot data size = 0x%x @ 0x%x\n", size, off);
        // gen_ubt_bch
        pimg = realloc(pimg, size);
        pread(info->ifd, pimg, size, FDT_ALIGN_SIZE(fdt_size)+off);

        ubt_info.img = ubt_info.encode_img = pimg;
        ubt_info.img_size = ubt_info.encode_img_size = size;
        ubt_info.img_off = plr_info.img_off + plr_info.encode_img_size;

        fdt_set_ubt_datasize(fdt, &ubt_info.encode_img_size);
        fdt_set_ubt_dataoff(fdt, &plr_info.encode_img_size);
        if(info->dt == CRYPTO_NAND_DEVICE) {
            ubt_info.img_copies = info->copies;
            printf("II: Generate BCH info for U-Boot (%d copies)\n", info->copies);
            bch_gen_ubt(info->ofd, &ubt_info);
        }  else {
            pwrite(info->ofd, ubt_info.img, ubt_info.img_size, ubt_info.img_off);
        }
    }

    // gen ubt sign
    crypto_sign_gen(hdr.signu, pimg, size, info->sign_sk);
    fdt_set_ubt_sign(fdt, &hdr, sizeof(CRYPTO_HEADER_T));
    memset(hdr.sign_pk, 0xff, sizeof(hdr.sign_pk));

    // update final FDT
    if(info->dt == CRYPTO_NAND_DEVICE) {
        bch_gen_fdt(info->ofd, &fdt_info);
    } else {
        pwrite(info->ofd, fdt_info.img, fdt_info.img_size, fdt_info.img_off);
    }
done:
    if(fdt) free(fdt);
    if(pimg) free(pimg);
    return 0;
}

// fdtbch
//    - crypto options, 0: none, 1: signature-only, 2: signature-encrypt, 3: verify (ToDo)
//    - device type, 0: nor, 1: nand
//    - pagesize, 0: 2048, 1: 4096
//    - input fdt file
//    - output fdt file
//    - import key file

int main(int argc, char **argv)
{
    CRYPTO_INFO_T info CRYPTO_ALIGN(4);
    memset(&info, 0, sizeof(CRYPTO_INFO_T));
    info.copies = 1;

    struct argp_option opts[] = {
        { "keygen",     GEN_KEY,    "KEYFILE",      0,  "Generate Key Pairs"},
        { "keyin",      IMPORT_KEY, "KEYFILE",      0,  "Import Key"},
        { "input",      INPUT_IMG,  "INPUT",        0,  "Input File"},
        { "output",     OUTPUT_IMG, "OUTPUT",       0,  "Output File"},
        { "psize",      PAGE_SIZE,  "PSIZE",        0,  "Page Size (bytes), 0: 2048, 1: 4096"},
        { "devtype",    DEV_TYPE,   "DTYPE",        0,  "Dev Type, 0: spi-nor, 1: spi-nand"},
        { "imgcopies",  IMG_COPIES, "IMGCOPIES",    0,  "Copies of Image"},
        { 0 }
    };
    struct argp argp = { opts, parse_opts};
    argp_parse(&argp, argc, argv, 0, 0, &info);

    if(info.op_key_pair != NULL) {
        crypto_keygen(&info);
        fdtbch_generate_keyfile(&info);
        return 0;
    }

    if(info.input == NULL || info.output == NULL) {
        printf("EE: missing input/output file\n");
        return 1;
    }

    if(info.keys) {
        if(0!=fdtbch_import_keyfile(&info)) { goto keygen; }
    } else {
keygen:
        crypto_keygen(&info);
        fdtbch_generate_keyfile(&info);
    }

    // init bch parameters
    if(info.dt == CRYPTO_NAND_DEVICE) {
        bch_init_parameters(info.psize?4096:2048);
    }

    // process input/output file
    if((info.ifd = open(info.input, O_RDONLY))<0) {
        printf("EE: unable to open input file '%s'\n", info.input);
        return 1;
    }

    if((info.ofd = open(info.output, O_RDWR|O_TRUNC|O_CREAT, 0666))<0) {
        printf("EE: unable to open output file '%s'\n", info.output);
        return 1;
    }

    if(do_img_process(&info)) {
        printf("EE: image generation failed\n");
        return 1;
    }

    if(info.ifd) close(info.ifd);
    if(info.ofd) close(info.ofd);

    return 0;
}
