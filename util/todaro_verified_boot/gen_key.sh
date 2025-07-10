#!/bin/sh

BASEDIR=$(dirname "$0")
export RANDFILE=${BASEDIR}/.rnd

cd ${BASEDIR}
work_dir=verifed_key_`date +"%d-%m-%Y"`

if [ ! -d ${work_dir} ];then
	mkdir ${work_dir}
fi
openssl genrsa -F4 -out ${work_dir}/dev.key 2048
openssl req -batch -new -x509 -key ${work_dir}/dev.key -out ${work_dir}/dev.crt
