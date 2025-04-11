#!/bin/bash -e

# Tools differ program:
# $1 = full command to run, with
#    - $EXEEXT being replaced with .exe on windows
#    - $DATA_DIR being replaced with ${srcdir}/test/data
#    - $SLEUTHKIT_TEST_DATA_DIR being replaced with $SLEUTHKIT_TEST_DATA_DIR

# get basedir for normalizing output
basedir=$(realpath "$(dirname $0)/../..")

if [ -n "$WINE" ]; then
  EXEEXT=.exe
fi

DATA_DIR=$srcdir/test/data

CMD="${1/\$EXEEXT/$EXEEXT}"
CMD="${CMD/\$DATA_DIR/$DATA_DIR}"
CMD="${CMD/\$SLEUTHKIT_TEST_DATA_DIR/$SLEUTHKIT_TEST_DATA_DIR}"
EXPECTED="$2"

echo -n "checking '$CMD': "

DIFF_EXIT=0
# diff, normalizing against basedir
RESULT=$(diff --strip-trailing-cr -u "$EXPECTED" <($WINE $CMD 2>&1 | sed -e "\|^${basedir}/.*: |d")) || DIFF_EXIT=$?
if [ $DIFF_EXIT -ne 0 ]; then
  echo failed
  echo "$RESULT"
  exit 1
else
  echo ok
fi
