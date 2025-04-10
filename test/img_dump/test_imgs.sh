#!/bin/bash -e

ID=test/img_dump/img_differ.sh
IMG=${srcdir}/test/data/image.dd

if [ -e $IMG ]; then
    echo $IMG does not exist
    exit 77
fi

$ID $IMG ${srcdir}/test/dump/image.dd.json
# FIXME: check iso on MinGW after fixing timestamp bug
#$ID ../data/image.iso dump/image.iso.json
