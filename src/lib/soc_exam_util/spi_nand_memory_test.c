#include <spi_nand/spi_nand_util.h>
#include <spi_nand/spi_nand_ctrl.h>
#include <spi_nand/spi_nand_common.h>
#include <ecc/ecc_struct.h>
#include <util.h>

#ifndef SECTION_SPI_NAND_MT
    #define SECTION_SPI_NAND_MT
#endif


typedef struct {
	u32_t blk_start;
	u32_t blk_end;
} snaf_mt_info_t;


#define BLOCK_PAGE_ADDR(block_0_1023, page_0_63) ((0x00<<16)|(block_0_1023<<6)|(page_0_63))

const u32_t flash_patterns[] = {
    0x01010101,
    0xfefefefe,
    0x00000000,
    0xff00ff00,
    0x00ff00ff,
    0x0000ffff,
    0xffff0000,
    0xffffffff,
    0x5a5aa5a5,
    0xa5a5a5a5,
    0x55555555,
    0xaaaaaaaa,
    0x01234567,
    0x76543210,
    0x89abcdef,
    0xfedcba98,
};

SECTION_SPI_NAND_MT int _mt_bad_block_search(spi_nand_flash_info_t *fi, u32_t bi) 
{
    u32_t buf[2];  
    
    if(0!=nsu_block_erase(fi, BLOCK_PAGE_ADDR(bi, 0))){
        printf("II: BBI: %04d\n", bi);
        return -1;
    }else{
        nsu_pio_read(fi, buf, 4, BLOCK_PAGE_ADDR(bi, 0), 0x800);
        if(0xFFFFFFFF != buf[0]){
            printf("II: BBI: %04d\n", bi);
            return -1;
        }
     }
    return 0;
}

SECTION_SPI_NAND_MT void ram_content_init(u32_t pattern, u32_t page_size_with_spare)
{
    volatile u32_t *src_adr = (volatile u32_t *)src_page_buf;
    volatile u32_t *dst_adr = (volatile u32_t *)chk_page_buf;
    u32_t i;
    for(i=0 ; i<(page_size_with_spare/4) ; i++){
        *(src_adr+i) = pattern; 
        *(dst_adr+i) = 0x00000000;
    }
    if((2048+64) == page_size_with_spare) *(src_adr+(2048/4)) = 0xFFFFFFFF; 
    else if((4096+128) == page_size_with_spare) *(src_adr+(4096/4)) = 0xFFFFFFFF; 
}

SECTION_SPI_NAND_MT void page_dma_write_read_with_ecc_test(spi_nand_flash_info_t *fi, snaf_mt_info_t *ti)
{
    int ret_sts=0;
    u32_t bi, pi, i, blk_pge_addr;
    u32_t src_val, chk_val, err_cnt=0;
    u32_t page_size_with_spare  = SNAF_PAGE_SIZE(fi) + SNAF_SPARE_SIZE(fi);

    printf("II: Start %s\n",__FUNCTION__);

    for(i=0; i<=(sizeof(flash_patterns)/sizeof(u32_t)) ; i++){
        for(bi=ti->blk_start ; bi<=ti->blk_end ; bi++){
            // 1. Check BBI and then erase
            blk_pge_addr = BLOCK_PAGE_ADDR(bi,0);
            if(0 != _mt_bad_block_search(fi, bi)){
                continue;
            } 


            // 2. Doing Chunk Write/Read between sram and spi nand (ECC)
            for(pi=0 ; pi<=63 ; pi++){ //page
                ram_content_init(flash_patterns[i], page_size_with_spare);
                
                blk_pge_addr = BLOCK_PAGE_ADDR(bi,pi);
                ret_sts = nsu_page_write_ecc(fi, src_page_buf, blk_pge_addr, mt_ecc_buf);
                if(ret_sts != 0){
                    printf("EE: page_write FAIL, blk%04d page%04d\n", bi, pi);
                    continue;
                } 
                
                ret_sts  = nsu_page_read_ecc(fi, chk_page_buf, blk_pge_addr, mt_ecc_buf);
                if(IS_ECC_DECODE_FAIL(ret_sts)){
                    printf("EE: ECC Fail(0x%x), blk%04d page%04d\n",ret_sts, bi, pi);
                    continue;
                }

                err_cnt=0;
                for(i=0 ; i<(page_size_with_spare/4) ; i++){
                    src_val = *(volatile u32_t *)(src_page_buf+i*4); 
                    chk_val = *(volatile u32_t *)(chk_page_buf+i*4);
                    if(chk_val != src_val){
                        printf("EE: blk%04d page%04d, src(0x%x):0x%8x != chk(0x%x):0x%8x\n",bi, pi, (u32_t)(src_page_buf+i),src_val,(u32_t)(chk_page_buf+i),chk_val);
                        if(++err_cnt == 5){
                            printf("EE: More than 5 words error!!!\n");
                            continue;
                        } 
                    }
                }
                if(!err_cnt) printf("\r    blk%04d page%04d OK",bi, pi);
            }
        }
    }  
    puts("\n"); 
}

