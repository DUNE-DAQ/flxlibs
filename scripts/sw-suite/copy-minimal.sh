#!/bin/bash

set -e

usage()
{
  echo "usage: $(basename $0)  [<target_dir>]"
  exit 1
}

if [ $# -lt 0 ]; then
    usage
fi

if [ -z "${1}" -o  "${1}" == "--" ]
then
  if [ -z "${TARGET_DIR}" ]
  then
    echo "target directory is unset";
    UNAME=`uname -r`
    case "${UNAME}" in
      *)
          export TARGET_DIR="flxlibs-deps"
          ;;
    esac
  fi
else
  export TARGET_DIR=${1}
  shift
fi

mkdir ${TARGET_DIR}
mkdir ${TARGET_DIR}/include
mkdir ${TARGET_DIR}/lib

cd drivers_rcc/src
make clean
make -j
cd ../..

cp -r regmap/regmap ${TARGET_DIR}/include/
cp -r drivers_rcc ${TARGET_DIR}/
cp -r drivers_rcc/cmem_rcc ${TARGET_DIR}/include/
cp -r drivers_rcc/rcc_error ${TARGET_DIR}/include/
cp -r flxcard/flxcard ${TARGET_DIR}/include/
cp -r packetformat/packetformat ${TARGET_DIR}/include/

cp drivers_rcc/lib64/lib* ${TARGET_DIR}/lib
cp x86_64-centos7-gcc8-opt/flxcard/lib* ${TARGET_DIR}/lib
cp x86_64-centos7-gcc8-opt/packetformat/lib* ${TARGET_DIR}/lib
cp x86_64-centos7-gcc8-opt/regmap/lib* ${TARGET_DIR}/lib
cp x86_64-centos7-gcc8-opt/drivers_rcc/lib* ${TARGET_DIR}/lib
cp x86_64-centos7-gcc8-opt/flxcard_py/lib* ${TARGET_DIR}/lib


