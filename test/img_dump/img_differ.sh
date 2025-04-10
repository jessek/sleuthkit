#!/bin/bash -e

cd test/img_dump

if [ -n "$WINE" ]; then
  EXEEXT=.exe
fi

IMG="$1"
EXP="$2"

if [ ! -e ]; then
   echo $IMG does not exist
   exit 77
fi

diff -b "$EXP" <(TZ=America/New_York $WINE ./img_dump$EXEEXT "$IMG") \
    && echo "SUCCESS: img_dump $IMG" || { echo "FAILED: img_dump $IMG" ; exit 1 ; }
