/*
 * Copyright (C) 2020 Realtek
 */
#include <common.h>
#include <command.h>
#include <asm/io.h>
#include "cortina_smc.h"

#define OTP_READ_ROTPK_HASH    CA_SMC_CALL_OTP_READ_ROTPK
#define OTP_WRITE_ROTPK_HASH   CA_SMC_CALL_OTP_WRITE_ROTPK
#define OTP_READ_JTAG_PWD      CA_SMC_CALL_OTP_READ_JTAG_PWD
#define OTP_WRITE_JTAG_PWD     CA_SMC_CALL_OTP_WRITE_JTAG_PWD
#define OTP_READ_CHIP_MODE     CA_SMC_CALL_OTP_READ_CHIP_MODE
#define OTP_READ_DRAM_MODEL    CA_SMC_CALL_OTP_READ_DRAM_MODEL

int do_otp_command(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	const char *cmd;
    char *endp;
	int ret=0;
	u32 i=1;
	u8 *rbuf = (u8 *)0x05000000;
	u8 min_rclean_byte = 32;

	/* need at least two arguments */
	if (argc < 3) goto usage;

	cmd = argv[i++];

	if (strncmp(cmd, "write", 5) == 0) {
        if (argc < 4) goto usage;

            cmd = argv[i++];
        if (argc == 4) {
            if (strcmp(cmd, "rotpk") == 0) {
                ret = OTP_WRITE_ROTPK_HASH(simple_strtoul(argv[i++], &endp, 16));
            } else if (strcmp(cmd, "jtagpwd") == 0) {
                ret = OTP_WRITE_JTAG_PWD(simple_strtoul(argv[i++], &endp, 16));
            } else {
                goto usage;
            }
        } else {
            goto usage;
        }
    } else if(strncmp(cmd, "read", 4) == 0) {
        if (argc < 3) goto usage;

        memset(&rbuf[0], 0, min_rclean_byte);

        cmd = argv[i++];
        if (argc == 3) {
            if (strcmp(cmd, "rotpk") == 0) {
                ret = OTP_READ_ROTPK_HASH();
            } else if (strcmp(cmd, "jtagpwd") == 0) {
                ret = OTP_READ_JTAG_PWD();
            } else if (strcmp(cmd, "chip_mode") == 0) {
                ret = OTP_READ_CHIP_MODE((unsigned long)&rbuf[0]);
                printf("chip_mode: 0x%02x 0x%02x\n",rbuf[0],rbuf[1]);
            } else if (strcmp(cmd, "dram_model") == 0) {
                ret = OTP_READ_DRAM_MODEL((unsigned long)&rbuf[0]);
                printf("dram_model: 0x%02x 0x%02x\n",rbuf[0],rbuf[1]);
            } else {
                goto usage;
            }
        } else {
            goto usage;
        }
	} else {
		goto usage;
    }
    return ret;

usage:
	return CMD_RET_USAGE;
}


U_BOOT_CMD(
    otp,  6,  1,  do_otp_command,
	"otp sub-system:",
	"write rotpk <src_addr>   => Write ROTPK assigned in <src_addr> to OTP.\n"
	"otp write jtagpwd <src_addr> => Write jtag_pwd assigned in <src_addr> to OTP.\n"
	"otp read rotpk               => Read back ROTPK from OTP\n"
    "otp read jtagpwd             => Read back jtag password from OTP\n"
	"otp read chip_mode           => Read back chip_mode from OTP\n"
	"otp read dram_model          => Read back dram_model from OTP\n"
	"\n"
	"rotpk     => 256-bit ROTPK hash\n"
    "jtagpwd   => 64-bit jtag password\n"
	"chip_mode => Chip mode number\n"
	"dram_model=> DRAM model\n"
	"src_addr  => The data source in dram, which will be written into secure OTP memory.\n"
);

