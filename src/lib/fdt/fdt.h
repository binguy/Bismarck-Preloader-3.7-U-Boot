#ifndef _FDT_H_
#define _FDT_H

#ifndef alloca
#define alloca(size) __builtin_alloca(size)
#endif

#define FDT_MAGIC 0xd00dfeed    /* 4: version, 4: total size */
#define FDT_SW_MAGIC (~FDT_MAGIC)
#define FDT_FIRST_SUPPORTED_VERSION    0x02
#define FDT_LAST_SUPPORTED_VERSION    0x11

#define FDT_ERR_BADSTATE 7
#define FDT_ERR_BADMAGIC 9
#define FDT_ERR_BADVERSION 10

#define FDT_NODE_START 0x1
#define FDT_NODE_END 0x2
#define FDT_PROP 0x3
#define FDT_NOP 0x4
#define FDT_END 0x9

#define FDT_ALIGN_SIZE(sz)	(((sz)+0x3)&~(0x3))
/* legacy image header */
typedef struct image_header {
    unsigned int    ih_magic; /* Image Header Magic Number */
    unsigned int    ih_hcrc; /* Image Header CRC Checksum */
    unsigned int    ih_time; /* Image Creation Timestamp */
    unsigned int    ih_size; /* Image Data Size */
    unsigned int    ih_load; /* Data Load Address */
    unsigned int    ih_ep; /* Entry Point Address */
    unsigned int    ih_dcrc; /* Image Data CRC Checksum */
    unsigned char   ih_os; /* Operating System */
    unsigned char   ih_arch; /* CPU architecture */
    unsigned char   ih_type; /* Image Type */
    unsigned char   ih_comp; /* Compression Type */
    unsigned char   ih_name[32]; /* Image Name */
} image_header_t;

struct fdt_header {
    unsigned int magic; /* magic word FDT_MAGIC */
    unsigned int totalsize; /* total size of DT block */
    union {
        unsigned int off_dt_struct; /* offset to structure */
        unsigned int *dt_struct;
    };
    union {
        unsigned int off_dt_strings; /* offset to strings */
        char *dt_strings;
    };
    unsigned int off_mem_rsvmap; /* offset to memory reserve map */
    unsigned int version; /* format version */
    unsigned int last_comp_version; /* last compatible version */

    /* version 2 fields below */
    unsigned int boot_cpuid_phys; /* Which physical CPU id we're booting on */

    /* version 3 fields below */
    unsigned int size_dt_strings; /* size of the strings block */

    /* version 17 fields below */
    unsigned int size_dt_struct; /* size of the structure block */
};

/* Otto secure boot image info. */
typedef struct {
    unsigned int blob_addr; //blob address, mostly on flash

    unsigned int img_addr; //image address, after blob_header parsed
    unsigned int img_size; //image size

    unsigned int load_addr; //load address of image
    unsigned int entry_addr; //entry address of the image

    unsigned int sig_len; //signature length
    unsigned char sig_signature[256]; //signature
    unsigned char sig_crypto_algo; //signature crypto algo. of image
    unsigned char sig_hash_algo; //signature hash algo. of image

    unsigned char enc_algo; //encryption algo. of image

    unsigned char comp_algo; //compression algo. of image

    void *to_be_defined;
} image_t;

struct image_sign_info {
    const char *keydir;        /* Directory conaining keys */
    const char *keyname;        /* Name of key to use */
    void *fit;            /* Pointer to FIT blob */
    int node_offset;        /* Offset of signature node */
    const char *name;        /* Algorithm name */
    struct checksum_algo *checksum;    /* Checksum algorithm information */
    struct crypto_algo *crypto;    /* Crypto algorithm information */
    const void *fdt_blob;        /* FDT containing public keys */
    int required_keynode;        /* Node offset of key to use: -1=any */
    const char *require_keys;    /* Value for 'required' property */
    const char *engine_id;        /* Engine to use for signing */
};

struct checksum_algo {
    const char *name;
    const int checksum_len;
    const int der_len;
    const unsigned char *der_prefix;
    int (*calculate)(image_t *info, unsigned char *checksum);
};

struct crypto_algo {
    const char *name;        /* Name of algorithm */
    const int key_len;

    /**
     * sign() - calculate and return signature for given input data
     *
     * @info:    Specifies key and FIT information
     * @data:    Pointer to the input data
     * @data_len:    Data length
     * @sigp:    Set to an allocated buffer holding the signature
     * @sig_len:    Set to length of the calculated hash
     *
     * This computes input data signature according to selected algorithm.
     * Resulting signature value is placed in an allocated buffer, the
     * pointer is returned as *sigp. The length of the calculated
     * signature is returned via the sig_len pointer argument. The caller
     * should free *sigp.
     *
     * @return: 0, on success, -ve on error
     */
    int (*sign)(struct image_sign_info *info, image_t *img, unsigned char **sigp, unsigned int *sig_len);

    /**
     * add_verify_data() - Add verification information to FDT
     *
     * Add public key information to the FDT node, suitable for
     * verification at run-time. The information added depends on the
     * algorithm being used.
     *
     * @info:    Specifies key and FIT information
     * @keydest:    Destination FDT blob for public key data
     * @return: 0, on success, -ve on error
     */
    int (*add_verify_data)(struct image_sign_info *info, void *keydest);

    /**
     * verify() - Verify a signature against some data
     *
     * @info:    Specifies key and FIT information
     * @data:    Pointer to the input data
     * @data_len:    Data length
     * @sig:    Signature
     * @sig_len:    Number of bytes in signature
     * @return 0 if verified, -ve on error
     */
    /* int (*verify)(struct image_sign_info *info, image_t *img, unsigned char *sig, unsigned int sig_len); */
    int (*verify)(const image_t *info,
                  const int key_len,
                  const struct checksum_algo *hash_algo,
                  const unsigned char *hash_val);
};

void *fdt_get_prop(const struct fdt_header *fdt, const char *path);
int fdt_get_sig_algo(const struct fdt_header *fdt, image_t *info);
int fdt_check_header(const void *fdt);
#endif
