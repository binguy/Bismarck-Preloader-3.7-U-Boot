#include <arpa/inet.h>
#include <bch6.h>
#include <conf.h>
#include <util.h>

#define BBI_GOODBLOCK       (0xFFFF)
#define RESERVE             (0xFFFF)
#define round_to_fl_page(n) (((n)+fl_data_size_per_page-1)/fl_data_size_per_page)
#define round_to_page(n)    (((n)+page_size-1)/page_size)
#define round_to_block(n)   (((n)+page_per_block-1)/page_per_block)
#define BCH6_SECTOR_SIZE    (2048)
#define BCH6_UNIT_PER_PAGE  (BCH6_SECTOR_SIZE/BCH6_PAGE_SIZE)

// FIXME
//   2K =>  |sector(512)|sector(512)|sector(512)|sector(512)|bbi(2)-oob(4)|oob(6)|oob(6)|ecc(10)|ecc(10)|ecc(10)|ecc(10)|

uint32_t fl_data_size_per_sector = 512;
uint32_t fl_data_size_per_page   = 1024;
uint32_t page_size               = 2048;
uint32_t sector_size             = 0;
uint32_t oob_unit_size           = BCH6_OOB_SIZE;
uint32_t ecc_unit_size           = BCH6_ECC_SIZE;
uint32_t bch6_unit_size          = BCH6_UNIT_SIZE;
uint32_t page_size_with_spare    = 2112;
uint32_t unit_per_page           = 0;
uint32_t page_per_block          = 64;

#define RETRIEVE_LEN(total, count, unit) (((total)-(count)) >= (unit))?(unit):((total)-(count))

void bch_init_parameters(uint32_t psz)
{
    page_size               = psz;
    sector_size             = BCH6_PAGE_SIZE;
    oob_unit_size           = BCH6_OOB_SIZE;
    ecc_unit_size           = BCH6_ECC_SIZE;
    bch6_unit_size          = BCH6_UNIT_SIZE;
    unit_per_page           = page_size/sector_size;
    page_size_with_spare    = unit_per_page * bch6_unit_size;

    fl_data_size_per_sector = BCH6_PAGE_SIZE;
    fl_data_size_per_page   = BCH6_PAGE_SIZE*2;
    printf("II: page size = %d\n", psz);
}

typedef plr_oob_t oob_t;
typedef plr_oob_4kpage_t oob_4kpage_t;

void bch_set_fl_oob32(char *oob_location, uint32_t value32_le) {
    char *t=oob_location;
    uint32_t v=value32_le;
    *(t++)=(v>>24)&0xff;
    *(t++)=(v>>16)&0xff;
    *(t++)=(v>>8)&0xff;
    *(t++)=(v&0xff);
}

void bch_set_fl_oob16(char *oob_location, uint16_t value16_le) {
    char *t=oob_location;
    uint32_t v=value16_le;
    *(t++)=(v>>8)&0xff;
    *(t++)=(v&0xff);
}

void bch6_ecc_512B_encode(uint8_t *ecc,               // ecc: output 10 bytes of ECC code
                          const uint8_t *input_buf,   // input_buf: the 512 bytes input data (BCH6_PAGE_SIZE bytes)
                          const uint8_t *oob);        // oob: 6 bytes out-of-band for input (BCH6_OOB_SIZE bytes)

static
void bch_append_ecc_one_page(uint8_t *buf, const uint8_t *data, uint32_t len, const uint8_t *spare) {
    struct page_layout {
        uint8_t local_page_data[page_size];
        uint8_t local_spare[unit_per_page*oob_unit_size];
        uint8_t ecc[unit_per_page*ecc_unit_size];
    } *l = (struct page_layout *)buf;
    uint32_t i;
    // copy source into local buffer
    memset(l->local_page_data, 0xff, sizeof(l->local_page_data));

    assert(data!=NULL && spare!=NULL);

    memcpy(l->local_page_data, data, len);

    memcpy(l->local_spare, spare, sizeof(l->local_spare));

    // compute ECC and write out
    for (i=0; i<unit_per_page; i++) {
        bch6_ecc_512B_encode(l->ecc+(i*ecc_unit_size),
                             l->local_page_data+(i*sector_size),
                             spare+(i*oob_unit_size));
    }
}

