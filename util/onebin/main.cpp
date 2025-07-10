/*
 * onebin tool for Whole-Image and program to NOR and NAND Flash
 *
 * Copyright 2013 Realtek Semiconductor Corp.
 * Author: JeT Kuo <jet.kuo@realtek.com>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include "list.h"
#include "bch6.h"

#define ONEBIN_VERSION			"v2.3"
//#define ONEBIN_DEBUG

#ifdef ONEBIN_DEBUG
	#define debug(fmt, args...)	printf(fmt, ##args)
#else
	#define debug(fmt, args...)
#endif

static void
show_usage(const char *n="onebin") {
	fprintf(stderr, 
		"\n"
		"This is a images merging tool with bch6 encoder.\n"
		"\n"
		"usage: %s -ft [nor|nand] -npt [small|large] -csz <n> -ppb <n> -bso <n> -bdo <n> -l <file name> -f <file name> <address> -o <file name>\n"
		"\n"
		"\t-ft [--flash-type] [nor|nand]: flash type\n"
		"\t-npt [--nand-page-type] [small|large]: small (512B data + 16B Spare Area) and large (2048B/4096B data + 64B/128B Spare Area)\n"
		"\t-csz [--chunk-size] <n>: data chunk size in byte, default value is 2048 (4096 for 4K page nand)\n"
		"\t-ppb [--chunk-per-block] <n>: chunks per block, default value is 64\n"
		"\t-bso [--bbi-swap-offset] <n>: swap offset for bbi\n"
		"\t-bdo [--bbi-dma-offset] <n>: dma offset for bbi\n"
		"\t-l <file name>: loader file name, will be appened at address 0\n"
		"\t-f <file name> <address>: file name and address\n"
		"\t-o <file name>: Target Output File\n"
		"\n"
		"\tEx. Command for 9607: $./onebin -ft nand -npt small -bso 23 -bdo 2000 "
		"-l 9607/loader.img -f 9607/uImage c00000 -f 9607/rootfs 1400000 "
		"-o onebin_9607.img\n"
		"\n"
		"\tNotice: -bso and -bdo should be defined together, if neither -bso nor -bdo is defined, no swap will be performanced.\n"
		"\n"
		, n);
}

typedef enum {
	unknown_ft,
	nor,
	nand
}E_FLASH_TYPE;

E_FLASH_TYPE		flash_type = unknown_ft;

typedef enum {
	unknown_npt,
	small,
	large
}E_NAND_PAGE_TYPE;

E_NAND_PAGE_TYPE			nand_page_type = unknown_npt;

#define MINUS_ONE			0xffffffff
#define DEFAULT_CHUNK_SIZE		0x800
#define DEFAULT_PPB			0x40
#define NO_LIMITED_CHUNKS		0xffffffff
#define MAX_FILES_NUMBER		12
#define NOR_SECTOR_SIZE			DEFAULT_CHUNK_SIZE

unsigned int chunk_size			= DEFAULT_CHUNK_SIZE;
unsigned int chunk_per_block		= DEFAULT_PPB;
unsigned int bbi_swap_offset		= MINUS_ONE;
unsigned int bbi_dma_offset		= MINUS_ONE;
unsigned int page_size			= 0;
unsigned int oob_size			= 0;
unsigned int ecc_size			= 0;
unsigned int unit_size			= 0;

const char *output_file_name		= NULL;		/* target output file */
int output_handle			= 0;

bool enable_swap			= true;
ecc_encoder_t *ecc_encoder		= NULL;

#define BLOCK_SIZE			(chunk_size * chunk_per_block)
#define CHUNK_SIZE			(chunk_size + (chunk_size / page_size * (oob_size + ecc_size)))
#define ECC_BLOCK_SIZE			CHUNK_SIZE * chunk_per_block
#define NAND_ADDRESS(address)		CHUNK_SIZE * (address / BLOCK_SIZE) * chunk_per_block

#define append_raw_one_chunk(file_handle, chunk_data, chunk_size)	do { write(output_handle, chunk_data, chunk_size); } while(0)

struct image_file {
	char			*name;
	unsigned int		address;
	unsigned int		loader;
	//unsigned int		ecc;
	struct	list_head	list;
};

