#!/bin/bash -e

TD=test/tools/tool_differ.sh

if [ ! "${SLEUTHKIT_TEST_DATA_DIR+x}" ]; then
    echo SLEUTHKIT_TEST_DATA_DIR is not set
    exit 77                     # autoconf 'SKIP'
fi

if [ ! -d "${SLEUTHKIT_TEST_DATA_DIR+x}" ]; then
    echo $SLEUTHKIT_TEST_DATA_DIR does not exist
    exit 77                     # autoconf 'SKIP'
fi

if [ ! "${src_dir+x}" ]; then
    echo src_dir is not set
    exit 77                     # autoconf 'SKIP'
fi

EXFAT1=$SLEUTHKIT_TEST_DATA_DIR/exfat/exfat1.E01

if [ ! -e $EXFAT1 ]; then
    echo $EXFAT1 not found
    exit 77
fi

if [ ! -e ${src_dir}/test/tools/vstools/mmls_output/2 ]; then
    echo ${src_dir}/test/tools/vstools/mmls_output/2 does not exit
    exit 77
fi
$TD 'tools/vstools/mmls$EXEEXT -r $EXFAT1' ${src_dir}/test/tools/vstools/mmls_output/2


if [ ! -e ${src_dir}/test/tools/vstools/mmls_output/3 ]; then
    echo ${src_dir}/test/tools/vstools/mmls_output/3 does not exit
    exit 77
fi
$TD 'tools/vstools/mmls$EXEEXT -c $EXFAT1' ${src_dir}/test/tools/vstools/mmls_output/3
