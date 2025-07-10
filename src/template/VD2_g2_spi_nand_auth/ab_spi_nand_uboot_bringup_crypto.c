#include <spi_nand/spi_nand_util.h>
#include <ecc/ecc_ctrl.h>
#include <init_define.h>
#include <lib/lzma/tlzma.h>
#include <dram/memcntlr_util.h>
#include <misc/osc.h>
#include <sodium.h>
#include <fdt/fdt.h>

u32_t get_pkey_info(u8_t *pk)
{
    u32_t entry;
    u16_t key_per_entry;
    u8_t *ptr8 = pk;
    u32_t count = 0;

    for(entry=40 ; entry<=55 ; entry++,ptr8+=2){
        REG32(0xb8000644) = (0x01<<24)|(entry<<16);
        REG32(0xb8000644) = (0x00<<24)|(entry<<16);
        mdelay(1);
        key_per_entry = REG32(0xb8000648)&0xFFFF;
        memcpy(ptr8, (u8_t *)&key_per_entry, sizeof(u16_t));
        count += key_per_entry;
    }
    return (count==0)?0:1;
}

int fdt_probe(void *fdt, u32_t dev_type)
{
    if(CRYPTO_NAND_DEVICE == dev_type) {
        u32_t blk_page_idx=0;
        int num_page_to_copy=-1;
        u32_t page_log=0;
        u32_t max_fl_try_page_num = SNAF_NUM_OF_BLOCK(_spi_nand_info)*SNAF_NUM_OF_PAGE_PER_BLK(_spi_nand_info);
        plr_first_load_layout_t *plr_fl_buf=(plr_first_load_layout_t *)page_buffer;
        while (blk_page_idx<max_fl_try_page_num) {
            nsu_page_read(_spi_nand_info, plr_fl_buf, blk_page_idx++);
            if(nsu_ecc_engine_action(((u32_t)_spi_nand_info->_ecc_ability), plr_fl_buf->data0, plr_fl_buf->oob0, 0) == ECC_CTRL_ERR) return ECC_CTRL_ERR;
            if(nsu_ecc_engine_action(((u32_t)_spi_nand_info->_ecc_ability), plr_fl_buf->data1, plr_fl_buf->oob1, 0) == ECC_CTRL_ERR) return ECC_CTRL_ERR;

            // check signature
            if (PLR_FL_GET_SIGNATURE(plr_fl_buf)!=~(FDT_MAGIC)) continue;

            // set num_page_to_copy
            if (num_page_to_copy<0) {
                num_page_to_copy=PLR_FL_GET_TOTAL_NUM_PAGE(plr_fl_buf);
            }
            // copy and log page
            uint16_t index=PLR_FL_GET_PAGE_INDEX(plr_fl_buf);
            if (index>=PLR_FL_GET_TOTAL_NUM_PAGE(plr_fl_buf)) continue;   // unreasonable page index
            u32_t log_index=1<<index;
            if ((page_log&log_index)==0) {
                page_log |= log_index;
                char *plr_buf =(char*)(fdt+(index*PLR_FL_PAGE_USAGE));
                memcpy(plr_buf, plr_fl_buf->data0, BCH_SECTOR_SIZE);
                memcpy(plr_buf+BCH_SECTOR_SIZE, plr_fl_buf->data1, BCH_SECTOR_SIZE);
                if (0==(--num_page_to_copy)) break;
            }
        }
    } else {
        struct fdt_header *fhdr = (struct fdt_header *)OTTO_SPI_NOR_START;
        memcpy(fdt, (char*)OTTO_SPI_NOR_START, fhdr->totalsize);
    }

    return 0;
}

int auth_signature_verify(u32_t img, const u32_t size, u8_t *sign, u8_t *key)
{
    u8_t *addr = (u8_t *)img;
    u32_t i=0, len;
    crypto_sign_state state;
    crypto_sign_init(&state);
    while(i < size) {
        if((size-i)>=BLK_SIZE)  len = BLK_SIZE;
        else len = size-i;
        crypto_sign_update(&state, (const unsigned char *)(addr+i), len);
        i += len;
    }
    if (crypto_sign_final_verify(&state, sign, key) != 0) {
        /* message forged! */
        return 1;
    }
    return 0;
}