SECTION_SPI_NAND_MT void pio_write_read_with_ecc_test(spi_nand_flash_info_t *fi, snaf_mt_info_t *ti)
{
    int ret_sts=0;
    u32_t bi, pi, i, blk_pge_addr;
    u32_t src_val, chk_val, err_cnt=0;
    u32_t page_size_with_spare  = SNAF_PAGE_SIZE(fi) + SNAF_SPARE_SIZE(fi);
 
    printf("II: Start %s\n",__FUNCTION__);

    for(i=0; i<=(sizeof(flash_patterns)/sizeof(u32_t)); i++){
        for(bi=ti->blk_start ; bi<=ti->blk_end ; bi++){
            // 1. Check BBI and then erase
            blk_pge_addr = BLOCK_PAGE_ADDR(bi,0);
            if(0 != _mt_bad_block_search(fi, bi)){
                continue;
            } 

            // 2. Doing PIO Write/Read between sram and spi nand (ECC)
            for(pi=0 ; pi<=63 ; pi++){ //page
                ram_content_init(flash_patterns[i], page_size_with_spare);
                
                blk_pge_addr = BLOCK_PAGE_ADDR(bi,pi);
                nsu_ecc_encode(ECC_CORRECT_BITS(fi), src_page_buf, mt_ecc_buf);
                ret_sts = nsu_pio_write(fi, src_page_buf, page_size_with_spare, blk_pge_addr, 0);
                if(ret_sts != 0){
                    printf("EE: pio_write FAIL, blk%04d page%04d\n", bi, pi);
                    continue;
                }
                
                nsu_pio_read(fi, chk_page_buf, page_size_with_spare, blk_pge_addr, 0);
                ret_sts  = nsu_ecc_decode(ECC_CORRECT_BITS(fi), chk_page_buf, mt_ecc_buf);
                if(IS_ECC_DECODE_FAIL(ret_sts)){
                    printf("EE: ECC Fail(0x%x), blk%04d page%04d\n",ret_sts, bi, pi);
                    continue;
                }
                
                err_cnt=0;
                for(i=0 ; i<(page_size_with_spare/4) ; i++){
                    src_val = *(volatile u32_t *)(src_page_buf+i*4); 
                    chk_val = *(volatile u32_t *)(chk_page_buf+i*4);
                    if(chk_val != src_val){
                        printf("EE: blk%04d page%04d, src(0x%x):0x%8x != chk(0x%x):0x%8x\n",bi, pi, (u32_t)(src_page_buf+i*4),src_val,(u32_t)(chk_page_buf+i*4),chk_val);
                        if(++err_cnt == 5){
                            puts("EE: More than 5 words error!!!\n");
                            continue;
                        } 
                    }
                }
                if(!err_cnt) printf("\r    blk%04d page%04d OK",bi, pi);
            }
        }
    }   
    puts("\n");
}


#define SET_SEED 0
#define GET_SEED 1
/*
  get_or_set = GET_SEED: get seed
  get_or_set = SET_SEED: set seed
*/
static void __srandom32(u32_t *a1, u32_t *a2, u32_t *a3, u32_t get_or_set)
{
    static int s1, s2, s3;
    if(GET_SEED==get_or_set){
        *a1=s1;
        *a2=s2;
        *a3=s3;
    }else{
        s1 = *a1;
        s2 = *a2;
        s3 = *a3;
    }
}

