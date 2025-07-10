#!/bin/bash
# build_uImage.sh -- created 2020-07-13, C.Y. Yang
# @Last Change:
# @Revision:    0.0


CURPATH=`pwd`
OTTO_FOLDER=`pwd`/../..
TOOLS_FOLDER=`pwd`/../../uboot_todaro/tools
UBOOT_FOLDER=`pwd`/../../uboot_todaro
P_CFG=APro_spi_nand_auth
U_CFG=rtk_apro_03cvd_spi_nand_ubi_yueme_fit_defconfig
ITS_FILE="${CURPATH}/apro_03cvd_verified_boot.its"
KEY_SRC="${CURPATH}/UOOT_FIT_KEY"
FINAL_FIT_IMAGE=final_image.it
#KERNEL_BUILD_FOLDER=path/to/sdk
NM=nm
UIMAGE_ENTRY_POINT=""


chk_kernel_path(){
#
# export KERNEL_BUILD_FOLDER=path/to/sdk ; ./build.sh
# or
# modify the KERNEL_BUILD_FOLDER above
	if [ "${KERNEL_BUILD_FOLDER}" = "" ]; then
		echo "ERROR: Please set KERNEL_BUILD_FOLDER first!"
		exit 1
	fi
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

prepare_img_dir(){
	if [ ! -e "${CURPATH}/kernel_image/" ]; then
		mkdir -p "${CURPATH}/kernel_image/"
	fi

}

echo_current_dir()
{
	echo "> current path is" "${CURPATH}"
}

echo_its_info()
{
	echo "> its file is" "${ITS_FILE}"
	echo "> key folder is" "${KEY_SRC}"
}

copy_uboot_verity_script()
{
	[ -f ${KERNEL_BUILD_FOLDER}/images/uboot_verity_script ] || return 1
	cp ${KERNEL_BUILD_FOLDER}/images/uboot_verity_script ${CURPATH}/kernel_image/
	[ "$?" = "0" ] && echo "Get uboot verity boot script from ${KERNEL_BUILD_FOLDER}"
}
copy_kernel_image()
{
	echo "Get kernel from ${KERNEL_BUILD_FOLDER}"
	cp ${KERNEL_BUILD_FOLDER}/linux-4.4.x/arch/mips/boot/vmlinux.bin.lzma ${CURPATH}/kernel_image/
}

restore_kernel_image()
{
	cp ${OTTO_FOLDER}/release/uImage ${KERNEL_BUILD_FOLDER}/linux-4.4.x/arch/mips/boot/
}

get_kernel_entry_point()
{
	UIMAGE_ENTRY_POINT="0x"$(${NM} ${KERNEL_BUILD_FOLDER}/linux-4.4.x/vmlinux 2>/dev/null \
                           | grep "\bkernel_entry\b" | cut -f1 -d \ )
	[ "$?" = "0" ]  && echo "Kernel Entry Point : $UIMAGE_ENTRY_POINT"
}

modify_entry_of_its()
{
	sed -i '{/entry = </{s/0x[0-9a-fA-F]*/'"${UIMAGE_ENTRY_POINT}"'/}}' ${ITS_FILE}
}

build_fit_uimage()
{
	echo "> build FIT uImage"
	${TOOLS_FOLDER}/mkimage -k ${CURPATH}/UBOOT_FIT_KEY -K ${UBOOT_FOLDER}/arch/otto/dts/apro_03cvd_fit_control.dtb -r -f ${ITS_FILE} ${FINAL_FIT_IMAGE}
}

copy_to_release()
{
	cp ${FINAL_FIT_IMAGE} ${OTTO_FOLDER}/release/uImage
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


kernel_make_vmimg()
{
	cd ${KERNEL_BUILD_FOLDER}
	make vmimg
}


main(){
	chk_kernel_path
	echo_current_dir
	echo_its_info
	prepare_img_dir
	copy_kernel_image
	ret_error_chk "Can't get KERNEL image!"
	copy_uboot_verity_script
	ret_error_chk "Can't get uboot verity script!"
	get_kernel_entry_point
	ret_error_chk "Can't get KERNEL entry point!"
	modify_entry_of_its
	ret_error_chk "Can't modify its for KERNEL entry point!"
	build_fit_uimage
	ret_error_chk "Build FIT uImage error!"
	copy_to_release
	mk_img_2k_alignment
	restore_kernel_image
	ret_error_chk "Can't restore KERNEL image!"
	kernel_make_vmimg
	ret_error_chk "Build KERNEL vmimg fail!"
}

main
# vi:
