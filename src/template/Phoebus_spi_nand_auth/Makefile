AUTHBCH := ./util/bin/authbch6
UB_CONFIG := uboot_v2020.01/.config
PAGE_SIZE = $(shell grep -w CONFIG_OTTO_SNAF_PAGE_SIZE ${UB_CONFIG} | sed 's/CONFIG_OTTO_SNAF_PAGE_SIZE=//')

all ::
	$(MAKE) -C ./util/authbch ${AUTHBCH}
	@fallocate -l ${MAX_CRYPTO_HEADER_SIZE}  release/signature
	@./uboot_v2020.01/tools/mkimage -E -f ${template_dir}/${FDT_CONFIG} ${FDT_IMG}
	@if [ "$(PAGE_SIZE)" = "2048" ]; then \
		${AUTHBCH} -i ${FDT_IMG} -o ${ENCODE_IMG_2K} -p 0 -d 1 -c 2 -k keys; \
	fi
	@if [ "$(PAGE_SIZE)" = "4096" ]; then \
		${AUTHBCH} -i ${FDT_IMG} -o ${ENCODE_IMG_4K} -p 1 -d 1 -c 2 -k keys;\
	fi
	@rm release/signature
	@rm ./util/authbch/authbch6