LIST_HEAD(head);

/*
 *
 * add one file to file list
 *
 */
static int add_to_file_list(char *file_name, unsigned int file_address, unsigned int file_loader) {
	int i = 0, add = 0;
	struct image_file	*image;
	struct list_head	*iterator;

	list_for_each (iterator, &head)	i++;
	if(i >= MAX_FILES_NUMBER) {
		printf("Reach MAX file list number(%d)\n", MAX_FILES_NUMBER);
		return 1;
	}
	debug("File list number(%d)\n", i);

	image = (struct image_file *)malloc(sizeof(struct image_file));
	
	image->name = file_name;
	image->address = file_address;
	image->loader = file_loader;

	if (list_empty(&head)) {
		debug("list is empty\n");
		list_add(&image->list, &head);
		add = 1;
	}
	else {
		//i = 0;
		list_for_each (iterator, &head){
			//debug("#%X node (%p): 0x%08X\t0x%04X\t\t%s\n", i++,
			//	list_entry(iterator, struct image_file, list),
			//	list_entry(iterator, struct image_file, list)->address,
			//	list_entry(iterator, struct image_file, list)->loader,
			//	list_entry(iterator, struct image_file, list)->name);

			if (file_address < list_entry(iterator, struct image_file, list)->address) {
				list_add_tail(&image->list, iterator);
				add = 1;
				break;
			}
			else if (file_address == list_entry(iterator, struct image_file, list)->address) /* The Address is existed */
				return 1;
			else
				continue;
		}
	}

	if(!add)	list_add_tail(&image->list, &head);	/* File with MAXMUM Address */
	debug("Add new node (%p): 0x%08X\t0x%04X\t\t%s\n", image, file_address, file_loader, file_name);

	return 0;
}

/*
 *
 * add one file to file list
 *
 */
static int clear_file_list() {

#ifdef ONEBIN_DEBUG
	int i = 0;
#endif
	struct list_head	*iterator;

	list_for_each (iterator, &head){
		debug("Del %X node (%p): 0x%08X\t0x%04X\t\t%s\n", i++,
			list_entry(iterator, struct image_file, list),
			list_entry(iterator, struct image_file, list)->address,
			list_entry(iterator, struct image_file, list)->loader,
			list_entry(iterator, struct image_file, list)->name);

		list_del(iterator);
		debug("free: %p\n", list_entry(iterator, struct image_file, list));
		free((struct image_file *)(list_entry(iterator, struct image_file, list)));
	}

	/* Check File List */
	//if (list_empty(&head))
	//	printf("list is empty\n");

	return 0;
}

/*
 *
 * Append ecc code
 *
 */
static void append_ecc_one_chunk(const unsigned char *chunk_data, unsigned int len, const unsigned char *oob)
{
	unsigned int iteration = chunk_size / page_size;
	unsigned char local_chunk_data[chunk_size];
	unsigned char local_oob[iteration * oob_size];
	unsigned char local_ecc[iteration * ecc_size];
	unsigned int i = 0, j = 0, k = 0;

	if (len > chunk_size)	len = chunk_size;

	/* Init buffer as all 0xff */
	memset(local_chunk_data, 0xff, sizeof(local_chunk_data));
	memset(local_oob, 0xff, sizeof(local_oob));
	memset(local_ecc, 0xff, sizeof(local_ecc));

	/* compute ECC and write out */
	if ((chunk_data == NULL) && (oob == NULL)) {
		/* write one chunk with all 0xff */
		write(output_handle, local_chunk_data, sizeof(local_chunk_data));
		write(output_handle, local_oob, sizeof(local_oob));
		write(output_handle, local_ecc, sizeof(local_ecc));
	} else {
		if (chunk_data != NULL)	{
			/* Find all is 0xff */
			if( memcmp(local_chunk_data, chunk_data, chunk_size) == 0 ) {
				//debug("Find all is 0xff\n");
				write(output_handle, local_chunk_data, sizeof(local_chunk_data));
				write(output_handle, local_oob, sizeof(local_oob));
				write(output_handle, local_ecc, sizeof(local_ecc));
				return;
			}
			else {
				memcpy(local_chunk_data, chunk_data, len);
			}
		}

		if (oob != NULL)	memcpy(local_oob, oob, sizeof(local_oob));

		/* bbi swap */
		if (enable_swap) {
			local_oob[bbi_swap_offset] = local_chunk_data[bbi_dma_offset];
			local_chunk_data[bbi_dma_offset] = 0xff;
		}

		/* ecc encode for one chunk */
		for (i = 0, j = 0, k = 0; i < chunk_size; i += page_size, j += oob_size, k += ecc_size)
			ecc_encoder->encode_512B(local_ecc + k, local_chunk_data + i, local_oob + j);

		/*
		 * Write one chunk
		 * small page: 512B (Data) + 16B (Spare Area) + 512B (Data) + 16B (Spare Area) + 512B (Data) + 16B (Spare Area) + 512B (Data) + 16B (Spare Area)
		 * large page: 2048B (Data) + 64B (Spare Area) 
		 */
		if (nand_page_type == small) {
			for (i = 0, j = 0, k = 0; i < chunk_size; i += page_size, j += oob_size, k += ecc_size) {
				write(output_handle, local_chunk_data + i, page_size);
				write(output_handle, local_oob + j, oob_size);
				write(output_handle, local_ecc + k, ecc_size);
			}
		}
		else if (nand_page_type == large) {
			write(output_handle, local_chunk_data, sizeof(local_chunk_data));
			write(output_handle, local_oob, sizeof(local_oob));
			write(output_handle, local_ecc, sizeof(local_ecc));
		}
	}
}

