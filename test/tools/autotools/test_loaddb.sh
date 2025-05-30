#!/bin/bash -e

if [ ! "${SLEUTHKIT_TEST_DATA_DIR+x}" ]; then
    echo SLEUTHKIT_TEST_DATA_DIR is not set
    exit 77                     # autoconf 'SKIP'
fi
if [ ! -d ${SLEUTHKIT_TEST_DATA_DIR} ]; then
    echo  $SLEUTHKIT_TEST_DATA_DIR does not exist
    exit 77
fi

if [ -n "$WINE" ]; then
  EXEEXT=.exe
fi

doit () {
    echo scan $1
    /bin/rm -f testdb.db
    $WINE "tools/autotools/tsk_loaddb$EXEEXT" -d testdb.db $1
    echo scan $1 done
}

doit $SLEUTHKIT_TEST_DATA_DIR/from_brian/1-extend-part/ext-part-test-2.dd
doit $SLEUTHKIT_TEST_DATA_DIR/apfs/apfs_pool.E01
doit $SLEUTHKIT_TEST_DATA_DIR/fuzzing/lvm_test_issue_3235.E01
