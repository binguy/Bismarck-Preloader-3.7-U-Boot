/*
 * Copyright (C) 2020 Realtek
 */
#include <common.h>
#include <command.h>
#include <asm/io.h>
#include "cortina_smc.h"

int do_get_atf_version(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    char cut[2] = {CA_SMC_CALL_GET_ROM_VERSION(), 0};
    printf("BL1:  %s\n", cut);

    CA_SMC_CALL_GET_ATF_VERSION();

    return 0;
}


U_BOOT_CMD(
    bootver,  4,  1,  do_get_atf_version,
	"Getting SoC cut-nubmer, and BL2/BL31 GIT version information\n",
	"\n"
);