int bch_gen_fdt(int fd, BCH_INFO_T *info)
{
    assert(fd>=0);
    uint8_t *buf;
    int cnt, cn, idx, r;
    uint32_t len;
    if (0!=fdtbch_malloc(&buf, page_size_with_spare)) {
        return -1;
    }

    plr_first_load_layout_t *fl_layout;
    void *fdt = info->img;
    uint32_t nop = (round_to_fl_page(info->img_size));  // number of pages
    for (r=0, idx=0; r<info->img_copies; r++)
    {
        cnt = cn = 0;
        while (cnt < info->img_size) {
            memset(buf, 0xff, page_size_with_spare);
            fl_layout = (plr_first_load_layout_t *)buf;

            bch_set_fl_oob32(PLR_FL_SIG_LOCATION(fl_layout), ~(FDT_MAGIC));
            bch_set_fl_oob16(PLR_FL_TOTAL_NUM_LOCATION(fl_layout), nop);
            bch_set_fl_oob16(PLR_FL_PAGE_INDEX_LOCATION(fl_layout), cn);

            len = RETRIEVE_LEN(info->img_size, cnt, fl_data_size_per_sector);
            memcpy(fl_layout->data0, fdt+cnt, len);
            cnt += len;

            len = RETRIEVE_LEN(info->img_size, cnt, fl_data_size_per_sector);
            memcpy(fl_layout->data1, fdt+cnt, len);
            cnt += len;

            bch6_ecc_512B_encode((uint8_t*)fl_layout->syndrome0, (uint8_t*)fl_layout->data0, (uint8_t*)fl_layout->oob0);
            bch6_ecc_512B_encode((uint8_t*)fl_layout->syndrome1, (uint8_t*)fl_layout->data1, (uint8_t*)fl_layout->oob1);

            pwrite(fd, buf, page_size_with_spare, info->img_off+idx*page_size_with_spare);
            cn++;idx++;
        }
    }
    info->encode_img_size = idx * page_size_with_spare;
    return 0;
}

uint32_t get_first_load_signature(void *addr)
{
    basic_io_t *bios = addr + OTTO_HEADER_OFFSET;

    return ntohl(bios->header.signature);
}

uint32_t get_first_load_bytes(void *addr)
{
    basic_io_t *bios = addr + OTTO_HEADER_OFFSET;

    return ntohl(bios->size_of_plr_load_firstly);
}

