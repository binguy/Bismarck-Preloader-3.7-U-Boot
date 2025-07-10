CURPATH=`pwd`
OTTO_FOLDER=`pwd`/../..
echo "${CURPATH}"
P_CFG=Phoebus_spi_nand_auth
U_CFG=Phoebus_spi_nand_fit_yueme_defconfig
ITS_FILE="${CURPATH}/phoebus_verified_boot_linux510.its"
KEY_SRC="${CURPATH}/UOOT_FIT_KEY"
FINAL_FIT_IMAGE=final_image.it
#KERNEL_BUILD_FOLDER=/path/to/sdk
NM=nm
UIMAGE_ENTRY_POINT=""

#
# export KERNEL_BUILD_FOLDER=path/to/sdk ; ./build.sh
# or
# modify the KERNEL_BUILD_FOLDER above
if [ "${KERNEL_BUILD_FOLDER}" = "" ]; then
	echo "ERROR: Please set KERNEL_BUILD_FOLDER first!"
	exit 1
fi

echo "${ITS_FILE}"
echo "${KEY_SRC}"

get_kernel_entry_point()
{
	UIMAGE_ENTRY_POINT="0x"$(${NM} ${KERNEL_BUILD_FOLDER}/linux-5.10.x/vmlinux 2>/dev/null \
                           | grep "\bkernel_entry\b" | cut -f1 -d \ )
	[ "$?" = "0" ]  && echo "Kernel Entry Point : $UIMAGE_ENTRY_POINT"
}

modify_entry_of_its()
{
	sed -i '{/entry = </{s/0x[0-9a-fA-F]*/'"${UIMAGE_ENTRY_POINT}"'/}}' ${ITS_FILE}
}

copy_uboot_verity_script()
{
	[ -f ${KERNEL_BUILD_FOLDER}/images/uboot_verity_script ] || return 1
	cp ${KERNEL_BUILD_FOLDER}/images/uboot_verity_script ${CURPATH}/kernel_image/
	[ "$?" = "0" ] && echo "Get uboot verity boot script from ${KERNEL_BUILD_FOLDER}"
}

ret_error_chk(){
	if [ "$?" != "0" ]; then
		echo "ERROR:" "${1}"
		exit 1
	fi

	# use $2 as message for return value = 0
	if [ "$2" != "" ]; then
		echo "$2"
	fi
}

mk_img_2k_alignment()
{
	echo Aligning uImage to 2k/page boundary for NAND platform
	mv ${OTTO_FOLDER}/release/uImage ${OTTO_FOLDER}/release/uImage.orig
	sz=`stat --printf="%s" ${OTTO_FOLDER}/release/uImage.orig`
	pagecnt=$(( (sz+2047) / 2048 ))
	dd if=${OTTO_FOLDER}/release/uImage.orig ibs=2k count=$pagecnt of=${OTTO_FOLDER}/release/uImage conv=sync
	ls -l ${OTTO_FOLDER}/release/uImage.orig ${OTTO_FOLDER}/release/uImage
	echo
}


if [ ! -e "${CURPATH}/kernel_image/" ]; then
	mkdir -p "${CURPATH}/kernel_image/"
fi


cd ../../ && make clean distclean && make preconfig_${P_CFG}_demo
# get kernel image
cp ${KERNEL_BUILD_FOLDER}/linux-5.10.x/arch/mips/boot/vmlinux.bin.lzma ${CURPATH}/kernel_image/
get_kernel_entry_point
ret_error_chk "Can't get KERNEL entry point!"
modify_entry_of_its
copy_uboot_verity_script
ret_error_chk "Can't get uboot verity script!"
cd uboot_v2020.01 && make ${U_CFG} && make all && ./tools/mkimage -k ${CURPATH}/UBOOT_FIT_KEY -K ./arch/otto/dts/phoebus_fit_control.dtb -e ${UIMAGE_ENTRY_POINT} -r -f ${ITS_FILE} ${FINAL_FIT_IMAGE} && make all
cd ../ && make all && cp uboot_v2020.01/${FINAL_FIT_IMAGE} release/uImage
mk_img_2k_alignment
# restore FIT image
cp release/uImage ${KERNEL_BUILD_FOLDER}/linux-5.10.x/arch/mips/boot/
# rebuild other image
cd ${KERNEL_BUILD_FOLDER}; make vmimg


