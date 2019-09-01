#!/bin/bash
./makeversions.sh
BASE=`pwd`
for i in ver/version*; do
  cd $BASE/$i
  cp $BASE/grub.img .
  make -w || exit 1
  ./mkbootimage.sh || exit 1
done
