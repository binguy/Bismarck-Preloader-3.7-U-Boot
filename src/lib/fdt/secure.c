#include <lib/lzma/tlzma.h>
#include <dram/memcntlr_reg.h>
#include <misc/osc.h>

#include <secure_boot.h>

extern u32_t next_env;

#define NEXT_BLOB_ADDR (&next_env) //for spi nor
#define IS_NOR() (1)
#define IS_SIG() (1)

#ifndef NORSF_READ
#	define NORSF_READ(src, len, dst, verbose) memcpy(dst, (void *)(src | 0x94000000), len)
#endif

static inline int seb_is_legacy(const void *hdr) {
	return (((const image_header_t *)hdr)->ih_magic == IH_MAGIC);
}

static int seb_get_format(const void *header) {
	if (seb_is_legacy(header))
		return IMAGE_FORMAT_LEGACY;

	if (seb_is_fdt(header) == 0)
		return IMAGE_FORMAT_FIT;

	return IMAGE_FORMAT_INVALID;
}

static void seb_flash_to_mem(void *dst, const void *src, uint32_t len) {
	if (((uint32_t)src) < (0x80000000)) {
		if (IS_NOR()) {
			NORSF_READ((uint32_t)src, len, (void *)dst, 0);
		} else {
			/* spi nand read here. */
			/* ???????? */
		}
	} else {
		memcpy((void *)dst, (const void *)src, len);
	}
	return;
}

static void seb_load_image(image_t *info) {
	/* Assuming sig. and enc. can be done in-place; copy from flash to load_addr directly.
	   If comp_algo, another temp. buffer will be allocated for decompression. */
	seb_flash_to_mem((void *)info->load_addr, (const void *)info->img_addr, info->img_size);
	return;
}

static void seb_get_header(void *blob_header, const void *blob_addr, uint32_t hdr_sz_b) {
	seb_flash_to_mem(blob_header, blob_addr, hdr_sz_b);
	return;
}

static void seb_init_image_info(image_t *info, void *blob_addr) {
#define UHDR (blob_header.uhdr)
#define FHDR (blob_header.fhdr)
	union blob_header_u {
		uimage_header_t uhdr;
		struct fdt_header fhdr;
	} blob_header;

	uint32_t *dt_struct;
	char *dt_strings;

	char *dt_sig_algo _unused_;
	char *dt_crypto _unused_;

	seb_get_header((void *)&blob_header, blob_addr, sizeof(blob_header));

	info->blob_addr = (uint32_t)blob_addr;

	switch (seb_get_format(&blob_header)) {
	case IMAGE_FORMAT_LEGACY:
		//fill imghdr the legacy way.
		info->img_addr = info->blob_addr + sizeof(UHDR);
		info->img_size = UHDR.ih_size;
		info->load_addr = UHDR.ih_load;
		info->entry_addr = UHDR.ih_ep;

		info->comp_algo = UHDR.ih_comp;

		//legacy doesn't supt. signature and encryption.
		info->sig_crypto_algo = SEB_CRYPTO_NONE;
		info->sig_hash_algo = SEB_HASH_NONE;
		info->enc_algo = 0;
		break;
	case IMAGE_FORMAT_FIT:
		dt_struct = alloca(FHDR.size_dt_struct);
		dt_strings = alloca(FHDR.size_dt_strings);
		seb_flash_to_mem(dt_struct, (blob_addr + FHDR.off_dt_struct), FHDR.size_dt_struct);
		seb_flash_to_mem(dt_strings, (blob_addr + FHDR.off_dt_strings), FHDR.size_dt_strings);
		FHDR.dt_struct = dt_struct;
		FHDR.dt_strings = dt_strings;

		/* image is appended at the end of a FDT header with 4-byte alignment. */
		info->img_addr = info->blob_addr + ((FHDR.totalsize + 3) & (~0x3));
		puts("II: getting FDT:/images/uboot/data-size...\n");
		info->img_size = *(uint32_t *)fdt_get_prop(&FHDR, "/images/uboot/data-size");
		puts("II: getting FDT:/images/uboot/load...\n");
		info->load_addr = *(uint32_t *)fdt_get_prop(&FHDR, "/images/uboot/load");
		puts("II: getting FDT:/images/uboot/entry...\n");
		info->entry_addr = *(uint32_t *)fdt_get_prop(&FHDR, "/images/uboot/entry");

		fdt_get_sig_algo(&FHDR, info);

		info->comp_algo = UIH_COMP_LZMA;

		info->enc_algo = 0;
		break;
	default:
		/* Can't help with a plain image know not target addr. */
		printf("EE: Unknown image type\n");
	}
	return;
}

static uint32_t seb_decrypt_image(image_t *info) {
	return 0;
}

static uint32_t seb_decompress_image(image_t *info) {
	uint32_t res;

	if (info->comp_algo == UIH_COMP_LZMA) {
		const uint32_t comp_size = info->img_size;
		u8_t *comp_buf = (u8_t *)alloca(comp_size);

		memcpy((void *)comp_buf, (void *)info->load_addr, comp_size);
		printf("II: Decompressing U-Boot (%p <- %p)... ", info->load_addr, comp_buf);

		res = lzma_decompress(comp_buf, (u8_t *)info->load_addr, &info->img_size);

		if (res != DECOMPRESS_OK) {
			printf("FAIL(%d)\n", res);
			goto decompress_failure;
		} else {
			printf("(%d KB <- %d KB)\n", info->img_size >> 10, comp_size >> 10);
		}
	} else {
		puts("EE: Unsupported compression type\n");
		goto decompress_failure;
	}
	return 0;
decompress_failure:
	return 1;
}

static void seb_snof_mmio_en(void) {
	RMOD_MCR(flash_map1_dis, 0);
}

static void seb_secure_boot(void) {
	image_t info;
	otto_soc_context_t otto_sc;

	if (IS_NOR()) {
		seb_snof_mmio_en();
	}

	seb_init_image_info(&info, NEXT_BLOB_ADDR);

	seb_load_image(&info);

	if (seb_verify_image(&info)) {
		puts("EE: verification fail\n");
		while (1);
	};

	if (info.enc_algo && seb_decrypt_image(&info)){
		/* decryption fail */
		while (1);
	}

	if (info.comp_algo && seb_decompress_image(&info)){
		puts("EE: decompression fail\n");
		while (1);
	}

	puts("II: Starting U-boot... \n");

	/* osc, otto soc context */
	osc_init(&otto_sc);

	dcache_wr_inv_all();
	icache_inv_all();

	((fpv_u32_t *)info.entry_addr)((u32_t)&otto_sc);

	puts("EE: Should not run to here... \n");
	while(1); // should never return

	return;
}
REG_INIT_FUNC(seb_secure_boot, 37);

int memcmp(const uint8_t *s0, const uint8_t *s1, const int size) {
	const uint8_t *s0_end = s0 + size;

	while (s0 < s0_end) {
		if (*s0 != *s1) break;
		s0++;
		s1++;
	}

	return (s0 != s0_end);
}