/*
 *
 * Hendle files and write data
 *
 */
static int file_write_out(const char *path, unsigned int ecc_encode, unsigned int total_chunk)
{
	unsigned char chunk_data[CHUNK_SIZE];
	unsigned int cc = 0;			/* chunk count: 0 ~ (total_chunk -1) */
	unsigned int ci = 0;			/* chunk index: 0 ~ (chunk_per_block - 1) */
	unsigned int r = 0;			/* read data size from input file */

	int hi = open(path, O_EXCL|O_RDONLY);
	if (hi < 0) {
		fprintf(stderr, "unable to open input file (%s)\n", path);
		return 1;
	}

	if (ecc_encode) {
		if (flash_type == nor) {
			printf("ERROR: Should NOT use ecc encoding for NOR\n");
			return 1;
		}

		while (1) {
			memset(chunk_data, 0xff, chunk_size);
			r = read(hi, chunk_data, chunk_size);

			if (r > 0) {
				append_ecc_one_chunk(chunk_data, r, NULL);
				cc++;

				/* If this is least file, do block alignment */
				if (total_chunk == NO_LIMITED_CHUNKS)	ci = (ci + 1) % chunk_per_block;
			}

			if (r != chunk_size)	break;

			if ((total_chunk != NO_LIMITED_CHUNKS) && (cc > total_chunk)) {
				printf("ERROR: Space is NOT enough!!\n");
				return 1;
			}
		}

		printf("    Data Chunk %d (ECC)\n", cc);

		memset(chunk_data, 0xff, chunk_size);
		if (total_chunk == NO_LIMITED_CHUNKS) {			/* Last file, do block alignment with 0xff (NOR will not reach here)*/
			if (ci > 0) {
				for (; ci < chunk_per_block ; ci++)
					append_ecc_one_chunk(chunk_data, chunk_size, NULL);
			}
		}
		else {							/* Not last file, Fill out the entire partition with 0xff */
			for (; cc < total_chunk; cc++)
				append_ecc_one_chunk(chunk_data, chunk_size, NULL);
		}
	}
	else {
		while (1) {
			memset(chunk_data, 0xff, CHUNK_SIZE);
			r = read(hi, chunk_data, CHUNK_SIZE);

			if (r > 0) {
				append_raw_one_chunk(output_handle, chunk_data, CHUNK_SIZE);
				cc++;

				/* If this is least file, record chunk index for doing block alignment */
				if (total_chunk == NO_LIMITED_CHUNKS)	ci = (ci + 1) % chunk_per_block;
			}

			if (r != CHUNK_SIZE)	break;

			if ((total_chunk != NO_LIMITED_CHUNKS) && (cc > total_chunk)) {
				printf("ERROR: Space is NOT enough!!\n");
				return 1;
			}
		}

		printf("    Data Chunk %d (RAW)\n", cc);

		memset(chunk_data, 0xff, CHUNK_SIZE);
		if (total_chunk == NO_LIMITED_CHUNKS) {			/* Last file, do block alignment with 0xff for NAND */
			if (flash_type == nand) {
				if (ci > 0) {
					for (; ci < chunk_per_block ; ci++)
						append_raw_one_chunk(output_handle, chunk_data, CHUNK_SIZE);
				}
			}
		}
		else {							/* Not last file, Fill out the entire partition with 0xff */
			for (; cc < total_chunk; cc++)
				append_raw_one_chunk(output_handle, chunk_data, CHUNK_SIZE);
		}
	}

	close(hi);

	return 0;
}

