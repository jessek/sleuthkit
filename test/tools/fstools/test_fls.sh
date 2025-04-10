#!/bin/bash -e

if [ ! "${src_dir+x}" ]; then
    echo src_dir is not set
    exit 77                     # autoconf 'SKIP'
fi

IMAGE_DATA=${src_dir}/test/data/image_ext2.dd

if [ ! -r $IMAGE_DATA ]; then
    echo cannot read $IMAGE_DATA
    exit 77
fi

TD=test/tools/tool_differ.sh

$TD 'tools/fstools/fls$EXEEXT' ${src_dir}/test/tools/fstools/fls_output/1
$TD 'tools/fstools/fls$EXEEXT  ${src_dir}/test/data/image.dd' ${src_dir}/test/tools/fstools/fls_output/2
