UBOOT_IMG := uboot_v2020.01/u-boot.pimg

all ::
	@echo UBOOT_IMG := $(UBOOT_IMG)
	@$(GEN_LIBMEMCTL)
	@./util/bin/bchenc -o release/encode_uboot.img -plr release/plr.img -uboot $(UBOOT_IMG) -upimg
	@./util/bin/bchenc -o release/encode_uboot_4Kpage.img -plr release/plr.img -psz 4096 -uboot $(UBOOT_IMG) -upimg

BOOT0412_SRC := boot0412 ab_boot0412_integration.c ab_dram_setup.c ab_memctl_8685.c ab_efuse.c

del_boot0412_src:
	@rm -rf $(BOOT0412_SRC)

get_boot0412_src:
	@git checkout $(BOOT0412_SRC)