/*
 *
 * Check Parameters
 *
 */
static int check_parameters(int error_out)
{
#ifdef ONEBIN_DEBUG
	int i = 0;
#endif
	struct list_head	*iterator;

	list_for_each (iterator, &head){
		debug("#%X node (%p): 0x%08X\t0x%04X\t\t%s\n", i++,
			list_entry(iterator, struct image_file, list),
			list_entry(iterator, struct image_file, list)->address,
			list_entry(iterator, struct image_file, list)->loader,
			list_entry(iterator, struct image_file, list)->name);

		if(list_entry(iterator, struct image_file, list)->address & (page_size-1)){
			printf("File: [%s] Address is at 0x%08X ",
			list_entry(iterator, struct image_file, list)->name,
			list_entry(iterator, struct image_file, list)->address);
			printf("==> File Address must be page (%d bytes) alignment\n", page_size);
			error_out |= 1;
		}

		if((list_entry(iterator, struct image_file, list)->loader) && 
			list_entry(iterator, struct image_file, list)->address != 0) {
			printf("Address for Loader File must be 0\n");
			error_out |= 1;
		}
		else if((!list_entry(iterator, struct image_file, list)->loader) && 
			list_entry(iterator, struct image_file, list)->address == 0) {
			printf("Address for none-Loader File must NOT be 0\n");
			error_out |= 1;
		}
	}

	switch (chunk_size) {
		case 512:
		case 2048:
		case 4096:
			break;
		default:
			fprintf(stderr, "chunk size of %d bytes is not supported\n", chunk_size);
			error_out |= 1;
		break;
	}

	/* limitation */
	unsigned int max_swap_offset = oob_size * chunk_size / page_size;
    
	if ((bbi_swap_offset == MINUS_ONE) && (bbi_dma_offset == MINUS_ONE)) {
		enable_swap = false;
	} else {
		if (bbi_swap_offset == MINUS_ONE) {
			fprintf(stderr, "--bbi-swap-offset and --bbi-dma-offset should be defined together.\n");
			error_out |= 1;
		} else if (bbi_swap_offset >= max_swap_offset) {
			fprintf(stderr, "bbi-swap-offset should be between 0 and %d\n", oob_size - 1);
			error_out |= 1;
		} else if (bbi_dma_offset == MINUS_ONE) {
			fprintf(stderr, "--bbi-swap-offset and --bbi-dma-offset should be defined together.\n");
			error_out |= 1;
		} else if (bbi_dma_offset >= chunk_size) {
			fprintf(stderr, "bbi-dma-offset should be less than the chunk size\n");
			error_out |= 1;
		}
	}

	return error_out;
}

/*
 *
 * display flash information
 *
 */
