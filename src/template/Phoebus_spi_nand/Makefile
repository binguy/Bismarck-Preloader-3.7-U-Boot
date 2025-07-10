# plr_vmpage_table records status of preloader chunk re-construction. Insufficient
# plr_vmpage_table shows "EE: plr mapping table init fail" after a notable delay.
#
# "first_load_page_num" reads image offset 0x41c for page number of first load
# "image_page_num" stands for the page required for plr.img; +1 is for roundup.
# "table_size" parse arch.h for defined number of plr_vmpage_table entry.
# reports error if table_size < image_page_num

UBOOT_IMG := uboot_v2020.01/u-boot.pimg
UB_CONFIG := uboot_v2020.01/.config
PAGE_SIZE = $(shell grep -w CONFIG_OTTO_SNAF_PAGE_SIZE ${UB_CONFIG} | sed 's/CONFIG_OTTO_SNAF_PAGE_SIZE=//')

all ::
	@if [ "$(PAGE_SIZE)" = "2048" ]; then \
		./util/bin/bchenc -o release/encode_uboot.img -plr release/plr.img -uboot $(UBOOT_IMG) -upimg; \
		hexdump -v -e '1/1 "%02X""\n"' release/encode_uboot.img > release/encode_uboot_hex.lst; \
		first_load_page_num=$$((0x`sed -n '1056,1056p' release/encode_uboot_hex.lst`)); \
		image_size=`stat --printf=%s release/plr.img`; \
		image_page_raw_num=$$(($${image_size} / 2048)); \
		image_page_num=$$(($${image_page_raw_num} + 1 + $${first_load_page_num} / 2)); \
		table_size=`grep PLR_MAP_TABLE_SIZE $(template_dir)/arch.h | tr -s ' ' ' ' | cut -d ' ' -f3`; \
		if [ "$${table_size}" -lt "$${image_page_num}" ]; \
			then echo -e "$(LIGHT_RED)EE: PLR_MAP_TABLE_SIZE is not enough!$(NONE)"; fi \
	fi
	@if [ "$(PAGE_SIZE)" = "4096" ]; then \
		./util/bin/bchenc -o release/encode_uboot_4Kpage.img -plr release/plr.img -psz 4096 -uboot $(UBOOT_IMG) -upimg; \
		hexdump -v -e '1/1 "%02X""\n"' release/encode_uboot_4Kpage.img > release/encode_uboot_hex.lst; \
		first_load_page_num=$$((0x`sed -n '1056,1056p' release/encode_uboot_hex.lst`)); \
		image_size=`stat --printf=%s release/plr.img`; \
		image_page_raw_num=$$(($${image_size} / 4096)); \
		image_page_num=$$(($${image_page_raw_num} + 1 - $${first_load_page_num} / 4 + $${first_load_page_num})); \
		table_size=`grep PLR_MAP_TABLE_SIZE $(template_dir)/arch.h | tr -s ' ' ' ' | cut -d ' ' -f3`; \
		if [ "$${table_size}" -lt "$${image_page_num}" ]; \
			then echo -e "$(LIGHT_RED)EE: PLR_MAP_TABLE_SIZE is not enough!$(NONE)"; fi \
	fi
