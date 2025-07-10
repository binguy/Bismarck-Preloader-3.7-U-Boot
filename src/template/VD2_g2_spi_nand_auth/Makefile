AUTHBCH := ./util/bin/authbch6

all ::
	$(MAKE) -C ./util/authbch ${AUTHBCH}
	@fallocate -l ${MAX_CRYPTO_HEADER_SIZE}  release/signature
	@./uboot_v2020.01/tools/mkimage -E -f ${template_dir}/${FDT_CONFIG} ${FDT_IMG}
	@${AUTHBCH} -i ${FDT_IMG} -o ${ENCODE_IMG_2K} -p 0 -d 1 -c 2 -k keys
	@${AUTHBCH} -i ${FDT_IMG} -o ${ENCODE_IMG_4K} -p 1 -d 1 -c 2 -k keys
	@rm release/signature
	@rm ./util/authbch/authbch6
