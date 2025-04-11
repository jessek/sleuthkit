#!/bin/bash

# This script is called by 'make check'
#

EXIT_PASS=0
EXIT_FAIL=1
EXIT_SKIP=77

NOHARDFAIL=yes

if [ ! "${SLEUTHKIT_TEST_DATA_DIR+x}" ]; then
    echo SLEUTHKIT_TEST_DATA_DIR is not set
    exit 77                     # autoconf 'SKIP'
fi

if [ ! -d "${SLEUTHKIT_TEST_DATA_DIR}" ]; then
    echo $SLEUTHKIT_TEST_DATA_DIR does not exist
    exit 77                     # autoconf 'SKIP'
fi

if [ ! "${srcdir+x}" ]; then
    echo srcdir is not set
    exit 1
fi

IMAGE_EXT2=$srcdir/test/data/image_ext2.dd
IMAGE_UFS=$srcdir/test/data/image_ufs.dd

NTHREADS=4
NITERS=2

if [ -n "$WINE" ]; then
  EXEEXT=.exe
fi

check_diffs()
{
    for LOG_FILE in thread-*.log ; do
        echo diff base.log ${LOG_FILE}
        diff base.log ${LOG_FILE} || return ${EXIT_FAIL}
    done

    return ${EXIT_PASS}
}

if ! test -d ${IMAGE_DIR} ; then
    echo "Missing image directory: ${IMAGE_DIR}"
    exit ${EXIT_SKIP}
fi

FS_THREAD_TEST="test/legacy/fs_thread_test$EXEEXT"

if ! test -x ${FS_THREAD_TEST} ; then
    echo "Missing test executable: ${FS_THREAD_TEST}"
    exit ${EXIT_SKIP};
fi

if test -f $IMAGE_EXT2 ; then
    rm -f base.log thread-*.log
    echo collecting $IMAGE_EXT2 output with 1 thread 1 iteration.
    ${WINE} ${FS_THREAD_TEST} -f ext2 $IMAGE_EXT2 1 1
    mv thread-0.log base.log
    echo testing $IMAGE_EXT2. threads=$NTHREADS iterations=$NITERS
    ${WINE} ${FS_THREAD_TEST} -f ext2 $IMAGE_EXT2 ${NTHREADS} ${NITERS}

    if ! check_diffs ; then
        exit ${EXIT_FAIL}
    fi
else
    echo $IMAGE_EXT2 missing
    [ -z "$NOHARDFAIL" ] && exit ${EXIT_SKIP}
fi

if test -f ${IMAGE_DIR}/test_hfs.dmg ; then
    echo testing ${IMAGE_DIR}/test_hfs.dmg
    rm -f base.log thread-*.log
    ${WINE} ${FS_THREAD_TEST} -f hfs -o 64 ${IMAGE_DIR}/test_hfs.dmg 1 1
    mv thread-0.log base.log
    ${WINE} ${FS_THREAD_TEST} -f hfs -o 64 ${IMAGE_DIR}/test_hfs.dmg ${NTHREADS} ${NITERS}

    if ! check_diffs ; then
        exit ${EXIT_FAIL}
    fi
else
    echo ${IMAGE_DIR}/test_hfs.dmg missing
    [ -z "$NOHARDFAIL" ] && exit ${EXIT_SKIP}
fi

if test -f ${IMAGE_DIR}/ntfs-img-kw-1.dd ; then
    echo testing ${IMAGE_DIR}/ntfs-img-kw-1.dd
    rm -f base.log thread-*.log
    ${WINE} ${FS_THREAD_TEST} -f ntfs ${IMAGE_DIR}/ntfs-img-kw-1.dd 1 1
    mv thread-0.log base.log
    ${WINE} ${FS_THREAD_TEST} -f ntfs ${IMAGE_DIR}/ntfs-img-kw-1.dd ${NTHREADS} ${NITERS}

    if ! check_diffs ; then
        exit ${EXIT_FAIL}
    fi
else
    echo ${IMAGE_DIR}/ntfs-img-kw-1.dd missing
    [ -z "$NOHARDFAIL" ] && exit ${EXIT_SKIP}
fi


if test -f ${IMAGE_DIR}/fat32.dd ; then
    echo testing ${IMAGE_DIR}/fat32.dd
    rm -f base.log thread-*.log
    ${WINE} ${FS_THREAD_TEST} -f fat ${IMAGE_DIR}/fat32.dd 1 1
    mv thread-0.log base.log
    ${WINE} ${FS_THREAD_TEST} -f fat ${IMAGE_DIR}/fat32.dd ${NTHREADS} ${NITERS}

    if ! check_diffs; then
        exit ${EXIT_FAIL}
    fi
else
    echo ${IMAGE_DIR}/fat32.dd missing
    [ -z "$NOHARDFAIL" ] && exit ${EXIT_SKIP}
fi


exit ${EXIT_PASS}