int bch_gen_plr(int fd, BCH_INFO_T *info)
{
    assert(fd>=0);

    void *plr = info->img;

    /* following is referenced from bchenc  */
    uint32_t signature = get_first_load_signature(plr);
    /* Check the FL signature */
    if (SIGNATURE_PLR_FL != signature) {
        printf("EE: Unknown Signature (0x%x)\n", signature);
        return -1;
    }

    /* Find "size_of_plr_load_firstly in bytes" */
    uint32_t first_load_bytes = get_first_load_bytes(plr);
    uint32_t plr_load_bytes = info->img_size - first_load_bytes;

    /* Set signature & total chunks to OOB */
    plr_first_load_layout_t *fl_layout;
    uint8_t *buf;
    if (0!=fdtbch_malloc(&buf, page_size_with_spare)) {
        return -1;
    }
    fl_layout = (plr_first_load_layout_t *)buf;
    uint32_t num_of_fl_page = (round_to_fl_page(first_load_bytes));

    /* Write content and  Set chunk_index to OOB */
    uint32_t r, cnt, cn, len, idx;
    for (r=0, idx=0; r<info->img_copies; ++r) {
        cn=cnt=len=0;
        plr=info->img;
        while (cn < num_of_fl_page){
            memset(buf, 0xff, page_size_with_spare);

            bch_set_fl_oob32(PLR_FL_SIG_LOCATION(fl_layout), SIGNATURE_PLR_FL);
            bch_set_fl_oob16(PLR_FL_TOTAL_NUM_LOCATION(fl_layout), num_of_fl_page);
            bch_set_fl_oob16(PLR_FL_PAGE_INDEX_LOCATION(fl_layout), cn);

            len = RETRIEVE_LEN(first_load_bytes, cnt, fl_data_size_per_sector);
            memcpy((fl_layout)->data0, plr+cnt, len);
            cnt += len;

            len = RETRIEVE_LEN(first_load_bytes, cnt, fl_data_size_per_sector);
            memcpy((fl_layout)->data1, plr+cnt, len);
            cnt += len;

            bch6_ecc_512B_encode((uint8_t*)fl_layout->syndrome0, (uint8_t*)fl_layout->data0, (uint8_t*)fl_layout->oob0);
            bch6_ecc_512B_encode((uint8_t*)fl_layout->syndrome1, (uint8_t*)fl_layout->data1, (uint8_t*)fl_layout->oob1);

            pwrite(fd, buf, page_size_with_spare, info->img_off+idx*page_size_with_spare);
            cn++;idx++;
            //dbg_print(buf, page_size_with_spare, "FL");
        }

        oob_4kpage_t oob;
        memset((void*)&oob, 0xff, sizeof(oob_4kpage_t));

        oob.plr_oob.bbi         = BBI_GOODBLOCK;
        oob.plr_oob.reserved    = RESERVE;
        oob.plr_oob.signature   = htonl(SIGNATURE_PLR);
        oob.plr_oob.startup_num = htonl(num_of_fl_page);
        oob.plr_oob.total_chunk = htonl(round_to_page(plr_load_bytes)+num_of_fl_page);
        oob.plr_oob.num_copy    = htonl(info->img_copies);

        cn=(r<<16)|cn;
        plr = info->img + first_load_bytes;
        cnt = 0;
        while (cnt < plr_load_bytes) {
            memset(buf, 0xff, page_size_with_spare);
            oob.plr_oob.chunk_num=htonl(cn);
            len = RETRIEVE_LEN(plr_load_bytes, cnt, page_size);
            bch_append_ecc_one_page(buf, plr+cnt, len, (uint8_t*)&oob);
            cnt+=len;
            pwrite(fd, buf, page_size_with_spare, info->img_off+idx*page_size_with_spare);
            cn++;idx++;
        }
    }

    // pending to block alignment
    cnt = lseek(fd, 0, SEEK_END)%(page_per_block*page_size_with_spare);
    len = page_per_block*page_size_with_spare;
    memset(buf, 0xff, page_size_with_spare);
    while (cnt < len) {
        pwrite(fd, buf, page_size_with_spare, info->img_off+idx*page_size_with_spare);
        cnt+=page_size_with_spare;
        idx++;
    }

    fprintf(stderr, "bchenc: FL pages(%d)\n", num_of_fl_page);

    // update information
    info->encode_img_size = idx * page_size_with_spare;
    return 0;
}

int bch_gen_ubt(int fd, BCH_INFO_T *info)
{
    oob_4kpage_t oob;
    memset((void*)&oob, 0xff, sizeof(oob_4kpage_t));

    uint8_t *buf;
    if (0!=fdtbch_malloc(&buf, page_size_with_spare)) {
        return -1;
    }

    uint8_t *ubt;

    oob.plr_oob.bbi=BBI_GOODBLOCK;
    oob.plr_oob.num_copy=htonl(info->img_copies);
    oob.plr_oob.signature=htonl(SIGNATURE_UBOOT);
    oob.plr_oob.total_chunk=htonl(round_to_page(info->img_size));
    oob.plr_oob.startup_num=0xFFFFFFFF;

    uint32_t r, cnt, cn, len, idx;
    for (r=0, idx=0; r<info->img_copies; ++r) {
        cn = r<<16;
        cnt = 0;
        ubt = info->img;
        while (cnt < info->img_size) {
            memset(buf, 0xff, page_size_with_spare);
            oob.plr_oob.chunk_num=htonl(cn);
            len = RETRIEVE_LEN(info->img_size, cnt, page_size);
            bch_append_ecc_one_page(buf, ubt+cnt, len, (uint8_t*)&oob);
            cnt+=len;
            pwrite(fd, buf, page_size_with_spare, info->img_off+idx*page_size_with_spare);
            cn++;idx++;
        }
    }
    return 0;
}
