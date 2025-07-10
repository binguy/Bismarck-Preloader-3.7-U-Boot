
UBOOT_IMG := uboot_v2020.01/u-boot.pimg

ifndef BUILD_CMD
release/objs/ab_after_preloader.o: src/template/VD2_g2_nor/ab_after_preloader.S $(AFTER_PRELOADER_IMG)
endif

$(TARGET_IMG):: $(target_img_dep)

all ::
	echo UBOOT_IMG := $(UBOOT_IMG)
	$(GEN_LIBMEMCTL)

BOOT0412_SRC := boot0412 ab_boot0412_integration.c ab_dram_setup.c ab_memctl_8685.c ab_efuse.c

del_boot0412_src:
	rm -rf $(BOOT0412_SRC)

get_boot0412_src:
	git checkout $(BOOT0412_SRC)
