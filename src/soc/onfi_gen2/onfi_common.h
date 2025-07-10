#ifndef ONFI_COMMON_H
#define ONFI_COMMON_H

/****************************************************/
/************** ONFI Flash Command Sets ***************/
/****************************************************/
#define CMD_READ_ID			0x90
#define CMD_READ_STATUS		0x70

#define CMD_RESET           0xFF

#define CMD_PG_READ_C1		0x00
#define CMD_PG_READ_C2		0x30

#define CMD_PG_WRITE_C1		0x80
#define CMD_PG_WRITE_C2		0x10

#define CMD_BLK_ERASE_C1	0x60	//Auto Block Erase Setup command
#define CMD_BLK_ERASE_C2	0xd0	//CMD_ERASE_START

#define CMD_FEATURE_SET_OP_ONFI 0xEF
#define CMD_FEATURE_GET_OP_ONFI 0xEE

//ONFI common code
s32_t ofc_read_id(void);
void ofc_reset_nand_chip(void);
s32_t ofc_block_erase(onfi_info_t *info, u32_t blk_page_idx);
void ofc_wait_nand_chip_ready(void);
u8_t ofc_status_read(void);
s32_t ofc_check_program_erase_status(void);
void ofc_set_feature(u32_t feature_addr, u32_t features);
u32_t ofc_get_feature(u32_t feature_addr);


#define DEFAULT_DATA_BASE (0xDEFA)


#endif //ONFI_COMMON_H