static int display_flash_info() {

	int i = 0;
	struct list_head	*iterator;

	printf("flash type: %s\n", (flash_type - 1) ? "NAND" : "NOR");

	if (flash_type == nor) {
		printf("Chunk Size = %d\n\n", page_size);
		printf("No. Address\tPartition Size\tChunk Count\tLoader\tFile\n");
		printf("===================================================================================\n");
		list_for_each (iterator, &head){
			if (iterator->next == &head)
				printf("#%X: 0x%08X\tN/A ------\tN/A --\t\t%s\t%s\n", i++,
					list_entry(iterator, struct image_file, list)->address,
					list_entry(iterator, struct image_file, list)->loader?"Y":"N",
					list_entry(iterator, struct image_file, list)->name);
			else
				printf("#%X: 0x%08X\t0x%08X\t0x%04X\t\t%s\t%s\n", i++,
					list_entry(iterator, struct image_file, list)->address,
					list_entry(iterator->next, struct image_file, list)->address - list_entry(iterator, struct image_file, list)->address,
					(list_entry(iterator->next, struct image_file, list)->address - list_entry(iterator, struct image_file, list)->address)/page_size,
					list_entry(iterator, struct image_file, list)->loader?"Y":"N",
					list_entry(iterator, struct image_file, list)->name);
		}
	}
	else if (flash_type == nand) {
		printf("page_size = %d; chunk_size = %d; chunk_per_block = %d\n\n",
			page_size, chunk_size, chunk_per_block);
		printf("No. Address\tNAND Address\tPartition Size\tBlock Count\tLoader\tFile\n");
		printf("===================================================================================\n");
		list_for_each (iterator, &head){
			if (iterator->next == &head)
				printf("#%X: 0x%08X\t0x%08X\tN/A ------\tN/A --\t\t%s\t%s\n", i++,
					list_entry(iterator, struct image_file, list)->address,
					NAND_ADDRESS(list_entry(iterator, struct image_file, list)->address),
					list_entry(iterator, struct image_file, list)->loader?"Y":"N",
					list_entry(iterator, struct image_file, list)->name);
			else
				printf("#%X: 0x%08X\t0x%08X\t0x%08X\t0x%04X\t\t%s\t%s\n", i++,
					list_entry(iterator, struct image_file, list)->address,
					NAND_ADDRESS(list_entry(iterator, struct image_file, list)->address),
					list_entry(iterator->next, struct image_file, list)->address - list_entry(iterator, struct image_file, list)->address,
					(list_entry(iterator->next, struct image_file, list)->address - list_entry(iterator, struct image_file, list)->address)/BLOCK_SIZE,
					list_entry(iterator, struct image_file, list)->loader?"Y":"N",
					list_entry(iterator, struct image_file, list)->name);
		}
	}

	printf("===================================================================================\n");
	printf("Output File: %s\n\n", output_file_name);

	return 0;
}

static int generate_onebin()
{
	unsigned int i = 0, ecc_encode = 0, chunk_per_partition = 0;
	struct list_head *iterator;

	printf("Generating onebin .......................... \n");

	if (flash_type == nor)
		ecc_encode = 0;
	else if (flash_type == nand)
		ecc_encode = 1;
 
	/* Open Output File for Writing */ 
    	output_handle = open(output_file_name, O_WRONLY|O_TRUNC|O_CREAT, 0666);
	if (output_handle < 0) {
		fprintf(stderr, "unable to create output file (%s)\n", output_file_name);
		return 1;
	}

	list_for_each (iterator, &head){
		debug("#%X node (%p): 0x%08X\t0x%04X\t\t%s\n", i,
			list_entry(iterator, struct image_file, list),
			list_entry(iterator, struct image_file, list)->address,
			list_entry(iterator, struct image_file, list)->loader,
			list_entry(iterator, struct image_file, list)->name);

		if (iterator->next != &head) {
			if (flash_type == nor) {
				chunk_per_partition = 
					(list_entry(iterator->next, struct image_file, list)->address -
					list_entry(iterator, struct image_file, list)->address) / page_size;
			}
			else if (flash_type == nand) {
				chunk_per_partition = 
					(list_entry(iterator->next, struct image_file, list)->address -
					list_entry(iterator, struct image_file, list)->address) / BLOCK_SIZE * chunk_per_block;
			}
			printf("[%X] Appending file: %s (%d chunks) ... \n", i, list_entry(iterator, struct image_file, list)->name, chunk_per_partition);
		}
		else {
			chunk_per_partition = NO_LIMITED_CHUNKS;
			printf("[%X] Appending file: %s (no limited chunks) ... \n", i, list_entry(iterator, struct image_file, list)->name);
		}

		if (list_entry(iterator, struct image_file, list)->loader) {
			if( file_write_out(list_entry(iterator, struct image_file, list)->name, 0, chunk_per_partition) ) {
				printf("    ---> Fail\n");
				show_usage();
				return 1;
			} else {
				printf("    ---> Done\n\n");
			}
		}
		else {
			if( file_write_out(list_entry(iterator, struct image_file, list)->name, ecc_encode, chunk_per_partition) ) {
				printf("    ---> Fail\n");
				show_usage();
				return 1;
			} else {
				printf("    ---> Done\n\n");
			}
		}
		i++;
	}

	/* Close Output File */ 
	close(output_handle);
	
	return 0;
}


