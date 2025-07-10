#ifndef _SECURE_BOOT_H_
#define _SECURE_BOOT_H_ 1

#include <soc.h>

#define alloca(size) __builtin_alloca(size)

#define _noinline_ __attribute__((noinline))
#define _unused_ __attribute__((unused))

#define NULL ((void *)0)

#define SEB_COMP_NONE 0
#define SEB_COMP_LZMA 3

#define SEB_CRYPTO_NONE 0
#define SEB_CRYPTO_RSA2048 1
#define SEB_CRYPTO_RSA4096 2

#define SEB_HASH_NONE 0
#define SEB_HASH_SHA1 1
#define SEB_HASH_SHA256 2
#define SEB_HASH_SHA512 3

#define IMAGE_FORMAT_INVALID 0x00
#define IMAGE_FORMAT_LEGACY 0x01	/* legacy image_header based format */
#define IMAGE_FORMAT_FIT 0x02	/* new, libfdt based format */

#define CHUNKSZ_SHA1 (64 * 1024)

#define IH_MAGIC 0x27051956 /* Image Magic Number		*/

#define FDT_MAGIC 0xd00dfeed	/* 4: version, 4: total size */
#define FDT_SW_MAGIC (~FDT_MAGIC)
#define FDT_FIRST_SUPPORTED_VERSION	0x02
#define FDT_LAST_SUPPORTED_VERSION	0x11

#define FDT_ERR_BADSTATE 7
#define FDT_ERR_BADMAGIC 9
#define FDT_ERR_BADVERSION 10

#define FDT_NODE_START 0x1
#define FDT_NODE_END 0x2
#define FDT_PROP 0x3
#define FDT_NOP 0x4
#define FDT_END 0x9

typedef unsigned long long uint64_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
typedef long long int64_t;
typedef int int32_t;
typedef short int16_t;
typedef char int8_t;

/* legacy image header */
typedef struct image_header {
	uint32_t ih_magic; /* Image Header Magic Number */
	uint32_t ih_hcrc; /* Image Header CRC Checksum */
	uint32_t ih_time; /* Image Creation Timestamp */
	uint32_t ih_size; /* Image Data Size */
	uint32_t ih_load; /* Data Load Address */
	uint32_t ih_ep; /* Entry Point Address */
	uint32_t ih_dcrc; /* Image Data CRC Checksum */
	uint8_t  ih_os; /* Operating System */
	uint8_t  ih_arch; /* CPU architecture */
	uint8_t  ih_type; /* Image Type */
	uint8_t  ih_comp; /* Compression Type */
	uint8_t  ih_name[32]; /* Image Name */
} image_header_t;

struct fdt_header {
	uint32_t magic; /* magic word FDT_MAGIC */
	uint32_t totalsize; /* total size of DT block */
	union {
		uint32_t off_dt_struct; /* offset to structure */
		uint32_t *dt_struct;
	};
	union {
		uint32_t off_dt_strings; /* offset to strings */
		char *dt_strings;
	};
	uint32_t off_mem_rsvmap; /* offset to memory reserve map */
	uint32_t version; /* format version */
	uint32_t last_comp_version; /* last compatible version */

	/* version 2 fields below */
	uint32_t boot_cpuid_phys; /* Which physical CPU id we're
																booting on */
	/* version 3 fields below */
	uint32_t size_dt_strings; /* size of the strings block */

	/* version 17 fields below */
	uint32_t size_dt_struct; /* size of the structure block */
};

/* Otto secure boot image info. */
typedef struct {
	uint32_t blob_addr; //blob address, mostly on flash

	uint32_t img_addr; //image address, after blob_header parsed
	uint32_t img_size; //image size

	uint32_t load_addr; //load address of image
	uint32_t entry_addr; //entry address of the image

	uint32_t sig_len; //signature length
	uint8_t sig_signature[256]; //signature
	uint8_t sig_crypto_algo; //signature crypto algo. of image
	uint8_t sig_hash_algo; //signature hash algo. of image

	uint8_t enc_algo; //encryption algo. of image

	uint8_t comp_algo; //compression algo. of image

	void *to_be_defined;
} image_t;

struct image_sign_info {
	const char *keydir;		/* Directory conaining keys */
	const char *keyname;		/* Name of key to use */
	void *fit;			/* Pointer to FIT blob */
	int node_offset;		/* Offset of signature node */
	const char *name;		/* Algorithm name */
	struct checksum_algo *checksum;	/* Checksum algorithm information */
	struct crypto_algo *crypto;	/* Crypto algorithm information */
	const void *fdt_blob;		/* FDT containing public keys */
	int required_keynode;		/* Node offset of key to use: -1=any */
	const char *require_keys;	/* Value for 'required' property */
	const char *engine_id;		/* Engine to use for signing */
};

struct checksum_algo {
	const char *name;
	const int checksum_len;
	const int der_len;
	const uint8_t *der_prefix;
	int (*calculate)(image_t *info, uint8_t *checksum);
};

struct crypto_algo {
	const char *name;		/* Name of algorithm */
	const int key_len;

	/**
	 * sign() - calculate and return signature for given input data
	 *
	 * @info:	Specifies key and FIT information
	 * @data:	Pointer to the input data
	 * @data_len:	Data length
	 * @sigp:	Set to an allocated buffer holding the signature
	 * @sig_len:	Set to length of the calculated hash
	 *
	 * This computes input data signature according to selected algorithm.
	 * Resulting signature value is placed in an allocated buffer, the
	 * pointer is returned as *sigp. The length of the calculated
	 * signature is returned via the sig_len pointer argument. The caller
	 * should free *sigp.
	 *
	 * @return: 0, on success, -ve on error
	 */
	int (*sign)(struct image_sign_info *info, image_t *img, uint8_t **sigp, uint32_t *sig_len);

	/**
	 * add_verify_data() - Add verification information to FDT
	 *
	 * Add public key information to the FDT node, suitable for
	 * verification at run-time. The information added depends on the
	 * algorithm being used.
	 *
	 * @info:	Specifies key and FIT information
	 * @keydest:	Destination FDT blob for public key data
	 * @return: 0, on success, -ve on error
	 */
	int (*add_verify_data)(struct image_sign_info *info, void *keydest);

	/**
	 * verify() - Verify a signature against some data
	 *
	 * @info:	Specifies key and FIT information
	 * @data:	Pointer to the input data
	 * @data_len:	Data length
	 * @sig:	Signature
	 * @sig_len:	Number of bytes in signature
	 * @return 0 if verified, -ve on error
	 */
	/* int (*verify)(struct image_sign_info *info, image_t *img, uint8_t *sig, uint32_t sig_len); */
	int (*verify)(const image_t *info,
	              const int key_len,
	              const struct checksum_algo *hash_algo,
	              const uint8_t *hash_val);
};

void *fdt_get_prop(const struct fdt_header *fdt, const char *path);
int fdt_get_sig_algo(const struct fdt_header *fdt, image_t *info);

int seb_is_fdt(const void *fdt);

int seb_verify_image(image_t *info);

int sha1_calculate(image_t *info, uint8_t *checksum);

int rsa_verify(const image_t *info,
               const int key_len,
               const struct checksum_algo *hash_algo,
               const uint8_t *hash_val);

int memcmp(const uint8_t *s0, const uint8_t *s1, const int size);

#endif
