#!/bin/bash -e

if [ ! "${SLEUTHKIT_TEST_DATA_DIR+x}" ]; then
    echo SLEUTHKIT_TEST_DATA_DIR is not set
    exit 77                     # autoconf 'SKIP'
if [ ! -d ${SLEUTHKIT_TEST_DATA_DIR} ]; then
    echo  $SLEUTHKIT_TEST_DATA_DIR does not exist
    exit 77
fi

# get basedir for normalizing output
basedir=$(realpath "$(dirname $0)/../..")

if [ -n "$WINE" ]; then
  EXEEXT=.exe
fi

CMD="${1/\$EXEEXT/$EXEEXT}"
CMD="${CMD/\$DATA_DIR/$SLEUTHKIT_TEST_DATA_DIR}"
EXP="$2"

echo -n "checking '$CMD': "

DIFF_EXIT=0
# diff, normalizing against basedir
RESULT=$(diff --strip-trailing-cr -u "$EXP" <($WINE $CMD 2>&1 | sed -e "\|^${basedir}/.*: |d")) || DIFF_EXIT=$?
if [ $DIFF_EXIT -ne 0 ]; then
  echo failed
  echo "$RESULT"
  exit 1
else
  echo ok
fi