static u32_t __random32(void)
{
    #define TAUSWORTHE(s,a,b,c,d) ((s&c)<<d) ^ (((s <<a) ^ s)>>b)
    u32_t s1, s2, s3;
    __srandom32(&s1, &s2, &s3, GET_SEED);

    s1 = TAUSWORTHE(s1, 13, 19, 4294967294UL, 12);
    s2 = TAUSWORTHE(s2, 2, 25, 4294967288UL, 4);
    s3 = TAUSWORTHE(s3, 3, 11, 4294967280UL, 17);

    __srandom32(&s1, &s2, &s3, SET_SEED);

    return (s1 ^ s2 ^ s3);
}

SECTION_SPI_NAND_MT void ram_random_data_init(u32_t pi, u32_t bi, u32_t page_size_with_spare)
{
    u32_t i;
    u32_t SEED1 =  ((0x13243*(pi+bi))&0xffffff);
    u32_t SEED2 =  (0xaaa0bdd+pi);
    u32_t SEED3 =  (0xfffbda0-pi);
    __srandom32(&SEED1, &SEED2, &SEED3, SET_SEED);
    for(i=0; i<page_size_with_spare/4; i++) {
        *(volatile u32_t *)(src_page_buf+i*4) = __random32();
    }

    volatile u32_t *src_adr = (volatile u32_t *)src_page_buf;
    if((2048+64) == page_size_with_spare) *(src_adr+(2048/4)) = 0xFFFFFFFF; 
    else if((4096+128) == page_size_with_spare) *(src_adr+(4096/4)) = 0xFFFFFFFF; 
}

SECTION_SPI_NAND_MT void pioW_dmaR_with_ecc_test(spi_nand_flash_info_t *fi, snaf_mt_info_t *ti)
{
    int ret_sts=0;
    u32_t bi, pi, i, blk_pge_addr;
    u32_t src_val, chk_val, err_cnt=0;
    u32_t page_size_with_spare  = SNAF_PAGE_SIZE(fi) + SNAF_SPARE_SIZE(fi);
    
    
    printf("II: Start %s\n",__FUNCTION__);

    for(bi=ti->blk_start ; bi<=ti->blk_end ; bi++){
        blk_pge_addr = BLOCK_PAGE_ADDR(bi,0);
        if(0 != _mt_bad_block_search(fi, bi)){
            continue;
        } 

        for(pi=0 ; pi<=63 ; pi++){ //page          
            ram_random_data_init(pi, bi, page_size_with_spare);
            dcache_wr_inv_all();

            //Pio write, DMA read
            blk_pge_addr = BLOCK_PAGE_ADDR(bi,pi);
            nsu_ecc_encode(ECC_CORRECT_BITS(fi), src_page_buf, mt_ecc_buf);
            ret_sts = nsu_pio_write(fi, src_page_buf, page_size_with_spare, blk_pge_addr, 0);
            if(ret_sts != 0){
                printf("\rEE: blk%04d page%04d, pio_write FAIL\n", bi, pi);
                continue;
            }
            
            ret_sts = nsu_page_read_ecc(fi, chk_page_buf, blk_pge_addr, mt_ecc_buf);
            if(IS_ECC_DECODE_FAIL(ret_sts)){
                printf("\rEE: blk%04d page%04d, ECC Fail(0x%x)\n",ret_sts, bi, pi);
                continue;
            }
            
            err_cnt=0;
            for(i=0 ; i<(page_size_with_spare/4) ; i++){
                src_val = *(volatile u32_t *)(src_page_buf+i*4); 
                chk_val = *(volatile u32_t *)(chk_page_buf+i*4);
                if(chk_val != src_val){
                    printf("\rEE: blk%04d page%04d, src(0x%x):0x%8x != chk(0x%x):0x%8x\n",bi, pi, (u32_t)(src_page_buf+i*4),src_val,(u32_t)(chk_page_buf+i*4),chk_val);
                    if(++err_cnt == 5){
                        puts("\rEE: More than 5 words error!!!\n");
                        continue;
                    } 
                }
            }
            if(!err_cnt) printf("\r    blk%04d page%04d OK",bi, pi);
        }
    }
    puts("\n");
}