void auth_mod_verify_img(void)
{
    void *fdt = alloca(2048);
    struct fdt_header *fhdr = (struct fdt_header *)fdt;
    CRYPTO_HEADER_T *shdr;
    uint8_t pk[crypto_sign_PUBLICKEYBYTES];

    // Check if the public key is existed in EFUSE.
    if(!get_pkey_info(pk)){
        puts("II: Continue to normal boot mode\n");
        return;
    }
    fdt_probe(fdt, CRYPTO_NAND_DEVICE);
   if(0==fdt_check_header(fhdr))  { // if image header is FDT
        fhdr->dt_struct = (uint32_t *)(fdt + fhdr->off_dt_struct);
        fhdr->dt_strings = fdt + fhdr->off_dt_strings;
        uint32_t img_size = *(uint32_t *)fdt_get_prop(fhdr, "/images/uboot/data-size");

        // get sideband information
        shdr = (CRYPTO_HEADER_T *)fdt_get_prop(fhdr, "/images/uboot/sign");
        memcpy(&shdr->sign_pk, pk, sizeof(shdr->sign_pk));
        puts("II: Verify Image ... ");

        if(0!=auth_signature_verify((ECC_DECODED_UBOOT_ADDR), img_size, shdr->signu, shdr->sign_pk)) {
            puts("Fail\n");
            goto error;
        }
        puts("OK\n");

        if(CRYOPT_SIG_ENC == shdr->type) {
            puts("EE: Encryped signature is unsupported.\n");
            goto error;
        }
        return;
    }
    puts("EE: Unknown image header type \n");
error:
    while(1);
}

s32_t ecc_uboot_to_dram(void)
{
    int ecc_sts;
    u32_t start_blk = 1;
    u32_t blk_pge_addr;
    u32_t max_blk_pge = SNAF_NUM_OF_BLOCK(_spi_nand_info)*SNAF_NUM_OF_PAGE_PER_BLK(_spi_nand_info);
    u32_t page_size = SNAF_PAGE_SIZE(_spi_nand_info);
    u32_t need_cpy;              //Denote which chunk need to be copied to DRAM
    u32_t total_uboot_pages = 0;//OOB
    u32_t total_uboot_copies = 0;//OOB
    u32_t cnt_valid_page = 0;   //Used to count how many chunks be loaded successfully
    u32_t page_idx = 0;         //Used to count how many chunks be checked
    u32_t uboot_idx= 0;       //Used to count if there is another UBOOT can be read
    u32_t dma_addr;
    u8_t page_valid_table[(MAX_UBOOT_SIZE/page_size)];   //Used to denote which chunk is readed
    oob_t *oob = (oob_t *)oob_buffer;

    inline_bzero(page_valid_table, sizeof(page_valid_table));
    for(blk_pge_addr=(start_blk<<6); blk_pge_addr<max_blk_pge; blk_pge_addr++){
        need_cpy = 0;
        ecc_sts = nsu_page_read_ecc(_spi_nand_info, page_buffer, blk_pge_addr, ecc_buffer);
        if(ECC_CTRL_ERR == ecc_sts) continue;
        page_idx = oob->idx_chunk;

        if(0 == total_uboot_pages){
            if(SIGNATURE_UBOOT != oob->signature) continue;
            total_uboot_pages  = oob->total_chunk;
            total_uboot_copies = oob->num_copy;
            uboot_idx = oob->idx_copy;
            if(((uboot_idx+1) == total_uboot_copies) && ((page_idx+1) == total_uboot_pages)) return -1;
            need_cpy = 1;
        }else if(0 == page_valid_table[page_idx]){
            if(SIGNATURE_UBOOT != oob->signature) continue;
            need_cpy = 1;
        }

        if(1 == need_cpy){
            #ifdef DBG_PRINT_SNAF_BRINGUP
                printf("DD: blk[%d], page[%2d], (0x08%x)\n",SNAF_BLOCK_ADDR(blk_pge_addr),SNAF_PAGE_ADDR(blk_pge_addr),blk_pge_addr);
            #endif
            dma_addr = ECC_DECODED_UBOOT_ADDR + (page_size*page_idx);
            inline_memcpy(dma_addr, page_buffer, page_size);
            page_valid_table[page_idx] = 1;
            cnt_valid_page++;
            if(cnt_valid_page == total_uboot_pages){
                dcache_wr_inv_all();
                icache_inv_all();
                return total_uboot_pages;
            }
        }

        if(((uboot_idx+1) == total_uboot_copies) && ((page_idx+1) == total_uboot_pages)) return -2;
    }
    return -3;
}