/*
 *
 * main
 *
 */
int main(int argc, char *argv[])
{
	int error_out = 0;
    
	char *file_name			= NULL;
	unsigned int file_address	= 0;

	printf("\n###### onebin Tool %s ######\n", ONEBIN_VERSION);
	if (argc==1) {
		show_usage(argv[0]);
		return 0;
	}

	while (*(++argv) != NULL) {
		if ((strcmp(*argv, "--flash-type") == 0) || (strcmp(*argv, "-ft") == 0)) {
			if (strcmp(*(++argv), "nor") == 0) {
				flash_type = nor;
			}
			else if (strcmp(*argv, "nand") == 0) {
				flash_type = nand;
			}
			else {
				fprintf(stderr, "Unknown Flash Type\n");
				error_out |= 1;
			}
		} else if ((strcmp(*argv, "--nand-page-type") == 0) || (strcmp(*argv, "-npt") == 0)) {
			if (strcmp(*(++argv), "small") == 0) {
				nand_page_type = small;
			}
			else if (strcmp(*argv, "large") == 0) {
				nand_page_type = large;
			}
			else {
				fprintf(stderr, "Unknown NAND Page Type\n");
				error_out |= 1;
			}
		} else if ((strcmp(*argv, "--chunk-size") == 0) || (strcmp(*argv, "-csz") == 0)) {
			chunk_size = strtoul(*(++argv), NULL, 0);
		} else if ((strcmp(*argv, "--chunk-per-block") == 0) || (strcmp(*argv, "-ppb") == 0)) {
			chunk_per_block = strtoul(*(++argv), NULL, 0);
 		} else if ((strcmp(*argv, "--bbi-swap-offset") == 0) || (strcmp(*argv, "-bso") == 0)) {
			bbi_swap_offset = strtoul(*(++argv), NULL, 0);
		} else if ((strcmp(*argv, "--bbi-dma-offset") == 0) || (strcmp(*argv, "-bdo") == 0)) {
			bbi_dma_offset = strtoul(*(++argv), NULL, 0);
		} else if ((strcmp(*argv, "--loader") == 0) || (strcmp(*argv, "-l") == 0)) {
			//loader_file_name = *(++argv);
			file_name = *++argv;
			if (add_to_file_list(file_name, 0, 1)){
				printf("Error! Add File Fail ..\n.");
				return 1;
			}
		} else if ((strcmp(*argv, "-f") == 0) ||
			(strcmp(*argv, "-f1") == 0) || (strcmp(*argv, "-f2") == 0) ||	/* Accept f1/f2/f3/f4 to be compatible with previous version */
			(strcmp(*argv, "-f3") == 0) || (strcmp(*argv, "-f4") == 0)) {
			file_name = *(++argv);
			file_address = strtoul(*(++argv), NULL, 16);
			if (add_to_file_list(file_name, file_address, 0)){
				printf("Error! Add File Fail ...\n");
				return 1;
			}
		} else if (strcmp(*argv, "-o") == 0) {
			output_file_name = *(++argv);
		} else {
			fprintf(stderr, "unknown options '%s'\n", *argv);
		}
	}

	if (flash_type == nor) {
		page_size = NOR_SECTOR_SIZE;
	}
	else if (flash_type == nand) {
		/* select bch6 by default */
		ecc_encoder = new ecc_bch6_encode_t();
		page_size = ecc_encoder->get_page_size();
		oob_size = ecc_encoder->get_oob_size();
		ecc_size = ecc_encoder->get_ecc_size();
		unit_size = ecc_encoder->get_unit_size();
	}

	if (check_parameters(error_out)) {
		clear_file_list();
		show_usage();
		return 1;
	}

	display_flash_info();

	generate_onebin();

	clear_file_list();

	printf("onebin Tool Done\n");
	return 0;
}