SECTION_SPI_NAND_MT void dmaW_pioR_with_ecc_test(spi_nand_flash_info_t *fi, snaf_mt_info_t *ti)
{
    int ret_sts=0;
    u32_t bi, pi, i, blk_pge_addr;
    u32_t src_val, chk_val, err_cnt=0;
    u32_t page_size_with_spare  = SNAF_PAGE_SIZE(fi) + SNAF_SPARE_SIZE(fi);
    
    printf("II: Start %s\n",__FUNCTION__);

    for(bi=ti->blk_start ; bi<=ti->blk_end ; bi++){
        blk_pge_addr = BLOCK_PAGE_ADDR(bi,0);
        if(0 != _mt_bad_block_search(fi, bi)){
            continue;
        } 

        for(pi=0 ; pi<=63 ; pi++){ //page   
            ram_random_data_init(pi, bi, page_size_with_spare);
            dcache_wr_inv_all();

            //DMA write, PIO read
            blk_pge_addr = BLOCK_PAGE_ADDR(bi,pi);
            ret_sts = nsu_page_write_ecc(fi, src_page_buf, blk_pge_addr, mt_ecc_buf);
            if(ret_sts != 0){
                printf("\rEE: blk%04d page%04d, page_write_ecc FAIL\n", bi, pi);
                continue;
            }
            
            nsu_pio_read(fi, chk_page_buf, page_size_with_spare, blk_pge_addr, 0);
            ret_sts  = nsu_ecc_decode(ECC_CORRECT_BITS(fi), chk_page_buf, mt_ecc_buf);
            if(IS_ECC_DECODE_FAIL(ret_sts)){
                printf("\rEE: blk%04d page%04d, ECC Fail(0x%x)\n",ret_sts, bi, pi);
                continue;
            }
            
            err_cnt=0;
            for(i=0 ; i<(page_size_with_spare/4) ; i++){
                src_val = *(volatile u32_t *)(src_page_buf+i*4); 
                chk_val = *(volatile u32_t *)(chk_page_buf+i*4);
                if(chk_val != src_val){
                    printf("\rEE: blk%04d page%04d, src(0x%x):0x%8x != chk(0x%x):0x%8x\n",bi, pi, (u32_t)(src_page_buf+i*4),src_val,(u32_t)(chk_page_buf+i*4),chk_val);
                    if(++err_cnt == 5){
                        puts("\rEE: More than 5 words error!!!\n");
                        continue;
                    } 
                }
            }
            if(!err_cnt) printf("\r    blk%04d page%04d OK",bi, pi);
        }
    }
    puts("\n");
}


SECTION_SPI_NAND_MT void spi_nand_flash_memory_test(u32_t blk_start, u32_t blk_cnt, u32_t loops)
{
    snaf_mt_info_t mti;
    mti.blk_start = blk_start;
    mti.blk_end = blk_start+blk_cnt-1;

    printf("II: Start %s (block %d to %d):\n",__FUNCTION__,mti.blk_start, mti.blk_end);
    printf("    (Wcmd=0x%02x) (Rcmd=0x%02x) (src=0x%x) (chk=0x%x) (ecc=0x%x) (BCH%d)\n", _spi_nand_info->_cmd_info->w_cmd, _spi_nand_info->_cmd_info->r_cmd,(u32_t)src_page_buf, (u32_t)chk_page_buf, (u32_t)mt_ecc_buf, ECC_CORRECT_BITS((_spi_nand_info)));

    u32_t i;
    for(i=0; i<loops; i++){
        pio_write_read_with_ecc_test(_spi_nand_info, &mti);
        page_dma_write_read_with_ecc_test(_spi_nand_info, &mti);
        pioW_dmaR_with_ecc_test(_spi_nand_info, &mti);
        dmaW_pioR_with_ecc_test(_spi_nand_info, &mti);
    }
}