void uboot_bring_up(void)
{
    void *uboot_inflate_addr, *load_addr, *entry_addr, *mcp_src_addr;
    u32_t mcp_size, comp_type;
    uimage_header_t *uhdr_info;
    otto_soc_context_t otto_sc;

    load_addr = entry_addr = uboot_inflate_addr = (void *)UBOOT_DECOMP_ADDR;
    uhdr_info = (uimage_header_t *)ECC_DECODED_UBOOT_ADDR;
    s32_t total_uboot_pages= ecc_uboot_to_dram();

    if(total_uboot_pages < 0){
        printf("EE: Loading Uboot Fail (%d)!!\n",total_uboot_pages);
        while(1);
    }

    /* Authenticate u-boot */
    auth_mod_verify_img();

    if((UIMG_MAGIC_NUM != uhdr_info->ih_magic) && (MAGIC_UBOOT_2011 != uhdr_info->ih_magic)) {
        /* U-boot image header does NOT exists, Copy the RAW data from 7MB-DDR to 0MB-DDR */
        mcp_src_addr = (void *)ECC_DECODED_UBOOT_ADDR;
        mcp_size = total_uboot_pages*SNAF_PAGE_SIZE(_spi_nand_info);
        printf("II: Copying %dKB deflated U-Boot (%p -> %p) ... ",mcp_size>>10, mcp_src_addr, uboot_inflate_addr);
        inline_memcpy(uboot_inflate_addr, mcp_src_addr, mcp_size);
    }else{
        /* Copy the U-boot based on the information in header structure */
        mcp_src_addr = (void *)(uhdr_info+1);
        mcp_size = uhdr_info->ih_size;
        entry_addr = (void *)uhdr_info->ih_ep;
        load_addr = (void *)uhdr_info->ih_load;
        comp_type = uhdr_info->ih_comp;

        printf("II: U-boot Magic Number is 0x%x\n",uhdr_info->ih_magic);
        if(comp_type == UIH_COMP_LZMA){
            printf("II: Inflating U-Boot (%p -> %p)... ",mcp_src_addr, load_addr);
            s32_t res;
            if(ISTAT_GET(cal) == MEM_CAL_OK){
                res = lzma_decompress((u8_t *)mcp_src_addr, (u8_t *)uboot_inflate_addr, &mcp_size);
            }else{
                res = lzma_chsp_jump((u8_t *)mcp_src_addr, (u8_t *)uboot_inflate_addr, &mcp_size, NEW_STACK_AT_DRAM);
            }

            if(res != DECOMPRESS_OK) {
                printf("\nEE: decompress failed: %d\n", res);
                while (1);
            }
        }else if(comp_type == UIH_COMP_NONE){
            printf("II: Copying %dKB inflated U-Boot (%p -> %p)... ",mcp_size>>10, mcp_src_addr, load_addr);
            inline_memcpy(uboot_inflate_addr, mcp_src_addr, mcp_size);
        }else{
            puts("EE: Unsupported U-Boot format, copy it as uncompressed.\n");
            while(1);
        }
    }

    if(uboot_inflate_addr != load_addr){
        inline_backward_memcpy(load_addr, uboot_inflate_addr, mcp_size);
    }
    puts("OK\n");
    puts("II: Starting U-boot... \n");

    osc_init(&otto_sc);

    dcache_wr_inv_all();
    icache_inv_all();
    ((fpv_u32_t *)(entry_addr))((u32_t)&otto_sc);
    puts("EE: Should not run to here... \n");
    while(1); // should never return
}

REG_INIT_FUNC(uboot_bring_up, 38);

